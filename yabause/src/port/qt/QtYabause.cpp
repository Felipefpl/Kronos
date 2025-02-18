/*  Copyright 2005 Guillaume Duhamel
	Copyright 2005-2006, 2013 Theo Berkau
	Copyright 2008 Filipe Azevedo <pasnox@gmail.com>

	This file is part of Yabause.

	Yabause is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Yabause is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Yabause; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
*/
#include "QtYabause.h"
#include "ui/UIYabause.h"
#include "Settings.h"
#include "VolatileSettings.h"

#include <QApplication>
#include <QLabel>
#include <QGroupBox>
#include <QTreeWidget>
#include <QPointer>
#include <QDir>

#ifndef ARCH_IS_WINDOWS
#include <sys/time.h>
#include <unistd.h>
#else
#include <windows.h>
#endif

// cores

#ifdef Q_OS_WIN
extern CDInterface SPTICD;
#endif

extern "C" {
M68K_struct * M68KCoreList[] = {
&M68KDummy,
&M68KMusashi,
NULL
};

SH2Interface_struct *SH2CoreList[] = {
&SH2Interpreter,
&SH2DebugInterpreter,
#ifdef SH2_DYNAREC
&SH2Dynarec,
#endif
#if DYNAREC_DEVMIYAX
&SH2Dyn,
&SH2DynDebug,
#endif
#if DYNAREC_KRONOS
&SH2KronosInterpreter,
#endif
NULL
};

PerInterface_struct *PERCoreList[] = {
&PERDummy,
&PERQT,
#ifdef HAVE_LIBSDL
&PERSDLJoy,
#endif
#ifdef __APPLE__
&PERMacJoy,
#endif
#ifdef ARCH_IS_LINUX
&PERLinuxJoy,
#endif
#if defined HAVE_DIRECTINPUT
&PERDIRECTX,
#endif
NULL
};

CDInterface *CDCoreList[] = {
&DummyCD,
&ISOCD,
#ifndef UNKNOWN_ARCH
&ArchCD,
#endif
NULL
};

SoundInterface_struct *SNDCoreList[] = {
&SNDDummy,
#ifdef HAVE_LIBSDL
&SNDSDL,
#endif
#ifdef HAVE_LIBAL
&SNDAL,
#endif
#ifdef ARCH_IS_MACOSX
&SNDMac,
#endif
#if defined HAVE_DIRECTSOUND
&SNDDIRECTX,
#endif
NULL
};

VideoInterface_struct *VIDCoreList[] = {
#ifdef HAVE_LIBGL
&VIDOGL,
&VIDCS,
#endif
#ifdef USE_SOFT_RENDER
&VIDSoft,
#endif
NULL
};

#ifdef YAB_PORT_OSD
#include "nanovg_osdcore.h"
OSD_struct *OSDCoreList[] = {
#ifdef HAVE_LIBGL
&OSDNnovg,
#endif
&OSDSoft,
&OSDDummy,
NULL
};
#endif

}

// main window
QPointer<UIYabause> mUIYabause = 0;
// settings object
QPointer<Settings> mSettings = 0;
QPointer<VolatileSettings> mVolatileSettings = 0;
// ports padbits
QMap<uint, PerPad_struct*> mPort1PadsBits;
QMap<uint, PerPad_struct*> mPort2PadsBits;
QMap<uint, PerCab_struct*> mPortIOBits;
QMap<uint, PerGun_struct*> mPort1GunBits;
QMap<uint, PerGun_struct*> mPort2GunBits;
QMap<uint, PerMouse_struct*> mPort1MouseBits;
QMap<uint, PerMouse_struct*> mPort2MouseBits;
QMap<uint, PerAnalog_struct*> mPort1AnalogBits;
QMap<uint, PerAnalog_struct*> mPort2AnalogBits;

extern "C"
{

#ifdef WIN32
#ifndef vasprintf
		int vasprintf(char **strp, const char *fmt, va_list ap) {
			int len = 512;
			char *str = (char*)malloc((size_t)len);
			int r = vsnprintf_s(str, len, _TRUNCATE, fmt, ap); /* "secure" version of vsprintf */
			if (r == -1) return free(str), -1;
			*strp = str;
			return r;
		}
#endif
        void YuiMsg(const char *format, ...) {
          int r;
	  char *str=NULL;
          va_list arglist;
          va_start( arglist, format );
          r = vasprintf(&str, format, arglist);
	  va_end( arglist );
	  if (r > 0) {
            //QtYabause::mainWindow()->appendLog( str );
		  wchar_t wtext[512];
		  mbstowcs(wtext, str, strlen(str) + 1);//Plus null
		  LPWSTR ptr = wtext;
		    ::OutputDebugString(ptr);
	      free(str);
	  }
       }
#else
	void YuiMsg(const char *format, ...) {
		char dest[512];
		va_list argptr;
		va_start(argptr, format);
		vsnprintf(dest, 512, format, argptr);
		va_end(argptr);
		printf("%s", dest);
}

#endif

       void YuiErrorMsg(const char *error_text)
       {
         YuiMsg("Error: %s\n", error_text);
       }

  void YuiEndOfFrame()
	{
		
	}
	void YuiSwapBuffers()
	{
          QtYabause::mainWindow()->swapBuffers();
        }

	int YuiGetFB()
	{
          return 0;
        }

#if defined(HAVE_DIRECTINPUT) || defined(HAVE_DIRECTSOUND)
	HWND DXGetWindow()
	{
		return (HWND)mUIYabause->winId();
	}
#endif
}

void QtYabause::appendLog( const char* str )
{
#ifdef _WIN32
  wchar_t wtext[512];
  mbstowcs(wtext, str, strlen(str) + 1);//Plus null
  LPWSTR ptr = wtext;
	::OutputDebugString(ptr);
#else
  printf("%s\n", str);
#endif
}


UIYabause* QtYabause::mainWindow( bool create )
{
        qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "1");
	if ( !mUIYabause && create )
		mUIYabause = new UIYabause;
	return mUIYabause;
}

Settings* QtYabause::settings( bool create )
{
	if ( !mSettings && create )
		mSettings = new Settings();
	return mSettings;
}

VolatileSettings* QtYabause::volatileSettings( bool create )
{
	if ( !mVolatileSettings && create )
		mVolatileSettings = new VolatileSettings();
	return mVolatileSettings;
}

QList <translation_struct> QtYabause::getTranslationList()
{
	QList <translation_struct> translations;
#ifdef HAVE_LIBMINI18N
	QDir transDir=QDir(YTSDIR, "*.yts");

	foreach(QString file, transDir.entryList())
	{
		if (!file.endsWith(".yts"))
			continue;
		translation_struct trans;
		trans.file = QString(YTSDIR) + QString("/") + file;

		// Let's get it down just to language code
		QStringList string = file.remove(".yts").split("_");
		if (file.startsWith("yabause_"))
			string.removeFirst();
		QString localeStr = string.join("_");
		// Find the locale
		QLocale locale = QLocale(localeStr);
		// Now we should be good for the language name
#if QT_VERSION < 0x040800
		trans.name = locale.name();
#else
		trans.name = locale.nativeLanguageName();
#endif
		translations.append(trans);
	}
#endif
	return translations;
}

int QtYabause::setTranslationFile()
{
#ifdef HAVE_LIBMINI18N
	const QString s = settings()->value( "General/Translation" ).toString();
	if ( ! s.isEmpty() )
	{
		if (s == "#") return 0; // magic value for "no translation" (aka english)

		const char* filePath = qstrdup( s.toLocal8Bit().constData() );
		if ( mini18n_set_locale( filePath ) == 0 )
		{
			QtYabause::retranslateApplication();
			if ( logTranslation() != 0 )
				qWarning( "Can't log translation !" );
			return 0;
		}
	}

	if ( mini18n_set_domain( YTSDIR ) == 0 )
	{
		QtYabause::retranslateApplication();
		if ( logTranslation() != 0 )
			qWarning( "Can't log translation !" );
		return 0;
	}
#endif
	return -1;
}

int QtYabause::logTranslation()
{
#ifdef HAVE_LIBMINI18N
	if (settings()->value( "General/LogUntranslated", false ).toBool())
	{
		const QString s = settings()->value( "General/Translation" ).toString().replace( ".yts", "_log.yts" );
		if ( s.isEmpty() )
			return 0;
		const char* filePath = qstrdup( s.toLocal8Bit().constData() );
		return mini18n_set_log( filePath );
	}
#endif
	return 0;
}

void QtYabause::closeTranslation()
{
#ifdef HAVE_LIBMINI18N
	mini18n_close();
#endif
}

QString QtYabause::translate( const QString& string )
{
#ifdef HAVE_LIBMINI18N
	return QString::fromUtf8( _( string.toUtf8().constData() ) );
#else
	return string;
#endif
}

void QtYabause::retranslateWidgetOnly( QWidget* widget )
{
#ifdef HAVE_LIBMINI18N
	if ( !widget )
		return;
	widget->setAccessibleDescription( translate( widget->accessibleDescription() ) );
	widget->setAccessibleName( translate( widget->accessibleName() ) );
	widget->setStatusTip( translate( widget->statusTip() ) );
	widget->setStyleSheet( translate( widget->styleSheet() ) );
	widget->setToolTip( translate( widget->toolTip() ) );
	widget->setWhatsThis( translate( widget->whatsThis() ) );
	widget->setWindowIconText( translate( widget->windowIconText() ) );
	widget->setWindowTitle( translate( widget->windowTitle() ) );
#endif
}

void QtYabause::retranslateWidget( QWidget* widget )
{
#ifdef HAVE_LIBMINI18N
	if ( !widget )
		return;
	// translate all widget based members
	retranslateWidgetOnly( widget );
	// get class name
	const QString className = widget->metaObject()->className();
	if ( className == "QWidget" )
		return;
	else if ( className == "QLabel" )
	{
		QLabel* l = qobject_cast<QLabel*>( widget );
		l->setText( translate( l->text() ) );
	}
	else if ( className == "QAbstractButton" || widget->inherits( "QAbstractButton" ) )
	{
		QAbstractButton* ab = qobject_cast<QAbstractButton*>( widget );
		ab->setText( translate( ab->text() ) );
	}
	else if ( className == "QGroupBox" )
	{
		QGroupBox* gb = qobject_cast<QGroupBox*>( widget );
		gb->setTitle( translate( gb->title() ) );
	}
	else if ( className == "QMenu" || className == "QMenuBar" )
	{
		QList<QMenu*> menus;
		if ( className == "QMenuBar" )
			menus = qobject_cast<QMenuBar*>( widget )->findChildren<QMenu*>();
		else
			menus << qobject_cast<QMenu*>( widget );
		foreach ( QMenu* m, menus )
		{
			m->setTitle( translate( m->title() ) );
			// retranslate menu actions
			foreach ( QAction* a, m->actions() )
			{
				a->setIconText( translate( a->iconText() ) );
				a->setStatusTip( translate( a->statusTip() ) );
				a->setText( translate( a->text() ) );
				a->setToolTip( translate( a->toolTip() ) );
				a->setWhatsThis( translate( a->whatsThis() ) );
			}
		}
	}
	else if ( className == "QTreeWidget" )
	{
		QTreeWidget* tw = qobject_cast<QTreeWidget*>( widget );
		QTreeWidgetItem* twi = tw->headerItem();
		for ( int i = 0; i < twi->columnCount(); i++ )
		{
			twi->setStatusTip( i, translate( twi->statusTip( i ) ) );
			twi->setText( i, translate( twi->text( i ) ) );
			twi->setToolTip( i, translate( twi->toolTip( i ) ) );
			twi->setWhatsThis( i, translate( twi->whatsThis( i ) ) );
		}
	}
	else if ( className == "QTabWidget" )
	{
		QTabWidget* tw = qobject_cast<QTabWidget*>( widget );
		for ( int i = 0; i < tw->count(); i++ )
			tw->setTabText( i, translate( tw->tabText( i ) ) );
	}
	// translate children
	foreach ( QWidget* w, widget->findChildren<QWidget*>() )
		retranslateWidget( w );
#endif
}

void QtYabause::retranslateApplication()
{
#ifdef HAVE_LIBMINI18N
	foreach ( QWidget* widget, QApplication::allWidgets() )
		retranslateWidget( widget );
#endif
}

const char* QtYabause::getCurrentCdSerial()
{ return cdip ? cdip->itemnum : 0; }

M68K_struct* QtYabause::getM68KCore( int id )
{
	for ( int i = 0; M68KCoreList[i] != NULL; i++ )
		if ( M68KCoreList[i]->id == id )
			return M68KCoreList[i];
	return 0;
}

SH2Interface_struct* QtYabause::getSH2Core( int id )
{
	for ( int i = 0; SH2CoreList[i] != NULL; i++ )
		if ( SH2CoreList[i]->id == id )
			return SH2CoreList[i];
	return 0;
}

PerInterface_struct* QtYabause::getPERCore( int id )
{
	for ( int i = 0; PERCoreList[i] != NULL; i++ )
		if ( PERCoreList[i]->id == id )
			return PERCoreList[i];
	return 0;
}

CDInterface* QtYabause::getCDCore( int id )
{
	for ( int i = 0; CDCoreList[i] != NULL; i++ )
		if ( CDCoreList[i]->id == id )
			return CDCoreList[i];
	return 0;
}

SoundInterface_struct* QtYabause::getSNDCore( int id )
{
	for ( int i = 0; SNDCoreList[i] != NULL; i++ )
		if ( SNDCoreList[i]->id == id )
			return SNDCoreList[i];
	return 0;
}

VideoInterface_struct* QtYabause::getVDICore( int id )
{
	for ( int i = 0; VIDCoreList[i] != NULL; i++ )
		if ( VIDCoreList[i]->id == id )
			return VIDCoreList[i];
	return 0;
}

CDInterface QtYabause::defaultCDCore()
{
#ifndef UNKNOWN_ARCH
	return ArchCD;
#else
	return DummyCD;
#endif
}

SoundInterface_struct QtYabause::defaultSNDCore()
{
#ifdef HAVE_LIBSDL
	return SNDSDL;
#else
	return SNDDummy;
#endif
}

VideoInterface_struct QtYabause::defaultVIDCore()
{
#ifdef HAVE_LIBGL
        return VIDOGL;
#else
	return VIDSoft;
#endif
}

OSD_struct QtYabause::defaultOSDCore()
{
#ifdef HAVE_LIBGL
        return OSDNnovg;
#else
	return OSDDummy;
#endif
}

PerInterface_struct QtYabause::defaultPERCore()
{
#if defined HAVE_LIBSDL
	return PERSDLJoy;
#elif defined HAVE_DIRECTINPUT
		return PERDIRECTX;
#else
	return PERQT;
#endif
}

M68K_struct QtYabause::default68kCore()
{
   return M68KMusashi;
}

SH2Interface_struct QtYabause::defaultSH2Core()
{
#if DYNAREC_KRONOS
   return SH2KronosInterpreter;
#else
   return SH2Interpreter;
#endif
}

QMap<uint, PerPad_struct*>* QtYabause::portPadsBits( uint portNumber )
{
	switch ( portNumber )
	{
		case 1:
			return &mPort1PadsBits;
			break;
		case 2:
			return &mPort2PadsBits;
			break;
		default:
			return 0;
			break;
	}
}

void QtYabause::clearPadsBits()
{
	mPort1PadsBits.clear();
	mPort2PadsBits.clear();
}

QMap<uint, PerCab_struct*>* QtYabause::portIOBits( ) {
  return &mPortIOBits;
}

void QtYabause::clearIOBits()
{
   mPortIOBits.clear();
}

QMap<uint, PerAnalog_struct*>* QtYabause::portAnalogBits( uint portNumber )
{
   switch ( portNumber )
   {
   case 1:
      return &mPort1AnalogBits;
      break;
   case 2:
      return &mPort2AnalogBits;
      break;
   default:
      return 0;
      break;
   }
}

void QtYabause::clear3DAnalogBits()
{
   mPort1AnalogBits.clear();
   mPort2AnalogBits.clear();
}

QMap<uint, PerGun_struct*>* QtYabause::portGunBits( uint portNumber )
{
	switch ( portNumber )
	{
	case 1:
		return &mPort1GunBits;
		break;
	case 2:
		return &mPort2GunBits;
		break;
	default:
		return 0;
		break;
	}
}

void QtYabause::clearGunBits()
{
	mPort1GunBits.clear();
	mPort2GunBits.clear();
}

QMap<uint, PerMouse_struct*>* QtYabause::portMouseBits( uint portNumber )
{
   switch ( portNumber )
   {
   case 1:
      return &mPort1MouseBits;
      break;
   case 2:
      return &mPort2MouseBits;
      break;
   default:
      return 0;
      break;
   }
}

void QtYabause::clearMouseBits()
{
   mPort1MouseBits.clear();
   mPort2MouseBits.clear();
}

QString QtYabause::DefaultPaths::Screenshots() { return getDataDirPath() + "/screenshots"; }

QString QtYabause::DefaultPaths::Cartridge() { return getDataDirPath(); }
