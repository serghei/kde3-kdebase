/****************************************************************************
**
**
** KRDB - puts current KDE color scheme into preprocessor statements
** cats specially written application default files and uses xrdb -merge to
** write to RESOURCE_MANAGER. Thus it gives a  simple way to make non-KDE
** applications fit in with the desktop
**
** Copyright (C) 1998 by Mark Donohoe
** Copyright (C) 1999 by Dirk A. Mueller (reworked for KDE 2.0)
** Copyright (C) 2001 by Matthias Ettrich (add support for GTK applications )
** Copyright (C) 2001 by Waldo Bastian <bastian@kde.org>
** Copyright (C) 2002 by Karol Szwed <gallium@kde.org>
** This application is freely distributable under the GNU Public License.
**
*****************************************************************************/

#include <config.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#undef Unsorted
#include <qbuffer.h>
#include <qdir.h>
#include <qsettings.h>
#include <qtooltip.h>

#include <dcopclient.h>

#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kprocio.h>
#include <ksavefile.h>
#include <ktempfile.h>
#include <klocale.h>
#include <kstyle.h>

#include "krdb.h"

#include <X11/Xlib.h>

inline const char *gtkEnvVar(int version)
{
    return 2 == version ? "GTK2_RC_FILES" : "GTK_RC_FILES";
}

inline const char *sysGtkrc(int version)
{
    if(2 == version)
    {
        if(access("/etc/opt/gnome/gtk-2.0", F_OK) == 0)
            return "/etc/opt/gnome/gtk-2.0/gtkrc";
        else
            return "/etc/gtk-2.0/gtkrc";
    }
    else
    {
        if(access("/etc/opt/gnome/gtk", F_OK) == 0)
            return "/etc/opt/gnome/gtk/gtkrc";
        else
            return "/etc/gtk/gtkrc";
    }
}

inline const char *userGtkrc(int version)
{
    return 2 == version ? "/.gtkrc-2.0" : "/.gtkrc";
}

// -----------------------------------------------------------------------------
static void applyGtkStyles(bool active, int version)
{
    QString gtkkde = locateLocal("config", 2 == version ? "gtkrc-2.0" : "gtkrc");
    QCString gtkrc = getenv(gtkEnvVar(version));
    QStringList list = QStringList::split(':', QFile::decodeName(gtkrc));
    if(list.count() == 0)
    {
        list.append(QString::fromLatin1(sysGtkrc(version)));
        list.append(QDir::homeDirPath() + userGtkrc(version));
    }
    list.remove(gtkkde);
    list.append(gtkkde);
    if(!active)
        ::unlink(QFile::encodeName(gtkkde));

    // Pass env. var to kdeinit.
    QCString name = gtkEnvVar(version);
    QCString value = QFile::encodeName(list.join(":"));
    QByteArray params;
    QDataStream stream(params, IO_WriteOnly);
    stream << name << value;
    kapp->dcopClient()->send("klauncher", "klauncher", "setLaunchEnv(QCString,QCString)", params);
}

// -----------------------------------------------------------------------------

static void applyQtColors(KConfig &kglobals, QSettings &settings, QPalette &newPal)
{
    QStringList actcg, inactcg, discg;

    /* export kde color settings */
    int i;
    for(i = 0; i < QColorGroup::NColorRoles; i++)
        actcg << newPal.color(QPalette::Active, (QColorGroup::ColorRole)i).name();
    for(i = 0; i < QColorGroup::NColorRoles; i++)
        inactcg << newPal.color(QPalette::Inactive, (QColorGroup::ColorRole)i).name();
    for(i = 0; i < QColorGroup::NColorRoles; i++)
        discg << newPal.color(QPalette::Disabled, (QColorGroup::ColorRole)i).name();

    while(!settings.writeEntry("/qt/Palette/active", actcg))
        ;
    settings.writeEntry("/qt/Palette/inactive", inactcg);
    settings.writeEntry("/qt/Palette/disabled", discg);

    // export kwin's colors to qtrc for kstyle to use
    kglobals.setGroup("WM");

    // active colors
    QColor clr = newPal.active().background();
    clr = kglobals.readColorEntry("activeBackground", &clr);
    settings.writeEntry("/qt/KWinPalette/activeBackground", clr.name());
    if(QPixmap::defaultDepth() > 8)
        clr = clr.dark(110);
    clr = kglobals.readColorEntry("activeBlend", &clr);
    settings.writeEntry("/qt/KWinPalette/activeBlend", clr.name());
    clr = newPal.active().highlightedText();
    clr = kglobals.readColorEntry("activeForeground", &clr);
    settings.writeEntry("/qt/KWinPalette/activeForeground", clr.name());
    clr = newPal.active().background();
    clr = kglobals.readColorEntry("frame", &clr);
    settings.writeEntry("/qt/KWinPalette/frame", clr.name());
    clr = kglobals.readColorEntry("activeTitleBtnBg", &clr);
    settings.writeEntry("/qt/KWinPalette/activeTitleBtnBg", clr.name());

    // inactive colors
    clr = newPal.inactive().background();
    clr = kglobals.readColorEntry("inactiveBackground", &clr);
    settings.writeEntry("/qt/KWinPalette/inactiveBackground", clr.name());
    if(QPixmap::defaultDepth() > 8)
        clr = clr.dark(110);
    clr = kglobals.readColorEntry("inactiveBlend", &clr);
    settings.writeEntry("/qt/KWinPalette/inactiveBlend", clr.name());
    clr = newPal.inactive().background().dark();
    clr = kglobals.readColorEntry("inactiveForeground", &clr);
    settings.writeEntry("/qt/KWinPalette/inactiveForeground", clr.name());
    clr = newPal.inactive().background();
    clr = kglobals.readColorEntry("inactiveFrame", &clr);
    settings.writeEntry("/qt/KWinPalette/inactiveFrame", clr.name());
    clr = kglobals.readColorEntry("inactiveTitleBtnBg", &clr);
    settings.writeEntry("/qt/KWinPalette/inactiveTitleBtnBg", clr.name());

    kglobals.setGroup("KDE");
    settings.writeEntry("/qt/KDE/contrast", kglobals.readNumEntry("contrast", 7));
}

// -----------------------------------------------------------------------------

static void applyQtSettings(KConfig &kglobals, QSettings &settings)
{
    /* export kde's plugin library path to qtrc */

    QMap< QString, bool > pathDb;
    // OK, this isn't fun at all.
    // KApp adds paths ending with /, QApp those without slash, and if
    // one gives it something that is other way around, it will complain and scare
    // users. So we need to know whether a path being added is from KApp, and in this case
    // end it with.. So keep a QMap to bool, specifying whether the path is KDE-specified..

    QString qversion = qVersion();
    if(qversion.contains('.') > 1)
        qversion.truncate(qversion.findRev('.'));
    if(qversion.contains('-'))
        qversion.truncate(qversion.findRev('-'));

    QStringList kdeAdded = settings.readListEntry("/qt/KDE/kdeAddedLibraryPaths");
    QString libPathKey = QString("/qt/%1/libraryPath").arg(qversion);

    // Read qt library path..
    QStringList plugins = settings.readListEntry(libPathKey, ':');
    for(QStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
    {
        QString path = *it;
        if(path.endsWith("/"))
            path.truncate(path.length() - 1);

        pathDb[path] = false;
    }

    // Get rid of old KDE-added ones...
    for(QStringList::ConstIterator it = kdeAdded.begin(); it != kdeAdded.end(); ++it)
    {
        // Normalize..
        QString path = *it;
        if(path.endsWith("/"))
            path.truncate(path.length() - 1);

        // Remove..
        pathDb.remove(path);
    }

    kdeAdded.clear();

    // Merge in KDE ones..
    plugins = KGlobal::dirs()->resourceDirs("qtplugins");

    for(QStringList::ConstIterator it = plugins.begin(); it != plugins.end(); ++it)
    {
        QString path = *it;
        if(path.endsWith("/"))
            path.truncate(path.length() - 1);

        pathDb[path] = true;

        if(path.contains("/lib64/"))
            path.replace("/lib64/", "/lib/");
        pathDb[path] = true;
    }

    QStringList paths;
    for(QMap< QString, bool >::ConstIterator it = pathDb.begin(); it != pathDb.end(); ++it)
    {
        QString path = it.key();
        bool fromKDE = it.data();

        char new_path[PATH_MAX + 1];
        if(realpath(QFile::encodeName(path), new_path))
            path = QFile::decodeName(new_path);

        if(fromKDE)
        {
            if(!path.endsWith("/"))
                path += "/";
            kdeAdded.push_back(path); // Add for the new list -- do it here to have it in the right form..
        }

        paths.append(path);
    }

    // Write the list out..
    settings.writeEntry("/qt/KDE/kdeAddedLibraryPaths", kdeAdded);
    settings.writeEntry(libPathKey, paths, ':');

    /* export widget style */
    kglobals.setGroup("General");
    QString style = kglobals.readEntry("widgetStyle", KStyle::defaultStyle());
    if(!style.isEmpty())
        settings.writeEntry("/qt/style", style);

    /* export font settings */
    settings.writeEntry("/qt/font", KGlobalSettings::generalFont().toString());

    /* ##### looks like kcmfonts skips this, so we don't do this here */
    /*bool usexft = kglobals.readBoolEntry("AntiAliasing", false);
      kconfig.setGroup("General");
      settings.writeEntry("/qt/enableXft", usexft);
      settings.writeEntry("/qt/useXft", usexft); */

    /* export effects settings */
    kglobals.setGroup("KDE");
    bool effectsEnabled = kglobals.readBoolEntry("EffectsEnabled", false);
    bool fadeMenus = kglobals.readBoolEntry("EffectFadeMenu", false);
    bool fadeTooltips = kglobals.readBoolEntry("EffectFadeTooltip", false);
    bool animateCombobox = kglobals.readBoolEntry("EffectAnimateCombo", false);

    QStringList guieffects;
    if(effectsEnabled)
    {
        guieffects << QString("general");
        if(fadeMenus)
            guieffects << QString("fademenu");
        if(animateCombobox)
            guieffects << QString("animatecombo");
        if(fadeTooltips)
            guieffects << QString("fadetooltip");
    }
    else
        guieffects << QString("none");

    settings.writeEntry("/qt/GUIEffects", guieffects);
}

// -----------------------------------------------------------------------------

static void addColorDef(QString &s, const char *n, const QColor &col)
{
    QString tmp;

    tmp.sprintf("#define %s #%02x%02x%02x\n", n, col.red(), col.green(), col.blue());

    s += tmp;
}


// -----------------------------------------------------------------------------

static void copyFile(QFile &tmp, QString const &filename, bool)
{
    QFile f(filename);
    if(f.open(IO_ReadOnly))
    {
        QCString buf(8192);
        while(!f.atEnd())
        {
            int read = f.readBlock(buf.data(), buf.size());
            if(read > 0)
                tmp.writeBlock(buf.data(), read);
        }
    }
}


// -----------------------------------------------------------------------------

static QString item(int i)
{
    return QString::number(i / 255.0, 'f', 3);
}

static QString color(const QColor &col)
{
    return QString("{ %1, %2, %3 }").arg(item(col.red())).arg(item(col.green())).arg(item(col.blue()));
}

static void createGtkrc(bool exportColors, const QColorGroup &cg, int version)
{
    // lukas: why does it create in ~/.kde/share/config ???
    // pfeiffer: so that we don't overwrite the user's gtkrc.
    // it is found via the GTK_RC_FILES environment variable.
    KSaveFile saveFile(locateLocal("config", 2 == version ? "gtkrc-2.0" : "gtkrc"));
    if(saveFile.status() != 0 || saveFile.textStream() == 0L)
        return;

    QTextStream &t = *saveFile.textStream();
    t.setEncoding(QTextStream::Locale);

    t << i18n(
             "# created by KDE, %1\n"
             "#\n"
             "# If you do not want KDE to override your GTK settings, select\n"
             "# Appearance & Themes -> Colors in the Control Center and disable the checkbox\n"
             "# \"Apply colors to non-KDE applications\"\n"
             "#\n"
             "#\n")
             .arg(QDateTime::currentDateTime().toString());

    t << "style \"default\"" << endl;
    t << "{" << endl;
    if(exportColors)
    {
        t << "  bg[NORMAL] = " << color(cg.background()) << endl;
        t << "  bg[SELECTED] = " << color(cg.highlight()) << endl;
        t << "  bg[INSENSITIVE] = " << color(cg.background()) << endl;
        t << "  bg[ACTIVE] = " << color(cg.mid()) << endl;
        t << "  bg[PRELIGHT] = " << color(cg.background()) << endl;
        t << endl;
        t << "  base[NORMAL] = " << color(cg.base()) << endl;
        t << "  base[SELECTED] = " << color(cg.highlight()) << endl;
        t << "  base[INSENSITIVE] = " << color(cg.background()) << endl;
        t << "  base[ACTIVE] = " << color(cg.highlight()) << endl;
        t << "  base[PRELIGHT] = " << color(cg.highlight()) << endl;
        t << endl;
        t << "  text[NORMAL] = " << color(cg.text()) << endl;
        t << "  text[SELECTED] = " << color(cg.highlightedText()) << endl;
        t << "  text[INSENSITIVE] = " << color(cg.mid()) << endl;
        t << "  text[ACTIVE] = " << color(cg.highlightedText()) << endl;
        t << "  text[PRELIGHT] = " << color(cg.highlightedText()) << endl;
        t << endl;
        t << "  fg[NORMAL] = " << color(cg.foreground()) << endl;
        t << "  fg[SELECTED] = " << color(cg.highlightedText()) << endl;
        t << "  fg[INSENSITIVE] = " << color(cg.mid()) << endl;
        t << "  fg[ACTIVE] = " << color(cg.foreground()) << endl;
        t << "  fg[PRELIGHT] = " << color(cg.foreground()) << endl;
    }

    t << "}" << endl;
    t << endl;
    t << "class \"*\" style \"default\"" << endl;
    t << endl;
    if(2 == version)
    { // we should maybe check for MacOS settings here
        t << "gtk-alternative-button-order = 1" << endl;
        t << endl;
    }

    if(exportColors)
    {
        // tooltips don't have the standard background color
        t << "style \"ToolTip\"" << endl;
        t << "{" << endl;
        QColorGroup group = QToolTip::palette().active();
        t << "  bg[NORMAL] = " << color(group.background()) << endl;
        t << "  base[NORMAL] = " << color(group.base()) << endl;
        t << "  text[NORMAL] = " << color(group.text()) << endl;
        t << "  fg[NORMAL] = " << color(group.foreground()) << endl;
        t << "}" << endl;
        t << endl;
        t << "widget \"gtk-tooltip\" style \"ToolTip\"" << endl;
        t << "widget \"gtk-tooltips\" style \"ToolTip\"" << endl;
        t << endl;


        // highlight the current (mouse-hovered) menu-item
        // not every button, checkbox, etc.
        t << "style \"MenuItem\"" << endl;
        t << "{" << endl;
        t << "  bg[PRELIGHT] = " << color(cg.highlight()) << endl;
        t << "  fg[PRELIGHT] = " << color(cg.highlightedText()) << endl;
        t << "}" << endl;
        t << endl;
        t << "class \"*MenuItem\" style \"MenuItem\"" << endl;
        t << endl;
    }
}

// -----------------------------------------------------------------------------

void runRdb(uint flags)
{
    // Obtain the application palette that is about to be set.
    QPalette newPal = KApplication::createApplicationPalette();
    bool exportColors = flags & KRdbExportColors;
    bool exportQtColors = flags & KRdbExportQtColors;
    bool exportQtSettings = flags & KRdbExportQtSettings;
    bool exportXftSettings = flags & KRdbExportXftSettings;

    KConfig kglobals("kdeglobals", true, false);
    kglobals.setGroup("KDE");

    KTempFile tmpFile;

    if(tmpFile.status() != 0)
    {
        kdDebug() << "Couldn't open temp file" << endl;
        exit(0);
    }

    QFile &tmp = *(tmpFile.file());

    // Export colors to non-(KDE/Qt) apps (e.g. Motif, GTK+ apps)
    if(exportColors)
    {
        KGlobal::dirs()->addResourceType("appdefaults", KStandardDirs::kde_default("data") + "kdisplay/app-defaults/");
        QColorGroup cg = newPal.active();
        KGlobal::locale()->insertCatalogue("krdb");
        createGtkrc(true, cg, 1);
        createGtkrc(true, cg, 2);

        QString preproc;
        QColor backCol = cg.background();
        addColorDef(preproc, "FOREGROUND", cg.foreground());
        addColorDef(preproc, "BACKGROUND", backCol);
        addColorDef(preproc, "HIGHLIGHT", backCol.light(100 + (2 * KGlobalSettings::contrast() + 4) * 16 / 1));
        addColorDef(preproc, "LOWLIGHT", backCol.dark(100 + (2 * KGlobalSettings::contrast() + 4) * 10));
        addColorDef(preproc, "SELECT_BACKGROUND", cg.highlight());
        addColorDef(preproc, "SELECT_FOREGROUND", cg.highlightedText());
        addColorDef(preproc, "WINDOW_BACKGROUND", cg.base());
        addColorDef(preproc, "WINDOW_FOREGROUND", cg.foreground());
        addColorDef(preproc, "INACTIVE_BACKGROUND", KGlobalSettings::inactiveTitleColor());
        addColorDef(preproc, "INACTIVE_FOREGROUND", KGlobalSettings::inactiveTitleColor());
        addColorDef(preproc, "ACTIVE_BACKGROUND", KGlobalSettings::activeTitleColor());
        addColorDef(preproc, "ACTIVE_FOREGROUND", KGlobalSettings::activeTitleColor());
        //---------------------------------------------------------------

        tmp.writeBlock(preproc.latin1(), preproc.length());

        QStringList list;

        QStringList adPaths = KGlobal::dirs()->findDirs("appdefaults", "");
        for(QStringList::ConstIterator it = adPaths.begin(); it != adPaths.end(); ++it)
        {
            QDir dSys(*it);

            if(dSys.exists())
            {
                dSys.setFilter(QDir::Files);
                dSys.setSorting(QDir::Name);
                dSys.setNameFilter("*.ad");
                list += dSys.entryList();
            }
        }

        for(QStringList::ConstIterator it = list.begin(); it != list.end(); it++)
            copyFile(tmp, locate("appdefaults", *it), true);
    }

    // Merge ~/.Xresources or fallback to ~/.Xdefaults
    QString homeDir = QDir::homeDirPath();
    QString xResources = homeDir + "/.Xresources";

    // very primitive support for ~/.Xresources by appending it
    if(QFile::exists(xResources))
        copyFile(tmp, xResources, true);
    else
        copyFile(tmp, homeDir + "/.Xdefaults", true);

    // Export the Xcursor theme & size settings
    KConfig mousecfg("kcminputrc");
    mousecfg.setGroup("Mouse");
    QString theme = mousecfg.readEntry("cursorTheme", QString());
    QString size = mousecfg.readEntry("cursorSize", QString());
    QString contents;

    if(!theme.isNull())
        contents = "Xcursor.theme: " + theme + '\n';

    if(!size.isNull())
        contents += "Xcursor.size: " + size + '\n';

    if(exportXftSettings)
    {
        kglobals.setGroup("General");

        if(kglobals.hasKey("XftAntialias"))
        {
            contents += "Xft.antialias: ";
            if(kglobals.readBoolEntry("XftAntialias", true))
                contents += "1\n";
            else
                contents += "0\n";
        }

        if(kglobals.hasKey("XftHintStyle"))
        {
            QString hintStyle = kglobals.readEntry("XftHintStyle", "hintmedium");
            contents += "Xft.hinting: ";
            if(hintStyle.isEmpty())
                contents += "-1\n";
            else
            {
                if(hintStyle != "hintnone")
                    contents += "1\n";
                else
                    contents += "0\n";
                contents += "Xft.hintstyle: " + hintStyle + '\n';
            }
        }

        if(kglobals.hasKey("XftSubPixel"))
        {
            QString subPixel = kglobals.readEntry("XftSubPixel");
            if(!subPixel.isEmpty())
                contents += "Xft.rgba: " + subPixel + '\n';
        }

        KConfig cfgfonts("kcmfonts", true);
        cfgfonts.setGroup("General");
        if(cfgfonts.readNumEntry("forceFontDPI", 0) != 0)
            contents += "Xft.dpi: " + cfgfonts.readEntry("forceFontDPI") + '\n';
    }

    if(contents.length() > 0)
        tmp.writeBlock(contents.latin1(), contents.length());

    tmpFile.close();

    KProcess proc;
#ifndef NDEBUG
    proc << "xrdb"
         << "-merge" << tmpFile.name();
#else
    proc << "xrdb"
         << "-quiet"
         << "-merge" << tmpFile.name();
#endif
    proc.start(KProcess::Block, KProcess::Stdin);

    tmpFile.unlink();

    applyGtkStyles(exportColors, 1);
    applyGtkStyles(exportColors, 2);

    /* Qt exports */
    if(exportQtColors || exportQtSettings)
    {
        QSettings *settings = new QSettings;

        if(exportQtColors)
            applyQtColors(kglobals, *settings, newPal); // For kcmcolors

        if(exportQtSettings)
            applyQtSettings(kglobals, *settings); // For kcmstyle

        delete settings;
        QApplication::flushX();

        // We let KIPC take care of ourselves, as we are in a KDE app with
        // QApp::setDesktopSettingsAware(false);
        // Instead of calling QApp::x11_apply_settings() directly, we instead
        // modify the timestamp which propagates the settings changes onto
        // Qt-only apps without adversely affecting ourselves.

        // Cheat and use the current timestamp, since we just saved to qtrc.
        QDateTime settingsstamp = QDateTime::currentDateTime();

        static Atom qt_settings_timestamp = 0;
        if(!qt_settings_timestamp)
        {
            QString atomname("_QT_SETTINGS_TIMESTAMP_");
            atomname += XDisplayName(0); // Use the $DISPLAY envvar.
            qt_settings_timestamp = XInternAtom(qt_xdisplay(), atomname.latin1(), False);
        }

        QBuffer stamp;
        QDataStream s(stamp.buffer(), IO_WriteOnly);
        s << settingsstamp;
        XChangeProperty(qt_xdisplay(), qt_xrootwin(), qt_settings_timestamp, qt_settings_timestamp, 8, PropModeReplace,
                        (unsigned char *)stamp.buffer().data(), stamp.buffer().size());
        QApplication::flushX();
    }
}
