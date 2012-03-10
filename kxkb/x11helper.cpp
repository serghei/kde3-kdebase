#include <qdir.h>
#include <qstring.h>
#include <qwindowdefs.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qdict.h>
#include <qregexp.h>

#include <kdebug.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#define explicit int_explicit        // avoid compiler name clash in XKBlib.h
#include <X11/XKBlib.h>
#undef explicit
#include <X11/extensions/XKBrules.h>

#include "x11helper.h"
#include "config.h"


// Compiler will size array automatically.
static const char* X11DirList[] =
    {
        XLIBDIR,
        "/usr/share/X11/",
        "/etc/X11/",
        "/usr/local/share/X11/",
        "/usr/X11R6/lib/X11/",
        "/usr/X11R6/lib64/X11/",
        "/usr/local/X11R6/lib/X11/",
        "/usr/local/X11R6/lib64/X11/",
        "/usr/lib/X11/",
        "/usr/lib64/X11/",
        "/usr/local/lib/X11/",
        "/usr/local/lib64/X11/",
        "/usr/pkg/share/X11/",
        "/usr/pkg/xorg/lib/X11/"
    };

// Compiler will size array automatically.
static const char* rulesFileList[] =
    {
	"xkb/rules/xorg",
	"xkb/rules/xfree86"
    };

// Macro will return number of elements in any static array as long as the
// array has at least one element.
#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

static const int X11_DIR_COUNT = ARRAY_SIZE(X11DirList);
static const int X11_RULES_COUNT = ARRAY_SIZE(rulesFileList);

const QString X11Helper::X11_WIN_CLASS_ROOT = "<root>";
const QString X11Helper::X11_WIN_CLASS_UNKNOWN = "<unknown>";

static const QRegExp NON_CLEAN_LAYOUT_REGEXP("[^a-z]");

bool X11Helper::m_layoutsClean = true;

const QString
X11Helper::findX11Dir()
{
	for(int ii=0; ii<X11_DIR_COUNT; ii++) {
		const char* xDir = X11DirList[ii];
		if( xDir != NULL && QDir(QString(xDir) + "xkb").exists() ) {
//			for(int jj=0; jj<X11_RULES_COUNT; jj++) {
//				
//			}
			return QString(xDir);
	    }
	
//	  if( X11_DIR.isEmpty() ) {
//	    return;
//	  }
	}
	return NULL;	
}

const QString
X11Helper::findXkbRulesFile(QString x11Dir, Display *dpy)
{
	QString rulesFile;
	XkbRF_VarDefsRec vd;
	char *tmp = NULL;

	if (XkbRF_GetNamesProp(dpy, &tmp, &vd) && tmp != NULL ) {
// 			kdDebug() << "namesprop " << tmp  << endl;
	  	rulesFile = x11Dir + QString("xkb/rules/%1").arg(tmp);
// 			kdDebug() << "rulesF " << rulesFile  << endl;
	}
	else {
    // old way
    	for(int ii=0; ii<X11_RULES_COUNT; ii++) {
    		const char* ruleFile = rulesFileList[ii];
     		QString xruleFilePath = x11Dir + ruleFile;
//  			kdDebug() << "trying " << xruleFilePath << endl;
		    if( QFile(xruleFilePath).exists() ) {
				rulesFile = xruleFilePath;
			break;
		    }
    	}
    }
	
	return rulesFile;
}

RulesInfo*
X11Helper::loadRules(const QString& file, bool layoutsOnly) 
{
	XkbRF_RulesPtr xkbRules = XkbRF_Load(QFile::encodeName(file).data(), "", true, true);

	if (xkbRules == NULL) {
// throw Exception
		return NULL;
	}

	RulesInfo* rulesInfo = new RulesInfo();

	for (int i = 0; i < xkbRules->layouts.num_desc; ++i) {
		QString layoutName(xkbRules->layouts.desc[i].name);
		rulesInfo->layouts.replace( layoutName, qstrdup( xkbRules->layouts.desc[i].desc ) );

		if( m_layoutsClean == true
				  && layoutName.find( NON_CLEAN_LAYOUT_REGEXP ) != -1 
				  && layoutName.endsWith("/jp") == false ) {
			kdDebug() << "Layouts are not clean (Xorg < 6.9.0 or XFree86)" << endl;
			m_layoutsClean = false;
		}
	}

	if( layoutsOnly == true ) {
		XkbRF_Free(xkbRules, true);
		return rulesInfo;
	}
  
  for (int i = 0; i < xkbRules->models.num_desc; ++i)
      rulesInfo->models.replace(xkbRules->models.desc[i].name, qstrdup( xkbRules->models.desc[i].desc ) );
  for (int i = 0; i < xkbRules->options.num_desc; ++i)
      rulesInfo->options.replace(xkbRules->options.desc[i].name, qstrdup( xkbRules->options.desc[i].desc ) );

  XkbRF_Free(xkbRules, true);

// workaround for empty 'compose' options group description
   if( rulesInfo->options.find("compose:menu") && !rulesInfo->options.find("compose") ) {
     rulesInfo->options.replace("compose", "Compose Key Position");
   }

  for(QDictIterator<char> it(rulesInfo->options) ; it.current() != NULL; ++it ) {
	  QString option(it.currentKey());
	  int columnPos = option.find(":");
	  
	  if( columnPos != -1 ) {
		  QString group = option.mid(0, columnPos);
		  if( rulesInfo->options.find(group) == NULL ) {
			  rulesInfo->options.replace(group, group.latin1());
			  kdDebug() << "Added missing option group: " << group << endl;
		  }
	  }
  }
  
//   // workaround for empty misc options group description in XFree86 4.4.0
//   if( rulesInfo->options.find("numpad:microsoft") && !rulesInfo->options.find("misc") ) {
//     rulesInfo->options.replace("misc", "Miscellaneous compatibility options" );
//   }

  return rulesInfo;
}

// check $oldlayouts and $nonlatin groups for XFree 4.3 and later
OldLayouts*
X11Helper::loadOldLayouts(const QString& rulesFile)
{
  static const char* oldLayoutsTag = "! $oldlayouts";
  static const char* nonLatinLayoutsTag = "! $nonlatin";
  QStringList m_oldLayouts;
  QStringList m_nonLatinLayouts;
  
  QFile f(rulesFile);
  
  if (f.open(IO_ReadOnly))
    {
      QTextStream ts(&f);
      QString line;

      while (!ts.eof()) {
	  line = ts.readLine().simplifyWhiteSpace();

	  if( line.find(oldLayoutsTag) == 0 ) {

	    line = line.mid(strlen(oldLayoutsTag));
	    line = line.mid(line.find('=')+1).simplifyWhiteSpace();
	    while( !ts.eof() && line.endsWith("\\") )
		line = line.left(line.length()-1) + ts.readLine();
	    line = line.simplifyWhiteSpace();

	    m_oldLayouts = QStringList::split(QRegExp("\\s"), line);
//	    kdDebug() << "oldlayouts " << m_oldLayouts.join("|") << endl;
	    if( !m_nonLatinLayouts.empty() )
	      break;
	    
	  }
	  else
	  if( line.find(nonLatinLayoutsTag) == 0 ) {

	    line = line.mid(strlen(nonLatinLayoutsTag)+1).simplifyWhiteSpace();
	    line = line.mid(line.find('=')+1).simplifyWhiteSpace();
	    while( !ts.eof() && line.endsWith("\\") )
		line = line.left(line.length()-1) + ts.readLine();
	    line = line.simplifyWhiteSpace();

	    m_nonLatinLayouts = QStringList::split(QRegExp("\\s"), line);
//	    kdDebug() << "nonlatin " << m_nonLatinLayouts.join("|") << endl;
	    if( !m_oldLayouts.empty() )
	      break;
	    
	  }
      }

      f.close();
    }
	
	OldLayouts* oldLayoutsStruct = new OldLayouts();
	oldLayoutsStruct->oldLayouts = m_oldLayouts;
	oldLayoutsStruct->nonLatinLayouts = m_nonLatinLayouts;
	
	return oldLayoutsStruct;
}


/* pretty simple algorithm - reads the layout file and
    tries to find "xkb_symbols"
    also checks whether previous line contains "hidden" to skip it
*/
QStringList*
X11Helper::getVariants(const QString& layout, const QString& x11Dir, bool oldLayouts)
{
  QStringList* result = new QStringList();

  QString file = x11Dir + "xkb/symbols/";
  // workaround for XFree 4.3 new directory for one-group layouts
  if( QDir(file+"pc").exists() && !oldLayouts )
    file += "pc/";
    
  file += layout;

//  kdDebug() << "reading variants from " << file << endl;
  
  QFile f(file);
  if (f.open(IO_ReadOnly))
    {
      QTextStream ts(&f);

      QString line;
      QString prev_line;

      while (!ts.eof()) {
    	  prev_line = line;
	  line = ts.readLine().simplifyWhiteSpace();

	    if (line[0] == '#' || line.left(2) == "//" || line.isEmpty())
		continue;

	    int pos = line.find("xkb_symbols");
	    if (pos < 0)
		continue;

	    if( prev_line.find("hidden") >=0 )
		continue;

	    pos = line.find('"', pos) + 1;
	    int pos2 = line.find('"', pos);
	    if( pos < 0 || pos2 < 0 )
		continue;

	    result->append(line.mid(pos, pos2-pos));
//  kdDebug() << "adding variant " << line.mid(pos, pos2-pos) << endl;
      }

      f.close();
    }

	return result;
}

QString 
X11Helper::getWindowClass(WId winId, Display* dpy)
{
  unsigned long nitems_ret, bytes_after_ret;
  unsigned char* prop_ret;
  Atom     type_ret;
  int      format_ret;
  Window w = (Window)winId;	// suppose WId == Window
  QString  property;

  if( winId == X11Helper::UNKNOWN_WINDOW_ID ) {
	  kdDebug() << "Got window class for " << winId << ": '" << X11_WIN_CLASS_ROOT << "'" << endl;
	  return X11_WIN_CLASS_ROOT;
  }
  
//  kdDebug() << "Getting window class for " << winId << endl;
  if((XGetWindowProperty(dpy, w, XA_WM_CLASS, 0L, 256L, 0, XA_STRING,
			&type_ret, &format_ret, &nitems_ret,
			&bytes_after_ret, &prop_ret) == Success) && (type_ret != None)) {
    property = QString::fromLocal8Bit(reinterpret_cast<char*>(prop_ret));
    XFree(prop_ret);
  }
  else {
	  property = X11_WIN_CLASS_UNKNOWN;
  }
  kdDebug() << "Got window class for " << winId << ": '" << property << "'" << endl;
  
  return property;
}

bool X11Helper::areSingleGroupsSupported()
{
	return true; //TODO:
}
