/* This file is part of the KDE Project
   Copyright (c) 2004-2005 Jérôme Lodewyck <jerome dot lodewyck at normalesup dot org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "halbackend.h"
#include "linuxcdpolling.h"

#include <stdlib.h>

#include <kapplication.h>
#include <qeventloop.h>
#include <qfile.h>
#include <klocale.h>
#include <kurl.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kconfig.h>
#include <qstylesheet.h>
#include <kmountpoint.h>
#include <kmessagebox.h>
#include <kio/job.h>

#define MOUNT_SUFFIX	(libhal_volume_is_mounted(halVolume) ? QString("_mounted") : QString("_unmounted"))
#define MOUNT_ICON_SUFFIX	(libhal_volume_is_mounted(halVolume) ? QString("_mount") : QString("_unmount"))

/* Static instance of this class, for static HAL callbacks */
static HALBackend* s_HALBackend;

/* A macro function to convert HAL string properties to QString */
QString libhal_device_get_property_QString(LibHalContext *ctx, const char* udi, const char *key)
{
    char*   _ppt_string;
    QString _ppt_QString;
    DBusError error;
    dbus_error_init(&error);
    _ppt_string = libhal_device_get_property_string(ctx, udi, key, &error);
    if ( _ppt_string )
        _ppt_QString = _ppt_string;
    libhal_free_string(_ppt_string);
    return _ppt_QString;
}

/* Constructor */
HALBackend::HALBackend(MediaList &list, QObject* parent)
    : QObject()
    , BackendBase(list)
    , m_halContext(NULL)
    , m_halStoragePolicy(NULL)
    , m_parent(parent)
{
    s_HALBackend = this;
}

/* Destructor */
HALBackend::~HALBackend()
{
    /* Close HAL connection */
    if (m_halContext)
    {
        const QPtrList<Medium> medlist = m_mediaList.list();
        QPtrListIterator<Medium> it (medlist);
        for ( const Medium *current_medium = it.current(); current_medium; current_medium = ++it)
        {
            if( !current_medium->id().startsWith( "/org/kde" ))
                unmount(current_medium->id());
        }


        /* Remove all the registered media first */
        int numDevices;
        char** halDeviceList = libhal_get_all_devices( m_halContext, &numDevices, NULL );

        if ( halDeviceList )
        {
            for ( int i = 0; i < numDevices; i++ )
            {
                m_mediaList.removeMedium( halDeviceList[i], false );
            }
        }

        libhal_free_string_array( halDeviceList );

        DBusError error;
        dbus_error_init(&error);
        libhal_ctx_shutdown(m_halContext, &error);
        libhal_ctx_free(m_halContext);
    }

    if (m_halStoragePolicy)
        libhal_storage_policy_free(m_halStoragePolicy);
}

/* Connect to the HAL */
bool HALBackend::InitHal()
{
    kdDebug(1219) << "Context new" << endl;
    m_halContext = libhal_ctx_new();
    if (!m_halContext)
    {
        kdDebug(1219) << "Failed to initialize HAL!" << endl;
        return false;
    }

    // Main loop integration
    kdDebug(1219) << "Main loop integration" << endl;
    DBusError error;
    dbus_error_init(&error);
    dbus_connection = dbus_bus_get_private(DBUS_BUS_SYSTEM, &error);

    if (!dbus_connection || dbus_error_is_set(&error)) {
        dbus_error_free(&error);
        libhal_ctx_free(m_halContext);
        m_halContext = NULL;
        return false;
    }

    dbus_connection_set_exit_on_disconnect (dbus_connection, FALSE);

    MainLoopIntegration(dbus_connection);
    libhal_ctx_set_dbus_connection(m_halContext, dbus_connection);

    // HAL callback functions
    kdDebug(1219) << "Callback functions" << endl;
    libhal_ctx_set_device_added(m_halContext, HALBackend::hal_device_added);
    libhal_ctx_set_device_removed(m_halContext, HALBackend::hal_device_removed);
    libhal_ctx_set_device_new_capability (m_halContext, NULL);
    libhal_ctx_set_device_lost_capability (m_halContext, NULL);
    libhal_ctx_set_device_property_modified (m_halContext, HALBackend::hal_device_property_modified);
    libhal_ctx_set_device_condition(m_halContext, HALBackend::hal_device_condition);

    kdDebug(1219) << "Context Init" << endl;
    if (!libhal_ctx_init(m_halContext, &error))
    {
        if (dbus_error_is_set(&error))
            dbus_error_free(&error);
        libhal_ctx_free(m_halContext);
        m_halContext = NULL;
        kdDebug(1219) << "Failed to init HAL context!" << endl;
        return false;
    }

    /** @todo customize watch policy */
    kdDebug(1219) << "Watch properties" << endl;
    if (!libhal_device_property_watch_all(m_halContext, &error))
    {
        kdDebug(1219) << "Failed to watch HAL properties!" << endl;
        return false;
    }

    /* libhal-storage initialization */
    kdDebug(1219) << "Storage Policy" << endl;
    m_halStoragePolicy = libhal_storage_policy_new();
    /** @todo define libhal-storage icon policy */

    /* List devices at startup */
    return ListDevices();
}

/* List devices (at startup)*/
bool HALBackend::ListDevices()
{
    kdDebug(1219) << "ListDevices" << endl;

    int numDevices;
    char** halDeviceList = libhal_get_all_devices(m_halContext, &numDevices, NULL);

    if (!halDeviceList)
        return false;

    kdDebug(1219) << "HALBackend::ListDevices : " << numDevices << " devices found" << endl;
    for (int i = 0; i < numDevices; i++)
        AddDevice(halDeviceList[i], false);

    libhal_free_string_array( halDeviceList );

    return true;
}

/* Create a media instance for the HAL device "udi".
   This functions checks whether the device is worth listing */
void HALBackend::AddDevice(const char *udi, bool allowNotification)
{
    /* We don't deal with devices that do not expose their capabilities.
       If we don't check this, we will get a lot of warning messages from libhal */
    if (!libhal_device_property_exists(m_halContext, udi, "info.capabilities", NULL))
        return;

    /* If the device is already listed, do not process.
       This should not happen, but who knows... */
    /** @todo : refresh properties instead ? */
    if (m_mediaList.findById(udi))
        return;

    if (libhal_device_get_property_bool(m_halContext, "/org/freedesktop/Hal/devices/computer", "storage.disable_volume_handling", NULL))
        allowNotification=false;

    /* Add volume block devices */
    if (libhal_device_query_capability(m_halContext, udi, "volume", NULL))
    {
        /* We only list volume that have a filesystem or volume that have an audio track*/
        if ( libhal_device_get_property_QString(m_halContext, udi, "volume.fsusage") != "filesystem" &&
             !libhal_device_get_property_bool(m_halContext, udi, "volume.disc.has_audio", NULL) &&
             !libhal_device_get_property_bool(m_halContext, udi, "volume.disc.is_blank", NULL) )
            return;

        /* Query drive udi */
        QString driveUdi = libhal_device_get_property_QString(m_halContext, udi, "block.storage_device");
        if ( driveUdi.isNull() ) // no storage - no fun
            return;

        // if the device is locked do not act upon it
        if (libhal_device_get_property_bool(m_halContext, driveUdi.ascii(), "info.locked", NULL))
            allowNotification=false;

        // if the device is locked do not act upon it
        if (libhal_device_get_property_bool(m_halContext, driveUdi.ascii(), "storage.partition_table_changed", NULL))
            allowNotification=false;

        /** @todo check exclusion list **/

        /* Create medium */
        Medium* medium = new Medium(udi, "");
        setVolumeProperties(medium);

        if ( isInFstab( medium ).isNull() )
        {
            // if it's not mountable by user and not by HAL, don't show it at all
            if ( ( libhal_device_get_property_QString(m_halContext, udi, "volume.fsusage") == "filesystem" &&
                   !libhal_device_get_property_bool(m_halContext, udi, "volume.is_mounted", NULL ) ) &&
                 ( libhal_device_get_property_bool(m_halContext, udi, "volume.ignore", NULL ) ) )
            {
                delete medium;
                return;
            }
        }
        QMap<QString,QString> options = MediaManagerUtils::splitOptions(mountoptions(udi));
        kdDebug() << "automount " << options["automount"] << endl;
        if (options["automount"] == "true" && allowNotification ) {
            QString error = mount(medium);
            if (!error.isEmpty())
                kdDebug() << "error " << error << endl;
        }
        m_mediaList.addMedium(medium, allowNotification);

        return;
    }

    /* Floppy & zip drives */
    if (libhal_device_query_capability(m_halContext, udi, "storage", NULL))
        if ((libhal_device_get_property_QString(m_halContext, udi, "storage.drive_type") == "floppy") ||
            (libhal_device_get_property_QString(m_halContext, udi, "storage.drive_type") == "zip") ||
            (libhal_device_get_property_QString(m_halContext, udi, "storage.drive_type") == "jaz"))
        {
            if (! libhal_device_get_property_bool(m_halContext, udi, "storage.removable.media_available", NULL) )
                allowNotification = false;
            /* Create medium */
            Medium* medium = new Medium(udi, "");
            // if the storage has a volume, we ignore it
            if ( setFloppyProperties(medium) )
                m_mediaList.addMedium(medium, allowNotification);
            else
                delete medium;
            return;
        }

    /* Camera handled by gphoto2*/
    if (libhal_device_query_capability(m_halContext, udi, "camera", NULL) &&
        ((libhal_device_get_property_QString(m_halContext, udi, "camera.access_method")=="ptp") ||

         (libhal_device_property_exists(m_halContext, udi, "camera.libgphoto2.support", NULL) &&
          libhal_device_get_property_bool(m_halContext, udi, "camera.libgphoto2.support", NULL)))
        )
    {
        /* Create medium */
        Medium* medium = new Medium(udi, "");
        setCameraProperties(medium);
        m_mediaList.addMedium(medium, allowNotification);
        return;
    }
}

void HALBackend::RemoveDevice(const char *udi)
{
    m_mediaList.removeMedium(udi, true);
}

void HALBackend::ModifyDevice(const char *udi, const char* key)
{
    const char* mediumUdi = findMediumUdiFromUdi(udi);
    if (!mediumUdi)
        return;
    bool allowNotification = false;
    if (strcmp(key, "storage.removable.media_available") == 0)
        allowNotification = libhal_device_get_property_bool(m_halContext, udi, key, NULL);
    ResetProperties(mediumUdi, allowNotification);
}

void HALBackend::DeviceCondition(const char* udi, const char* condition)
{
    QString conditionName = QString(condition);
    kdDebug(1219) << "Processing device condition " << conditionName << " for " << udi << endl;

    if (conditionName == "EjectPressed") {
        const Medium* medium = m_mediaList.findById(udi);
        if (!medium) {
	    /* the ejectpressed appears on the drive and we need to find the volume */
	    const QPtrList<Medium> medlist = m_mediaList.list();
	    QPtrListIterator<Medium> it (medlist);
	    for ( const Medium *current_medium = it.current(); current_medium; current_medium = ++it)
            {
                if( current_medium->id().startsWith( "/org/kde" ))
                    continue;
		QString driveUdi = libhal_device_get_property_QString(m_halContext, current_medium->id().latin1(), "block.storage_device");
		if (driveUdi == udi)
                {
		    medium = current_medium;
		    break;
                }
            }
        }
        if (medium) {
	    KProcess p;
	    p << "kio_media_mounthelper" << "-e" << medium->name();
	    p.start(KProcess::DontCare);
        }
    }

    const char* mediumUdi = findMediumUdiFromUdi(udi);
    kdDebug() << "findMedumUdiFromUdi " << udi << " returned " << mediumUdi << endl;
    if (!mediumUdi)
        return;

    /* TODO: Warn the user that (s)he should unmount devices before unplugging */
    if (conditionName == "VolumeUnmountForced")
        ResetProperties(mediumUdi);

    /* Reset properties after mounting */
    if (conditionName == "VolumeMount")
        ResetProperties(mediumUdi);

    /* Reset properties after unmounting */
    if (conditionName == "VolumeUnmount")
        ResetProperties(mediumUdi);

}

void HALBackend::MainLoopIntegration(DBusConnection *dbusConnection)
{
    m_dBusQtConnection = new DBusQt::Connection(m_parent);
    m_dBusQtConnection->dbus_connection_setup_with_qt_main(dbusConnection);
}

/******************************************
 ** Properties attribution                **
 ******************************************/

/* Return the medium udi that should be updated when recieving a call for
   device udi */
const char* HALBackend::findMediumUdiFromUdi(const char* udi)
{
    /* Easy part : this Udi is already registered as a device */
    const Medium* medium = m_mediaList.findById(udi);
    if (medium)
        return medium->id().ascii();

    /* Hard part : this is a volume whose drive is registered */
    if (libhal_device_property_exists(m_halContext, udi, "info.capabilities", NULL))
        if (libhal_device_query_capability(m_halContext, udi, "volume", NULL))
        {
            QString driveUdi = libhal_device_get_property_QString(m_halContext, udi, "block.storage_device");
            return findMediumUdiFromUdi(driveUdi.ascii());
        }

    return NULL;
}

void HALBackend::ResetProperties(const char* mediumUdi, bool allowNotification)
{
    kdDebug(1219) << "HALBackend::setProperties" << endl;
    if ( QString::fromLatin1( mediumUdi ).startsWith( "/org/kde/" ) )
    {
        const Medium *cmedium = m_mediaList.findById(mediumUdi);
        if ( cmedium )
        {
            Medium m( *cmedium );
            if ( setFstabProperties( &m ) ) {
                kdDebug() << "setFstabProperties worked" << endl;
                m_mediaList.changeMediumState(m, allowNotification);
            }
            return;
       }
    }

    Medium* m = new Medium(mediumUdi, "");

    if (libhal_device_query_capability(m_halContext, mediumUdi, "volume", NULL))
        setVolumeProperties(m);
    if (libhal_device_query_capability(m_halContext, mediumUdi, "storage", NULL))
        setFloppyProperties(m);
    if (libhal_device_query_capability(m_halContext, mediumUdi, "camera", NULL))
        setCameraProperties(m);

    m_mediaList.changeMediumState(*m, allowNotification);

    delete m;
}

void HALBackend::setVolumeProperties(Medium* medium)
{
    kdDebug(1219) << "HALBackend::setVolumeProperties for " << medium->id() << endl;

    const char* udi = medium->id().ascii();
    /* Check if the device still exists */
    if (!libhal_device_exists(m_halContext, udi, NULL))
        return;

    /* Get device information from libhal-storage */
    LibHalVolume* halVolume = libhal_volume_from_udi(m_halContext, udi);
    if (!halVolume)
        return;
    QString driveUdi = libhal_volume_get_storage_device_udi(halVolume);
    LibHalDrive*  halDrive  = 0;
    if ( !driveUdi.isNull() )
        halDrive = libhal_drive_from_udi(m_halContext, driveUdi.ascii());
    if (!halDrive) {
        // at times HAL sends an UnmountForced event before the device is removed
        libhal_volume_free(halVolume);
        return;
    }

    medium->setName(
        generateName(libhal_volume_get_device_file(halVolume)) );

    medium->mountableState(
        libhal_volume_get_device_file(halVolume),		/* Device node */
        libhal_volume_get_mount_point(halVolume),		/* Mount point */
        libhal_volume_get_fstype(halVolume),			/* Filesystem type */
        libhal_volume_is_mounted(halVolume) );			/* Mounted ? */

    char* name = libhal_volume_policy_compute_display_name(halDrive, halVolume, m_halStoragePolicy);
    QString volume_name = QString::fromUtf8(name);
    QString media_name = volume_name;
    medium->setLabel(media_name);
    free(name);

    QString mimeType;
    if (libhal_volume_is_disc(halVolume))
    {
        mimeType = "media/cdrom" + MOUNT_SUFFIX;

        LibHalVolumeDiscType discType = libhal_volume_get_disc_type(halVolume);
        if ((discType == LIBHAL_VOLUME_DISC_TYPE_CDROM) ||
            (discType == LIBHAL_VOLUME_DISC_TYPE_CDR) ||
            (discType == LIBHAL_VOLUME_DISC_TYPE_CDRW))
            if (libhal_volume_disc_is_blank(halVolume))
            {
                mimeType = "media/blankcd";
                medium->unmountableState("");
            }
            else
                mimeType = "media/cdwriter" + MOUNT_SUFFIX;

        if ((discType == LIBHAL_VOLUME_DISC_TYPE_DVDROM) || (discType == LIBHAL_VOLUME_DISC_TYPE_DVDRAM) ||
            (discType == LIBHAL_VOLUME_DISC_TYPE_DVDR) || (discType == LIBHAL_VOLUME_DISC_TYPE_DVDRW) ||
            (discType == LIBHAL_VOLUME_DISC_TYPE_DVDPLUSR) || (discType == LIBHAL_VOLUME_DISC_TYPE_DVDPLUSRW) )
            if (libhal_volume_disc_is_blank(halVolume))
            {
                mimeType = "media/blankdvd";
                medium->unmountableState("");
            }
            else
                mimeType = "media/dvd" + MOUNT_SUFFIX;

        if (libhal_volume_disc_has_audio(halVolume) && !libhal_volume_disc_has_data(halVolume))
        {
            mimeType = "media/audiocd";
            medium->unmountableState( "audiocd:/?device=" + QString(libhal_volume_get_device_file(halVolume)) );
        }

        medium->setIconName(QString::null);

        /* check if the disc id a vcd or a video dvd */
        DiscType type = LinuxCDPolling::identifyDiscType(libhal_volume_get_device_file(halVolume));
        switch (type)
        {
        case DiscType::VCD:
            mimeType = "media/vcd";
            break;
        case DiscType::SVCD:
            mimeType = "media/svcd";
            break;
        case DiscType::DVD:
            mimeType = "media/dvdvideo";
            break;
        }
    }
    else
    {
        mimeType = "media/hdd" + MOUNT_SUFFIX;
        medium->setIconName(QString::null); // reset icon
        if (libhal_drive_is_hotpluggable(halDrive))
        {
            mimeType = "media/removable" + MOUNT_SUFFIX;
            medium->needMounting();
            switch (libhal_drive_get_type(halDrive)) {
            case LIBHAL_DRIVE_TYPE_COMPACT_FLASH:
                medium->setIconName("compact_flash" + MOUNT_ICON_SUFFIX);
                break;
            case LIBHAL_DRIVE_TYPE_MEMORY_STICK:
                medium->setIconName("memory_stick" + MOUNT_ICON_SUFFIX);
                break;
            case LIBHAL_DRIVE_TYPE_SMART_MEDIA:
                medium->setIconName("smart_media" + MOUNT_ICON_SUFFIX);
                break;
            case LIBHAL_DRIVE_TYPE_SD_MMC:
                medium->setIconName("sd_mmc" + MOUNT_ICON_SUFFIX);
                break;
            case LIBHAL_DRIVE_TYPE_PORTABLE_AUDIO_PLAYER:
            {
                medium->setIconName("ipod" + MOUNT_ICON_SUFFIX);
                break;
            }
            case LIBHAL_DRIVE_TYPE_CAMERA:
            {
                mimeType = "media/camera" + MOUNT_SUFFIX;
                const char *physdev = libhal_drive_get_physical_device_udi(halDrive);
                // get model from camera
                if (physdev && libhal_device_query_capability(m_halContext, physdev, "camera", NULL))
                {
                    if (libhal_device_property_exists(m_halContext, physdev, "usb_device.product", NULL))
                        medium->setLabel(libhal_device_get_property_QString(m_halContext, physdev, "usb_device.product"));
                    else if (libhal_device_property_exists(m_halContext, physdev, "usb.product", NULL))
                        medium->setLabel(libhal_device_get_property_QString(m_halContext, physdev, "usb.product"));
                }
                break;
            }
            case LIBHAL_DRIVE_TYPE_TAPE:
                medium->setIconName(QString::null); //FIXME need icon
                break;
            default:
                medium->setIconName(QString::null);
            }

            if (medium->isMounted() && QFile::exists(medium->mountPoint() + "/dcim"))
            {
                mimeType = "media/camera" + MOUNT_SUFFIX;
            }
        }
    }
    medium->setMimeType(mimeType);

    libhal_drive_free(halDrive);
    libhal_volume_free(halVolume);
}

bool HALBackend::setFstabProperties( Medium *medium )
{
    QString mp = isInFstab(medium);

    if (!mp.isNull() && !medium->id().startsWith( "/org/kde" ) )
    {
        // now that we know it's in fstab, we have to find out if it's mounted
        KMountPoint::List mtab = KMountPoint::currentMountPoints();

        KMountPoint::List::iterator it = mtab.begin();
        KMountPoint::List::iterator end = mtab.end();

        bool mounted = false;

        for (; it!=end; ++it)
        {
            if ((*it)->mountedFrom() == medium->deviceNode() && (*it)->mountPoint() == mp )
            {
                mounted = true;
                break;
            }
        }

        kdDebug() << mp << " " << mounted << " " << medium->deviceNode() << " " <<  endl;
        QString fstype = medium->fsType();
        if ( fstype.isNull() )
            fstype = "auto";

        medium->mountableState(
            medium->deviceNode(),
            mp,							/* Mount point */
            fstype,						/* Filesystem type */
            mounted );						/* Mounted ? */

        return true;
    }

    return false;

}

// Handle floppies and zip drives
bool HALBackend::setFloppyProperties(Medium* medium)
{
    kdDebug(1219) << "HALBackend::setFloppyProperties for " << medium->id() << endl;

    const char* udi = medium->id().ascii();
    /* Check if the device still exists */
    if (!libhal_device_exists(m_halContext, udi, NULL))
        return false;

    LibHalDrive*  halDrive  = libhal_drive_from_udi(m_halContext, udi);
    if (!halDrive)
        return false;

    QString drive_type = libhal_device_get_property_QString(m_halContext, udi, "storage.drive_type");

    if (drive_type == "zip") {
        int numVolumes;
        char** volumes = libhal_drive_find_all_volumes(m_halContext, halDrive, &numVolumes);
        libhal_free_string_array(volumes);
        kdDebug(1219) << " found " << numVolumes << " volumes" << endl;
        if (numVolumes)
        {
            libhal_drive_free(halDrive);
            return false;
        }
    }

    medium->setName( generateName(libhal_drive_get_device_file(halDrive)) );
    medium->setLabel(i18n("Unknown Drive"));

    // HAL hates floppies - so we have to do it twice ;(
    medium->mountableState(libhal_drive_get_device_file(halDrive), QString::null, QString::null, false);
    setFloppyMountState(medium);

    if (drive_type == "floppy")
    {
        if (medium->isMounted()) // don't use _SUFFIX here as it accesses the volume
            medium->setMimeType("media/floppy_mounted" );
        else
            medium->setMimeType("media/floppy_unmounted");
        medium->setLabel(i18n("Floppy Drive"));
    }
    else if (drive_type == "zip") 
    {
        if (medium->isMounted())
            medium->setMimeType("media/zip_mounted" );
        else
            medium->setMimeType("media/zip_unmounted");
        medium->setLabel(i18n("Zip Drive"));
    }

    /** @todo And mimtype for JAZ drives ? */

    medium->setIconName(QString::null);

    libhal_drive_free(halDrive);

    return true;
}

void HALBackend::setFloppyMountState( Medium *medium )
{
    if ( !medium->id().startsWith( "/org/kde" ) )
    {
        KMountPoint::List mtab = KMountPoint::currentMountPoints();
        KMountPoint::List::iterator it = mtab.begin();
        KMountPoint::List::iterator end = mtab.end();

        QString fstype;
        QString mountpoint;
        for (; it!=end; ++it)
        {
            if ((*it)->mountedFrom() == medium->deviceNode() )
            {
                fstype = (*it)->mountType().isNull() ? (*it)->mountType() : "auto";
                mountpoint = (*it)->mountPoint();
                medium->mountableState( medium->deviceNode(), mountpoint, fstype, true );
                return;
            }
        }
    }
}

void HALBackend::setCameraProperties(Medium* medium)
{
    kdDebug(1219) << "HALBackend::setCameraProperties for " << medium->id() << endl;

    const char* udi = medium->id().ascii();
    /* Check if the device still exists */
    if (!libhal_device_exists(m_halContext, udi, NULL))
        return;

    /** @todo find name */
    medium->setName("camera");

    QString device = "camera:/";

    char *cam = libhal_device_get_property_string(m_halContext, udi, "camera.libgphoto2.name", NULL);
    DBusError error;
    dbus_error_init(&error);
    if (cam &&
        libhal_device_property_exists(m_halContext, udi, "usb.linux.device_number", NULL) &&
        libhal_device_property_exists(m_halContext, udi, "usb.bus_number", NULL))
        device.sprintf("camera://%s@[usb:%03d,%03d]/", cam,
                       libhal_device_get_property_int(m_halContext, udi, "usb.bus_number", &error),
                       libhal_device_get_property_int(m_halContext, udi, "usb.linux.device_number", &error));

    libhal_free_string(cam);

    /** @todo find the rest of this URL */
    medium->unmountableState(device);
    medium->setMimeType("media/gphoto2camera");
    medium->setIconName(QString::null);
    if (libhal_device_property_exists(m_halContext, udi, "usb_device.product", NULL))
        medium->setLabel(libhal_device_get_property_QString(m_halContext, udi, "usb_device.product"));
    else if (libhal_device_property_exists(m_halContext, udi, "usb.product", NULL))
        medium->setLabel(libhal_device_get_property_QString(m_halContext, udi, "usb.product"));
    else
        medium->setLabel(i18n("Camera"));
}

QString HALBackend::generateName(const QString &devNode)
{
    return KURL(devNode).fileName();
}

/******************************************
 ** HAL CALL-BACKS                        **
 ******************************************/

void HALBackend::hal_device_added(LibHalContext *ctx, const char *udi)
{
    kdDebug(1219) << "HALBackend::hal_device_added " << udi <<  endl;
    Q_UNUSED(ctx);
    s_HALBackend->AddDevice(udi);
}

void HALBackend::hal_device_removed(LibHalContext *ctx, const char *udi)
{
    kdDebug(1219) << "HALBackend::hal_device_removed " << udi << endl;
    Q_UNUSED(ctx);
    s_HALBackend->RemoveDevice(udi);
}

void HALBackend::hal_device_property_modified(LibHalContext *ctx, const char *udi,
                                              const char *key, dbus_bool_t is_removed, dbus_bool_t is_added)
{
    kdDebug(1219) << "HALBackend::hal_property_modified " << udi << " -- " << key << endl;
    Q_UNUSED(ctx);
    Q_UNUSED(is_removed);
    Q_UNUSED(is_added);
    s_HALBackend->ModifyDevice(udi, key);
}

void HALBackend::hal_device_condition(LibHalContext *ctx, const char *udi,
                                      const char *condition_name,
                                      const char* message
    )
{
    kdDebug(1219) << "HALBackend::hal_device_condition " << udi << " -- " << condition_name << endl;
    Q_UNUSED(ctx);
    Q_UNUSED(message);
    s_HALBackend->DeviceCondition(udi, condition_name);
}

QStringList HALBackend::mountoptions(const QString &name)
{
    const Medium* medium = m_mediaList.findById(name);
    if (medium && !isInFstab(medium).isNull())
        return QStringList(); // not handled by HAL - fstab entry

    KConfig config("mediamanagerrc");
    config.setGroup(name);

    char ** array = libhal_device_get_property_strlist(m_halContext, name.latin1(), "volume.mount.valid_options", NULL);
    QMap<QString,bool> valids;

    for (int index = 0; array && array[index]; ++index) {
        QString t = array[index];
        if (t.endsWith("="))
            t = t.left(t.length() - 1);
        valids[t] = true;
        kdDebug() << "valid " << t << endl;
    }
    libhal_free_string_array(array);
    QStringList result;
    QString tmp;

    QString fstype = libhal_device_get_property_QString(m_halContext, name.latin1(), "volume.fstype");
    if (fstype.isNull())
        fstype = libhal_device_get_property_QString(m_halContext, name.latin1(), "volume.policy.mount_filesystem");

    QString drive_udi = libhal_device_get_property_QString(m_halContext, name.latin1(), "block.storage_device");

    bool removable = false;
    if ( !drive_udi.isNull() )
        removable = libhal_device_get_property_bool(m_halContext, drive_udi.latin1(), "storage.removable", NULL)
                     || libhal_device_get_property_bool(m_halContext, drive_udi.latin1(), "storage.hotpluggable", NULL);

    config.setGroup(drive_udi);
    bool value = config.readBoolEntry("automount", false);
    config.setGroup(name);

    if (libhal_device_get_property_bool(m_halContext, name.latin1(), "volume.disc.is_blank", NULL)
        || libhal_device_get_property_bool(m_halContext, name.latin1(), "volume.disc.is_vcd", NULL)
        || libhal_device_get_property_bool(m_halContext, name.latin1(), "volume.disc.is_svcd", NULL)
        || libhal_device_get_property_bool(m_halContext, name.latin1(), "volume.disc.is_videodvd", NULL)
        || libhal_device_get_property_bool(m_halContext, name.latin1(), "volume.disc.has_audio", NULL))
        value = false;

    result << QString("automount=%1").arg(value ? "true" : "false");

    if (valids.contains("ro"))
    {
        value = config.readBoolEntry("ro", false);
        tmp = QString("ro=%1").arg(value ? "true" : "false");
        if (fstype != "iso9660") // makes no sense
            result << tmp;
    }

    if (valids.contains("quiet"))
    {
        value = config.readBoolEntry("quiet", false);
        tmp = QString("quiet=%1").arg(value ? "true" : "false");
        if (fstype != "iso9660") // makes no sense
            result << tmp;
    }

    if (valids.contains("flush"))
    {
        value = config.readBoolEntry("flush", fstype.endsWith("fat"));
        tmp = QString("flush=%1").arg(value ? "true" : "false");
        result << tmp;
    }

    if (valids.contains("uid"))
    {
        value = config.readBoolEntry("uid", true);
        tmp = QString("uid=%1").arg(value ? "true" : "false");
        result << tmp;
    }

	if ( valids.contains("locale") )
	{
		value = config.readBoolEntry( "locale", true );
		tmp = QString( "locale=%1" ).arg( value ? "true" : "false" );
		result << tmp;
	}

    if (valids.contains("utf8"))
    {
        value = config.readBoolEntry("utf8", true);
        tmp = QString("utf8=%1").arg(value ? "true" : "false");
        result << tmp;
    }

    if (valids.contains("shortname"))
    {
        QString svalue = config.readEntry("shortname", "lower").lower();
        if (svalue == "winnt")
            result << "shortname=winnt";
        else if (svalue == "win95")
            result << "shortname=win95";
        else if (svalue == "mixed")
            result << "shortname=mixed";
        else
            result << "shortname=lower";
    }

    if (valids.contains("sync"))
    {
        value = config.readBoolEntry("sync", ( valids.contains("flush") && !fstype.endsWith("fat") ) && removable);
        tmp = QString("sync=%1").arg(value ? "true" : "false");
        if (fstype != "iso9660") // makes no sense
            result << tmp;
    }

    if (valids.contains("noatime"))
    {
        value = config.readBoolEntry("atime", !fstype.endsWith("fat"));
        tmp = QString("atime=%1").arg(value ? "true" : "false");
        if (fstype != "iso9660") // makes no sense
            result << tmp;
    }

    QString mount_point = libhal_device_get_property_QString(m_halContext, name.latin1(), "volume.mount_point");
    if (mount_point.isEmpty())
        mount_point = libhal_device_get_property_QString(m_halContext, name.latin1(), "volume.policy.desired_mount_point");

    mount_point = config.readEntry("mountpoint", mount_point);

    if (!mount_point.startsWith("/"))
        mount_point = "/media/" + mount_point;

    result << QString("mountpoint=%1").arg(mount_point);
    result << QString("filesystem=%1").arg(fstype);

    if (valids.contains("data"))
    {
        QString svalue = config.readEntry("journaling").lower();
        if (svalue == "ordered")
            result << "journaling=ordered";
        else if (svalue == "writeback")
            result << "journaling=writeback";
        else if (svalue == "data")
            result << "journaling=data";
        else
            result << "journaling=ordered";
    }

    return result;
}

bool HALBackend::setMountoptions(const QString &name, const QStringList &options )
{
    kdDebug() << "setMountoptions " << name << " " << options << endl;

    KConfig config("mediamanagerrc");
    config.setGroup(name);

    QMap<QString,QString> valids = MediaManagerUtils::splitOptions(options);

    const char *names[] = { "ro", "quiet", "atime", "uid", "utf8", "flush", "sync", "locale", 0 };
    for (int index = 0; names[index]; ++index)
        if (valids.contains(names[index]))
            config.writeEntry(names[index], valids[names[index]] == "true");

    if (valids.contains("shortname"))
        config.writeEntry("shortname", valids["shortname"]);

    if (valids.contains("journaling"))
        config.writeEntry("journaling", valids["journaling"]);

    if (!mountoptions(name).contains(QString("mountpoint=%1").arg(valids["mountpoint"])))
        config.writeEntry("mountpoint", valids["mountpoint"]);

    if (valids.contains("automount")) {
        QString drive_udi = libhal_device_get_property_QString(m_halContext, name.latin1(), "block.storage_device");
        config.setGroup(drive_udi);
        config.writeEntry("automount", valids["automount"]);
    }

	if (valids.contains("locale") ) {
		config.writeEntry("locale", valids["locale"]);
	}

    return true;
}

static QString mount_priv(const char *udi, const char *mount_point, const char **poptions, int noptions,
			  DBusConnection *dbus_connection)
{
    DBusMessage *dmesg, *reply;
    DBusError error;

    const char *fstype = "";
    if (!(dmesg = dbus_message_new_method_call ("org.freedesktop.Hal", udi,
                                                "org.freedesktop.Hal.Device.Volume",
                                                "Mount"))) {
        kdDebug() << "mount failed for " << udi << ": could not create dbus message\n";
        return i18n("Internal Error");
    }

    if (!dbus_message_append_args (dmesg, DBUS_TYPE_STRING, &mount_point, DBUS_TYPE_STRING, &fstype,
                                   DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &poptions, noptions,
                                   DBUS_TYPE_INVALID))
    {
        kdDebug() << "mount failed for " << udi << ": could not append args to dbus message\n";
        dbus_message_unref (dmesg);
        return i18n("Internal Error");
    }

    QString qerror;

    dbus_error_init (&error);
    if (!(reply = dbus_connection_send_with_reply_and_block (dbus_connection, dmesg, -1, &error)))
    {
        QString qerror = error.message;
        kdError() << "mount failed for " << udi << ": " << error.name << " - " << qerror << endl;
        if ( !strcmp(error.name, "org.freedesktop.Hal.Device.Volume.UnknownFilesystemType"))
            qerror = i18n("Invalid filesystem type");
        else if ( !strcmp(error.name, "org.freedesktop.Hal.Device.Volume.PermissionDenied"))
            qerror = i18n("Permissions denied");
        else if ( !strcmp(error.name, "org.freedesktop.Hal.Device.Volume.AlreadyMounted"))
            qerror = i18n("Device is already mounted.");
        else if ( !strcmp(error.name, "org.freedesktop.Hal.Device.Volume.InvalidMountpoint") && strlen(mount_point)) {
            dbus_message_unref (dmesg);
            dbus_error_free (&error);
            return mount_priv(udi, "", poptions, noptions, dbus_connection);
        }
        dbus_message_unref (dmesg);
        dbus_error_free (&error);
        return qerror;
    }

    kdDebug() << "mount queued for " << udi << endl;

    dbus_message_unref (dmesg);
    dbus_message_unref (reply);

    return qerror;

}

QString HALBackend::listUsingProcesses(const Medium* medium)
{
    QString proclist, fullmsg;
    QString cmdline = QString("/usr/bin/env fuser -vm %1 2>&1").arg(KProcess::quote(medium->mountPoint()));
    FILE *fuser = popen(cmdline.latin1(), "r");

    uint counter = 0;
    if (fuser) {
        proclist += "<pre>";
        QTextIStream is(fuser);
        QString tmp;
        while (!is.atEnd()) {
            tmp = is.readLine();
            tmp = QStyleSheet::escape(tmp) + "\n";

            proclist += tmp;
            if (counter++ > 10)
            {
                proclist += "...";
                break;
            }
        }
        proclist += "</pre>";
        (void)pclose( fuser );
    }
    if (counter) {
        fullmsg = i18n("Moreover, programs still using the device "
            "have been detected. They are listed below. You have to "
            "close them or change their working directory before "
            "attempting to unmount the device again.");
        fullmsg += "<br>" + proclist;
        return fullmsg;
    } else {
        return QString::null;
    }
}

void HALBackend::slotResult(KIO::Job *job)
{
    kdDebug() << "slotResult " << mount_jobs[job] << endl;

    struct mount_job_data *data = mount_jobs[job];
    QString& qerror = data->errorMessage;
    const Medium* medium = data->medium;

    if (job->error() == KIO::ERR_COULD_NOT_UNMOUNT) {
        QString proclist(listUsingProcesses(medium));

        qerror = "<qt>";
        qerror += "<p>" + i18n("Unfortunately, the device <b>%1</b> (%2) named <b>'%3'</b> and "
                       "currently mounted at <b>%4</b> could not be unmounted. ").arg(
                          "system:/media/" + medium->name(),
                          medium->deviceNode(),
                          medium->prettyLabel(),
                          medium->prettyBaseURL().pathOrURL()) + "</p>";
        qerror += "<p>" + i18n("The following error was returned by umount command:");
        qerror += "</p><pre>" + job->errorText() + "</pre>";

        if (!proclist.isEmpty()) {
            qerror += proclist;
        }
        qerror += "</qt>";
    } else if (job->error()) {
        qerror = job->errorText();
    }

    ResetProperties( medium->id().latin1() );
    mount_jobs.remove(job);

    /* Job completed. Notify the caller */
    data->error = job->error();
    data->completed = true;
    kapp->eventLoop()->exitLoop();
}

QString HALBackend::isInFstab(const Medium *medium)
{
    KMountPoint::List fstab = KMountPoint::possibleMountPoints(KMountPoint::NeedMountOptions|KMountPoint::NeedRealDeviceName);

    KMountPoint::List::iterator it = fstab.begin();
    KMountPoint::List::iterator end = fstab.end();

    for (; it!=end; ++it)
    {
        QString reald = (*it)->realDeviceName();
        if ( reald.endsWith( "/" ) )
            reald = reald.left( reald.length() - 1 );
        kdDebug() << "isInFstab -" << medium->deviceNode() << "- -" << reald << "- -" << (*it)->mountedFrom() << "-" << endl;
        if ((*it)->mountedFrom() == medium->deviceNode() || ( !medium->deviceNode().isEmpty() && reald == medium->deviceNode() ) )
	{
            QStringList opts = (*it)->mountOptions();
            if (opts.contains("user") || opts.contains("users"))
                return (*it)->mountPoint();
        }
    }

    return QString::null;
}

QString HALBackend::mount(const Medium *medium)
{
    if (medium->isMounted())
        return QString(); // that was easy

    QString mountPoint = isInFstab(medium);
    if (!mountPoint.isNull())
    {
        struct mount_job_data data;
        data.completed = false;
        data.medium = medium;

        kdDebug() << "triggering user mount " << medium->deviceNode() << " " << mountPoint << " " << medium->id() << endl;
        KIO::Job *job = KIO::mount( false, 0, medium->deviceNode(), mountPoint );
        connect(job, SIGNAL( result (KIO::Job *)),
                SLOT( slotResult( KIO::Job *)));
        mount_jobs[job] = &data;
        // The caller expects the device to be mounted when the function
        // completes. Thus block until the job completes.
        while (!data.completed) {
            kapp->eventLoop()->enterLoop();
        }
        // Return the error message (if any) to the caller
        return (data.error) ? data.errorMessage : QString::null;

    } else if (medium->id().startsWith("/org/kde/") )
	    return i18n("Permissions denied");

    QStringList soptions;

    kdDebug() << "mounting " << medium->id() << "..." << endl;

    QMap<QString,QString> valids = MediaManagerUtils::splitOptions(mountoptions(medium->id()));
    if (valids["flush"] == "true")
        soptions << "flush";

    if (valids["uid"] == "true")
    {
        soptions << QString("uid=%1").arg(getuid());
    }

    if (valids["locale"] == "true")
	{
		soptions << QString("locale=%1").arg( KGlobal::locale()->language() ); 
	}

    if (valids["ro"] == "true")
        soptions << "ro";

    if (valids["atime"] != "true")
        soptions << "noatime";

    if (valids["quiet"] == "true")
        soptions << "quiet";

    if (valids["utf8"] == "true")
        soptions << "utf8";

    if (valids["sync"] == "true")
        soptions << "sync";

    QString mount_point = valids["mountpoint"];
    if (mount_point.startsWith("/media/"))
        mount_point = mount_point.mid(7);

    if (valids.contains("shortname"))
    {
        soptions << QString("shortname=%1").arg(valids["shortname"]);
    }

    if (valids.contains("journaling"))
    {
        QString option = valids["journaling"];
        if (option == "data")
            soptions << QString("data=journal");
        else if (option == "writeback")
            soptions << QString("data=writeback");
        else
            soptions << QString("data=ordered");
    }

    const char **options = new const char*[soptions.size() + 1];
    uint noptions = 0;
    for (QStringList::ConstIterator it = soptions.begin(); it != soptions.end(); ++it, ++noptions)
        options[noptions] = (*it).latin1();
    options[noptions] = NULL;

    QString qerror = mount_priv(medium->id().latin1(), mount_point.utf8(), options, noptions, dbus_connection);
    if (!qerror.isEmpty()) {
        kdError() << "mounting " << medium->id() << " returned " << qerror << endl;
        return qerror;
    }

    medium->setHalMounted(true);
    ResetProperties(medium->id().latin1());

    return QString();
}

QString HALBackend::mount(const QString &_udi)
{
    const Medium* medium = m_mediaList.findById(_udi);
    if (!medium)
        return i18n("No such medium: %1").arg(_udi);

    return mount(medium);
}

QString HALBackend::unmount(const QString &_udi)
{
    const Medium* medium = m_mediaList.findById(_udi);
    if (!medium)
    {   // now we get fancy: if the udi is no volume, it _might_ be a device with only one
        // volume on it (think CDs) - so we're so nice to the caller to unmount that volume
        LibHalDrive*  halDrive  = libhal_drive_from_udi(m_halContext, _udi.latin1());
        if (halDrive)
        {
            int numVolumes;
            char** volumes = libhal_drive_find_all_volumes(m_halContext, halDrive, &numVolumes);
            if (numVolumes == 1)
                medium = m_mediaList.findById( volumes[0] );
        }
    }

    if ( !medium )
        return i18n("No such medium: %1").arg(_udi);

    if (!medium->isMounted())
        return QString(); // that was easy

    QString mountPoint = isInFstab(medium);
    if (!mountPoint.isNull())
    {
        struct mount_job_data data;
        data.completed = false;
        data.medium = medium;

        kdDebug() << "triggering user unmount " << medium->deviceNode() << " " << mountPoint << endl;
        KIO::Job *job = KIO::unmount( medium->mountPoint(), false );
        connect(job, SIGNAL( result (KIO::Job *)),
                SLOT( slotResult( KIO::Job *)));
        mount_jobs[job] = &data;
        // The caller expects the device to be unmounted when the function
        // completes. Thus block until the job completes.
        while (!data.completed) {
            kapp->eventLoop()->enterLoop();
        }
        // Return the error message (if any) to the caller
        return (data.error) ? data.errorMessage : QString::null;
    }

    DBusMessage *dmesg, *reply;
    DBusError error;
    const char *options[2];

    const char *udi = medium->id().latin1();
    kdDebug() << "unmounting " << udi << "..." << endl;

    dbus_error_init(&error);
    DBusConnection *dbus_connection = dbus_bus_get(DBUS_BUS_SYSTEM, &error);
    if (dbus_error_is_set(&error))
    {
        dbus_error_free(&error);
        return false;
    }

    if (!(dmesg = dbus_message_new_method_call ("org.freedesktop.Hal", udi,
                                                "org.freedesktop.Hal.Device.Volume",
                                                "Unmount"))) {
        kdDebug() << "unmount failed for " << udi << ": could not create dbus message\n";
        return i18n("Internal Error");
    }

    options[0] = "force";
    options[1] = 0;

    if (!dbus_message_append_args (dmesg, DBUS_TYPE_ARRAY, DBUS_TYPE_STRING, &options, 0,
                                   DBUS_TYPE_INVALID))
    {
        kdDebug() << "unmount failed for " << udi << ": could not append args to dbus message\n";
        dbus_message_unref (dmesg);
        return i18n("Internal Error");
    }

    dbus_error_init (&error);
    if (!(reply = dbus_connection_send_with_reply_and_block (dbus_connection, dmesg, -1, &error)))
    {
        QString qerror, reason;

        kdDebug() << "unmount failed for " << udi << ": " << error.name << " " << error.message << endl;
        qerror = "<qt>";
        qerror += "<p>" + i18n("Unfortunately, the device <b>%1</b> (%2) named <b>'%3'</b> and "
                       "currently mounted at <b>%4</b> could not be unmounted. ").arg(
                          "system:/media/" + medium->name(),
                          medium->deviceNode(),
                          medium->prettyLabel(),
                          medium->prettyBaseURL().pathOrURL()) + "</p>";
        qerror += "<p>" + i18n("Unmounting failed due to the following error:") + "</p>";
        if (!strcmp(error.name, "org.freedesktop.Hal.Device.Volume.Busy")) {
            reason = i18n("Device is Busy:");
        } else if (!strcmp(error.name, "org.freedesktop.Hal.Device.Volume.NotMounted")) {
            // this is faking. The error is that the device wasn't mounted by hal (but by the system)
            reason = i18n("Permissions denied");
        } else {
            reason = error.message;
        }
        qerror += "<p><b>" + reason + "</b></p>";

        // Include list of processes (if any) using the device in the error message
        reason = listUsingProcesses(medium);
        if (!reason.isEmpty()) {
            qerror += reason;
        }

        dbus_message_unref (dmesg);
        dbus_error_free (&error);
        return qerror;
    }

    kdDebug() << "unmount queued for " << udi << endl;

    dbus_message_unref (dmesg);
    dbus_message_unref (reply);

    medium->setHalMounted(false);
    ResetProperties(udi);

    return QString();
}

#include "halbackend.moc"
