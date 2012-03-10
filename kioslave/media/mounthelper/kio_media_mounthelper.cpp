/* This file is part of the KDE project
   Copyright (c) 2004 Kévin Ottens <ervin ipsquad net>
   Parts of this file are
   Copyright 2003 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kcmdlineargs.h>
#include <klocale.h>
#include <kapplication.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <qtimer.h>
#include <stdlib.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kprocess.h>
#include <kstartupinfo.h>

#include "kio_media_mounthelper.h"

const Medium MountHelper::findMedium(const KURL &url)
{
	DCOPRef mediamanager("kded", "mediamanager");

	// Try filename first
	DCOPReply reply = mediamanager.call( "properties", url.fileName() );
	if ( !reply.isValid() ) {
		m_errorStr = i18n("The KDE mediamanager is not running.")+"\n";
		return Medium(QString::null, QString::null);
	}
	const Medium& medium = Medium::create(reply);
	if ( medium.id().isEmpty() ) {
		// Try full URL now
		reply = mediamanager.call( "properties", url.prettyURL() );
		if ( !reply.isValid() ) {
			m_errorStr = i18n("Internal Error");
			return Medium(QString::null, QString::null);
		}
		return Medium::create(reply);
	} else {
		return medium;
	}
}

MountHelper::MountHelper() : KApplication()
{
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

	m_errorStr = "";

	KURL url(args->url(0));
	const Medium medium = findMedium(url);

	if ( medium.id().isEmpty() )
	{
		if (m_errorStr.isEmpty())
			m_errorStr+= i18n("%1 cannot be found.").arg(url.prettyURL());
		QTimer::singleShot(0, this, SLOT(error()) );
		return;
	}

	if ( !medium.isMountable() && !args->isSet("e") && !args->isSet("s"))
	{
		m_errorStr = i18n("%1 is not a mountable media.").arg(url.prettyURL());
		QTimer::singleShot(0, this, SLOT(error()) );
		return;
	}

	QString device = medium.deviceNode();
	QString mount_point = medium.mountPoint();

	m_isCdrom = medium.mimeType().find("dvd")!=-1
	         || medium.mimeType().find("cd")!=-1;

	if (args->isSet("u"))
	{
	  DCOPRef mediamanager("kded", "mediamanager");
	  DCOPReply reply = mediamanager.call( "unmount", medium.id());
	  if (reply.isValid())
	    reply.get(m_errorStr);
	  kdDebug() << "medium unmount " << m_errorStr << endl;
	  if (m_errorStr.isNull())
	    ::exit(0);
	  else
	    error();
	}
	else if (args->isSet("s") || args->isSet("e"))
	{
		/*
		* We want to call mediamanager unmount before invoking eject. That's
		* because unmount would provide an informative error message in case of
		* failure. However, there are cases when unmount would fail
		* (supermount, slackware, see bug#116209) but eject would succeed.
		* Thus if unmount fails, save unmount error message and invokeEject()
		* anyway. Only if both unmount and eject fail, notify the user by
		* displaying the saved error message (see ejectFinished()).
		*/
		if (medium.isMounted())
		{
			DCOPRef mediamanager("kded", "mediamanager");
			DCOPReply reply = mediamanager.call( "unmount", medium.id());
			if (reply.isValid())
                            reply.get(m_errorStr);
                        if (m_errorStr.isNull())
                            invokeEject(device, true);
                        else
                            error();
			m_device = device;
		} else
                    invokeEject(device, true);
	}
	else
	{
	  DCOPRef mediamanager("kded", "mediamanager");
	  DCOPReply reply = mediamanager.call( "mount", medium.id());
	  if (reply.isValid())
	    reply.get(m_errorStr);
	  if (m_errorStr.isNull())
	    ::exit(0);
	  else
	    error();
	}
}

void MountHelper::invokeEject(const QString &device, bool quiet)
{
	KProcess *proc = new KProcess(this);
	*proc << "kdeeject";
	if (quiet)
	{
		*proc << "-q";
	}
	*proc << device;
	connect( proc, SIGNAL(processExited(KProcess *)),
		this, SLOT( ejectFinished(KProcess *) ) );
	proc->start();
}

void MountHelper::ejectFinished(KProcess* proc)
{
	/*
	* If eject failed, report the error stored in m_errorStr
	*/
	if (proc->normalExit() && proc->exitStatus() == 0) {
		::exit(0);
	} else {
		if (m_errorStr.isEmpty()) {
			if (m_isCdrom)
				m_errorStr = i18n("The device was successfully unmounted, but the tray could not be opened");
			else
				m_errorStr = i18n("The device was successfully unmounted, but could not be ejected");
		}
		QTimer::singleShot(0, this, SLOT(error()));
	}
}

void MountHelper::error()
{
	KMessageBox::error(0, m_errorStr);
	::exit(1);
}

static KCmdLineOptions options[] =
{
	{ "u", I18N_NOOP("Unmount given URL"), 0 },
	{ "m", I18N_NOOP("Mount given URL (default)"), 0 },
	{ "e", I18N_NOOP("Eject given URL via kdeeject"), 0},
	{ "s", I18N_NOOP("Unmount and Eject given URL (necessary for some USB devices)"), 0},
	{"!+URL",   I18N_NOOP("media:/ URL to mount/unmount/eject/remove"), 0 },
	KCmdLineLastOption
};


int main(int argc, char **argv)
{
	KCmdLineArgs::init(argc, argv, "kio_media_mounthelper",
	                   "kio_media_mounthelper", "kio_media_mounthelper",
	                   "0.1");

	KCmdLineArgs::addCmdLineOptions( options );
	KGlobal::locale()->setMainCatalogue("kio_media");
	KApplication::addCmdLineOptions();

	if (KCmdLineArgs::parsedArgs()->count()==0) KCmdLineArgs::usage();
	KApplication *app = new  MountHelper();

	KStartupInfo::appStarted();
	app->dcopClient()->attach();
	return app->exec();
}

#include "kio_media_mounthelper.moc"
