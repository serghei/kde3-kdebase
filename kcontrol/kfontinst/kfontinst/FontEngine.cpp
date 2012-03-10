///////////////////////////////////////////////////////////////////////////////
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


#include "FontEngine.h"
#include "Misc.h"
#include <kglobal.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <qregexp.h>
#include <qfile.h>
#include <ft2build.h>
#include FT_SFNT_NAMES_H
#include FT_TRUETYPE_IDS_H
#include FT_TRUETYPE_TABLES_H
#include FT_TYPE1_TABLES_H

namespace KFI
{

bool CFontEngine::openFont(const QString &file, int face)
{
    bool ok=false;

    if(file==itsPath && face==itsFaceIndex)
    {
        ok=NONE!=itsType;
    }
    else
    {
        closeFont();

        itsWeight=WEIGHT_MEDIUM;
        itsItalic=ITALIC_NONE;
        itsPath=file;
        itsFaceIndex=face;
        itsPsName=QString();

        if(!openFontFt(file) && !itsPsName.isEmpty())
            itsType=NONE;
    }

    return NONE!=itsType;
}

void CFontEngine::closeFont()
{
    closeFaceFt();
    itsPath=QString::null;
    itsFaceIndex=-1;
    itsType=NONE;
}

QString CFontEngine::weightStr(enum EWeight w)
{
    switch(w)
    {
        case WEIGHT_THIN:
            return "Thin";
        case WEIGHT_ULTRA_LIGHT:
            return "UltraLight";
        case WEIGHT_EXTRA_LIGHT:
            return "ExtraLight";
        case WEIGHT_DEMI:
            return "Demi";
        case WEIGHT_LIGHT:
            return "Light";
        case WEIGHT_BOOK:
            return "Book";
        case WEIGHT_MEDIUM:
            return "Medium";
        case WEIGHT_REGULAR:
            return "Regular";
        case WEIGHT_SEMI_BOLD:
            return "SemiBold";
        case WEIGHT_DEMI_BOLD:
            return "DemiBold";
        case WEIGHT_BOLD:
            return "Bold";
        case WEIGHT_EXTRA_BOLD:
            return "ExtraBold";
        case WEIGHT_ULTRA_BOLD:
            return "UltraBold";
        case WEIGHT_HEAVY:
            return "Heavy";
        case WEIGHT_BLACK:
            return "Black";
        case WEIGHT_UNKNOWN:
        default:
            return "Medium";
    }
}

CFontEngine::EWeight CFontEngine::strToWeight(const char *str)
{
    if(NULL==str)
        return WEIGHT_MEDIUM; // WEIGHT_UNKNOWN;
    else if(kasciistricmp(str, "Bold")==0)
        return WEIGHT_BOLD;
    else if(kasciistricmp(str, "Black")==0)
        return WEIGHT_BLACK;
    else if(kasciistricmp(str, "ExtraBold")==0)
        return WEIGHT_EXTRA_BOLD;
    else if(kasciistricmp(str, "UltraBold")==0)
        return WEIGHT_ULTRA_BOLD;
    else if(kasciistricmp(str, "ExtraLight")==0)
        return WEIGHT_EXTRA_LIGHT;
    else if(kasciistricmp(str, "UltraLight")==0)
        return WEIGHT_ULTRA_LIGHT;
    else if(kasciistricmp(str, "Light")==0)
        return WEIGHT_LIGHT;
    else if(kasciistricmp(str, "Medium")==0 || kasciistricmp(str, "Normal")==0 || kasciistricmp(str, "Roman")==0)
        return WEIGHT_MEDIUM;
    else if(kasciistricmp(str, "Regular")==0)
        return WEIGHT_MEDIUM; // WEIGHT_REGULAR;
    else if(kasciistricmp(str, "Demi")==0)
        return WEIGHT_DEMI;
    else if(kasciistricmp(str, "SemiBold")==0)
        return WEIGHT_SEMI_BOLD;
    else if(kasciistricmp(str, "DemiBold")==0)
        return WEIGHT_DEMI_BOLD;
    else if(kasciistricmp(str, "Thin")==0)
        return WEIGHT_THIN;
    else if(kasciistricmp(str, "Book")==0)
        return WEIGHT_BOOK;
    else
        return WEIGHT_MEDIUM; // WEIGHT_UNKNOWN;
}

static void removeSymbols(QString &str)
{
    str.replace(QRegExp("[\\-\\[\\]()]"), " ");

    int   len=str.length();
    QChar space(' ');

    for(int c=0; c<len; ++c)
        if(str[c].unicode()<0x20 || str[c].unicode()>0x7E)
            str[c]=space;

    str=str.simplifyWhiteSpace();
    str=str.stripWhiteSpace();
}

static bool lookupName(FT_Face face, int nid, int pid, int eid, FT_SfntName *nameReturn)
{
    int n = FT_Get_Sfnt_Name_Count(face);

    if(n>0)
    {
        int         i;
        FT_SfntName name;

        for(i=0; i<n; i++)
            if(0==FT_Get_Sfnt_Name(face, i, &name) && name.name_id == nid && name.platform_id == pid &&
               (eid < 0 || name.encoding_id == eid))
            {
                switch(name.platform_id)
                {
                    case TT_PLATFORM_APPLE_UNICODE:
                    case TT_PLATFORM_MACINTOSH:
                        if(name.language_id != TT_MAC_LANGID_ENGLISH)
                            continue;
                        break;
                    case TT_PLATFORM_MICROSOFT:
                        if(name.language_id != TT_MS_LANGID_ENGLISH_UNITED_STATES &&
                           name.language_id != TT_MS_LANGID_ENGLISH_UNITED_KINGDOM)
                            continue;
                        break;
                    default:
                        continue;
                }

                if(name.string_len > 0)
                {
                    *nameReturn = name;
                    return true;
                }
            }
    }

    return false;
}

static QCString getName(FT_Face face, int nid)
{
    FT_SfntName name;
    QCString    str;

    if(lookupName(face, nid, TT_PLATFORM_MICROSOFT, TT_MS_ID_UNICODE_CS, &name) ||
       lookupName(face, nid, TT_PLATFORM_APPLE_UNICODE, -1, &name))
        for(unsigned int i=0; i < name.string_len / 2; i++)
            str+=0 == name.string[2*i] ? name.string[(2*i)+1] : '_';
    else if(lookupName(face, nid, TT_PLATFORM_MACINTOSH, TT_MAC_ID_ROMAN, &name)) // Pretend that Apple Roman is ISO 8859-1
        for(unsigned int i=0; i < name.string_len; i++)
            str+=name.string[i];

    return str;
}

bool CFontEngine::openFontFt(const QString &file)
{
    enum ETtfWeight
    {
        TTF_WEIGHT_UNKNOWN    = 0,
        TTF_WEIGHT_THIN       = 100 +50,
        TTF_WEIGHT_EXTRALIGHT = 200 +50,
        TTF_WEIGHT_LIGHT      = 300 +50,
        TTF_WEIGHT_NORMAL     = 400 +50,
        TTF_WEIGHT_MEDIUM     = 500 +50,
        TTF_WEIGHT_SEMIBOLD   = 600 +50,
        TTF_WEIGHT_BOLD       = 700 +50,
        TTF_WEIGHT_EXTRABOLD  = 800 +50,
        TTF_WEIGHT_BLACK      = 900 +50
    };

    bool status=FT_New_Face(itsFt.library, QFile::encodeName(file), 0, &itsFt.face) ? false : true;

    if(status)
        itsFt.open=true;

    PS_FontInfoRec t1info;

    if(0==FT_Get_PS_Font_Info(itsFt.face, &t1info))
    {
        itsFamily=t1info.family_name;
        itsType=TYPE_1;
    }
    else
    {
        itsFamily=getName(itsFt.face, TT_NAME_ID_FONT_FAMILY);
        itsType=TRUE_TYPE;
    }

    if(itsFamily.isEmpty())
        itsFamily=FT_Get_Postscript_Name(itsFt.face);

    if(itsFamily.isEmpty())
        status=false;   // Hmm... couldn't find any of the names!

    if(status)
    {
        removeSymbols(itsFamily);
        itsPsName=(FT_Get_Postscript_Name(itsFt.face));

        if(TYPE_1==itsType)
        {
            itsWeight=strToWeight(t1info.weight);
            itsItalic=t1info.italic_angle <= -4 || t1info.italic_angle >= 4 ? ITALIC_ITALIC : ITALIC_NONE;
        }
        else // TrueType...
        {
            TT_Postscript *post=NULL;
            TT_OS2        *os2=NULL;
            TT_Header     *head=NULL;
            bool          gotItalic=false;

            if(NULL==(os2=(TT_OS2 *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_os2)) || 0xFFFF==os2->version)
                itsWeight=WEIGHT_UNKNOWN;
            else
            {
                FT_UShort weight=(os2->usWeightClass>0 && os2->usWeightClass<100) ? os2->usWeightClass*100 : os2->usWeightClass;

                if(weight<TTF_WEIGHT_THIN)
                    itsWeight=WEIGHT_THIN;
                else if(weight<TTF_WEIGHT_EXTRALIGHT)
                    itsWeight=WEIGHT_EXTRA_LIGHT;
                else if(weight<TTF_WEIGHT_LIGHT)
                    itsWeight=WEIGHT_LIGHT;
                else if(/*weight<TTF_WEIGHT_NORMAL || */ weight<TTF_WEIGHT_MEDIUM)
                    itsWeight=WEIGHT_MEDIUM;
                else if(weight<TTF_WEIGHT_SEMIBOLD)
                    itsWeight=WEIGHT_SEMI_BOLD;
                else if(weight<TTF_WEIGHT_BOLD)
                    itsWeight=WEIGHT_BOLD;
                else if(weight<TTF_WEIGHT_EXTRABOLD)
                    itsWeight=WEIGHT_EXTRA_BOLD;
                else if(weight<TTF_WEIGHT_BLACK)
                    itsWeight=WEIGHT_BLACK;
                else if(os2->fsSelection&(1 << 5))
                    itsWeight=WEIGHT_BOLD;
                else
                    itsWeight=WEIGHT_UNKNOWN;

                itsItalic=os2->fsSelection&(1 << 0) ? ITALIC_ITALIC : ITALIC_NONE;
                gotItalic=true;
            }

            if(WEIGHT_UNKNOWN==itsWeight)
                itsWeight=NULL!=(head=(TT_Header *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head)) && head->Mac_Style & 1
                             ? WEIGHT_BOLD
                             : WEIGHT_MEDIUM;

            if(!gotItalic && (head!=NULL || NULL!=(head=(TT_Header *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_head))))
            {
                gotItalic=true;
                itsItalic=head->Mac_Style & 2 ? ITALIC_ITALIC: ITALIC_NONE;
            }

            if(!gotItalic && NULL!=(post=(TT_Postscript *)FT_Get_Sfnt_Table(itsFt.face, ft_sfnt_post)))
            {
                struct TFixed
                {
                    TFixed(unsigned long v) : upper(v>>16), lower(v&0xFFFF) {}

                    short upper,
                          lower;

                    float value() { return upper+(lower/65536.0); }
                };

                gotItalic=true;
                itsItalic=0.0f==((TFixed)post->italicAngle).value() ? ITALIC_NONE : ITALIC_ITALIC;
            }
        }
    }

    if(!status)
        closeFaceFt();

    return status;
}

void CFontEngine::closeFaceFt()
{
    if(itsFt.open)
    {
        FT_Done_Face(itsFt.face);
        itsFt.open=false;
    }
}

CFontEngine::TFtData::TFtData()
                    : open(false)
{
    if(FT_Init_FreeType(&library))
    {
        std::cerr << "ERROR: FreeType2 failed to initialise\n";
        exit(0);
    }
}

CFontEngine::TFtData::~TFtData()
{
    FT_Done_FreeType(library);
}

}
