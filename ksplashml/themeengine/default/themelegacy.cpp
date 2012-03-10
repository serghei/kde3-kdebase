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
#include <kconfig.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kprogress.h>

#include <qcheckbox.h>
#include <qdesktopwidget.h>
#include <qlabel.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <qwidget.h>
#include <qtimer.h>

#include "objkstheme.h"
#include "themeengine.h"
#include "themelegacy.h"
#include "themelegacy.moc"

DefaultConfig::DefaultConfig( QWidget *parent, KConfig *config )
    :ThemeEngineConfig( parent, config )
{
  mConfig->setGroup( QString("KSplash Theme: Default") );
  QVBox *hbox = new QVBox( this );
  mFlash = new QCheckBox( i18n("Icons flash while they are starting"), hbox );
  mFlash->setChecked( mConfig->readBoolEntry("Icons Flashing",true) );
  mAlwaysShow = new QCheckBox( i18n("Always show progress bar"), hbox );
  mAlwaysShow->setChecked( mConfig->readBoolEntry("Always Show Progress",true) );
}

void DefaultConfig::save()
{
  kdDebug() << "DefaultConfig::save()" << endl;
  mConfig->setGroup( QString("KSplash Theme: Default") );
  mConfig->writeEntry( "Icons Flashing", mFlash->isChecked() );
  mConfig->writeEntry( "Always Show Progress", mAlwaysShow->isChecked() );
  mConfig->sync();
}

#define BIDI 0

ThemeDefault::ThemeDefault( QWidget *parent, const char *name, const QStringList &args )
    :ThemeEngine( parent, name, args )
{

  mActivePixmap = mInactivePixmap = 0L;
  mState = 0;

  _readSettings();
  _initUi();

  if( mIconsFlashing )
  {
    mFlashTimer = new QTimer( this );
    connect( mFlashTimer, SIGNAL(timeout()), this, SLOT(flash()) );
    mFlashPixmap1 = new QPixmap();
    mFlashPixmap2 = new QPixmap();

  }
  else
  {
    mFlashTimer = 0L;
    mFlashPixmap1 = 0L;
    mFlashPixmap2 = 0L;
  }
}

ThemeDefault::~ThemeDefault()
{
    delete mFlashPixmap1;
    delete mFlashPixmap2;
}

void ThemeDefault::_initUi()
{
  QString resource_prefix;

  QVBox *vbox = new QVBox( this );
  vbox->setBackgroundMode(NoBackground);


  QString activePix, inactivePix;
#if BIDI
  if ( QApplication::reverseLayout() )
  {
      activePix = _findPicture(QString("splash_active_bar_bidi.png"));
      inactivePix = _findPicture(QString("splash_inactive_bar_bidi.png"));
  }
  else
#endif
  {
    activePix = _findPicture(QString("splash_active_bar.png"));
    inactivePix = _findPicture(QString("splash_inactive_bar.png"));
  }
  kdDebug() << "Inactive pixmap: " << inactivePix << endl;
  kdDebug() << "Active pixmap:   " <<   activePix << endl;

  mActivePixmap = new QPixmap( activePix );
  mInactivePixmap = new QPixmap( inactivePix );

  if (mActivePixmap->isNull())
  {
    mActivePixmap->resize(200,100);
    mActivePixmap->fill(Qt::blue);
  }
  if (mInactivePixmap->isNull())
  {
    mInactivePixmap->resize(200,100);
    mInactivePixmap->fill(Qt::black);
  }

  QPixmap tlimage( _findPicture(QString("splash_top.png")) );
  if (tlimage.isNull())
  {
    tlimage.resize(200,100);
    tlimage.fill(Qt::blue);
  }
  QLabel *top_label = new QLabel( vbox );
  top_label->setPixmap( tlimage );
  top_label->setFixedSize( tlimage.width(), tlimage.height() );
  top_label->setBackgroundMode(NoBackground);

  mBarLabel = new QLabel( vbox );
  mBarLabel->setPixmap(*mInactivePixmap);
  mBarLabel->setBackgroundMode(NoBackground);

  QPixmap blimage( _findPicture(QString("splash_bottom.png")) );
  if (blimage.isNull())
  {
    blimage.resize(200,100);
    blimage.fill(Qt::black);
  }
  QLabel *bottom_label = new QLabel( vbox );
  bottom_label->setPaletteBackgroundPixmap( blimage );

  
      mLabel = new QLabel( bottom_label );
      mLabel->setBackgroundOrigin( QWidget::ParentOrigin );
      mLabel->setPaletteForegroundColor( mLabelForeground );
      mLabel->setPaletteBackgroundPixmap( blimage );
      QFont f(mLabel->font());
      f.setBold(TRUE);
      mLabel->setFont(f);

      mProgressBar = new KProgress( mLabel );
      int h, s, v;
      mLabelForeground.getHsv( &h, &s, &v );
      mProgressBar->setPalette( QPalette( v > 128 ? black : white ));
      mProgressBar->setBackgroundOrigin( QWidget::ParentOrigin );
      mProgressBar->setPaletteBackgroundPixmap( blimage );

      bottom_label->setFixedWidth( QMAX(blimage.width(),tlimage.width()) );
      bottom_label->setFixedHeight( mLabel->sizeHint().height()+4 );

      // 3 pixels of whitespace between the label and the progressbar.
      mLabel->resize( bottom_label->width(), bottom_label->height() );

      mProgressBar->setFixedSize( 120, mLabel->height() );
      
      if (QApplication::reverseLayout()){
            mProgressBar->move( 2, 0 );   
//	    mLabel->move( mProgressBar->width() + 4, 0);
      }
      else{
            mProgressBar->move( bottom_label->width() - mProgressBar->width() - 4, 0);
	    mLabel->move( 2, 0 );
      }
      
      mProgressBar->hide();
  
  setFixedWidth( mInactivePixmap->width() );
  setFixedHeight( mInactivePixmap->height() +
                  top_label->height() + bottom_label->height() );

  const QRect rect = kapp->desktop()->screenGeometry( mTheme->xineramaScreen() );
  // KGlobalSettings::splashScreenDesktopGeometry(); cannot be used here.
  // kdDebug() << "ThemeDefault::_initUi" << rect << endl;

  move( rect.x() + (rect.width() - size().width())/2,
        rect.y() + (rect.height() - size().height())/2 );
}

// Attempt to find overrides elsewhere?
void ThemeDefault::_readSettings()
{
  if( !mTheme )
    return;

  KConfig *cfg = mTheme->themeConfig();
  if( !cfg )
    return;

  cfg->setGroup( QString("KSplash Theme: %1").arg(mTheme->theme()) );

  mIconsFlashing = cfg->readBoolEntry( "Icons Flashing", true );
  QColor df(Qt::white);
  mLabelForeground = cfg->readColorEntry( "Label Foreground", &df );
}

/*
 * ThemeDefault::slotUpdateState(): IF in Default mode, THEN adjust the bar
 * pixmap label. Whee, phun!
 *
 * A similar method exists in the old KSplash.
 */
void ThemeDefault::slotUpdateState()
{
  if( mState > 8 )
    mState = 8;

  if( mIconsFlashing )
  {

    *mFlashPixmap1 = updateBarPixmap( mState );
    *mFlashPixmap2 = updateBarPixmap( mState+1 );
    mBarLabel->setPixmap(*mFlashPixmap2);
    mFlashTimer->stop();

    if( mState < 8 )
      mFlashTimer->start(400);
  }
  else
    mBarLabel->setPixmap( updateBarPixmap( mState ) );

  mState++;
}

/*
 * ThemeDefault::updateBarPixmap(): IF in Default mode, THEN adjust the
 * bar pixmap to reflect the current state. WARNING! KSplash Default
 * does NOT support our "Restoring Session..." state. We will need
 * to reflect that somehow.
 */
QPixmap ThemeDefault::updateBarPixmap( int state )
{
  int offs;

  QPixmap x;
  if( !mActivePixmap ) return( x );
#if BIDI


  if( QApplication::reverseLayout() )
    {
      if ( state > 7 ) 
	return (  x );
    }
#endif

  offs = state * 58;
  if (state == 3)
    offs += 8;
  else if (state == 6)
    offs -= 8;

  QPixmap tmp(*mActivePixmap);
  QPainter p(&tmp);
#if BIDI
  if ( QApplication::reverseLayout() )
    p.drawPixmap(0, 0, *mInactivePixmap, 0, 0, tmp.width()-offs );
  else
#endif
    p.drawPixmap(offs, 0, *mInactivePixmap, offs, 0);
  return tmp ;
}

void ThemeDefault::flash()
{
  if( !mIconsFlashing )
    return;
  QPixmap *swap = mFlashPixmap1;
  mFlashPixmap1 = mFlashPixmap2;
  mFlashPixmap2 = swap;
  mBarLabel->setPixmap(*mFlashPixmap2);
}

QString ThemeDefault::_findPicture( const QString &pic )
{
  // Don't use ObjKsTheme::locateThemeData here for compatibility reasons.
  QString f = pic;
  if (mTheme->loColor())
    f = QString("locolor/")+f;
  //kdDebug() << "Searching for " << f << endl;
  //kdDebug() << "Theme directory: " << mTheme->themeDir() << endl;
  //kdDebug() << "Theme name:      " << mTheme->theme() << endl;
  QString p = QString::null;
  if ((p = locate("appdata",mTheme->themeDir()+f)).isEmpty())
    if ((p = locate("appdata",mTheme->themeDir()+"pics/"+f)).isEmpty())
      if ((p = locate("appdata", QString("pics/")+mTheme->theme()+"/"+f)).isEmpty())
        if ((p = locate("appdata",f)).isEmpty())
          if ((p = locate("appdata",QString("pics/")+f)).isEmpty())
            if ((p = locate("data",QString("pics/")+f)).isEmpty()) {
              ; // No more places to search
            }
  return p;
}
