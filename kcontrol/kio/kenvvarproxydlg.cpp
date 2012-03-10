/*
   kenvvarproxydlg.cpp - Proxy configuration dialog

   Copyright (C) 2001- Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License (GPL) version 2 as published by the Free Software
   Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <stdlib.h>

#include <qlabel.h>
#include <qlayout.h>
#include <qcheckbox.h>
#include <qwhatsthis.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <klocale.h>
#include <klineedit.h>
#include <kmessagebox.h>

#include "envvarproxy_ui.h"
#include "kenvvarproxydlg.h"


#define ENV_FTP_PROXY     "FTP_PROXY,ftp_proxy,FTPPROXY,ftpproxy,PROXY,proxy"
#define ENV_HTTP_PROXY    "HTTP_PROXY,http_proxy,HTTPPROXY,httpproxy,PROXY,proxy"
#define ENV_HTTPS_PROXY   "HTTPS_PROXY,https_proxy,HTTPSPROXY,httpsproxy,PROXY,proxy"
#define NO_PROXY          "NO_PROXY,no_proxy"


static QString getProxyEnv(const QString& var)
{
  return QString::fromLocal8Bit(::getenv(var.local8Bit().data()));
}

static bool autoDetectProxySetting(const QString& type, QString& proxyEnv)
{
  QStringList list = QStringList::split( ',', type);
  QStringList::ConstIterator it = list.begin();
  QStringList::ConstIterator end = list.end();

  for(; it != end; ++it)
  {
    if(!getProxyEnv(*it).isEmpty())
    {
      proxyEnv = *it;
      return true;
    }
  }

  return false;
}

KEnvVarProxyDlg::KEnvVarProxyDlg( QWidget* parent, const char* name )
                :KProxyDialogBase( parent, name, true,
                                   i18n( "Variable Proxy Configuration" ) )
{
  mDlg = new EnvVarProxyDlgUI( this );
  setMainWidget( mDlg );
  mDlg->leHttp->setMinimumWidth( mDlg->leHttp->fontMetrics().maxWidth() * 20 );
  init();
}

KEnvVarProxyDlg::~KEnvVarProxyDlg ()
{
}

void KEnvVarProxyDlg::init()
{
  m_bHasValidData = false;

  connect( mDlg->cbShowValue, SIGNAL( clicked() ), SLOT( showValuePressed() ) );
  connect( mDlg->pbVerify, SIGNAL( clicked() ), SLOT( verifyPressed() ) );
  connect( mDlg->pbDetect, SIGNAL( clicked() ), SLOT( autoDetectPressed() ) );
}

void KEnvVarProxyDlg::setProxyData( const KProxyData& data )
{
  // Setup HTTP Proxy...
  if (!getProxyEnv(data.proxyList["http"]).isEmpty())
    mEnvVarsMap["http"] = data.proxyList["http"];

  // Setup HTTPS Proxy...
  if (!getProxyEnv(data.proxyList["https"]).isEmpty())
    mEnvVarsMap["https"] = data.proxyList["https"];

  // Setup FTP Proxy...
  if (!getProxyEnv(data.proxyList["ftp"]).isEmpty())
    mEnvVarsMap["ftp"] = data.proxyList["ftp"];

  // Setup NO Proxy For...
  QString noProxyFor = data.noProxyFor.join("");
  if (!getProxyEnv(noProxyFor).isEmpty())
    mEnvVarsMap["noProxy"] = noProxyFor;

  mDlg->cbShowValue->setChecked( data.showEnvVarValue );
  showValue();
}

const KProxyData KEnvVarProxyDlg::data() const
{
  KProxyData data;

  if (m_bHasValidData)
  {
    data.proxyList["http"] = mEnvVarsMap["http"];
    data.proxyList["https"] = mEnvVarsMap["https"];
    data.proxyList["ftp"] = mEnvVarsMap["ftp"];
    data.noProxyFor = mEnvVarsMap["noProxy"];
    data.type = KProtocolManager::EnvVarProxy;
    data.showEnvVarValue = mDlg->cbShowValue->isChecked();
  }

  return data;
}


void KEnvVarProxyDlg::verifyPressed()
{
  if ( !validate() )
  {
    QString msg = i18n("You must specify at least one valid proxy "
                       "environment variable.");

    QString details = i18n("<qt>Make sure you entered the actual environment "
                           "variable name rather than its value. For "
                           "example, if the environment variable is <br><b>"
                           "HTTP_PROXY=http://localhost:3128</b><br> you need "
                           "to enter <b>HTTP_PROXY</b> here instead of the "
                           "actual value http://localhost:3128.</qt>");

    KMessageBox::detailedSorry( this, msg, details,
                                i18n("Invalid Proxy Setup") );
  }
  else
  {
    KMessageBox::information( this, i18n("Successfully verified."),
                                    i18n("Proxy Setup") );
  }
}

void KEnvVarProxyDlg::autoDetectPressed()
{
  bool found = false;

  setHighLight (mDlg->lbHttp, false);
  setHighLight (mDlg->lbHttps, false);
  setHighLight (mDlg->lbFtp, false);
  setHighLight (mDlg->lbNoProxy, false);

  // Detect HTTP proxy settings...
  found |= autoDetectProxySetting (QString::fromLatin1(ENV_HTTP_PROXY), mEnvVarsMap["http"]);

  // Detect HTTPS proxy settings...
  found |= autoDetectProxySetting (QString::fromLatin1(ENV_HTTPS_PROXY), mEnvVarsMap["https"]);

  // Detect FTP proxy settings...
  found |= autoDetectProxySetting (QString::fromLatin1(ENV_FTP_PROXY), mEnvVarsMap["ftp"]);

  // Detect the NO_PROXY settings...
  found |= autoDetectProxySetting (QString::fromLatin1(NO_PROXY), mEnvVarsMap["noProxy"]);

  if ( !found )
  {
    QString msg = i18n("Did not detect any environment variables "
                       "commonly used to set system wide proxy "
                       "information.");

    QString details = i18n("<qt>To learn about the variable names the "
                           "automatic detection process searches for, "
                           "press OK, click on the quick help button "
                           "on the window title bar of the "
                           "previous dialog and then click on the "
                           "\"<b>Auto Detect</b>\" button.</qt>");

    KMessageBox::detailedSorry( this, msg, details,
                                i18n("Automatic Proxy Variable Detection") );
    return;
  }

  showValue();
}

void KEnvVarProxyDlg::updateVariables()
{
  QString text = mDlg->leHttp->text();
  if (mEnvVarsMap["http"] != text)
    mEnvVarsMap["http"] = text;

  text = mDlg->leHttps->text();
  if (mEnvVarsMap["https"] != text)
    mEnvVarsMap["https"] = text;

  text = mDlg->leFtp->text();
  if (mEnvVarsMap["ftp"] != text)
    mEnvVarsMap["ftp"] = text;

  text = mDlg->leNoProxy->text();
  if (mEnvVarsMap["noProxy"] != text)
    mEnvVarsMap["noProxy"] = text;
}

void KEnvVarProxyDlg::showValuePressed()
{
  // Only update the variables whenever
  if (mDlg->cbShowValue->isChecked())
    updateVariables();

  showValue();
}

void KEnvVarProxyDlg::showValue()
{
  bool enable = mDlg->cbShowValue->isChecked();

  mDlg->leHttp->setReadOnly (enable);
  mDlg->leHttps->setReadOnly (enable);
  mDlg->leFtp->setReadOnly (enable);
  mDlg->leNoProxy->setReadOnly (enable);

  if (enable)
  {
    mDlg->leHttp->setText(getProxyEnv(mEnvVarsMap["http"]));
    mDlg->leHttps->setText(getProxyEnv(mEnvVarsMap["https"]));
    mDlg->leFtp->setText(getProxyEnv(mEnvVarsMap["ftp"]));
    mDlg->leNoProxy->setText(getProxyEnv(mEnvVarsMap["noProxy"]));
  }
  else
  {
    mDlg->leHttp->setText(mEnvVarsMap["http"]);
    mDlg->leHttps->setText(mEnvVarsMap["https"]);
    mDlg->leFtp->setText(mEnvVarsMap["ftp"]);
    mDlg->leNoProxy->setText(mEnvVarsMap["noProxy"]);
  }
}

bool KEnvVarProxyDlg::validate(bool erase)
{
  m_bHasValidData = false;

  if(!mDlg->cbShowValue->isChecked())
    updateVariables();

  bool notFound = getProxyEnv(mEnvVarsMap["http"]).isEmpty();
  m_bHasValidData |= !notFound;
  setHighLight (mDlg->lbHttp, notFound);
  if(notFound && erase) mEnvVarsMap["http"] = QString::null;

  notFound = getProxyEnv(mEnvVarsMap["https"]).isEmpty();
  m_bHasValidData |= !notFound;
  setHighLight (mDlg->lbHttps, notFound);
  if(notFound && erase) mEnvVarsMap["https"] = QString::null;

  notFound = getProxyEnv(mEnvVarsMap["ftp"]).isEmpty();
  m_bHasValidData |= !notFound;
  setHighLight (mDlg->lbFtp, notFound);
  if(notFound && erase) mEnvVarsMap["ftp"] = QString::null;

  notFound = getProxyEnv(mEnvVarsMap["noProxy"]).isEmpty();
  m_bHasValidData |= !notFound;
  setHighLight (mDlg->lbNoProxy, notFound);
  if(notFound && erase) mEnvVarsMap["noProxy"] = QString::null;

  return m_bHasValidData;
}

void KEnvVarProxyDlg::slotOk()
{
  if(!validate(true))
  {
    QString msg = i18n("You must specify at least one valid proxy "
                       "environment variable.");

    QString details = i18n("<qt>Make sure you entered the actual environment "
                           "variable name rather than its value. For "
                           "example, if the environment variable is <br><b>"
                           "HTTP_PROXY=http://localhost:3128</b><br> you need "
                           "to enter <b>HTTP_PROXY</b> here instead of the "
                           "actual value http://localhost:3128.</qt>");

    KMessageBox::detailedError( this, msg, details,
                                i18n("Invalid Proxy Setup") );
    return;
  }

  KDialogBase::slotOk ();
}

#include "kenvvarproxydlg.moc"
