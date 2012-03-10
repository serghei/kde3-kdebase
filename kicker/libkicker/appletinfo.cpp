/*****************************************************************

Copyright (c) 1996-2000 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <qfileinfo.h>
#include <kdesktopfile.h>
#include <kapplication.h>

#include "appletinfo.h"

AppletInfo::AppletInfo( const QString& deskFile, const QString& configFile, const AppletInfo::AppletType type)
  : m_type (type),
    m_unique(true),
    m_hidden(false)
{
    QFileInfo fi(deskFile);
    m_desktopFile = fi.fileName();

    const char* resource = "applets";
    switch (type)
    {
        case Extension:
            resource = "extensions";
            break;
        case BuiltinButton:
            resource = "builtinbuttons";
            break;
        case SpecialButton:
            resource = "specialbuttons";
            break;
        case Undefined:
        case Applet:
        default:
            break;
    }

    KDesktopFile df(m_desktopFile, true, resource);

    // set the appletssimple attributes
    setName(df.readName());
    setComment(df.readComment());
    setIcon(df.readIcon());

    // library
    setLibrary(df.readEntry("X-KDE-Library"));

    // is it a unique applet?
    setIsUnique(df.readBoolEntry("X-KDE-UniqueApplet", false));

    // should it be shown in the gui?
    m_hidden = df.readBoolEntry("Hidden", false);

    if (configFile.isEmpty())
    {
        // generate a config file base name from the library name
        m_configFile = m_lib.lower();

        if (m_unique)
        {
            m_configFile.append("rc");
        }
        else
        {
            m_configFile.append("_")
                        .append(kapp->randomString(20).lower())
                        .append("_rc");
        }
    }
    else
    {
        m_configFile = configFile;
    }
}

bool AppletInfo::operator!=( const AppletInfo& rhs) const
{
    return configFile() != rhs.configFile();
}

bool AppletInfo::operator<( const AppletInfo& rhs ) const
{
    return ( QString::localeAwareCompare( name(), rhs.name() ) < 0 );
}

bool AppletInfo::operator> ( const AppletInfo& rhs ) const
{
    return ( QString::localeAwareCompare( name(), rhs.name() ) > 0 );
}

bool AppletInfo::operator<= ( const AppletInfo& rhs ) const
{
    return ( QString::localeAwareCompare( name(), rhs.name() ) <= 0 );
}
