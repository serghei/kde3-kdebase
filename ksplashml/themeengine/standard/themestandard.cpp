/***************************************************************************
 *   Copyright Brian Ledbetter 2001-2003 <brian@shadowcom.net>             *
 *   Copyright Ravikiran Rajagopal 2003 <ravi@kde.org>                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License (version 2) as   *
 *   published by the Free Software Foundation. (The original KSplash/ML   *
 *   codebase (upto version 0.95.3) is BSD-licensed.)                      *
 *                                                                         *
 ***************************************************************************/

#include <kapplication.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kconfig.h>

#include <qdesktopwidget.h>
#include <qlabel.h>
#include <qpalette.h>
#include <qpixmap.h>
#include <qwidget.h>

#include <objkstheme.h>
#include "themestandard.h"
#include "themestandard.moc"
#include "wndicon.h"
#include "wndstatus.h"

ThemeStandard::ThemeStandard( QWidget *parent, const char *name, const QStringList &args )
  :ThemeEngine( parent, name, args ), mIcon(0L), mPrevIcon(0L), mIconCount(0), mStdIconWidth(-1),
  mIconPos(WndIcon::HBottomLeft), mSbAtTop(false), mSbVisible(true), mSbPbVisible(true), mSbFontName("helvetica"),
  mSbFontSz(16), mSbFontBold(true), mSbFontItalic(false), mSbFont(QFont()), mSbFg(QColor()), mSbBg(QColor()),
  mSbIcon("run"), mIconsVisible(true), mIconsJumping(true), mSplashScreen("(Default)")
{
  _readSettings();
  _initUi();
}

void ThemeStandard::_initUi()
{
  setFrameStyle( QFrame::NoFrame );

  QString pixName = mTheme->locateThemeData( mSplashScreen );

  if( mSplashScreen == "(Default)" || pixName.isEmpty() )
  {
    QString resource_prefix = "pics/";
    if ( mTheme->loColor() )
      resource_prefix += "locolor/";
    pixName = locate( "appdata", resource_prefix + "splash.png");
  }

  QPixmap px = QPixmap( pixName );

  if( !px.isNull() )
  {
    //kdDebug() << "Loaded splash " << mSplashScreen << endl;
    int pw = px.width();
    int ph = px.height();

    QLabel *lbl = new QLabel( this );
    lbl->setBackgroundMode( NoBackground );
    lbl->setFixedSize( pw, ph );
    lbl->setPixmap( px );

    resize( pw, ph );
  }
  else
  {
    //kdDebug() << "Couldn't load splash " << mSplashScreen << endl;
    resize( 0, 0 );
  }

  const QRect rect = kapp->desktop()->screenGeometry( mTheme->xineramaScreen() );
  // KGlobalSettings::splashScreenDesktopGeometry(); cannot be used here.

  move( rect.x() + (rect.width() - size().width())/2,
        rect.y() + (rect.height() - size().height())/2 );

  mStatus = new WndStatus( QPalette(), mTheme->xineramaScreen(), mSbAtTop, mSbPbVisible, mSbFont, mSbFg, mSbBg, mSbIcon );
}

void ThemeStandard::showEvent( QShowEvent * )
{

  ThemeEngine::show();

  if( mSbVisible )
  {
    mStatus->show();
    mStatusBarHeight = mStatus->height();
  }
  else
  {
    mStatusBarHeight = 0L;
  }
}

// Adjust the visible icon.
void ThemeStandard::slotSetPixmap( const QString& pxn )
{
  QPixmap px = DesktopIcon( pxn );

  if ( px.isNull() )
    px = DesktopIcon( "go" );

  if ( !mIconsVisible )
    return;

  /* (We only use prev_i if jumping is enabled...) */
  if ( mIconsJumping && mPrevIcon )
    emit mPrevIcon->slotStopJumping();

  if ( mStdIconWidth < 0 )
    mStdIconWidth = DesktopIcon( "go" ).width();

  mIcon = new WndIcon( ++mIconCount, mStdIconWidth, mStatusBarHeight, mTheme->xineramaScreen(),
                       px, QString::null, mIconPos, mSbAtTop, mIconsJumping );
  mIcon->show();

  if( mIconsJumping )
  {
    emit mIcon->slotJump();
    mPrevIcon = mIcon;
  }
}

void ThemeStandard::_readSettings()
{

  if ( !mTheme )
    return;

  KConfig *cfg = mTheme->themeConfig();
  if ( !cfg )
    return;

  //if ( !cfg->hasGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) ) )
  //  return;
  cfg->setGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) );

  QString sbpos = cfg->readEntry( "Statusbar Position", "Bottom" ).upper();
  mSbAtTop = ( sbpos == "TOP" );
  mSbVisible = cfg->readBoolEntry( "Statusbar Visible", true);
  mSbPbVisible = cfg->readBoolEntry( "Progress Visible", true);

  mSbFontName = cfg->readEntry( "Statusbar Font", "Sans Serif" );
  mSbFontSz = cfg->readNumEntry( "Statusbar Font Size", 16 );
  mSbFontBold = cfg->readBoolEntry( "Statusbar Font Bold", true );
  mSbFontItalic = cfg->readBoolEntry( "Statusbar Font Italic", false );
  mSbFont = QFont( mSbFontName, mSbFontSz, ( mSbFontBold? QFont::Bold : QFont::Normal ) );
  if( mSbFontItalic )
    mSbFont.setItalic( true );

  mSbFg = cfg->readColorEntry( "Statusbar Foreground", &Qt::white );
  mSbBg = cfg->readColorEntry( "Statusbar Background", &Qt::black );
  mSbIcon = cfg->readEntry( "Statusbar Icon", "run" );
  mIconsVisible = cfg->readBoolEntry( "Icons Visible", true);
  mIconsJumping = cfg->readBoolEntry( "Icons Jumping", true);
  mIconPos = (WndIcon::Position)cfg->readNumEntry( "Icon Position", 0 );
  mSplashScreen = cfg->readEntry( "Splash Screen", "(Default)");
  // cfg->readBoolEntry( "Allow Configuration", true );
}
