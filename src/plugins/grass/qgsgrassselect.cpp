/***************************************************************************
    qgsgrassselect.cpp  -  Select GRASS layer dialog
                             -------------------
    begin                : March, 2004
    copyright            : (C) 2004 by Radim Blazek
    email                : blazek@itc.it
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsgrassselect.h"
#include "qgsgrass.h"

#include "qgslogger.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

extern "C"
{
#if GRASS_VERSION_MAJOR < 7
#include <grass/Vect.h>
#else
#include <grass/vector.h>
#define BOUND_BOX bound_box
#endif
}


QgsGrassSelect::QgsGrassSelect( QWidget *parent, int type )
    : QDialog( parent )
    , QgsGrassSelectBase()
    , selectedType( 0 )
{
  QgsDebugMsg( QString( "QgsGrassSelect() type = %1" ).arg( type ) );

  setupUi( this );
  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( accept() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );

  if ( first )
  {
    if ( QgsGrass::activeMode() )
    {
      lastGisdbase = QgsGrass::getDefaultGisdbase();
      lastLocation = QgsGrass::getDefaultLocation();
      lastMapset = QgsGrass::getDefaultMapset();
    }
    else
    {
      QSettings settings;
      lastGisdbase = settings.value( QStringLiteral( "/GRASS/lastGisdbase" ) ).toString();
      //check we got something from qsettings otherwise default to users home dir
      if ( lastGisdbase.isEmpty() )
      {
        QDir home = QDir::home();
        lastGisdbase = QString( home.path() );
      }
      lastMapset = settings.value( QStringLiteral( "/GRASS/lastMapset" ) ).toString();
    }
    first = false;
  }
  QgsGrassSelect::type = type;

  switch ( type )
  {
    case QgsGrassSelect::Vector:
      setWindowTitle( tr( "Select GRASS Vector Layer" ) );
      break;

    case QgsGrassSelect::Raster:
      /* Remove layer combo box */
      Layer->hide();
      elayer->hide();
      setWindowTitle( tr( "Select GRASS Raster Layer" ) );
      break;

    case QgsGrassSelect::MapCalc:
      /* Remove layer combo box */
      Layer->hide();
      elayer->hide();
      setWindowTitle( tr( "Select GRASS mapcalc schema" ) );
      break;

    case QgsGrassSelect::MapSet:
      Layer->hide();
      elayer->hide();
      MapName->hide();
      emap->hide();
      setWindowTitle( tr( "Select GRASS Mapset" ) );
      break;
  }

  egisdbase->setText( lastGisdbase );

  setLocations();
  adjustSize();
}

QgsGrassSelect::~QgsGrassSelect()
{
}

bool QgsGrassSelect::first = true;
QString QgsGrassSelect::lastGisdbase;
QString QgsGrassSelect::lastLocation;
QString QgsGrassSelect::lastMapset;
QString QgsGrassSelect::lastVectorMap;
QString QgsGrassSelect::lastRasterMap;
QString QgsGrassSelect::lastLayer;
QString QgsGrassSelect::lastMapcalc;

void QgsGrassSelect::setLocations()
{
  elocation->clear();
  emapset->clear();
  emap->clear();
  elayer->clear();

  QDir d = QDir( egisdbase->text() );

  int idx = 0;
  int sel = -1;
  // Add all subdirs containing PERMANENT/DEFAULT_WIND
  for ( unsigned int i = 0; i < d.count(); i++ )
  {
    if ( d[i] == QLatin1String( "." ) || d[i] == QLatin1String( ".." ) )
      continue;

    QString ldpath = egisdbase->text() + "/" + d[i];

    if ( QgsGrass::versionMajor() > 6 || QgsGrass::versionMinor() > 0 )
    {
      if ( !G_is_location( ldpath.toLocal8Bit().constData() ) )
        continue;
    }
    else
    {
      QString chf = egisdbase->text() + "/" + d[i] + "/PERMANENT/DEFAULT_WIND";
      if ( !QFile::exists( chf ) )
        continue;
    }

    // if type is MAPSET check also if at least one mapset owned by user exists
    if ( QgsGrassSelect::type == QgsGrassSelect::MapSet )
    {
      bool exists = false;

      QDir ld = QDir( ldpath );

      for ( unsigned int j = 0; j < ld.count(); j++ )
      {
        if ( !QgsGrass::isMapset( ldpath + "/" + ld[j] ) )
          continue;

        QFileInfo info( ldpath + "/" + ld[j] );
        if ( !info.isWritable() )
          continue;

        // TODO: check if owner == user: how to get uer name in QT

        exists = true;
        break;
      }

      if ( !exists )
        continue;
    }

    elocation->addItem( QString( d[i] ) );
    if ( QString( d[i] ) == lastLocation )
    {
      sel = idx;
    }
    idx++;
  }
  if ( sel >= 0 )
  {
    elocation->setCurrentIndex( sel );
  }
  buttonBox->button( QDialogButtonBox::Ok )->setDefault( true );
  GisdbaseBrowse->setDefault( elocation->count() == 0 );

  setMapsets();
}


void QgsGrassSelect::setMapsets()
{
  QgsDebugMsg( "setMapsets()" );

  emapset->clear();
  emap->clear();
  elayer->clear();

  if ( elocation->count() < 1 )
    return;

  // Location directory
  QString ldpath = egisdbase->text() + "/" + elocation->currentText();
  QDir ld = QDir( ldpath );

  int idx = 0;
  int sel = -1;

  // Go through all subdirs and add all subdirs from vector/
  for ( unsigned int i = 0; i < ld.count(); i++ )
  {
    if ( QgsGrass::isMapset( ldpath + "/" + ld[i] ) )
    {
      emapset->addItem( ld[i] );
      if ( ld[i] == lastMapset )
      {
        sel = idx;
      }
      idx++;
    }
  }
  if ( sel >= 0 )
  {
    emapset->setCurrentIndex( sel );
  }
  if ( emap->isHidden() )
  {
    buttonBox->button( QDialogButtonBox::Ok )->setDefault( emapset->count() > 0 );
  }

  setMaps();
}

void QgsGrassSelect::setMaps()
{
  QgsDebugMsg( "setMaps()" );

  // Replaced by text box to enable wild cards
  emap->clear();
  elayer->clear();

  if ( emapset->count() < 1 )
    return;

  // Mapset directory
  QString ldpath = egisdbase->text() + "/" + elocation->currentText() + "/" + emapset->currentText();

  int idx = 0;
  int sel = -1;

  if ( type == Vector ) // vector
  {
    QStringList list = QgsGrass::vectors( egisdbase->text(),
                                          elocation->currentText(), emapset->currentText() );

    for ( int j = 0; j < list.count(); j++ )
    {
      emap->addItem( list[j] );
      if ( list[j] == lastVectorMap )
        sel = idx;
      idx++;
    }

  }
  else if ( type == Raster )
  {
    /* add cells */
    QStringList list = QgsGrass::rasters( egisdbase->text(),
                                          elocation->currentText(), emapset->currentText() );

    for ( int j = 0; j < list.count(); j++ )
    {
      emap->addItem( list[j] );
      if ( list[j] == lastRasterMap )
        sel = idx;
      idx++;
    }

    /* add groups */
    // TODO add QgsGrass::groups ( use G_list( G_ELEMENT_GROUP) )
    QDir md = QDir( ldpath + "/group/" );
    md.setFilter( QDir::Dirs );

    for ( unsigned int j = 0; j < md.count(); j++ )
    {
      if ( md[j] == QLatin1String( "." ) || md[j] == QLatin1String( ".." ) )
        continue;

      QString m = QString( md[j] + " (GROUP)" );
      emap->addItem( m );
      if ( m == lastRasterMap )
      {
        sel = idx;
      }
      idx++;
    }
  }
  else if ( type == MapCalc )
  {
    QDir md = QDir( ldpath + "/mapcalc/" );
    md.setFilter( QDir::Files );

    for ( unsigned int j = 0; j < md.count(); j++ )
    {
      QString m = QString( md[j] );
      emap->addItem( m );
      if ( m == lastMapcalc )
      {
        sel = idx;
      }
      idx++;
    }
  }
  if ( sel >= 0 )
  {
    emap->setCurrentIndex( sel );
  }
#if 0
  else
  {
    emap->clearEdit(); // set box line empty
  }
#endif
  if ( !emap->isHidden() )
  {
    buttonBox->button( QDialogButtonBox::Ok )->setDefault( emap->count() > 0 );
  }

  setLayers();
}

void QgsGrassSelect::setLayers()
{
  QgsDebugMsg( "setLayers()" );

  elayer->clear();

  if ( type != Vector )
    return;
  if ( emap->count() < 1 )
    return;

  QStringList layers;
  try
  {
    layers  = QgsGrass::vectorLayers( egisdbase->text(),
                                      elocation->currentText(), emapset->currentText(),
                                      emap->currentText().toUtf8() );
  }
  catch ( QgsGrass::Exception &e )
  {
    QgsDebugMsg( e.what() );
    return;
  }

  int idx = 0;
  int sel = -1;
  for ( int i = 0; i < layers.count(); i++ )
  {
    elayer->addItem( layers[i] );
    if ( layers[i] == lastLayer )
      sel = idx;
    idx++;
  }

  // if last used layer has not been found
  // make default a map from layer 1
  if ( sel == -1 )
  {
    for ( int j = 0; j < layers.count(); j++ )
    {
      if ( layers[j].at( 0 ) == '1' )
      {
        sel = j;
        break;
      }
    }
  }

  if ( sel >= 0 )
  {
    elayer->setCurrentIndex( sel );
  }
  else
  {
    elayer->clearEditText(); // set box line empty
  }

  if ( elayer->count() == 1 )
  {
    elayer->setDisabled( true );
  }
  else
  {
    elayer->setDisabled( false );
  }
}

void QgsGrassSelect::on_GisdbaseBrowse_clicked()
{
  QString Gisdbase = QFileDialog::getExistingDirectory( this,
                     tr( "Choose existing GISDBASE" ), egisdbase->text() );

  if ( !Gisdbase.isNull() )
  {
    egisdbase->setText( Gisdbase );
  }
}

void QgsGrassSelect::accept()
{
  gisdbase = egisdbase->text();
  lastGisdbase = QString( gisdbase );

  if ( elocation->count() == 0 )
  {
    QString msg = tr( "Wrong GISDBASE, no locations available." );
    QMessageBox::warning( this, tr( "Wrong GISDBASE" ), msg );
    return;
  }

  //write to qgsettings as gisdbase seems to be valid
  QSettings settings;
  settings.setValue( QStringLiteral( "/GRASS/lastGisdbase" ), lastGisdbase );

  location = elocation->currentText();
  lastLocation = location;

  mapset = emapset->currentText();
  lastMapset = mapset;

  settings.setValue( QStringLiteral( "/GRASS/lastMapset" ), lastMapset );

  map = emap->currentText().trimmed();

  if ( type != QgsGrassSelect::MapSet && map.isEmpty() )
  {
    QString msg = tr( "Select a map." );
    QMessageBox::warning( 0, tr( "No map" ), msg );
    return;
  }

  if ( type == QgsGrassSelect::Vector )
  {
    if ( elayer->count() == 0 )
    {
      QMessageBox::warning( 0, tr( "No layer" ),
                            tr( "No layers available in this map" ) );
      return;
    }
    lastVectorMap = map;
    layer = elayer->currentText().trimmed();
    lastLayer = layer;
  }
  else if ( type == QgsGrassSelect::Raster )
  {
    lastRasterMap = map;
    if ( map.indexOf( QLatin1String( " (GROUP)" ) ) != -1 )
    {
      map.remove( QStringLiteral( " (GROUP)" ) );
      selectedType = QgsGrassSelect::Group;
    }
    else
    {
      selectedType = QgsGrassSelect::Raster;
    }
  }
  else if ( type == QgsGrassSelect::MapCalc )
  {
    lastMapcalc = map;
  }
  QDialog::accept();
}
