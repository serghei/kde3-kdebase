/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2001, 2004 Anders Lund <anders.lund@lund.tdcadsl.dk>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kategrepdialog.h"
#include "katemainwindow.h"

#include <qobject.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qevent.h>
#include <qlistbox.h>
#include <qregexp.h>
#include <qwhatsthis.h>
#include <qcursor.h>

#include <kapplication.h>
#include <kaccelmanager.h>
#include <kbuttonbox.h>
#include <kfiledialog.h>
#include <kprocess.h>
#include <kapplication.h>
#include <klocale.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kurlrequester.h>
#include <kurlcompletion.h>
#include <kcombobox.h>
#include <klineedit.h>

const char *template_desc[] = {
  "normal",
  "assignment",
  "->MEMBER(",
  "class::MEMBER(",
  "OBJECT->member(",
  0
};

const char *strTemplate[] = {
  "%s",
  "\\<%s\\>[\t ]*=[^=]",
  "\\->[\\t ]*\\<%s\\>[\\t ]*(",
  "[a-z0-9_$]\\+[\\t ]*::[\\t ]*\\<%s\\>[\\t ]*(",
  "\\<%s\\>[\\t ]*\\->[\\t ]*[a-z0-9_$]\\+[\\t ]*(",
  0
};


GrepTool::GrepTool(QWidget *parent, const char *name)
  : QWidget(parent, name/*, false*/), m_fixFocus(true), childproc(0)
{
  setCaption(i18n("Find in Files"));
  config = KGlobal::config();
  config->setGroup("GrepTool");
  lastSearchItems = config->readListEntry("LastSearchItems");
  lastSearchPaths = config->readListEntry("LastSearchPaths");
  lastSearchFiles = config->readListEntry("LastSearchFiles");

  if( lastSearchFiles.isEmpty() )
  {
    // if there are no entries, most probably the first Kate start.
    // Initialize with default values.
    lastSearchFiles << "*.h,*.hxx,*.cpp,*.cc,*.C,*.cxx,*.idl,*.c"
                    << "*.cpp,*.cc,*.C,*.cxx,*.c"
                    << "*.h,*.hxx,*.idl"
                    << "*";
  }

  QGridLayout *layout = new QGridLayout(this, 6, 3, 4, 4);
  layout->setColStretch(0, 10);
  layout->addColSpacing(1, 10);
  layout->setColStretch(1, 0);
  layout->setColStretch(2, 1);
  layout->setRowStretch(1, 0);
  layout->setRowStretch(2, 10);
  layout->setRowStretch(4, 0);

  QGridLayout *loInput = new QGridLayout(4, 2, 4);
  layout->addLayout(loInput, 0, 0);
  loInput->setColStretch(0, 0);
  loInput->setColStretch(1, 20);

  QLabel *lPattern = new QLabel(i18n("Pattern:"), this);
  lPattern->setFixedSize(lPattern->sizeHint());
  loInput->addWidget(lPattern, 0, 0, AlignRight | AlignVCenter);

  QBoxLayout *loPattern = new QHBoxLayout( 4 );
  loInput->addLayout( loPattern, 0, 1 );
  cmbPattern = new KComboBox(true, this);
  cmbPattern->setDuplicatesEnabled(false);
  cmbPattern->insertStringList(lastSearchItems);
  cmbPattern->setEditText(QString::null);
  cmbPattern->setInsertionPolicy(QComboBox::NoInsertion);
  lPattern->setBuddy(cmbPattern);
  cmbPattern->setFocus();
  cmbPattern->setMinimumSize(cmbPattern->sizeHint());
  loPattern->addWidget( cmbPattern );

  cbCasesensitive = new QCheckBox(i18n("Case sensitive"), this);
  cbCasesensitive->setMinimumWidth(cbCasesensitive->sizeHint().width());
  cbCasesensitive->setChecked(config->readBoolEntry("CaseSensitive", true));
  loPattern->addWidget(cbCasesensitive);

  cbRegex = new QCheckBox( i18n("Regular expression"), this );
  cbRegex->setMinimumWidth( cbRegex->sizeHint().width() );
  cbRegex->setChecked( config->readBoolEntry( "Regex", true ) );
  loPattern->addWidget( cbRegex );
  loPattern->setStretchFactor( cmbPattern, 100 );

  QLabel *lTemplate = new QLabel(i18n("Template:"), this);
  lTemplate->setFixedSize(lTemplate->sizeHint());
  loInput->addWidget(lTemplate, 1, 0, AlignRight | AlignVCenter);

  QBoxLayout *loTemplate = new QHBoxLayout(4);
  loInput->addLayout(loTemplate, 1, 1);

  leTemplate = new KLineEdit(this);
  lTemplate->setBuddy(leTemplate);
  leTemplate->setText(strTemplate[0]);
  leTemplate->setMinimumSize(leTemplate->sizeHint());
  loTemplate->addWidget(leTemplate);

  KComboBox *cmbTemplate = new KComboBox(false, this);
  cmbTemplate->insertStrList(template_desc);
  cmbTemplate->adjustSize();
  cmbTemplate->setFixedSize(cmbTemplate->size());
  loTemplate->addWidget(cmbTemplate);

  QLabel *lFiles = new QLabel(i18n("Files:"), this);
  lFiles->setFixedSize(lFiles->sizeHint());
  loInput->addWidget(lFiles, 2, 0, AlignRight | AlignVCenter);

  cmbFiles = new KComboBox(true, this);
  lFiles->setBuddy(cmbFiles->focusProxy());
  cmbFiles->setMinimumSize(cmbFiles->sizeHint());
  cmbFiles->setInsertionPolicy(QComboBox::NoInsertion);
  cmbFiles->setDuplicatesEnabled(false);
  cmbFiles->insertStringList(lastSearchFiles);
  loInput->addWidget(cmbFiles, 2, 1);

  QLabel *lDir = new QLabel(i18n("Folder:"), this);
  lDir->setFixedSize(lDir->sizeHint());
  loInput->addWidget(lDir, 3, 0, AlignRight | AlignVCenter);

  QBoxLayout *loDir = new QHBoxLayout(3);
  loInput->addLayout(loDir, 3, 1);

  KComboBox* cmbUrl = new KComboBox(true, this);
  cmbUrl->setMinimumWidth(80); // make sure that 800x600 res works
  cmbUrl->setDuplicatesEnabled(false);
  cmbUrl->setInsertionPolicy(QComboBox::NoInsertion);
  cmbDir = new KURLRequester( cmbUrl, this, "dir combo" );
  cmbDir->completionObject()->setMode(KURLCompletion::DirCompletion);
  cmbDir->comboBox()->insertStringList(lastSearchPaths);
  cmbDir->setMode( KFile::Directory|KFile::LocalOnly );
  loDir->addWidget(cmbDir, 1);
  lDir->setBuddy(cmbDir);

  cbRecursive = new QCheckBox(i18n("Recursive"), this);
  cbRecursive->setMinimumWidth(cbRecursive->sizeHint().width());
  cbRecursive->setChecked(config->readBoolEntry("Recursive", true));
  loDir->addWidget(cbRecursive);

  KButtonBox *actionbox = new KButtonBox(this, Qt::Vertical);
  layout->addWidget(actionbox, 0, 2);
  actionbox->addStretch();
  btnSearch = static_cast<KPushButton*>(actionbox->addButton(KGuiItem(i18n("Find"),"find")));
  btnSearch->setDefault(true);
  btnClear = static_cast<KPushButton*>(actionbox->addButton( KStdGuiItem::clear() ));
  actionbox->addStretch();
  actionbox->layout();

  lbResult = new QListBox(this);
  QFontMetrics rb_fm(lbResult->fontMetrics());
  layout->addMultiCellWidget(lbResult, 2, 2, 0, 2);

  layout->activate();

  KAcceleratorManager::manage( this );

  QWhatsThis::add(lPattern,
    i18n("<p>Enter the expression you want to search for here."
     "<p>If 'regular expression' is unchecked, any non-space letters in your "
     "expression will be escaped with a backslash character."
     "<p>Possible meta characters are:<br>"
     "<b>.</b> - Matches any character<br>"
     "<b>^</b> - Matches the beginning of a line<br>"
     "<b>$</b> - Matches the end of a line<br>"
     "<b>\\&lt;</b> - Matches the beginning of a word<br>"
     "<b>\\&gt;</b> - Matches the end of a word"
     "<p>The following repetition operators exist:<br>"
     "<b>?</b> - The preceding item is matched at most once<br>"
     "<b>*</b> - The preceding item is matched zero or more times<br>"
     "<b>+</b> - The preceding item is matched one or more times<br>"
     "<b>{<i>n</i>}</b> - The preceding item is matched exactly <i>n</i> times<br>"
     "<b>{<i>n</i>,}</b> - The preceding item is matched <i>n</i> or more times<br>"
     "<b>{,<i>n</i>}</b> - The preceding item is matched at most <i>n</i> times<br>"
     "<b>{<i>n</i>,<i>m</i>}</b> - The preceding item is matched at least <i>n</i>, "
     "but at most <i>m</i> times."
     "<p>Furthermore, backreferences to bracketed subexpressions are available "
     "via the notation <code>\\#</code>."
     "<p>See the grep(1) documentation for the full documentation."
     ));
  QWhatsThis::add(lFiles,
    i18n("Enter the file name pattern of the files to search here.\n"
     "You may give several patterns separated by commas."));
  QWhatsThis::add(lTemplate,
    i18n("You can choose a template for the pattern from the combo box\n"
     "and edit it here. The string %s in the template is replaced\n"
     "by the pattern input field, resulting in the regular expression\n"
     "to search for."));
  QWhatsThis::add(lDir,
    i18n("Enter the folder which contains the files in which you want to search."));
  QWhatsThis::add(cbRecursive,
    i18n("Check this box to search in all subfolders."));
  QWhatsThis::add(cbCasesensitive,
    i18n("If this option is enabled (the default), the search will be case sensitive."));
  QWhatsThis::add( cbRegex, i18n(
      "<p>If this is enabled, your pattern will be passed unmodified to "
      "<em>grep(1)</em>. Otherwise, all characters that are not letters will be "
      "escaped using a backslash character to prevent grep from interpreting "
      "them as part of the expression.") );
  QWhatsThis::add(lbResult,
    i18n("The results of the grep run are listed here. Select a\n"
     "filename/line number combination and press Enter or doubleclick\n"
     "on the item to show the respective line in the editor."));

  // event filter, do something relevant for RETURN
  cmbPattern->installEventFilter( this );
  leTemplate->installEventFilter( this );
  cmbFiles->installEventFilter( this );
  cmbDir->comboBox()->installEventFilter( this );

  connect( cmbTemplate, SIGNAL(activated(int)),
           SLOT(templateActivated(int)) );
  connect( lbResult, SIGNAL(selected(const QString&)),
           SLOT(itemSelected(const QString&)) );
  connect( btnSearch, SIGNAL(clicked()),
           SLOT(slotSearch()) );
  connect( btnClear, SIGNAL(clicked()),
           SLOT(slotClear()) );
  connect( cmbPattern->lineEdit(), SIGNAL(textChanged ( const QString & )),
           SLOT( patternTextChanged( const QString & )));

  patternTextChanged( cmbPattern->lineEdit()->text());
}


GrepTool::~GrepTool()
{
  delete childproc;
}

void GrepTool::patternTextChanged( const QString & _text)
{
  btnSearch->setEnabled( !_text.isEmpty() );
}

void GrepTool::templateActivated(int index)
{
  leTemplate->setText(strTemplate[index]);
}

void GrepTool::itemSelected(const QString& item)
{
  int pos;
  QString filename, linenumber;

  QString str = item;
  if ( (pos = str.find(':')) != -1)
  {
    filename = str.left(pos);
    str = str.mid(pos+1);
    if ( (pos = str.find(':')) != -1)
    {
      filename = m_workingDir + QDir::separator() + filename;
      linenumber = str.left(pos);
      emit itemSelected(filename,linenumber.toInt()-1);
    }
  }
}

void GrepTool::processOutput()
{
  int pos;
  while ( (pos = buf.find('\n')) != -1)
  {
    QString item = buf.mid(2,pos-2);
    if (!item.isEmpty())
      lbResult->insertItem(item);
    buf = buf.mid(pos+1);
  }
  kapp->processEvents();
}

void GrepTool::slotSearch()
{
  if ( cmbPattern->currentText().isEmpty() )
  {
    cmbPattern->setFocus();
    return;
  }

  if ( cmbDir->url().isEmpty() || ! QDir(cmbDir->url()).exists() )
  {
    cmbDir->setFocus();
    KMessageBox::information( this, i18n(
        "You must enter an existing local folder in the 'Folder' entry."),
        i18n("Invalid Folder"), "Kate grep tool: invalid folder" );
    return;
  }

  if ( ! leTemplate->text().contains("%s") )
  {
    leTemplate->setFocus();
    return;
  }

  if ( childproc && childproc->isRunning() )
  {
    childproc->kill();
    return;
  }

  slotClear ();

  m_workingDir = cmbDir->url();

  QString s = cmbPattern->currentText();
  if ( ! cbRegex->isChecked() )
    s.replace( QRegExp( "([^\\w'()<>])" ), "\\\\1" );
  QString pattern = leTemplate->text();
  pattern.replace( "%s", s );

  childproc = new KProcess();
  childproc->setWorkingDirectory( m_workingDir );
  *childproc << "find" << ".";
  if (!cbRecursive->isChecked())
    *childproc << "-maxdepth" << "1";
  if (!cmbFiles->currentText().isEmpty() )
  {
    QStringList files = QStringList::split ( ",", cmbFiles->currentText(), FALSE );
    *childproc << "(";
    bool first = true;
    for ( QStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
      if (!first)
        *childproc << "-o";
      *childproc << "-name" << (*it);
      first = false;
    }
    *childproc << ")";
  }
  *childproc << "-exec" << "grep";
  if (!cbCasesensitive->isChecked())
    *childproc << "-i";
  *childproc << "-n" << "-e" << pattern << "{}";
  *childproc << "/dev/null"; //trick to have grep always display the filename
  *childproc << ";";

  connect( childproc, SIGNAL(processExited(KProcess *)),
           SLOT(childExited()) );
  connect( childproc, SIGNAL(receivedStdout(KProcess *, char *, int)),
           SLOT(receivedOutput(KProcess *, char *, int)) );
  connect( childproc, SIGNAL(receivedStderr(KProcess *, char *, int)),
           SLOT(receivedErrOutput(KProcess *, char *, int)) );

  // actually it should be checked whether the process was started successfully
  lbResult->setCursor( QCursor(Qt::WaitCursor) );
  btnClear->setEnabled( false );
  btnSearch->setGuiItem( KGuiItem(i18n("Cancel"), "button_cancel"));
  childproc->start(KProcess::NotifyOnExit, KProcess::AllOutput);
}

void GrepTool::slotSearchFor(const QString &pattern)
{
  slotClear();
  cmbPattern->setEditText(pattern);
  slotSearch();
}

void GrepTool::finish()
{
  btnSearch->setEnabled( !cmbPattern->lineEdit()->text().isEmpty() );

  buf += '\n';
  processOutput();
  delete childproc;
  childproc = 0;

  config->setGroup("GrepTool");

  QString cmbText = cmbPattern->currentText();
  bool itemsRemoved = lastSearchItems.remove(cmbText) > 0;
  lastSearchItems.prepend(cmbText);
  if (itemsRemoved)
  {
    cmbPattern->removeItem(cmbPattern->currentItem());
  }
  cmbPattern->insertItem(cmbText, 0);
  cmbPattern->setCurrentItem(0);
  if (lastSearchItems.count() > 10) {
    lastSearchItems.pop_back();
    cmbPattern->removeItem(cmbPattern->count() - 1);
  }
  config->writeEntry("LastSearchItems", lastSearchItems);


  cmbText = cmbDir->url();
  itemsRemoved = lastSearchPaths.remove(cmbText) > 0;
  lastSearchPaths.prepend(cmbText);
  if (itemsRemoved)
  {
    cmbDir->comboBox()->removeItem(cmbDir->comboBox()->currentItem());
  }
  cmbDir->comboBox()->insertItem(cmbText, 0);
  cmbDir->comboBox()->setCurrentItem(0);
  if (lastSearchPaths.count() > 10)
  {
    lastSearchPaths.pop_back();
    cmbDir->comboBox()->removeItem(cmbDir->comboBox()->count() - 1);
  }
  config->writeEntry("LastSearchPaths", lastSearchPaths);


  cmbText = cmbFiles->currentText();
  itemsRemoved = lastSearchFiles.remove(cmbText) > 0;
  lastSearchFiles.prepend(cmbText);
  if (itemsRemoved)
  {
    cmbFiles->removeItem(cmbFiles->currentItem());
  }
  cmbFiles->insertItem(cmbText, 0);
  cmbFiles->setCurrentItem(0);
  if (lastSearchFiles.count() > 10) {
    lastSearchFiles.pop_back();
    cmbFiles->removeItem(cmbFiles->count() - 1);
  }
  config->writeEntry("LastSearchFiles", lastSearchFiles);

  config->writeEntry("Recursive", cbRecursive->isChecked());
  config->writeEntry("CaseSensitive", cbCasesensitive->isChecked());
  config->writeEntry("Regex", cbRegex->isChecked());
}

void GrepTool::slotCancel()
{
  finish();
}

void GrepTool::childExited()
{
//   int status = childproc->exitStatus();
  lbResult->unsetCursor();
  btnClear->setEnabled( true );
  btnSearch->setGuiItem( KGuiItem(i18n("Find"), "find") );

  if ( ! errbuf.isEmpty() )
  {
    KMessageBox::information( parentWidget(), i18n("<strong>Error:</strong><p>") + errbuf, i18n("Grep Tool Error") );
    errbuf.truncate(0);
  }
  else
    finish();
}

void GrepTool::receivedOutput(KProcess */*proc*/, char *buffer, int buflen)
{
  buf += QCString(buffer, buflen+1);
  processOutput();
}

void GrepTool::receivedErrOutput(KProcess */*proc*/, char *buffer, int buflen)
{
  errbuf += QCString( buffer, buflen + 1 );
}

void GrepTool::slotClear()
{
  finish();
  lbResult->clear();
}

void GrepTool::updateDirName(const QString &dir)
{
  if (m_lastUpdatedDir != dir)
  {
    setDirName (dir);
    m_lastUpdatedDir = dir;
  }
}

void GrepTool::setDirName(const QString &dir){
  cmbDir->setURL(dir);
}

bool GrepTool::eventFilter( QObject *o, QEvent *e )
{
  if ( e->type() == QEvent::KeyPress && (
       ((QKeyEvent*)e)->key() == Qt::Key_Return ||
       ((QKeyEvent*)e)->key() == Qt::Key_Enter ) )
  {
    slotSearch();
    return true;
  }

  return QWidget::eventFilter( o, e );
}

void GrepTool::focusInEvent ( QFocusEvent * ev )
{
  QWidget::focusInEvent(ev);
  if (m_fixFocus) {
    m_fixFocus = false;
    cmbPattern->setFocus();
  }
}

void GrepTool::showEvent( QShowEvent * ev )
{
  QWidget::showEvent(ev);
  m_fixFocus = true;
}

#include "kategrepdialog.moc"
