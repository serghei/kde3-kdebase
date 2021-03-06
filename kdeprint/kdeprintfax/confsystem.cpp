/*
 *   kdeprintfax - a small fax utility
 *   Copyright (C) 2001  Michael Goffioul
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "confsystem.h"
#include "defcmds.h"

#include <qlineedit.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qcombobox.h>

#include <kglobal.h>
#include <kconfig.h>
#include <klocale.h>
#include <kseparator.h>

#include <stdlib.h>

#define EFAX_ID 0
#define HYLAFAX_ID 1
#define MGETTY_ID 2
#define OTHER_ID 3

ConfSystem::ConfSystem(QWidget *parent, const char *name) : QWidget(parent, name)
{
    m_system = new QComboBox(this);
    m_system->insertItem("EFax");
    m_system->insertItem("HylaFax");
    m_system->insertItem("Mgetty-sendfax");
    m_system->insertItem("Other");
    m_command = new QLineEdit(this);
    QLabel *syslabel = new QLabel(i18n("F&ax system:"), this);
    QLabel *cmdlabel = new QLabel(i18n("Co&mmand:"), this);
    syslabel->setBuddy(m_system);
    cmdlabel->setBuddy(m_command);
    connect(m_system, SIGNAL(activated(int)), SLOT(slotSystemChanged(int)));
    m_current = 0;
    m_system->setCurrentItem(m_current);

    QWidget *m_dummy1 = new QWidget(this), *m_dummy2 = new QWidget(this);
    m_server = new QLineEdit(m_dummy1);
    m_device = new QComboBox(m_dummy2);
    QLabel *srvlabel = new QLabel(i18n("Fax &server (if any):"), m_dummy1);
    QLabel *devlabel = new QLabel(i18n("&Fax/Modem device:"), m_dummy2);
    srvlabel->setBuddy(m_server);
    devlabel->setBuddy(m_device);
    m_device->insertItem(i18n("Standard Modem Port"));
    for(int i = 0; i < 10; i++)
        m_device->insertItem(i18n("Serial Port #%1").arg(i));
    m_device->insertItem(i18n("Other"));
    connect(m_device, SIGNAL(activated(int)), SLOT(slotDeviceChanged(int)));
    m_device_edit = new QLineEdit(m_dummy2);
    slotDeviceChanged(0);

    KSeparator *sep = new KSeparator(this);
    sep->setMinimumHeight(10);

    QVBoxLayout *l0 = new QVBoxLayout(this, 10, 10);
    QGridLayout *l1 = new QGridLayout(0, 2, 2, 0, 10);
    l0->addLayout(l1);
    l1->setColStretch(1, 1);
    l1->addWidget(syslabel, 0, 0);
    l1->addWidget(cmdlabel, 1, 0);
    l1->addWidget(m_system, 0, 1);
    l1->addWidget(m_command, 1, 1);
    l0->addWidget(sep);
    l0->addWidget(m_dummy1);
    l0->addWidget(m_dummy2);
    l0->addStretch(1);
    QHBoxLayout *l4 = new QHBoxLayout(m_dummy1, 0, 10);
    l4->addWidget(srvlabel, 0);
    l4->addWidget(m_server, 1);
    QGridLayout *l5 = new QGridLayout(m_dummy2, 2, 2, 0, 10);
    l5->setColStretch(1, 1);
    l5->addWidget(devlabel, 0, 0);
    l5->addWidget(m_device, 0, 1);
    l5->addWidget(m_device_edit, 1, 1);
}

void ConfSystem::load()
{
    KConfig *conf = KGlobal::config();
    conf->setGroup("System");
    m_commands << conf->readPathEntry("EFax", defaultCommand(efax_default_cmd));
    m_commands << conf->readPathEntry("HylaFax", defaultCommand(hylafax_default_cmd));
    m_commands << conf->readPathEntry("Mgetty", defaultCommand(mgetty_default_cmd));
    m_commands << conf->readPathEntry("Other", QString::null);
    QString v = conf->readEntry("System", "efax");
    if(v == "mgetty")
        m_current = MGETTY_ID;
    else if(v == "hylafax")
        m_current = HYLAFAX_ID;
    else if(v == "other")
        m_current = OTHER_ID;
    else
        m_current = EFAX_ID;
    conf->setGroup("Fax");
    m_server->setText(conf->readEntry("Server", getenv("FAXSERVER")));
    v = conf->readEntry("Device", "modem");
    if(v.startsWith("ttyS"))
        m_device->setCurrentItem(v.right(v.length() - 4).toInt() + 1);
    else if(v == "modem")
        m_device->setCurrentItem(0);
    else
    {
        m_device->setCurrentItem(m_device->count() - 1);
        m_device_edit->setText("/dev/" + v);
        slotDeviceChanged(m_device->count() - 1);
    }

    m_system->setCurrentItem(m_current);
    m_command->setText(m_commands[m_current]);
    slotSystemChanged(m_current);
}

void ConfSystem::save()
{
    m_commands[m_current] = m_command->text();
    KConfig *conf = KGlobal::config();
    conf->setGroup("System");
    if(m_commands[EFAX_ID] != defaultCommand(efax_default_cmd))
        conf->writePathEntry("EFax", m_commands[EFAX_ID]);
    else
        conf->deleteEntry("EFax");
    if(m_commands[HYLAFAX_ID] != defaultCommand(hylafax_default_cmd))
        conf->writePathEntry("HylaFax", m_commands[HYLAFAX_ID]);
    else
        conf->deleteEntry("HylaFax");
    if(m_commands[MGETTY_ID] != defaultCommand(mgetty_default_cmd))
        conf->writePathEntry("Mgetty", m_commands[MGETTY_ID]);
    else
        conf->deleteEntry("Mgetty");
    if(!m_commands[OTHER_ID].isEmpty())
        conf->writeEntry("Other", m_commands[OTHER_ID]);
    else
        conf->deleteEntry("Other");
    int ID = m_system->currentItem();
    switch(ID)
    {
        case EFAX_ID:
            conf->writeEntry("System", "efax");
            break;
        case HYLAFAX_ID:
            conf->writeEntry("System", "hylafax");
            break;
        case MGETTY_ID:
            conf->writeEntry("System", "mgetty");
            break;
        case OTHER_ID:
            conf->writeEntry("System", "other");
            break;
    }
    conf->setGroup("Fax");
    if(m_device->currentItem() != (m_device->count() - 1))
        conf->writeEntry("Device", m_device->currentItem() == 0 ? QString("modem") : QString("ttyS%1").arg(m_device->currentItem() - 1));
    else
    {
        QString dev = m_device_edit->text();
        // strip leading "/dev/"
        if(dev.startsWith("/dev/"))
            dev = dev.mid(5);
        conf->writeEntry("Device", dev);
    }
    if(m_server->text().isEmpty())
        conf->deleteEntry("Server");
    else
        conf->writeEntry("Server", m_server->text());
}

void ConfSystem::slotSystemChanged(int ID)
{
    m_commands[m_current] = m_command->text();
    m_current = ID;
    if(ID == EFAX_ID)
        m_device->parentWidget()->show();
    else
        m_device->parentWidget()->hide();
    if(ID == HYLAFAX_ID)
        m_server->parentWidget()->show();
    else
        m_server->parentWidget()->hide();
    m_command->setText(m_commands[m_current]);
}

void ConfSystem::slotDeviceChanged(int ID)
{
    if(ID != (m_device->count() - 1))
        m_device_edit->hide();
    else
        m_device_edit->show();
}

#include "confsystem.moc"
