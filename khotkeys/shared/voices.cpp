/****************************************************************************

 KHotKeys
 
 Copyright (C) 2005 Olivier Goffart  <ogoffart @ kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "voices.h"
#include "voicesignature.h"
#include "triggers.h"
#include "soundrecorder.h"

#include <stdlib.h>
#include <math.h>
#include <assert.h>


#include <kapplication.h>
#include <kdebug.h>
#include <kxerrorhandler.h>
#include <kkeynative.h>
#include <qtimer.h>
#include <kglobalaccel.h>


#include <X11/Xlib.h>
#include <fixx11h.h>

namespace KHotKeys
{

Voice* voice_handler;

Voice::Voice( bool enabled_P, QObject* parent_P )
	: QObject( parent_P) ,_enabled( enabled_P ), _recording( false ), _recorder(0)
    {
    assert( voice_handler == NULL );
    voice_handler = this;

	_kga=0L;
	_timer=0L;

	kdDebug(1217) << k_funcinfo << endl;
	
    }
      
Voice::~Voice()
    {
    kdDebug(1217) << k_funcinfo << endl;
    enable( false );
    voice_handler = NULL;
    }


void Voice::enable( bool enabled_P )
    {
#ifndef HAVE_ARTS
    enabled_P = false; // never enabled when there's no support
#endif
    if( _enabled == enabled_P )
        return;
    _enabled = enabled_P;
    if( _enabled ) // activate the shortcut
        set_shortcut( _shortcut );
    else
        {
        delete _kga;
        _kga = NULL;
        }
    }

void Voice::register_handler( Voice_trigger *trigger_P )
    {
    if( !_references.contains( trigger_P ))
        _references.append(trigger_P);
    }

void Voice::unregister_handler( Voice_trigger *trigger_P )
    {
		_references.remove(trigger_P);
    }


void Voice::record_start()
{
	kdDebug(1217) << k_funcinfo << endl;
	if(!_recorder)
	{
		_recorder= SoundRecorder::create(this);
		connect(_recorder, SIGNAL(recorded(const Sound& )), this, SLOT(slot_sound_recorded(const Sound& )));
	}

	_recorder->start();
	_recording=true;
}

void Voice::record_stop()
{
	if(!_recording)
		return;
	
	kdDebug(1217) << k_funcinfo << endl;
	delete _timer;
	_timer=0L;
	_recording=false;
	if(_recorder)
		_recorder->stop();
}


void Voice::slot_sound_recorded(const Sound &sound_P)
{
	VoiceSignature signature(sound_P);

	Voice_trigger *trig=0L;
	Voice_trigger *sec_trig=0L;
	double minimum=800000;
	double second_minimum=80000;
	int got_count=0;
	QValueList<Voice_trigger*>::Iterator it;
	for ( it = _references.begin(); it != _references.end(); ++it )
	{
		for(int ech=1; ech<=2 ; ech++)
		{
			Voice_trigger *t=*it;
			
			double diff=VoiceSignature::diff(signature, t->voicesignature(ech));
			if(minimum>=diff)
			{
				second_minimum=minimum;
				minimum=diff;
				sec_trig=trig;
				trig=t;
			}
			else if(second_minimum>=diff)
			{
				second_minimum=diff;
				sec_trig=t;
			}
			if( diff < REJECT_FACTOR_DIFF )
				got_count++;
			kdDebug(1217) << k_funcinfo << ( (diff < REJECT_FACTOR_DIFF) ? "+++" : "---" )  <<t->voicecode() << ech << " : " << diff << endl;
		}
	}
//	double ecart_relatif=(second_minimum-minimum)/minimum;

//	kdDebug(1217) << k_funcinfo <<  ecart_relatif << endl;

	if(trig)
		kdDebug(1217) << k_funcinfo << "**** " << trig->voicecode() << " : " << minimum << endl;

	
//	if(trig && ecart_relatif > REJECT_FACTOR_ECART_REL)
//	if(trig && got_count==1)
	bool selected=trig &&  (got_count==1 || ( minimum < 1.5*REJECT_FACTOR_DIFF   &&  trig==sec_trig  ) );

	if(selected)
	{
		trig->handle_Voice();
	}
	
}


/*bool Voice::x11Event( XEvent* pEvent )
{
	if( pEvent->type != XKeyPress && pEvent->type != XKeyRelease )
		return false;
	
	KKeyNative keyNative( pEvent );
	
	//kdDebug(1217) << k_funcinfo << keyNative.key().toString() << endl; 

	if(_shortcut.contains(keyNative))
	{
		if(pEvent->type == XKeyPress  && !_recording )
		{
			record_start();
			return true;
		}
		if(pEvent->type == XKeyRelease  && _recording )
		{
			record_stop();
			return true;
		}
	}
	return false;
}


*/


void Voice::set_shortcut( const KShortcut &shortcut)
{
    _shortcut = shortcut;
    if( !_enabled )
        return;
    if(!_kga)
        _kga = new KGlobalAccel( this );	
    _kga->remove("voice");

	_kga->insert( "voice", i18n("Voice"), QString::null,  shortcut, 0, this, SLOT(slot_key_pressed())) ;
        _kga->updateConnections();
}

void Voice::slot_key_pressed()
{
    if( !haveArts())
        return;
	if( _recording )
		record_stop();
	else
	{
		record_start();
		if(!_timer)
		{
			_timer=new QTimer(this);
			connect(_timer, SIGNAL(timeout()) , this, SLOT(slot_timeout()));
		}
		
		_timer->start(1000*20,true);
	}
}


void Voice::slot_timeout()
{
	if(_recording && _recorder)
	{
		_recorder->abort();
		_recording=false;
	}
	_timer->deleteLater();
	_timer=0L;
}


QString Voice::isNewSoundFarEnough(const VoiceSignature& signature, const QString &currentTrigger)
{
	Voice_trigger *trig=0L;
	Voice_trigger *sec_trig=0L;
	double minimum=800000;
	double second_minimum=80000;
	int got_count=0;
	QValueList<Voice_trigger*>::Iterator it;
	for ( it = _references.begin(); it != _references.end(); ++it )
	{
		Voice_trigger *t=*it;
		if(t->voicecode()==currentTrigger)
			continue;
		
		for(int ech=1; ech<=2 ; ech++)
		{
			double diff=VoiceSignature::diff(signature, t->voicesignature(ech));
			if(minimum>=diff)
			{
				second_minimum=minimum;
				minimum=diff;
				sec_trig=trig;
				trig=t;
			}
			else if(second_minimum>=diff)
			{
				second_minimum=diff;
				sec_trig=t;
			}
			if( diff < REJECT_FACTOR_DIFF )
				got_count++;
			kdDebug(1217) << k_funcinfo << ( (diff < REJECT_FACTOR_DIFF) ? "+++" : "---" )  <<t->voicecode() << ech << " : " << diff << endl;
		}
	}

	if(trig)
		kdDebug(1217) << k_funcinfo << "**** " << trig->voicecode() << " : " << minimum << endl;

	bool selected=trig &&  ((got_count==1 && minimum < REJECT_FACTOR_DIFF*0.7 ) || ( minimum < REJECT_FACTOR_DIFF   &&  trig==sec_trig  ) );
	return selected ? trig->voicecode() : QString::null;
}

bool Voice::doesVoiceCodeExists(const QString &vc)
{
	QValueList<Voice_trigger*>::Iterator it;
	for ( it = _references.begin(); it != _references.end(); ++it )
	{
		Voice_trigger *t=*it;
		if(t->voicecode()==vc)
			return true;
	}
	return false;
}

} // namespace KHotKeys

#include "voices.moc"
