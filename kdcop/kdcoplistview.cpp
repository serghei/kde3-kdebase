/*
 * Copyright (C) 2000 by Ian Reinhart Geiser <geiseri@kde.org>
 *
 * Licensed under the Artistic License.
 */

#include "kdcoplistview.h"
#include "kdcoplistview.moc"
#include <kdebug.h>
#include <qdragobject.h>
#include <qstringlist.h>
#include <qregexp.h>


KDCOPListView::KDCOPListView(QWidget *parent, const char *name) : KListView(parent, name)
{
    kdDebug() << "Building new list." << endl;
    setDragEnabled(true);
}


KDCOPListView::~KDCOPListView()
{
}

QDragObject *KDCOPListView::dragObject()
{
    kdDebug() << "Drag object called... " << endl;
    if(!currentItem())
        return 0;
    else
        return new QTextDrag(encode(this->selectedItem()), this);
}

void KDCOPListView::setMode(const QString &theMode)
{
    mode = theMode;
}

QString KDCOPListView::encode(QListViewItem *theCode)
{
    DCOPBrowserItem *item = static_cast< DCOPBrowserItem * >(theCode);

    if(item->type() != DCOPBrowserItem::Function)
        return "";

    DCOPBrowserFunctionItem *fitem = static_cast< DCOPBrowserFunctionItem * >(item);

    QString function = QString::fromUtf8(fitem->function());
    QString application = QString::fromUtf8(fitem->app());
    QString object = QString::fromUtf8(fitem->object());

    kdDebug() << function << endl;
    QString returnType = function.section(' ', 0, 0);
    QString returnCode = "";
    QString normalisedSignature;
    QStringList types;
    QStringList names;

    QString unNormalisedSignature(function);

    int s = unNormalisedSignature.find(' ');

    if(s < 0)
        s = 0;
    else
        s++;

    unNormalisedSignature = unNormalisedSignature.mid(s);

    int left = unNormalisedSignature.find('(');
    int right = unNormalisedSignature.findRev(')');

    if(-1 == left)
    {
        // Fucked up function signature.
        return "";
    }
    if(left > 0 && left + 1 < right - 1)
    {
        types = QStringList::split(',', unNormalisedSignature.mid(left + 1, right - left - 1));
        for(QStringList::Iterator it = types.begin(); it != types.end(); ++it)
        {
            (*it) = (*it).stripWhiteSpace();
            int s = (*it).find(' ');
            if(-1 != s)
            {
                names.append((*it).mid(s + 1));
                (*it) = (*it).left(s);
            }
        }
    }

    if(mode == "C++")
    {
        QString args;
        for(unsigned int i = 0; i < names.count(); i++)
        {
            args += types[i] + " " + names[i] + ";\n";
        }
        QString dcopRef = "DCOPRef m_" + application + object + "(\"" + application + "\",\"" + object + "\");\n";

        QString stringNames = names.join(",");
        QString stringTypes = types.join(",");
        if(returnType != "void")
            returnType += " return" + returnType + " =";
        else
            returnType = "";
        returnCode =
            args + dcopRef + returnType + "m_" + application + object + ".call(\"" + unNormalisedSignature.left(left) + "(" + stringTypes + ")\"";
        if(!stringNames.isEmpty())
            returnCode += ", ";
        returnCode += stringNames + ");\n";
    }
    else if(mode == "Shell")
    {
        returnCode = "dcop " + application + " " + object + " " + unNormalisedSignature.left(left) + " " + names.join(" ");
    }
    else if(mode == "Python")
    {
        QString setup;
        setup = "m_" + application + object + " = dcop.DCOPObject( \"" + application + "\",\"" + object + "\")\n";

        for(unsigned int i = 0; i < names.count(); i++)
        {
            setup += names[i] + " #set value here.\n";
        }
        returnCode =
            setup + "reply" + returnType + " = m_" + application + object + "." + unNormalisedSignature.left(left) + "(" + names.join(",") + ")\n";
    }
    return returnCode;
}

QString KDCOPListView::getCurrentCode() const
{
    // fixing warning
    return QString::null;
}
