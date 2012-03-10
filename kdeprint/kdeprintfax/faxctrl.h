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

#ifndef FAXCTRL_H
#define FAXCTRL_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>

#include "kdeprintfax.h"

class KProcess;
class QTextEdit;

class FaxCtrl : public QObject
{
	Q_OBJECT
public:
	FaxCtrl(QWidget *parent = 0, const char *name = 0);
	~FaxCtrl();

	bool send(KdeprintFax *f);
	bool abort();
	void viewLog(QWidget *parent = 0);
	QString faxSystem();
	QString faxCommand();

signals:
	void message(const QString&);
	void faxSent(bool);

protected slots:
	void slotReceivedStdout(KProcess*, char*, int);
	void slotProcessExited(KProcess*);
	void cleanTempFiles();
	void slotClearLog();
	void slotCloseLog();
	void slotPrintLog();
	void slotSaveLog();

protected:
	void filter();
	void sendFax();
	void addLog(const QString&, bool = false);
	void addLogTitle( const QString& );

private:
	KProcess	*m_process;
	QString		m_log, m_command;
	QStringList	m_files, m_filteredfiles, m_tempfiles;
	QTextEdit	*m_logview;
	KdeprintFax::FaxItemList m_faxlist;
};

#endif
