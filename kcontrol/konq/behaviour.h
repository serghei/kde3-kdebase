/**
 *  Copyright (c) 2001 David Faure <david@mandrakesoft.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef __BEHAVIOUR_H__
#define __BEHAVIOUR_H__

#include <kcmodule.h>

class QCheckBox;
class QLabel;
class QRadioButton;
class QSpinBox;
class QVButtonGroup;

class KConfig;
class KURLRequester;

//-----------------------------------------------------------------------------


class KBehaviourOptions : public KCModule {
    Q_OBJECT
public:
    KBehaviourOptions(KConfig *config, QString group, QWidget *parent = 0, const char *name = 0);
    ~KBehaviourOptions();
    virtual void load();
    virtual void load(bool useDefaults);
    virtual void save();
    virtual void defaults();

protected slots:

    void updateWinPixmap(bool);
    void slotShowTips(bool);

private:
    KConfig *g_pConfig;
    QString groupname;

    QCheckBox *cbNewWin;
    QCheckBox *cbListProgress;

    QLabel *winPixmap;

    KURLRequester *homeURL;

    QVButtonGroup *bgOneProcess;
    // QLabel *fileTips;
    // QSpinBox  *sbToolTip;
    QCheckBox *cbShowTips;
    QCheckBox *cbShowPreviewsInTips;
    QCheckBox *cbRenameDirectlyIcon;

    QCheckBox *cbMoveToTrash;
    QCheckBox *cbDelete;
    QCheckBox *cbShowDeleteCommand;
};

#endif // __BEHAVIOUR_H__
