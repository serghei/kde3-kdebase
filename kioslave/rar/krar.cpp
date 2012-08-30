/***************************************************************************
 *   Copyright (C) 2006 by Raul Fernandes                                  *
 *   rgfbr@yahoo.com.br                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "krar.h"

#include <qfile.h>
#include <qdir.h>
#include <qdatetime.h>
#include <qtextcodec.h>

#include <kdebug.h>

KRar::KRar( const QString &filename )
  : KArchive( 0L )
{
  m_filename = filename;
  m_dirs = 0;
  m_files = 0;
  m_uncompressed = 0;
  m_compressed = 0;
  m_ratio = 0;
  m_volattr = false;
  volnum = -1;
  m_lock = false;
  m_solid = false;
  m_authenticity = false;
}


KRar::~KRar()
{
}


/*!
    \fn KRar::openArchive( int mode )
 */
bool KRar::openArchive( int mode )
{
  kdDebug() << "KRar::openArchive()" << endl;
  switch( mode )
  {
    case IO_WriteOnly:
    case IO_ReadWrite:
      // Mode not supported
      return false;
    case IO_ReadOnly:
      if ( !m_filename.isEmpty() )
      {
        setDevice( new QFile( m_filename ) );
        if ( !device()->open( mode ) ) return false;
      }
      break;
    default:
      return false;
  }
  QIODevice* dev = device();
  if( !dev ) return false;

  // Marker block
  // A valid RAR archive should have a marker block
  char marker_block[8];
  int a;
  a = dev->readBlock( marker_block, 7 );
  if( a < 7 ) return false;
  if( !memcmp( marker_block, "\x52\x61\x72\x21\x1a\x07\x00", 8 ) ) return false;

  char header[8];
  char fileHeader[25];
  Q_UINT16 fileNameSize;
  KArchiveFile *file;
  Q_UINT32 pos;

  // Archive block
  // The second block should be a archive block
  a = dev->readBlock( header, 7 );
  if( header[2] != '\x73' ) return false;
  m_volattr = header[3] & '\x01';
  m_lock = header[3] & '\x04';
  m_solid = header[3] & '\x08';
  m_authenticity = header[3] & '\x20';
  Q_UINT16 size = (uchar)header[5] | (uchar)header[6] << 8;
  Q_UINT32 fileSize;
  int second;
  int minute;
  int hour;
  int day;
  int month;
  int year;
  QDateTime *time;
  QString filename;
  a = dev->at(size+7);
  mode_t mode;

  // Other blocks
  do
  {
    pos = dev->at();
    fileNameSize = 0;
    fileSize = 0;
    size = 0;
    a = dev->readBlock( header, 7 );
    // TODO: read blocks other than file
    if( header[2] == '\x74' ) // File header
    {
      size = (uchar)header[5] | (uchar)header[6] << 8;
      a = dev->readBlock( fileHeader, 25 );
      fileNameSize = fileHeader[19] | fileHeader[20] << 8;

      char fileName[fileNameSize + 1];
      a = dev->readBlock( fileName, fileNameSize );
      fileName[fileNameSize] = '\0';
      filename = fileName;
      filename.replace( "\\", "/" );

      // Date and time
      second = (uchar)fileHeader[13] & '\x1f';
      minute = ((uchar)fileHeader[13] & '\xe0') >> 5 | ((uchar)fileHeader[14] & '\x03') << 3;
      hour = ((uchar)fileHeader[14] & '\xfc') >> 3;
      day = (uchar)fileHeader[15] & '\x1f';
      month = ((uchar)fileHeader[15] & '\xe0') >> 5 | ((uchar)fileHeader[16] & '\x01') << 3;
      year = ((uchar)fileHeader[16] & '\xfe') >> 1;
      time = new QDateTime( QDate( (1980 + year), month, day), QTime( hour, minute, second ) );

      // Uncompressed file size
      fileSize = (uchar)fileHeader[4] | (uchar)fileHeader[5] << 8 | (uchar)fileHeader[6] << 16 | (uchar)fileHeader[7] << 24;

        // fileHeader[8] - Operating system used for archiving
        // 0 - MS DOS
        // 1 - OS/2
        // 2 - Win32
        // 3 - Unix
/*
            In Windows version is also possible to use symbols D, S, H,
            A and R instead of a digital mask to denote directories
            and files with system, hidden, archive and read-only attributes.
            The order in which the attributes are given is not significant.
            Unix version supports D and V symbols to define directory
            and device attributes.
            Bits
                  A  D     (HS)
            0  0  1  1  0  1  1 0
*/
/*         Unix attributes
                     22                       21
            1  1  0  0  0  0  0  1  1  1  1  1  1  1  1  1
            A  D  L              r  w  x  r  w  x  r  w  x
*/
        mode = 0;
        if( fileHeader[8] == '\x03' )
        {
          // Unix attributes
          // owner permissions
          if(fileHeader[22] & 1) mode |= S_IRUSR;
          if(fileHeader[21] & 128) mode |= S_IWUSR;
          if(fileHeader[21] & 64) mode |= S_IXUSR;
          // group permissions
          if(fileHeader[21] & 32) mode |= S_IRGRP;
          if(fileHeader[21] & 16) mode |= S_IWGRP;
          if(fileHeader[21] & 8) mode |= S_IXGRP;
          // other permissions
          if(fileHeader[21] & 4) mode |= S_IROTH;
          if(fileHeader[21] & 2) mode |= S_IWOTH;
          if(fileHeader[21] & 1) mode |= S_IXOTH;
        }else{
          // Convert the filename from codec used by windows port of RAR
          QTextCodec *codec = QTextCodec::codecForName("IBM 850");
          filename = codec->toUnicode( filename.ascii() );
          // TODO: set the permissions to attributes for windows files
          mode = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH;
        }
        if( (fileHeader[8] == '\x03' && fileHeader[22] & '\x40') || (fileHeader[8] != '\x03' && fileHeader[21] & '\x10') )
        {
          m_dirs++;
          mode |= S_IFDIR;
          // TODO: set the permissions to directories
          if( !rootDir()->entry( filename ) )
            findOrCreate( filename );
        }
        else{
          m_files++;
          m_uncompressed += fileSize;
          // TODO: set the permissions to links
          //if(fileHeader[22] & 32)
          //{
            // link
            //mode |= S_IFLNK;
            //file = new KArchiveFile( this, filename.right( filename.length() - filename.findRev( '/' ) - 1 ), mode, time->toTime_t(), rootDir()->user(), rootDir()->group(), QString::null, dev->at(), fileSize );
          //}else{
            mode |= S_IFREG;
            file = new KArchiveFile( this, filename.right( filename.length() - filename.findRev( '/' ) - 1 ), mode, time->toTime_t(), rootDir()->user(), rootDir()->group(), QString::null, dev->at(), fileSize );
          //}

          // Test to see if the file is in root directory
          if( filename.find( '/' ) == -1 ) rootDir()->addEntry( file );
          else
          {
            // In some tar files we can find dir/./file => call cleanDirPath
            QString path = QDir::cleanDirPath( filename.left( filename.findRev( '/' ) ) );

            // Ensure container directory exists, create otherwise
            KArchiveDirectory * tdir = findOrCreate( path );
            tdir->addEntry(file);
          }
        }

      // Compressed file size
      fileSize = (uchar)fileHeader[0] | (uchar)fileHeader[1] << 8 | (uchar)fileHeader[2] << 16 | (uchar)fileHeader[3] << 24;
      dev->at( pos + size + fileSize );
      m_compressed += fileSize;
    }else if( header[2] == '\x75' ) // Comment header
    {
      kdDebug() << "Comment header" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      if( header[3] & 0x8000)
      {
        a = dev->readBlock( header, 4 );
        fileSize = (uchar)header[0] | (uchar)header[1] << 8 | (uchar)header[2] << 16 | (uchar)header[3] << 24;
      }else fileSize = 0;
      dev->at( pos + size + fileSize );
    }else if( header[2] == '\x76' ) // Extra information
    {
      kdDebug() << "Extra information" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      if( header[3] & 0x8000)
      {
        a = dev->readBlock( header, 4 );
        fileSize = (uchar)header[0] | (uchar)header[1] << 8 | (uchar)header[2] << 16 | (uchar)header[3] << 24;
      }else fileSize = 0;
      dev->at( pos + size + fileSize );
    }else if( header[2] == '\x77' ) // Subblock
    {
      kdDebug() << "Subblock" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      a = dev->readBlock( header, 4 );
      fileSize = (uchar)header[0] | (uchar)header[1] << 8 | (uchar)header[2] << 16 | (uchar)header[3] << 24;
      dev->at( pos + size + fileSize );
    }else if( header[2] == '\x78' ) // Recovery record
    {
      kdDebug() << "Recovery record" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      a = dev->readBlock( header, 4 );
      fileSize = (uchar)header[0] | (uchar)header[1] << 8 | (uchar)header[2] << 16 | (uchar)header[3] << 24;
      dev->at( pos + size + fileSize );
    }else if( header[2] == '\x79' )
    {
      kdDebug() << "SIGN_HEAD" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      if( header[3] & 0x8000)
      {
        a = dev->readBlock( header, 4 );
        fileSize = (uchar)header[0] | (uchar)header[1] << 8 | (uchar)header[2] << 16 | (uchar)header[3] << 24;
      }else fileSize = 0;
      dev->at( pos + size + fileSize );
    }else if( header[2] == '\x7a' )
    {
      kdDebug() << "NEWSUB_HEAD" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      a = dev->readBlock( header, 4 );
      fileSize = (uchar)header[0] | (uchar)header[1] << 8 | (uchar)header[2] << 16 | (uchar)header[3] << 24;
      dev->at( pos + size + fileSize );
    }else if( header[2] == '\x7b' )
    {
      kdDebug() << "ENDARC_HEAD" << endl;
      size = (uchar)header[5] | (uchar)header[6] << 8;
      if( m_volattr )
      {
        a = dev->readBlock( header, 6 );
        volnum = (uchar)header[4] | (uchar)header[5] << 8; // volume number
      }
      dev->at( pos + size );
    }else if( header[2] != '\x7b' )
    {
      kdDebug() << "No known header" << endl;
      return false;
    }
  }while( !dev->atEnd() );

  m_ratio = ( m_compressed * 100 / m_uncompressed );
  return true;
}


/*!
    \fn KRar::closeArchive()
 */
bool KRar::closeArchive()
{
  device()->close();
  return true;
}


/*!
    \fn KRar::doneWriting( uint size )
 */
bool KRar::doneWriting( uint /*size*/ )
{
  return false;
}


/*!
    \fn KRar::writeDir( const QString& name, const QString& user, const QString& group )
 */
bool KRar::writeDir( const QString& /*name*/, const QString& /*user*/, const QString& /*group*/ )
{
  return false;
}


/*!
    \fn KRar::prepareWriting( const QString& name, const QString& user, const QString& group, uint size )
 */
bool KRar::prepareWriting( const QString& /*name*/, const QString& /*user*/, const QString& /*group*/, uint /*size*/ )
{
  return false;
}
