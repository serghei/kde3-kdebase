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

#ifndef _AUTHENTICATION_DIALOG_H_
#define _AUTHENTICATION_DIALOG_H_

#include <kdialogbase.h>

class QComboBox;
class QLineEdit;


class AuthenticationDialog : public KDialogBase {
public:
    AuthenticationDialog(const QString &actionId, const QString &iconName, const QString &message, const QMap< QString, QString > &details,
                         const QStringList &listUsers);

    ~AuthenticationDialog();

    int exec();

    QString username() const;
    QString password() const;
    void setPasswordLabel(const QString &passwordLabel);

protected slots:
    virtual void done(int r);

private:
    QComboBox *users;
    QLabel *user;
    QLabel *labelPass;
    QLineEdit *pass;
};


#endif
