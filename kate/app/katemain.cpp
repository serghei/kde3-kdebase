/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#include "kateapp.h"

#include <kstandarddirs.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kinstance.h>
#include <kstartupinfo.h>
#include <dcopclient.h>
#include <dcopref.h>
#include <kdebug.h>

#include <qtextcodec.h>

#include <stdlib.h>

static KCmdLineOptions options[] = {{"s", 0, 0},
                                    {"start <name>", I18N_NOOP("Start Kate with a given session"), 0},
                                    {"u", 0, 0},
                                    {"use", I18N_NOOP("Use a already running kate instance (if possible)"), 0},
                                    {"p", 0, 0},
                                    {"pid <pid>", I18N_NOOP("Only try to reuse kate instance with this pid"), 0},
                                    {"e", 0, 0},
                                    {"encoding <name>", I18N_NOOP("Set encoding for the file to open"), 0},
                                    {"l", 0, 0},
                                    {"line <line>", I18N_NOOP("Navigate to this line"), 0},
                                    {"c", 0, 0},
                                    {"column <column>", I18N_NOOP("Navigate to this column"), 0},
                                    {"i", 0, 0},
                                    {"stdin", I18N_NOOP("Read the contents of stdin"), 0},
                                    {"+[URL]", I18N_NOOP("Document to open"), 0},
                                    KCmdLineLastOption};

extern "C" KDE_EXPORT int kdemain(int argc, char **argv)
{
    // here we go, construct the Kate version
    QString kateVersion = KateApp::kateVersion();

    KAboutData aboutData("kate", I18N_NOOP("Kate"), kateVersion.latin1(), I18N_NOOP("Kate - Advanced Text Editor"), KAboutData::License_LGPL_V2,
                         I18N_NOOP("(c) 2000-2005 The Kate Authors"), 0, "http://kate.kde.org");

    aboutData.addAuthor("Christoph Cullmann", I18N_NOOP("Maintainer"), "cullmann@kde.org", "http://www.babylon2k.de");
    aboutData.addAuthor("Anders Lund", I18N_NOOP("Core Developer"), "anders@alweb.dk", "http://www.alweb.dk");
    aboutData.addAuthor("Joseph Wenninger", I18N_NOOP("Core Developer"), "jowenn@kde.org", "http://stud3.tuwien.ac.at/~e9925371");
    aboutData.addAuthor("Hamish Rodda", I18N_NOOP("Core Developer"), "rodda@kde.org");
    aboutData.addAuthor("Waldo Bastian", I18N_NOOP("The cool buffersystem"), "bastian@kde.org");
    aboutData.addAuthor("Charles Samuels", I18N_NOOP("The Editing Commands"), "charles@kde.org");
    aboutData.addAuthor("Matt Newell", I18N_NOOP("Testing, ..."), "newellm@proaxis.com");
    aboutData.addAuthor("Michael Bartl", I18N_NOOP("Former Core Developer"), "michael.bartl1@chello.at");
    aboutData.addAuthor("Michael McCallum", I18N_NOOP("Core Developer"), "gholam@xtra.co.nz");
    aboutData.addAuthor("Jochen Wilhemly", I18N_NOOP("KWrite Author"), "digisnap@cs.tu-berlin.de");
    aboutData.addAuthor("Michael Koch", I18N_NOOP("KWrite port to KParts"), "koch@kde.org");
    aboutData.addAuthor("Christian Gebauer", 0, "gebauer@kde.org");
    aboutData.addAuthor("Simon Hausmann", 0, "hausmann@kde.org");
    aboutData.addAuthor("Glen Parker", I18N_NOOP("KWrite Undo History, Kspell integration"), "glenebob@nwlink.com");
    aboutData.addAuthor("Scott Manson", I18N_NOOP("KWrite XML Syntax highlighting support"), "sdmanson@alltel.net");
    aboutData.addAuthor("John Firebaugh", I18N_NOOP("Patches and more"), "jfirebaugh@kde.org");
    aboutData.addAuthor("Dominik Haumann", I18N_NOOP("Developer & Highlight wizard"), "dhdev@gmx.de");

    aboutData.addCredit("Matteo Merli", I18N_NOOP("Highlighting for RPM Spec-Files, Perl, Diff and more"), "merlim@libero.it");
    aboutData.addCredit("Rocky Scaletta", I18N_NOOP("Highlighting for VHDL"), "rocky@purdue.edu");
    aboutData.addCredit("Yury Lebedev", I18N_NOOP("Highlighting for SQL"), "");
    aboutData.addCredit("Chris Ross", I18N_NOOP("Highlighting for Ferite"), "");
    aboutData.addCredit("Nick Roux", I18N_NOOP("Highlighting for ILERPG"), "");
    aboutData.addCredit("Carsten Niehaus", I18N_NOOP("Highlighting for LaTeX"), "");
    aboutData.addCredit("Per Wigren", I18N_NOOP("Highlighting for Makefiles, Python"), "");
    aboutData.addCredit("Jan Fritz", I18N_NOOP("Highlighting for Python"), "");
    aboutData.addCredit("Daniel Naber", "", "");
    aboutData.addCredit("Roland Pabel", I18N_NOOP("Highlighting for Scheme"), "");
    aboutData.addCredit("Cristi Dumitrescu", I18N_NOOP("PHP Keyword/Datatype list"), "");
    aboutData.addCredit("Carsten Pfeiffer", I18N_NOOP("Very nice help"), "");
    aboutData.addCredit(I18N_NOOP("All people who have contributed and I have forgotten to mention"), "", "");

    aboutData.setTranslator(I18N_NOOP2("NAME OF TRANSLATORS", "Your names"), I18N_NOOP2("EMAIL OF TRANSLATORS", "Your emails"));

    // command line args init and co
    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineArgs::addCmdLineOptions(options);
    KCmdLineArgs::addTempFileOption();
    KateApp::addCmdLineOptions();

    // get our command line args ;)
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    // now, first try to contact running kate instance if needed
    if(args->isSet("use") || (::getenv("KATE_PID") != 0))
    {
        DCOPClient client;
        client.attach();

        // get all attached clients ;)
        KStringList allClients = client.registeredApplications();

        // search for a kate app client, use the first found
        QCString kateApp;

        if(args->isSet("start"))
        {
            for(unsigned int i = 0; i < allClients.count(); i++)
            {
                if(allClients[i] == "kate" || allClients[i].left(5) == "kate-")
                {
                    DCOPRef ref(allClients[i], "KateApplication");
                    QString s = ref.call("session");
                    if(QString(args->getOption("start")) == s)
                    {
                        kateApp = allClients[i];
                        break;
                    }
                }
            }
        }
        else if((args->isSet("pid")) || (::getenv("KATE_PID") != 0))
        {
            QCString tryApp;
            if(args->isSet("pid"))
                tryApp = args->getOption("pid");
            else
                tryApp = ::getenv("KATE_PID");

            if(client.isApplicationRegistered(tryApp.prepend("kate-")))
                kateApp = tryApp;
        }
        else
        {
            for(unsigned int i = 0; i < allClients.count(); ++i)
            {
                if(allClients[i] == "kate" || allClients[i].left(5) == "kate-")
                {
                    kateApp = allClients[i];
                    break;
                }
            }
        }

        // found a matching kate client ;)
        if(!kateApp.isEmpty())
        {
            kdDebug() << "kate app: " << kateApp << endl;
            // make kdeinit happy
            client.registerAs("kate");

            DCOPRef kRef(kateApp, "KateApplication");

            if(args->isSet("start"))
                kRef.call("activateSession", QString(args->getOption("start")));

            QString enc = args->isSet("encoding") ? args->getOption("encoding") : QCString("");

            bool tempfileSet = KCmdLineArgs::isTempFileSet();

            for(int z = 0; z < args->count(); z++)
                kRef.call("openURL", args->url(z), enc, tempfileSet);

            if(args->isSet("stdin"))
            {
                QTextIStream input(stdin);

                // set chosen codec
                QTextCodec *codec = args->isSet("encoding") ? QTextCodec::codecForName(args->getOption("encoding")) : 0;

                if(codec)
                    input.setCodec(codec);

                QString line;
                QString text;

                do
                {
                    line = input.readLine();
                    text.append(line + "\n");
                } while(!line.isNull());

                kRef.call("openInput", text);
            }

            int line = 0;
            int column = 0;
            bool nav = false;

            if(args->isSet("line"))
            {
                line = args->getOption("line").toInt();
                nav = true;
            }

            if(args->isSet("column"))
            {
                column = args->getOption("column").toInt();
                nav = true;
            }

            if(nav)
                kRef.call("setCursor", line, column);

            // since the user tried to open a document, let us assume [s]he
            // wants to see that document.
            // ### what to do about the infamous focus stealing prevention?
            uint mwn = kRef.call("activeMainWindowNumber");
            QCString smwn;
            DCOPRef wRef(kateApp, QCString("__KateMainWindow#") + smwn.setNum(mwn));
            if(wRef.call("minimized"))
            {
                if(wRef.call("maximized"))
                    wRef.call("maximize");
                else
                    wRef.call("restore");
            }
            wRef.call("raise");

            // stop startup notification
            KStartupInfo::appStarted();

            return 0;
        }
    }

    // construct the real kate app object ;)
    KateApp app(args);

    // app execution should already end :)
    if(app.shouldExit())
    {
        return 0;
    }

    // execute ourself ;)
    return app.exec();
}

// kate: space-indent on; indent-width 2; replace-tabs on; mixed-indent off;
