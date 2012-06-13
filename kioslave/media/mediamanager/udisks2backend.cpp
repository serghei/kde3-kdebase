/*
   This file is part of the KDE3 Fork Project
   Copyright (c) 2012 Serghei Amelian <serghei.amelian@gmail.com>

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

#include <qdict.h>

#include <dbus/qdbusconnection.h>
#include <dbus/qdbuserror.h>
#include <dbus/qdbusdata.h>
#include <dbus/qdbusdatalist.h>
#include <dbus/qdbusdatamap.h>
#include <dbus/qdbusmessage.h>
#include <dbus/qdbusobjectpath.h>
#include <dbus/qdbusproxy.h>
#include <dbus/qdbusvariant.h>

#include <klocale.h>
#include <kdebug.h>

#include "udisks2backend.h"


namespace UDisks2 {


/***** Class declarations **************************/


class Property {
public:
    Property(){}
    Property(const QDBusData &data);

    bool toBool() const;
    int toInt() const;
    Q_INT64 toInt64() const;
    QString toString() const;
    QStringList toStringList() const;
    QDBusObjectPath toObjectPath() const;

private:
    QDBusData m_data;
};


/***************************************************/


class Object : public QDBusProxy {
public:
    Object(ObjectManager *objectManager, const QDBusObjectPath &objectPath, const QDBusConnection &dbusConnection);
    ~Object();

    QString mount();
    QString unmount(bool force);

    bool isEmpty() const { return m_interfaces.isEmpty(); }

    void addInterfaces(const QDBusDataMap<QString> &interfaces);
    void removeInterfaces(const QValueList<QDBusData> &interfaces);

    bool callMethod(const QString &interface, const QString &method, const QValueList<QDBusData> &params, QDBusData &response, QDBusError &error);

private:
    Medium *createLoopMedium();
    Medium *createMountableMedium();
    Medium *createBlankOrAudioMedium();
    bool checkMediaAvailability();
    void propertiesChanged(const QString &interface, const QDBusDataMap<QString> &properties);
    void handleDBusSignal(const QDBusMessage &message);

private:
    ObjectManager *m_objectManager;
    QStringList m_interfaces;

    bool m_mediaAvailable;

    // loop interface
    bool m_loop;

    // drive interface
    bool m_optical;
    bool m_opticalBlank;
    bool m_opticalAudio;
    bool m_removable;
    bool m_ejectable;
    QString m_media;

    // block device interface
    bool m_mountable;
    QDBusObjectPath m_drive;
    QString m_device;
    QString m_label;
    QString m_fsType;
    Q_INT64 m_size;

    // filesystem interface
    bool m_filesystem;
    bool m_mounted;
    QString m_mountPoint;
};


/***************************************************/


class ObjectManager : public QDBusProxy {

    friend class Object;

public:
    ObjectManager(MediaList &mediaList);
    bool initialize();

    Object *findObject(const QString &path) { return m_objects.find(path); }

protected:
    void handleDBusSignal(const QDBusMessage &message);

private:
    void interfacesAdded(const QDBusObjectPath &objectPath, const QDBusDataMap<QString> &interfaces);
    void interfacesRemoved(const QDBusObjectPath &objectPath, const QValueList<QDBusData> &interfaces);

private:
    MediaList &m_mediaList;
    QDict<Object> m_objects;
    bool allowNotification;
};



/***** Helpers *************************************/


inline QString qDBusByteListToString(const QDBusData &data)
{
    QValueList<Q_UINT8> byteList = data.toList().toByteList();

    QString result;
    for(QValueList<Q_UINT8>::const_iterator it = byteList.begin(); it != byteList.end() && (*it); ++it)
      result.append(*it);
    return result;
}


QString qHumanReadableSize(Q_INT64 size)
{
    const char *units[] = { "B", "KiB", "MiB", "GiB", "TiB", "PiB" };
    int mod = 1024;

    int i;
    for(i = 0; size > mod && i < 5; i++)
        size /= mod;

    return QString("%1%2").arg(size).arg(units[i]);
}



/***** class Property ******************************/


Property::Property(const QDBusData &data)
    : m_data(data)
{
}


bool Property::toBool() const
{
    return m_data.toBool();
}


int Property::toInt() const
{
    if(QDBusData::UInt32 == m_data.type())
        return m_data.toUInt32();
    return m_data.toInt32();
}


Q_INT64 Property::toInt64() const
{
    if(QDBusData::UInt64 == m_data.type())
        return m_data.toUInt64();
    return m_data.toInt64();
}


QString Property::toString() const
{
    if(QDBusData::List == m_data.type())
      return qDBusByteListToString(m_data);
    return m_data.toString();
}


QStringList Property::toStringList() const
{
    QStringList stringList;

    QValueList<QDBusData> list = m_data.toList().toQValueList();
    for(QValueList<QDBusData>::const_iterator it = list.begin(); it != list.end(); ++it)
        stringList << qDBusByteListToString(*it);

    return stringList;
}


QDBusObjectPath Property::toObjectPath() const
{
    return m_data.toObjectPath();
}



/***** class Object ********************************/


Object::Object(ObjectManager *objectManager, const QDBusObjectPath &objectPath, const QDBusConnection &dbusConnection)
    : QDBusProxy(dbusConnection), m_objectManager(objectManager)
{
    setService("org.freedesktop.UDisks2");
    setPath(objectPath);

    m_mediaAvailable = false;
    m_loop = false;
    m_optical = false;
    m_opticalBlank = false;
    m_opticalAudio = false;
    m_removable = false;
    m_ejectable = false;
    m_mountable = false;
    m_filesystem = false;
    m_mounted = false;
}


Object::~Object()
{
}


QString Object::mount()
{
    QMap<QString, QDBusVariant> options;

    QValueList<QDBusData> params;
    params << QDBusData::fromStringKeyMap(QDBusDataMap<QString>(options));

    QDBusError error;
    QDBusData response;

    if(!callMethod("org.freedesktop.UDisks2.Filesystem", "Mount", params, response, error))
      return i18n("Unable to mount \"%1\".\nReason: %2").arg(m_device).arg(error.message());

    return QString::null;
}


QString Object::unmount(bool force)
{
    QMap<QString, QDBusVariant> options;

    if(force) {
        QDBusVariant force;
        force.value = QDBusData::fromBool(true);
        force.signature = force.value.buildDBusSignature();
        options["force"] = force;
    }

    QValueList<QDBusData> params;
    params << QDBusData::fromStringKeyMap(QDBusDataMap<QString>(options));

    QDBusError error;
    QDBusData response;

    if(!callMethod("org.freedesktop.UDisks2.Filesystem", "Unmount", params, response, error))
      return i18n("Unable to unmount \"%1\".\nReason: %2").arg(m_device).arg(error.message());

    return QString::null;
}


void Object::addInterfaces(const QDBusDataMap<QString> &interfaces)
{
    for(QDBusDataMap<QString>::const_iterator it = interfaces.begin(); it != interfaces.end(); ++it) {
        QString interface = it.key();

        // add the interface to list
        m_interfaces.append(interface);

        // update properties for given interface
        propertiesChanged(it.key(), it.data().toStringKeyMap());

        // filesystem interface was added
        if("org.freedesktop.UDisks2.Filesystem" == interface) {
            m_filesystem = true;
            checkMediaAvailability();
        }
    }
}


void Object::removeInterfaces(const QValueList<QDBusData> &interfaces)
{
    for(QValueList<QDBusData>::const_iterator it = interfaces.begin(); it != interfaces.end(); ++it) {
        QString interface = (*it).toString();

        // remove interface from list
        m_interfaces.remove(interface);

        // filesystem interface was removed
        if("org.freedesktop.UDisks2.Filesystem" == interface) {
            m_filesystem = false;
            checkMediaAvailability();
        }
    }
}


bool Object::callMethod(const QString &interface, const QString &method, const QValueList<QDBusData> &params, QDBusData &response, QDBusError &error)
{
    QDBusProxy proxy(service(), path(), interface, connection());

    QDBusMessage reply = proxy.sendWithReply(method, params);
    if(reply.count() != 1 || reply.type() != QDBusMessage::ReplyMessage) {
        error = proxy.lastError();
        if(QDBusError::InvalidError != error.type())
            return false;
    }

    response = reply.front();
    return true;
}


Medium *Object::createLoopMedium()
{
    QString name = (m_label.isEmpty() ? QString(m_device).section('/', -1, -1) : m_label);
    QString label = m_label;

    QString mimeType;
    QString iconName;

    mimeType = ("iso9660" == m_fsType ? "media/cdrom" : "media/hdd");

    if(label.isEmpty())
        label = i18n("Loop Device");

    mimeType += (m_mounted ? "_mounted" : "_unmounted");

    if(m_label.isEmpty())
        label = QString("%1 %2 (%3)").arg(qHumanReadableSize(m_size)).arg(label).arg(name);

    Medium *medium = new Medium(path(), name);
    medium->setLabel(label);
    medium->mountableState(m_device, m_mountPoint, m_fsType, m_mounted);
    medium->setMimeType(mimeType);
    medium->setIconName(iconName);

    return medium;
}


Medium *Object::createMountableMedium()
{
    Object *drive =  m_objectManager->m_objects.find(m_drive);
    Q_ASSERT(drive);

    QString name = (m_label.isEmpty() ? QString(m_device).section('/', -1, -1) : m_label);
    QString label = m_label;

    QString mimeType;
    QString iconName;

    // optical media
    if(drive->m_optical) {
        if("optical_cd_r" == drive->m_media.left(12))
            mimeType = "media/cdwriter";
        else if("optical_dvd" == drive->m_media.left(11))
            mimeType = "media/dvd";
        else
            mimeType = "media/cdrom";
        if(label.isEmpty())
            label = i18n("Optical Media");
    }

    // removable media
    else if(drive->m_removable) {
        mimeType = "media/removable";
        if(label.isEmpty())
            label = i18n("Removable Media");
    }

    // other media
    else {
        mimeType = "media/hdd";
        if(label.isEmpty())
            label = i18n("Hard Disk");
    }

    if("thumb" == drive->m_media)
        iconName = "usbpendrive";
    else if("flash_cf" == drive->m_media)
        iconName = "compact_flash";
    else if("flash_ms" == drive->m_media)
        iconName = "memory_stick";
    else if("flash_sm" == drive->m_media)
        iconName = "smart_media";
    else if("flash_sd" == drive->m_media.left(8))
        iconName = "sd_mmc";
    else if("flash_mmc" == drive->m_media)
        iconName = "sd_mmc";
    else if("floppy" == drive->m_media)
        iconName = "3floppy";
    else if("floppy_zip" == drive->m_media)
        iconName = "zip";

    mimeType += (m_mounted ? "_mounted" : "_unmounted");

    if(!iconName.isEmpty())
        iconName += (m_mounted ? "_mount" : "_unmount");

    if(m_label.isEmpty())
        label = QString("%1 %2 (%3)").arg(qHumanReadableSize(m_size)).arg(label).arg(name);

    Medium *medium = new Medium(path(), name);
    medium->setLabel(label);
    medium->mountableState(m_device, m_mountPoint, m_fsType, m_mounted);
    medium->setMimeType(mimeType);
    medium->setIconName(iconName);

    return medium;
}


Medium *Object::createBlankOrAudioMedium()
{
    QString name;
    QString mimeType;
    QString iconName;

    // blank cd or dvd
    if(m_opticalBlank) {
        if("optical_dvd" == m_media.left(11)) {
            name = i18n("Blank DVD");
            mimeType = "media/blankdvd";
        } else {
            name = i18n("Blank CD");
            mimeType = "media/blankcd";
        }
    }

    // audio cd
    else if(m_opticalAudio) {
        name = i18n("Audio CD");
        mimeType = "media/audiocd";
    }

    Medium *medium = new Medium(path(), name);
    medium->setLabel(name);
    medium->unmountableState("");
    medium->setMimeType(mimeType);

    return medium;
}


bool Object::checkMediaAvailability()
{
    bool mediaAvailable = (m_opticalBlank || m_opticalAudio || m_mountable && m_filesystem);
    bool availabilityChanged = (m_mediaAvailable != mediaAvailable);

    if(!availabilityChanged)
        return false;

    // media become available
    if(mediaAvailable) {
        Medium *medium = (m_mountable && m_filesystem ? (m_loop ? createLoopMedium() : createMountableMedium()) : createBlankOrAudioMedium());
        m_objectManager->m_mediaList.addMedium(medium, m_objectManager->allowNotification);
    }

    // media is not available anymore
    else {
        m_objectManager->m_mediaList.removeMedium(path(), true);
        if(m_mounted)
            unmount(true);
    }

    m_mediaAvailable = mediaAvailable;
    return true;
}


void Object::propertiesChanged(const QString &interface, const QDBusDataMap<QString> &properties)
{
    QMap<QString, QDBusVariant> map = properties.toVariantMap();

    bool mediumNeedUpdate = false;

    for(QMap<QString, QDBusVariant>::Iterator it = map.begin(); it != map.end(); ++it) {
        QString propertyName = it.key();
        Property propertyValue(it.data().value);

        // properties of drive
        if("org.freedesktop.UDisks2.Drive" == interface) {
            if("Optical" == propertyName)
                m_optical = propertyValue.toBool();
            else if("OpticalBlank" == propertyName)
                m_opticalBlank = propertyValue.toBool();
            else if("OpticalNumAudioTracks" == propertyName)
                m_opticalAudio = (0 < propertyValue.toInt());
            else if("Media" == propertyName)
                m_media = propertyValue.toString();
            else if("Removable" == propertyName)
                m_removable = propertyValue.toBool();
        }

        // properties of block device
        else if("org.freedesktop.UDisks2.Block" == interface) {
            if("IdUsage" == propertyName)
                m_mountable = ("filesystem" == propertyValue.toString());
            else if("Drive" == propertyName) {
                m_drive = propertyValue.toObjectPath();
                // if the block device haven't a drive,
                // we assume that is a loop device
                m_loop = ("/" == m_drive);
            }
            else if("PreferredDevice" == propertyName)
                m_device = propertyValue.toString();
            else if("IdLabel" == propertyName) {
                m_label = propertyValue.toString();
                mediumNeedUpdate = true;
            }
            else if("IdType" == propertyName)
                m_fsType = propertyValue.toString();
            else if("Size" == propertyName) {
                m_size = propertyValue.toInt64();
                mediumNeedUpdate = true;
            }
        }

        // properties of filesystem
        else if("org.freedesktop.UDisks2.Filesystem" == interface) {
            if("MountPoints" == propertyName) {
                QStringList mountPoints = propertyValue.toStringList();
                m_mounted = (0 < mountPoints.count());
                m_mountPoint = mountPoints.last();
                mediumNeedUpdate = true;
            }
        }
    }

    // media availability was changed
    if(checkMediaAvailability())
        return;

    if(mediumNeedUpdate) {
        Medium *medium = (m_mountable && m_filesystem ? (m_loop ? createLoopMedium() : createMountableMedium()) : createBlankOrAudioMedium());
        m_objectManager->m_mediaList.changeMediumState(*medium, false);
        delete medium;
    }
}


void Object::handleDBusSignal(const QDBusMessage &message)
{
    // the message is not for our object
    if(path() != message.path())
        return;

    // the message is not coming from dbus properties interface
    if("org.freedesktop.DBus.Properties" != message.interface())
        return;

    // this object haven't this kind of udisks2 interface
    QString interface = message[0].toString();
    if(!m_interfaces.contains(interface))
        return;

    QString method = message.member();
    if("PropertiesChanged" == method)
        propertiesChanged(interface, message[1].toStringKeyMap());
}



/***** class ObjectManager *************************/


ObjectManager::ObjectManager(MediaList &mediaList)
    : m_mediaList(mediaList)
{
    setService("org.freedesktop.UDisks2");
    setPath("/org/freedesktop/UDisks2");
    setInterface("org.freedesktop.DBus.ObjectManager");

    m_objects.setAutoDelete(true);
    allowNotification = false;
}


bool ObjectManager::initialize()
{
    // try to establish a connection to system bus
    QDBusConnection dbusConnection = QDBusConnection::systemBus();
    if(!dbusConnection.isConnected()) {
        kdDebug() << "UDisks2::ObjectManager: " << dbusConnection.lastError().message() << endl;
        return false;
    }

    setConnection(dbusConnection);

    // get udisks2 managed objects
    QDBusMessage reply = sendWithReply("GetManagedObjects", QValueList<QDBusData>());
    if(reply.count() != 1 || reply.type() != QDBusMessage::ReplyMessage) {
        kdDebug() << "UDisks2::ObjectManager: " << lastError().message() << endl;
        return false;
    }

    // convert dbus reply to array of ObjectPath
    QDBusDataMap<QDBusObjectPath> objectPathMap = reply.front().toObjectPathKeyMap();

    // we need to enumerate the drives first
    for(QDBusDataMap<QDBusObjectPath>::const_iterator it = objectPathMap.begin(); it != objectPathMap.end(); ++it) {
        if("/org/freedesktop/UDisks2/drives" == it.key().parentNode())
            interfacesAdded(it.key(), it.data().toStringKeyMap());
    }

    // enumerate block devices
    for(QDBusDataMap<QDBusObjectPath>::const_iterator it = objectPathMap.begin(); it != objectPathMap.end(); ++it) {
        if("/org/freedesktop/UDisks2/block_devices" == it.key().parentNode())
            interfacesAdded(it.key(), it.data().toStringKeyMap());
    }

    allowNotification = true;

    return true;
}


void ObjectManager::handleDBusSignal(const QDBusMessage &message)
{
    if((service().startsWith(":") && service() != message.sender()) || path() != message.path() || interface() != message.interface())
        return;

    QString method = message.member();
    QDBusObjectPath objectPath = message[0].toObjectPath();

    if("InterfacesAdded" == method)
        interfacesAdded(objectPath, message[1].toStringKeyMap());
    else if("InterfacesRemoved" == method)
        interfacesRemoved(objectPath, message[1].toQValueList());
}


void ObjectManager::interfacesAdded(const QDBusObjectPath &objectPath, const QDBusDataMap<QString> &interfaces)
{
    Object *object = m_objects.find(objectPath);

    if(!object) {
        object = new Object(this, objectPath, connection());
        m_objects.insert(objectPath, object);
    }

    object->addInterfaces(interfaces);
}


void ObjectManager::interfacesRemoved(const QDBusObjectPath &objectPath, const QValueList<QDBusData> &interfaces)
{
    Object *object = m_objects.find(objectPath);

    if(object) {
        object->removeInterfaces(interfaces);

        // destroy the object if it have no more interfaces
        if(object->isEmpty())
            m_objects.remove(objectPath);
    }
}


} // namespace UDisks2



/***** class UDisks2Backend ************************/


UDisks2Backend::UDisks2Backend(MediaList &mediaList)
    : BackendBase(mediaList)
{
    d = new UDisks2::ObjectManager(mediaList);
}


UDisks2Backend::~UDisks2Backend()
{
    delete d;
}


bool UDisks2Backend::initialize()
{
    return d->initialize();
}


QString UDisks2Backend::mount(const QString &name)
{
    const Medium *medium = m_mediaList.findById(name);
    if(!medium)
        return i18n("No such medium: %1").arg(name);

    UDisks2::Object *obj = d->findObject(name);
    if(!obj)
        return i18n("No such udisks2 object in cache: %1").arg(name);

   return obj->mount();
}


QString UDisks2Backend::unmount(const QString &name)
{
    const Medium *medium = m_mediaList.findById(name);
    if(!medium)
        return i18n("No such medium: %1").arg(name);

    UDisks2::Object *obj = d->findObject(name);
    if(!obj)
        return i18n("No such udisks2 object in cache: %1").arg(name);

   return obj->unmount(false);
}
