/***************************************************************************
    qgswkbptr.h
    ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWKBPTR_H
#define QGSWKBPTR_H

#include "qgis_core.h"
#include "qgswkbtypes.h"
#include "qgsapplication.h"
#include "qgis.h"
#include "qgsexception.h"
#include "qpolygon.h"

/** \ingroup core
 *  * Custom exception class for Wkb related exceptions.
 *   */
class CORE_EXPORT QgsWkbException : public QgsException
{
  public:
    QgsWkbException( QString const &what ) : QgsException( what ) {}
};


/** \ingroup core
 * \class QgsWkbPtr
 * \note not available in Python bindings
 */

class CORE_EXPORT QgsWkbPtr
{
    mutable unsigned char *mP;
    unsigned char *mStart;
    unsigned char *mEnd;

    void verifyBound( int size ) const;

    template<typename T> void read( T& v ) const
    {
      verifyBound( sizeof v );
      memcpy( &v, mP, sizeof v );
      mP += sizeof v;
    }

    template<typename T> void write( T& v ) const
    {
      verifyBound( sizeof v );
      memcpy( mP, &v, sizeof v );
      mP += sizeof v;
    }

    void write( const QByteArray& data ) const
    {
      verifyBound( data.length() );
      memcpy( mP, data.constData(), data.length() );
      mP += data.length();
    }

  public:
    //! Construct WKB pointer from QByteArray
    QgsWkbPtr( QByteArray& wkb );
    QgsWkbPtr( unsigned char *p, int size );

    inline const QgsWkbPtr &operator>>( double &v ) const { read( v ); return *this; }
    inline const QgsWkbPtr &operator>>( float &r ) const { double v; read( v ); r = v; return *this; }
    inline const QgsWkbPtr &operator>>( int &v ) const { read( v ); return *this; }
    inline const QgsWkbPtr &operator>>( unsigned int &v ) const { read( v ); return *this; }
    inline const QgsWkbPtr &operator>>( char &v ) const { read( v ); return *this; }
    inline const QgsWkbPtr &operator>>( QgsWkbTypes::Type &v ) const { read( v ); return *this; }

    inline QgsWkbPtr &operator<<( const double &v ) { write( v ); return *this; }
    inline QgsWkbPtr &operator<<( const float &r ) { double v = r; write( v ); return *this; }
    inline QgsWkbPtr &operator<<( const int &v ) { write( v ); return *this; }
    inline QgsWkbPtr &operator<<( const unsigned int &v ) { write( v ); return *this; }
    inline QgsWkbPtr &operator<<( const char &v ) { write( v ); return *this; }
    inline QgsWkbPtr &operator<<( const QgsWkbTypes::Type &v ) { write( v ); return *this; }
    //! Append data from a byte array
    inline QgsWkbPtr &operator<<( const QByteArray &data ) { write( data ); return *this; }

    inline void operator+=( int n ) { verifyBound( n ); mP += n; }

    inline operator unsigned char *() const { return mP; }
    inline int size() const { return mEnd - mStart; }
    inline int remaining() const { return mEnd - mP; }
    inline int writtenSize() const { return mP - mStart; }
};

/** \ingroup core
 * \class QgsConstWkbPtr
 * \note not available in Python bindings
 */

class CORE_EXPORT QgsConstWkbPtr
{
  protected:
    mutable unsigned char *mP;
    unsigned char *mEnd;
    mutable bool mEndianSwap;
    mutable QgsWkbTypes::Type mWkbType;

    //! Verify bounds
    void verifyBound( int size ) const;

    //! Read a value
    template<typename T> void read( T& v ) const
    {
      verifyBound( sizeof v );
      memcpy( &v, mP, sizeof( v ) );
      mP += sizeof( v );
      if ( mEndianSwap )
        QgsApplication::endian_swap( v );
    }

  public:
    //! Construct WKB pointer from QByteArray
    explicit QgsConstWkbPtr( const QByteArray& wkb );
    QgsConstWkbPtr( const unsigned char *p, int size );
    QgsWkbTypes::Type readHeader() const;

    inline const QgsConstWkbPtr &operator>>( double &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( float &r ) const { double v; read( v ); r = v; return *this; }
    inline const QgsConstWkbPtr &operator>>( int &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( unsigned int &v ) const { read( v ); return *this; }
    inline const QgsConstWkbPtr &operator>>( char &v ) const { read( v ); return *this; }

    //! Read a point
    virtual const QgsConstWkbPtr &operator>>( QPointF &point ) const;
    //! Read a point array
    virtual const QgsConstWkbPtr &operator>>( QPolygonF &points ) const;

    inline void operator+=( int n ) { verifyBound( n ); mP += n; }
    inline void operator-=( int n ) { mP -= n; }

    inline operator const unsigned char *() const { return mP; }
    inline int remaining() const { return mEnd - mP; }
};

#endif // QGSWKBPTR_H
