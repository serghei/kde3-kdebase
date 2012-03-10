/* This file is part of the KDE Project
   Copyright (c) 2004 Kévin Ottens <ervin ipsquad net>

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

#include "mediamanager.h"

#include <config.h>
#include <qtimer.h>

#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>

#include <kdirnotify_stub.h>
#include <kstandarddirs.h>

#include "mediamanagersettings.h"

#include "fstabbackend.h"

#ifdef COMPILE_HALBACKEND
#include "halbackend.h"
#endif //COMPILE_HALBACKEND

#ifdef COMPILE_LINUXCDPOLLING
#include "linuxcdpolling.h"
#endif //COMPILE_LINUXCDPOLLING


MediaManager::MediaManager(const QCString &obj)
    : KDEDModule(obj), m_dirNotify(m_mediaList)
{
    connect( &m_mediaList, SIGNAL(mediumAdded(const QString&, const QString&, bool)),
             SLOT(slotMediumAdded(const QString&, const QString&, bool)) );
    connect( &m_mediaList, SIGNAL(mediumRemoved(const QString&, const QString&, bool)),
             SLOT(slotMediumRemoved(const QString&, const QString&, bool)) );
    connect( &m_mediaList,
             SIGNAL(mediumStateChanged(const QString&, const QString&, bool, bool)),
             SLOT(slotMediumChanged(const QString&, const QString&, bool, bool)) );

    QTimer::singleShot( 10, this, SLOT( loadBackends() ) );
}

MediaManager::~MediaManager()
{
    while ( !m_backends.isEmpty() )
    {
        BackendBase *b = m_backends.first();
        m_backends.remove( b );
        delete b;
    }
}

void MediaManager::loadBackends()
{
    m_mediaList.blockSignals(true);

    while ( !m_backends.isEmpty() )
    {
        BackendBase *b = m_backends.first();
        m_backends.remove( b );
        delete b;
    }

    mp_removableBackend = 0L;
    m_halbackend = 0L;
    m_fstabbackend = 0L;

#ifdef COMPILE_HALBACKEND
    if ( MediaManagerSettings::self()->halBackendEnabled() )
    {
        m_halbackend = new HALBackend(m_mediaList, this);
        if (m_halbackend->InitHal())
        {
            m_backends.append( m_halbackend );
            m_fstabbackend = new FstabBackend(m_mediaList, true);
            m_backends.append( m_fstabbackend );
            // No need to load something else...
            m_mediaList.blockSignals(false);
            return;
        }
        else
        {
            delete m_halbackend;
            m_halbackend = 0;
        }
    }
#endif // COMPILE_HALBACKEND

    mp_removableBackend = new RemovableBackend(m_mediaList);
    m_backends.append( mp_removableBackend );

#ifdef COMPILE_LINUXCDPOLLING
    if ( MediaManagerSettings::self()->cdPollingEnabled() )
    {
        m_backends.append( new LinuxCDPolling(m_mediaList) );
    }
#endif //COMPILE_LINUXCDPOLLING

    m_fstabbackend = new FstabBackend(m_mediaList);
    m_backends.append( m_fstabbackend );
    m_mediaList.blockSignals(false);
}


QStringList MediaManager::fullList()
{
    QPtrList<Medium> list = m_mediaList.list();

    QStringList result;

    QPtrList<Medium>::const_iterator it = list.begin();
    QPtrList<Medium>::const_iterator end = list.end();
    for (; it!=end; ++it)
    {
        result+= (*it)->properties();
        result+= Medium::SEPARATOR;
    }

    return result;
}

QStringList MediaManager::properties(const QString &name)
{
    const Medium *m = m_mediaList.findByName(name);

    if (!m)
    {
        KURL u(name);
        kdDebug() << "Media::prop " << name << " " << u.isValid() << endl;
        if (u.isValid())
        {
            if (u.protocol() == "system")
            {
                QString path = u.path();
                if (path.startsWith("/media/"))
                    path = path.mid(strlen("/media/"));
                m = m_mediaList.findByName(path);
                kdDebug() << "findByName " << path << m << endl;
            }
            else if (u.protocol() == "media")
            {
                m = m_mediaList.findByName(u.filename());
                kdDebug() << "findByName " << u.filename() << m << endl;
            }
            else if (u.protocol() == "file")
            {
                // look for the mount point
                QPtrList<Medium> list = m_mediaList.list();
                QPtrList<Medium>::const_iterator it = list.begin();
                QPtrList<Medium>::const_iterator end = list.end();
                QString path;

                for (; it!=end; ++it)
                {
                    path = KStandardDirs::realFilePath(u.path());
                    kdDebug() << "comparing " << (*it)->mountPoint()  << " " << path << " " <<  (*it)->deviceNode() << endl;
                    if ((*it)->mountPoint() == path || (*it)->deviceNode() == path) {
			m = *it;
			break;
                    }
                }
            }
        }
    }

    if (m)
        return m->properties();
    else
        return QStringList();
}

QStringList MediaManager::mountoptions(const QString &name)
{
#ifdef COMPILE_HALBACKEND
    if (!m_halbackend)
        return QStringList();
    return m_halbackend->mountoptions(name);
#else
    return QStringList();
#endif
}

bool MediaManager::setMountoptions(const QString &name, const QStringList &options)
{
#ifdef COMPILE_HALBACKEND
    if (!m_halbackend)
        return false;
    return m_halbackend->setMountoptions(name, options);
#else
    return false;
#endif
}

QString MediaManager::mount(const QString &name)
{
#ifdef COMPILE_HALBACKEND
    if (!m_halbackend)
        return i18n("Feature only available with HAL");
    return m_halbackend->mount(name);
#else
    if ( !m_fstabbackend ) // lying :)
        return i18n("Feature only available with HAL");
    return m_fstabbackend->mount( name );
#endif
}

QString MediaManager::unmount(const QString &name)
{
#ifdef COMPILE_HALBACKEND
    if (!m_halbackend)
        return i18n("Feature only available with HAL");
    return m_halbackend->unmount(name);
#else
    if ( !m_fstabbackend ) // lying :)
        return i18n("Feature only available with HAL");
    return m_fstabbackend->unmount( name );
#endif
}

QString MediaManager::nameForLabel(const QString &label)
{
    const QPtrList<Medium> media = m_mediaList.list();

    QPtrList<Medium>::const_iterator it = media.begin();
    QPtrList<Medium>::const_iterator end = media.end();
    for (; it!=end; ++it)
    {
        const Medium *m = *it;

        if (m->prettyLabel()==label)
        {
            return m->name();
        }
    }

    return QString::null;
}

ASYNC MediaManager::setUserLabel(const QString &name, const QString &label)
{
    m_mediaList.setUserLabel(name, label);
}

ASYNC MediaManager::reloadBackends()
{
    MediaManagerSettings::self()->readConfig();
    loadBackends();
}

bool MediaManager::removablePlug(const QString &devNode, const QString &label)
{
    if (mp_removableBackend)
    {
        return mp_removableBackend->plug(devNode, label);
    }
    return false;
}

bool MediaManager::removableUnplug(const QString &devNode)
{
    if (mp_removableBackend)
    {
        return mp_removableBackend->unplug(devNode);
    }
    return false;
}

bool MediaManager::removableCamera(const QString &devNode)
{
    if (mp_removableBackend)
    {
        return mp_removableBackend->camera(devNode);
    }
    return false;
}


void MediaManager::slotMediumAdded(const QString &/*id*/, const QString &name,
                                   bool allowNotification)
{
    kdDebug(1219) << "MediaManager::slotMediumAdded: " << name << endl;

    KDirNotify_stub notifier("*", "*");
    notifier.FilesAdded( KURL("media:/") );

    emit mediumAdded(name, allowNotification);
    emit mediumAdded(name);
}

void MediaManager::slotMediumRemoved(const QString &/*id*/, const QString &name,
                                     bool allowNotification)
{
    kdDebug(1219) << "MediaManager::slotMediumRemoved: " << name << endl;

    KDirNotify_stub notifier("*", "*");
    notifier.FilesRemoved( KURL("media:/"+name) );

    emit mediumRemoved(name, allowNotification);
    emit mediumRemoved(name);
}

void MediaManager::slotMediumChanged(const QString &/*id*/, const QString &name,
                                     bool mounted, bool allowNotification)
{
    kdDebug(1219) << "MediaManager::slotMediumChanged: " << name << endl;

    KDirNotify_stub notifier("*", "*");
    if (!mounted)
    {
        notifier.FilesRemoved( KURL("media:/"+name) );
    }
    notifier.FilesChanged( KURL("media:/"+name) );

    emit mediumChanged(name, allowNotification);
    emit mediumChanged(name);
}


extern "C" {
    KDE_EXPORT KDEDModule *create_mediamanager(const QCString &obj)
    {
        KGlobal::locale()->insertCatalogue("kio_media");
        return new MediaManager(obj);
    }
}

#include "mediamanager.moc"
