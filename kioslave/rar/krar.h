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

#ifndef KRAR_H
#define KRAR_H

#include <karchive.h>

/**
@author Raul Fernandes
*/
class KIO_EXPORT KRar : public KArchive
{
public:
    KRar( const QString &filename );

    ~KRar();

    QString fileName() { return m_filename; }
    bool doneWriting( uint size );
    bool writeDir( const QString& name, const QString& user, const QString& group );
    bool prepareWriting( const QString& name, const QString& user, const QString& group, uint size );
    uint dirs() { return m_dirs; }
    uint files() { return m_files; }
    unsigned long long uncompressed() { return m_uncompressed; }
    unsigned long long compressed() { return m_compressed; }
    uint ratio() { return m_ratio; }
    bool isArchiveVolume() { return m_volattr; };
    int volNum() {
      if( m_volattr ) return volnum;
      else return -1;
    }
    bool isLocked() { return m_lock; };
    bool isSolid() { return m_solid; };
    bool hasAuthInfo() { return m_authenticity; };

protected:
    bool openArchive( int mode );
    bool closeArchive();
    uint m_dirs;
    uint m_files;
    unsigned long long m_uncompressed;
    unsigned long long m_compressed;
    uint m_ratio;
    bool m_volattr;
    int volnum;
    bool m_lock;
    bool m_solid;
    bool m_authenticity;

private:
  QString m_filename;
};

#endif
