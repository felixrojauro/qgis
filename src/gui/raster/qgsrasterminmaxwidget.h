/***************************************************************************
                         qgsrasterminmaxwidget.h
                         ---------------------------------
    begin                : July 2012
    copyright            : (C) 2012 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRASTERMINMAXWIDGET_H
#define QGSRASTERMINMAXWIDGET_H

#include "ui_qgsrasterminmaxwidgetbase.h"
#include "qgsrectangle.h"

#include "qgsraster.h"
#include "qgsrasterminmaxorigin.h"
#include "qgscontrastenhancement.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsRasterLayer;

/** \ingroup gui
 * \class QgsRasterMinMaxWidget
 */
class GUI_EXPORT QgsRasterMinMaxWidget: public QWidget, private Ui::QgsRasterMinMaxWidgetBase
{
    Q_OBJECT
  public:
    QgsRasterMinMaxWidget( QgsRasterLayer* theLayer, QWidget *parent = nullptr );

    /** Sets the extent to use for minimum and maximum value calculation.
     * @param theExtent extent in raster layer's CRS
     * @note if a map canvas is set using setMapCanvas(), its extent will take
     * precedence over any extent set using this method.
     */
    void setExtent( const QgsRectangle & theExtent ) { mExtent = theExtent; }

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map extent from the canvas. If a canvas is set it will take precedence over any extent
     * set from calling setExtent().
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.16
     */
    void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas()
     * @see canvasExtent()
     * @note added in QGIS 2.16
     */
    QgsMapCanvas* mapCanvas();

    void setBands( const QList<int> & theBands );

    /** Return the extent selected by the user.
     * Either an empty extent for 'full' or the current visible extent.
    */
    QgsRectangle extent();

    //! Return the selected sample size.
    int sampleSize() { return cboAccuracy->currentIndex() == 0 ? 250000 : 0; }

    //! \brief Set the "source" of min/max values.
    void setFromMinMaxOrigin( const QgsRasterMinMaxOrigin& );

    //! \brief Return a QgsRasterMinMaxOrigin object with the widget values.
    QgsRasterMinMaxOrigin minMaxOrigin();

    //! Hide updated extent choice
    void hideUpdatedExtent();

    //! Load programmaticaly with current values
    void doComputations();

    //! Uncheck cumulative cut, min/max, std-dev radio buttons
    void userHasSetManualMinMaxValues();

    //! Return if the widget is collaped.
    bool isCollapsed() const { return mLoadMinMaxValuesGroupBox->isCollapsed(); }

    //! Set collapsed state of widget
    void setCollapsed( bool b ) { mLoadMinMaxValuesGroupBox->setCollapsed( b ); }

  signals:

    /**
     * Emitted when something on the widget has changed.
     * All widgets will fire this event to notify of an internal change.
     */
    void widgetChanged();

    //! signal emitted when new min/max values are computed from statistics.
    void load( int theBandNo, double theMin, double theMax );

  private slots:

    void on_mUserDefinedRadioButton_toggled( bool );
    void on_mMinMaxRadioButton_toggled( bool b ) { if ( b ) emit widgetChanged(); }
    void on_mStdDevRadioButton_toggled( bool b ) { if ( b ) emit widgetChanged(); }
    void on_mCumulativeCutRadioButton_toggled( bool b ) { if ( b ) emit widgetChanged(); }
    void on_mStatisticsExtentCombo_currentIndexChanged( int ) { emit widgetChanged(); }
    void on_mCumulativeCutLowerDoubleSpinBox_valueChanged( double ) { emit widgetChanged(); }
    void on_mCumulativeCutUpperDoubleSpinBox_valueChanged( double ) { emit widgetChanged(); }
    void on_mStdDevSpinBox_valueChanged( double ) { emit widgetChanged(); }
    void on_cboAccuracy_currentIndexChanged( int ) { emit widgetChanged(); }

  private:
    QgsRasterLayer* mLayer;
    QList<int> mBands;
    QgsRectangle mExtent;

    QgsMapCanvas* mCanvas;

    bool mLastRectangleValid;
    QgsRectangle mLastRectangle;
    QgsRasterMinMaxOrigin mLastMinMaxOrigin;

    bool mBandsChanged;
};

#endif // QGSRASTERMINMAXWIDGET_H
