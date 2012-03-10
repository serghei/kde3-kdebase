/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <kdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "printpart.h"

#include <kdeprint/kmmainview.h>
#include <kdeprint/kiconselectaction.h>
#include <kaction.h>
#include <klocale.h>
#include <kinstance.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <kparts/genericfactory.h>
#include <qwidget.h>

typedef KParts::GenericFactory<PrintPart> PrintPartFactory;
K_EXPORT_COMPONENT_FACTORY( libkdeprint_part, PrintPartFactory )

PrintPart::PrintPart(QWidget *parentWidget, const char * /*widgetName*/ ,
	             QObject *parent, const char *name,
		     const QStringList & /*args*/ )
: KParts::ReadOnlyPart(parent, name)
{
	setInstance(PrintPartFactory::instance());
    instance()->iconLoader()->addAppDir("kdeprint");
	m_extension = new PrintPartExtension(this);

	m_view = new KMMainView(parentWidget, "MainView", actionCollection());
	m_view->setFocusPolicy(QWidget::ClickFocus);
	m_view->enableToolbar(false);
	setWidget(m_view);

	initActions();
}

PrintPart::~PrintPart()
{
}

KAboutData *PrintPart::createAboutData()
{
	return new KAboutData(I18N_NOOP("kdeprint_part"), I18N_NOOP("A Konqueror Plugin for Print Management"), "0.1");
}

bool PrintPart::openFile()
{
	return true;
}

void PrintPart::initActions()
{
	setXMLFile("kdeprint_part.rc");
}

PrintPartExtension::PrintPartExtension(PrintPart *parent)
: KParts::BrowserExtension(parent, "PrintPartExtension")
{
}

PrintPartExtension::~PrintPartExtension()
{
}

#include "printpart.moc"
