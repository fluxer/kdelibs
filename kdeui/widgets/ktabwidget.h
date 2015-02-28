/* This file is part of the KDE libraries
    Copyright (C) 2003 Stephan Binner <binner@kde.org>
    Copyright (C) 2003 Zack Rusin <zack@kde.org>
    Copyright (C) 2009 Urs Wolfer <uwolfer @ kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KTABWIDGET_H
#define KTABWIDGET_H

#include <kdeui_export.h>

#include <QtGui/QTabWidget>

class QTab;

/**
 * \brief A widget containing multiple tabs
 *
 * It extends the Qt QTabWidget, providing extra optionally features such as close buttons when you hover
 * over the icon in the tab, and also adds functionality such as responding to mouse wheel scroll events to switch
 * the active tab.
 *
 * It is recommended to use KTabWidget instead of QTabWidget unless you have a good reason not to.
 *
 * See also the QTabWidget documentation.
 *
 * \image html ktabwidget.png "KDE Tab Widget"
 */
class KDEUI_EXPORT KTabWidget : public QTabWidget //krazy:exclude=qclasses
{
    Q_OBJECT
    Q_PROPERTY( bool automaticResizeTabs READ automaticResizeTabs WRITE setAutomaticResizeTabs )

  public:

    /**
     * Creates a new tab widget.
     *
     * @param parent The parent widgets.
     * @param flags The Qt window flags @see QWidget.
     */
    explicit KTabWidget( QWidget *parent = 0, Qt::WindowFlags flags = 0 );

    /**
     * Destroys the tab widget.
     */
    virtual ~KTabWidget();

    /**
     * Set the tab of the given widget to \a color.
     * This is simply a convenience method for QTabBar::setTabTextColor.
     */
    void setTabTextColor( int index, const QColor& color );

    /**
     * Returns the tab color for the given widget.
     * This is simply a convenience method for QTabBar::tabTextColor.
     */
    QColor tabTextColor( int index ) const;

    /**
     * Returns true if calling setTitle() will resize tabs
     * to the width of the tab bar.
     */
    bool automaticResizeTabs() const;

    /**
     * If \a hide is true, the tabbar is hidden along with any corner
     * widgets.
     */
    void setTabBarHidden( bool hide );

    /**
     * Returns true if the tabbar was hidden by a call to setTabBarHidden( true ).
     * Returns false if the widget itself is hidden, but no call to setTabBarHidden( true )
     * has been made.
     */
    bool isTabBarHidden() const;

    /**
      Reimplemented for internal reasons.
     *
    virtual void insertTab( QWidget *, const QString &, int index = -1 );

    *!
      Reimplemented for internal reasons.
     *
    virtual void insertTab( QWidget *child, const QIcon& iconset,
                            const QString &label, int index = -1 );
    *!
      Reimplemented for internal reasons.
    *
    virtual void insertTab( QWidget *, QTab *, int index = -1 );*/

    /**
     * Reimplemented for internal reasons.
     */
    QString tabText( int ) const; // but it's not virtual...


    /**
     * Reimplemented for internal reasons.
     */
    void setTabText( int , const QString & );

    using QTabWidget::tabBar;

  public Q_SLOTS:
    /**
     * Move a widget's tab from first to second specified index and emit
     * signal movedTab( int, int ) afterwards.
     */
    virtual void moveTab( int, int );

    /**
     * Removes the widget, reimplemented for
     * internal reasons (keeping labels in sync).
     * @deprecated since 4.0
     */
    virtual QT_MOC_COMPAT void removePage ( QWidget * w );

    /**
     * Removes the widget, reimplemented for
     * internal reasons (keeping labels in sync).
     */
    virtual void removeTab(int index); // but it's not virtual in QTabWidget...

    /**
     * If \a enable is true, tabs will be resized to the width of the tab bar.
     *
     * Does not work reliably with "QTabWidget* foo=new KTabWidget()" and if
     * you change tabs via the tabbar or by accessing tabs directly.
     */
    void setAutomaticResizeTabs( bool enable );

  Q_SIGNALS:
    /**
     * Connect to this and set accept to true if you can and want to decode the event.
     */
    void testCanDecode(const QDragMoveEvent *e, bool &accept /* result */);

    /**
     * Received an event in the empty space beside tabbar. Usually creates a new tab.
     * This signal is only possible after testCanDecode and positive accept result.
     */
    void receivedDropEvent( QDropEvent * );

    /**
     * Received an drop event on given widget's tab.
     * This signal is only possible after testCanDecode and positive accept result.
     */
    void receivedDropEvent( QWidget *, QDropEvent * );

    /**
     * Request to start a drag operation on the given tab.
     */
    void initiateDrag( QWidget * );

    /**
     * The right mouse button was pressed over empty space besides tabbar.
     */
    void contextMenu( const QPoint & );

    /**
     * The right mouse button was pressed over a widget.
     */
    void contextMenu( QWidget *, const QPoint & );


    /**
     * A double left mouse button click was performed over empty space besides tabbar.
     * The signal is emitted on the second press of the mouse button, before the release.
     */
    void mouseDoubleClick();

    /**
     * A double left mouse button click was performed over the widget.
     * The signal is emitted on the second press of the mouse button, before the release.
     */
    void mouseDoubleClick( QWidget * );

    /**
     * A middle mouse button click was performed over empty space besides tabbar.
     * The signal is emitted on the release of the mouse button.
     */
    void mouseMiddleClick();

    /**
     * A middle mouse button click was performed over the widget.
     * The signal is emitted on the release of the mouse button.
     */
    void mouseMiddleClick( QWidget * );

    /**
     * The close button of a widget's tab was clicked. This signal is
     * only possible after you have called setCloseButtonEnabled( true ).
     */
    void closeRequest( QWidget * );

  protected:
    virtual void mouseDoubleClickEvent( QMouseEvent* );
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseReleaseEvent( QMouseEvent* );
    virtual void dragEnterEvent( QDragEnterEvent* );
    virtual void dragMoveEvent( QDragMoveEvent* );
    virtual void dropEvent( QDropEvent* );
    int tabBarWidthForMaxChars( int );
#ifndef QT_NO_WHEELEVENT
    virtual void wheelEvent( QWheelEvent* );
#endif
    virtual void resizeEvent( QResizeEvent* );
    virtual void tabInserted( int );
    virtual void tabRemoved ( int );

  protected Q_SLOTS:
    virtual void receivedDropEvent( int, QDropEvent* );
    virtual void initiateDrag( int );
    virtual void contextMenu( int, const QPoint& );
    virtual void mouseDoubleClick( int );
    virtual void mouseMiddleClick( int );
    virtual void closeRequest( int );
#ifndef QT_NO_WHEELEVENT
    virtual void wheelDelta( int );
#endif

  private:
    class Private;
    Private * const d;

    Q_PRIVATE_SLOT(d, void slotTabMoved(int, int))
};

#endif
