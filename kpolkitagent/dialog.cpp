/*
 * Copyright (C) 2012 Serghei Amelian <serghei.amelian@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 * Based on polkit-gnome, developed by David Zeuthen <davidz@redhat.com>
 */
#include <qapplication.h>
#include <qeventloop.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qtextbrowser.h>

#include <klocale.h>
#include <kactivelabel.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "dialog.h"


AuthenticationDialog::AuthenticationDialog(const QString &actionId, const QString &iconName, const QString &message,
                                           const QMap< QString, QString > &details, const QStringList &listUsers)
    : KDialogBase(Plain, i18n("Authentication"), Ok | Cancel | Details, Ok, 0, "KDE3-PolkitAgent-AuthenticationDialog", false, true)
    , users(0)
    , user(0)
{
    setIcon(SmallIcon("password"));

    setButtonOK(KGuiItem("&Authenticate"));

    QFont boldFont;
    boldFont.setBold(true);

    QWidget *page = plainPage();
    QHBoxLayout *mainLayout = new QHBoxLayout(page, 20, 30);

    QWidget *iconBox = new QWidget(page);
    mainLayout->addWidget(iconBox, 0, AlignTop);
    QVBoxLayout *iconBoxLayout = new QVBoxLayout(iconBox, 0, spacingHint());

    QPixmap icon = KGlobal::iconLoader()->loadIcon("password", KIcon::NoGroup, KIcon::SizeLarge);
    QLabel *iconPassword = new QLabel(iconBox);
    iconBoxLayout->addWidget(iconPassword);
    iconPassword->setPixmap(icon);

    if(!iconName.isEmpty())
    {
        icon = KGlobal::iconLoader()->loadIcon(iconName, KIcon::NoGroup, KIcon::SizeLarge);
        QLabel *iconLabel = new QLabel(iconBox);
        iconBoxLayout->addWidget(iconLabel);
        iconLabel->setPixmap(icon);
    }

    QWidget *content = new QWidget(page);
    mainLayout->addWidget(content);
    QVBoxLayout *contentLayout = new QVBoxLayout(content, 0, 20);
    content->setMinimumWidth(350);

    QLabel *labelMessage = new QLabel("<qt>" + message + "</qt>", content);
    labelMessage->setFont(boldFont);
    contentLayout->addWidget(labelMessage, 0, AlignTop);

    QLabel *labelExplain = new QLabel(i18n("<qt>An application is attempting to perform an action that requires privileges.<br>"
                                           "Authentication as one of the users below is required to perform this action.</qt>"),
                                      content);
    contentLayout->addWidget(labelExplain, 0);

    QWidget *inputBox = new QWidget(content);
    contentLayout->addWidget(inputBox, 0, AlignTop);
    QGridLayout *inputBoxLayout = new QGridLayout(inputBox, 2, 2, 0, spacingHint());

    QLabel *labelUser = new QLabel(i18n("Username:"), inputBox);
    inputBoxLayout->addWidget(labelUser, 0, 0);

    labelPass = new QLabel(i18n("Password:"), inputBox);
    inputBoxLayout->addWidget(labelPass, 1, 0);

    if(listUsers.count() > 1)
    {
        users = new QComboBox(inputBox);
        inputBoxLayout->addWidget(users, 0, 1);
        users->insertStringList(listUsers);
    }
    else
    {
        user = new QLabel(listUsers.front(), inputBox);
        inputBoxLayout->addWidget(user, 0, 1);
        user->setFont(boldFont);
    }

    pass = new QLineEdit(inputBox);
    inputBoxLayout->addWidget(pass, 1, 1);
    pass->setEchoMode(QLineEdit::Password);
    pass->setFocus();

    QFrame *detailsBox = new QFrame(this);
    QGridLayout *detailsBoxLayout = new QGridLayout(detailsBox, details.count(), 2, spacingHint());
    detailsBoxLayout->setColStretch(0, 0);
    detailsBoxLayout->setColStretch(1, 1);

    int i = 0;
    for(QMap< QString, QString >::ConstIterator it = details.begin(); it != details.end(); ++it, i++)
    {
        QLabel *label = new QLabel(it.key() + ":", detailsBox);
        detailsBoxLayout->addWidget(label, i, 0, AlignRight);

        // FIXME For unknown reason, KActiveLabel change the position of dialog
        // KActiveLabel *value = new KActiveLabel("<qt>" + it.data() + "</qt>", detailsBox);
        // This is a workaround for above problem
        QLabel *value = new QLabel("<qt>" + it.data() + "</qt>", detailsBox);

        value->setFont(boldFont);
        detailsBoxLayout->addWidget(value, i, 1);
    }

    setDetailsWidget(detailsBox);
}


AuthenticationDialog::~AuthenticationDialog()
{
}


int AuthenticationDialog::exec()
{
    show();
    qApp->eventLoop()->enterLoop();

    return result();
}


QString AuthenticationDialog::username() const
{
    return (users ? users->currentText() : user->text());
}


QString AuthenticationDialog::password() const
{
    return pass->text();
}


void AuthenticationDialog::setPasswordLabel(const QString &passwordLabel)
{
    labelPass->setText(passwordLabel);
}


void AuthenticationDialog::done(int r)
{
    KDialogBase::done(r);
    qApp->eventLoop()->exitLoop();
}
