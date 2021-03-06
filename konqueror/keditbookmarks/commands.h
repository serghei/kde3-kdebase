// kate: space-indent on; indent-width 3; replace-tabs on;
/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2002-2003 Alexander Kellett <lypanov@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License version 2 as published by the Free Software Foundation.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __commands_h
#define __commands_h

#include <kcommand.h>
#include <kbookmark.h>
#include <qvaluevector.h>

// Interface adds the affectedBookmarks method
// Any class should on call add those bookmarks which are
// affected by executing or unexecuting the command
// Or a common parent of the affected bookmarks
// see KBookmarkManager::notifyChange(KBookmarkGroup)
class IKEBCommand {
public:
    IKEBCommand(){};
    virtual ~IKEBCommand(){};
    virtual QString affectedBookmarks() const = 0;
    virtual QString currentAddress() const
    {
        return QString::null;
    }
};

class KEBMacroCommand : public KMacroCommand, public IKEBCommand {
public:
    KEBMacroCommand(const QString &name) : KMacroCommand(name){};
    virtual ~KEBMacroCommand(){};
    virtual QString affectedBookmarks() const;
};

class DeleteManyCommand : public KEBMacroCommand {
public:
    DeleteManyCommand(const QString &name, const QValueList< QString > &addresses);
    virtual ~DeleteManyCommand(){};
    virtual QString currentAddress() const;

private:
    QString prevOrParentAddress(QString addr);
    QString preOrderNextAddress(QString addr);
    bool isConsecutive(const QValueList< QString > &addresses);
    QString m_currentAddress;
};

class CreateCommand : public KCommand, public IKEBCommand {
public:
    // separator
    CreateCommand(const QString &address) : KCommand(), m_to(address), m_group(false), m_separator(true), m_originalBookmark(QDomElement())
    {
        ;
    }

    // bookmark
    CreateCommand(const QString &address, const QString &text, const QString &iconPath, const KURL &url)
        : KCommand()
        , m_to(address)
        , m_text(text)
        , m_iconPath(iconPath)
        , m_url(url)
        , m_group(false)
        , m_separator(false)
        , m_originalBookmark(QDomElement())
    {
        ;
    }

    // folder
    CreateCommand(const QString &address, const QString &text, const QString &iconPath, bool open)
        : KCommand()
        , m_to(address)
        , m_text(text)
        , m_iconPath(iconPath)
        , m_group(true)
        , m_separator(false)
        , m_open(open)
        , m_originalBookmark(QDomElement())
    {
        ;
    }

    // clone existing bookmark
    CreateCommand(const QString &address, const KBookmark &original, const QString &name = QString::null)
        : KCommand(), m_to(address), m_group(false), m_separator(false), m_open(false), m_originalBookmark(original), m_mytext(name)
    {
        ;
    }

    QString finalAddress() const;

    virtual ~CreateCommand()
    {
        ;
    }
    virtual void execute();
    virtual void unexecute();
    virtual QString name() const;
    virtual QString affectedBookmarks() const;
    virtual QString currentAddress() const;

private:
    QString m_to;
    QString m_text;
    QString m_iconPath;
    KURL m_url;
    bool m_group : 1;
    bool m_separator : 1;
    bool m_open : 1;
    KBookmark m_originalBookmark;
    QString m_mytext;
};

class EditCommand : public KCommand, public IKEBCommand {
public:
    struct Edition
    {
        Edition()
        {
            ;
        } // needed for QValueList
        Edition(const QString &a, const QString &v) : attr(a), value(v)
        {
        }
        QString attr;
        QString value;
    };

    // change one attribute
    EditCommand(const QString &address, Edition edition, const QString &name = QString::null) : KCommand(), m_address(address), m_mytext(name)
    {
        m_editions.append(edition);
    }

    // change multiple attributes
    EditCommand(const QString &address, const QValueList< Edition > &editions, const QString &name = QString::null)
        : KCommand(), m_address(address), m_editions(editions), m_mytext(name)
    {
        ;
    }

    void modify(const QString &a, const QString &v);

    virtual ~EditCommand()
    {
        ;
    }
    virtual void execute();
    virtual void unexecute();
    virtual QString name() const;
    virtual QString affectedBookmarks() const;

private:
    QString m_address;
    QValueList< Edition > m_editions;
    QValueList< Edition > m_reverseEditions;
    QString m_mytext;
};

class NodeEditCommand : public KCommand, public IKEBCommand {
public:
    NodeEditCommand(const QString &address, const QString &newText, const QString &nodeName)
        : KCommand(), m_address(address), m_newText(newText), m_nodename(nodeName)
    {
        ;
    }

    void modify(const QString &newText);

    virtual ~NodeEditCommand()
    {
        ;
    }
    virtual void execute();
    virtual void unexecute();
    virtual QString affectedBookmarks() const;
    virtual QString name() const;
    static QString getNodeText(KBookmark bk, const QStringList &nodehier);
    static QString setNodeText(KBookmark bk, const QStringList &nodehier, QString newValue);

private:
    QString m_address;
    QString m_newText;
    QString m_oldText;
    QString m_nodename;
};

class DeleteCommand : public KCommand, public IKEBCommand {
public:
    DeleteCommand(const QString &from, bool contentOnly = false) : KCommand(), m_from(from), m_cmd(0L), m_subCmd(0L), m_contentOnly(contentOnly)
    {
        ;
    }
    virtual ~DeleteCommand()
    {
        delete m_cmd;
        delete m_subCmd;
    }
    virtual void execute();
    virtual void unexecute();
    virtual QString name() const
    {
        // NOTE - DeleteCommand needs no name, its always embedded in a macrocommand
        return "";
    };
    virtual QString affectedBookmarks() const;
    static KEBMacroCommand *deleteAll(const KBookmarkGroup &parentGroup);

private:
    QString m_from;
    KCommand *m_cmd;
    KEBMacroCommand *m_subCmd;
    bool m_contentOnly;
};

class MoveCommand : public KCommand, public IKEBCommand {
public:
    // "Create it with itemsAlreadyMoved=true since
    // "KListView moves the item before telling us about it."
    MoveCommand(const QString &from, const QString &to, const QString &name = QString::null) : KCommand(), m_from(from), m_to(to), m_mytext(name)
    {
        ;
    }
    QString finalAddress() const;
    virtual ~MoveCommand()
    {
        ;
    }
    virtual void execute();
    virtual void unexecute();
    virtual QString name() const;
    virtual QString affectedBookmarks() const;

private:
    QString m_from;
    QString m_to;
    QString m_mytext;
};

class SortItem;

class SortCommand : public KEBMacroCommand {
public:
    SortCommand(const QString &name, const QString &groupAddress) : KEBMacroCommand(name), m_groupAddress(groupAddress)
    {
        ;
    }
    virtual ~SortCommand()
    {
        ;
    }
    virtual void execute();
    virtual void unexecute();
    virtual QString affectedBookmarks() const;
    // internal
    void moveAfter(const SortItem &moveMe, const SortItem &afterMe);

private:
    QString m_groupAddress;
};

class KEBListViewItem;

class CmdGen {
public:
    static KEBMacroCommand *setAsToolbar(const KBookmark &bk);
    static KEBMacroCommand *setShownInToolbar(const QValueList< KBookmark > &bk, bool show);
    static bool shownInToolbar(const KBookmark &bk);
    static KEBMacroCommand *deleteItems(const QString &commandName, const QValueVector< KEBListViewItem * > &items);
    static KEBMacroCommand *insertMimeSource(const QString &cmdName, QMimeSource *data, const QString &addr);
    static KEBMacroCommand *itemsMoved(const QValueVector< KEBListViewItem * > &items, const QString &newAddress, bool copy);

private:
    CmdGen()
    {
        ;
    }
};

#endif
