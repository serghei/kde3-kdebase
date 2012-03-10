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

#ifndef KDEPRINTFAX_H
#define KDEPRINTFAX_H

#include <kmainwindow.h>
#include <kurl.h>

class KListBox;
class KListView;
class QLineEdit;
class QTextEdit;
class FaxCtrl;
class QCheckBox;
class QLabel;
class QTimeEdit;
class QComboBox;
class QPushButton;
class QListViewItem;

class KdeprintFax : public KMainWindow
{
	Q_OBJECT
public:
	struct FaxItem
	{
		QString number;
		QString name;
		QString enterprise;
	};
	typedef QValueList<FaxItem> FaxItemList;
	typedef QValueList<FaxItem>::ConstIterator FaxItemListIterator;

	KdeprintFax(QWidget *parent = 0, const char *name = 0);
	~KdeprintFax();

	void addURL(KURL url);
	void setPhone(QString phone);
	void sendFax( bool quitAfterSend );
	QStringList files();
	int faxCount() const;
	//QString number( int i = 0 ) const;
	//QString name( int i = 0 ) const;
	//QString enterprise( int i = 0 ) const;
	FaxItemList faxList() const;
	QString comment() const;
	QString time() const;
	QString subject() const;
	bool cover() const;

protected slots:
	void slotToggleMenuBar();
	void slotKab();
	void slotAdd();
	void slotRemove();
	void slotFax();
	void slotAbort();
	void slotMessage(const QString&);
	void slotFaxSent(bool);
	void slotViewLog();
	void slotConfigure();
	void slotQuit();
	void slotView();
	void slotTimeComboActivated(int);
	void slotMoveUp();
	void slotMoveDown();
	void slotCurrentChanged();
	void slotFaxSelectionChanged();
	void slotFaxRemove();
	void slotFaxAdd();
	void slotFaxExecuted( QListViewItem* );
	void slotCoverToggled(bool toggle);

protected:
	void initActions();
	void dragEnterEvent(QDragEnterEvent*);
	void dropEvent(QDropEvent*);
	void updateState();
	bool manualFaxDialog( QString& number, QString& name, QString& enterprise );
	//QListViewItem* faxItem( int i = 0 ) const;

private:
	KListBox	*m_files;
	KListView   *m_numbers;
	QLineEdit	*m_subject;
	QTextEdit	*m_comment;
	FaxCtrl		*m_faxctrl;
	QCheckBox	*m_cover;
	QLabel		*m_msglabel;
	QTimeEdit	*m_time;
	QComboBox	*m_timecombo;
	QPushButton *m_upbtn, *m_downbtn;
	QPushButton *m_newbtn, *m_abbtn, *m_delbtn;
	bool m_quitAfterSend;
};

#endif
