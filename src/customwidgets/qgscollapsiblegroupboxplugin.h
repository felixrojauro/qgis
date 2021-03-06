/***************************************************************************
   qgscollapsiblegroupboxplugin.h
    --------------------------------------
   Date                 : 25.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSCOLLAPSIBLEGROUPBOXPLUGIN_H
#define QGSCOLLAPSIBLEGROUPBOXPLUGIN_H


#include <QtGlobal>
#if QT_VERSION < 0x050000
#include <QDesignerCustomWidgetCollectionInterface>
#include <QDesignerExportWidget>
#else
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#include <QtUiPlugin/QDesignerExportWidget>
#include "qgis_customwidgets.h"
#endif

class CUSTOMWIDGETS_EXPORT QgsCollapsibleGroupBoxPlugin : public QObject, public QDesignerCustomWidgetInterface
{
    Q_OBJECT
    Q_INTERFACES( QDesignerCustomWidgetInterface )

  public:
    explicit QgsCollapsibleGroupBoxPlugin( QObject *parent = 0 );

  private:
    bool mInitialized;

    // QDesignerCustomWidgetInterface interface
  public:
    QString name() const override;
    QString group() const override;
    QString includeFile() const override;
    QIcon icon() const override;
    bool isContainer() const override;
    QWidget *createWidget( QWidget *parent ) override;
    bool isInitialized() const override;
    void initialize( QDesignerFormEditorInterface *core ) override;
    QString toolTip() const override;
    QString whatsThis() const override;
    QString domXml() const override;
};
#endif // QGSCOLLAPSIBLEGROUPBOXPLUGIN_H
