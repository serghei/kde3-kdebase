#ifndef __FONT_ENGINE_H__
#define __FONT_ENGINE_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : KFI::CFontEngine
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 29/04/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003, 2004
////////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include <qstring.h>
#include <qstringlist.h>

namespace KFI
{

class CFontEngine
{
    public:

    enum EType
    {
        // These have PS Info / support AFM stuff...
        TRUE_TYPE,
        TT_COLLECTION,
        TYPE_1,
        NONE
    };

    enum EWeight
    {
        WEIGHT_UNKNOWN=0,
        WEIGHT_THIN,
        WEIGHT_ULTRA_LIGHT,
        WEIGHT_EXTRA_LIGHT,
        WEIGHT_DEMI,
        WEIGHT_LIGHT,
        WEIGHT_BOOK,
        WEIGHT_MEDIUM,
        WEIGHT_REGULAR,
        WEIGHT_SEMI_BOLD,
        WEIGHT_DEMI_BOLD,
        WEIGHT_BOLD,
        WEIGHT_EXTRA_BOLD,
        WEIGHT_ULTRA_BOLD,
        WEIGHT_HEAVY,
        WEIGHT_BLACK
    };

    enum EItalic
    {
        ITALIC_NONE,
        ITALIC_ITALIC,
        ITALIC_OBLIQUE
    };

    private:

    struct TFtData
    {
        TFtData();
        ~TFtData();

        FT_Library library;
        FT_Face    face;
        bool       open;
    };

    public:

    CFontEngine() : itsType(NONE)               { }
    ~CFontEngine()                              { closeFont(); }

    static EType   getType(const char *fname);
    static QString weightStr(EWeight w);
    static QString italicStr(EItalic i)         { return ITALIC_NONE==i ? "r" : ITALIC_ITALIC==i ? "i" : "o"; }

    //
    // General functions - these should be used instead of specfic ones below...
    //
    bool            openFont(const QString &file, int face=0);
    void            closeFont();

    //
    const QString & getFamilyName()   { return itsFamily; }
    const QString & getPsName()       { return itsPsName; }
    EWeight         getWeight()       { return itsWeight; }
    EItalic         getItalic()       { return itsItalic; }
    EType           getType()         { return itsType; }
    int             getNumFaces()     { return itsFt.open ? itsFt.face->num_faces : 1; }
    bool            hasPsInfo()       { return itsType!=NONE; }

    static EWeight  strToWeight(const char *str);

    private:

    bool            openFontFt(const QString &file);
    void            closeFaceFt();

    private:

    EWeight itsWeight;
    EType   itsType;
    EItalic itsItalic;
    QString itsFamily,
            itsPsName,
            itsPath;
    int     itsNumFaces,
            itsFaceIndex;   // Only for TTC fonts - at the moment...
    TFtData itsFt;
};

}

#endif
