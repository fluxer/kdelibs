 /* This file is part of the KDE libraries
     Copyright
     (C) 2000 Reginald Stadlbauer (reggie@kde.org)
     (C) 1997 Stephan Kulow (coolo@kde.org)
     (C) 1997-2000 Sven Radej (radej@kde.org)
     (C) 1997-2000 Matthias Ettrich (ettrich@kde.org)
     (C) 1999 Chris Schlaeger (cs@kde.org)
     (C) 2002 Joseph Wenninger (jowenn@kde.org)
     (C) 2005-2006 Hamish Rodda (rodda@kde.org)

     This library is free software; you can redistribute it and/or
     modify it under the terms of the GNU Library General Public
     License version 2 as published by the Free Software Foundation.

     This library is distributed in the hope that it will be useful,
     but WITHOUT ANY WARRANTY; without even the implied warranty of
     MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
     Library General Public License for more details.

     You should have received a copy of the GNU Library General Public License
     along with this library; see the file COPYING.LIB.  If not, write to
     the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
     Boston, MA 02110-1301, USA.
 */

#include "kxmlguiwindow.h"
#include "kmainwindow_p.h"
#include "kactioncollection.h"
#include "kmainwindowiface_p.h"
#include "ktoolbarhandler_p.h"
#include "kxmlguifactory.h"
#include "kcmdlineargs.h"
#include "ktoggleaction.h"
#include "ksessionmanager.h"
#include "kstandardaction.h"

#include <config.h>

#include <QDesktopWidget>
#include <QDockWidget>
#include <QtXml/qdom.h>
#include <QtGui/qevent.h>
#include <QtGui/QMenuBar>
#include <QtGui/QLayout>
#include <QtGui/QStyle>
#include <QtGui/QWidget>
#include <QtDBus/QDBusConnection>
#include <kaction.h>
#include <kapplication.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kedittoolbar.h>
#include <khelpmenu.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kstatusbar.h>
#include <ktoolbar.h>
#include <kwindowsystem.h>
#include <kconfiggroup.h>

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

class KXmlGuiWindowPrivate : public KMainWindowPrivate {
public:
    void _k_slotFactoryMakingChanges(bool b)
    {
        // While the GUI factory is adding/removing clients,
        // don't let KMainWindow think those are changes made by the user
        // #105525
        letDirtySettings = !b;
    }

    bool showHelpMenu:1;
    QSize defaultSize;

    KDEPrivate::ToolBarHandler *toolBarHandler;
    KToggleAction *showStatusBarAction;
    QPointer<KEditToolBar> toolBarEditor;
    KXMLGUIFactory *factory;
};

KXmlGuiWindow::KXmlGuiWindow( QWidget* parent, Qt::WindowFlags f )
    : KMainWindow(*new KXmlGuiWindowPrivate, parent, f), KXMLGUIBuilder( this )
{
    K_D(KXmlGuiWindow);
    d->showHelpMenu = true;
    d->toolBarHandler = 0;
    d->showStatusBarAction = 0;
    d->factory = 0;
    new KMainWindowInterface(this);
}


QAction *KXmlGuiWindow::toolBarMenuAction()
{
    K_D(KXmlGuiWindow);
    if ( !d->toolBarHandler )
        return 0;

    return d->toolBarHandler->toolBarMenuAction();
}


void KXmlGuiWindow::setupToolbarMenuActions()
{
    K_D(KXmlGuiWindow);
    if ( d->toolBarHandler )
        d->toolBarHandler->setupActions();
}


KXmlGuiWindow::~KXmlGuiWindow()
{
    K_D(KXmlGuiWindow);
    if (d->toolBarHandler)
        d->toolBarHandler->deleteLater();
    delete d->factory;
}

bool KXmlGuiWindow::event( QEvent* ev )
{
    bool ret = KMainWindow::event(ev);
    if (ev->type()==QEvent::Polish) {
        QDBusConnection::sessionBus().registerObject(dbusName() + "/actions", actionCollection(),
                                                     QDBusConnection::ExportScriptableSlots |
                                                     QDBusConnection::ExportScriptableProperties |
                                                     QDBusConnection::ExportNonScriptableSlots |
                                                     QDBusConnection::ExportNonScriptableProperties |
                                                     QDBusConnection::ExportChildObjects);
    }
    return ret;
}

void KXmlGuiWindow::setHelpMenuEnabled(bool showHelpMenu)
{
    K_D(KXmlGuiWindow);
    d->showHelpMenu = showHelpMenu;
}

bool KXmlGuiWindow::isHelpMenuEnabled() const
{
    K_D(const KXmlGuiWindow);
    return d->showHelpMenu;
}

KXMLGUIFactory *KXmlGuiWindow::guiFactory()
{
    K_D(KXmlGuiWindow);
    if (!d->factory) {
        d->factory = new KXMLGUIFactory( this, this );
        connect(d->factory, SIGNAL(makingChanges(bool)),
                this, SLOT(_k_slotFactoryMakingChanges(bool)));
    }
    return d->factory;
}

void KXmlGuiWindow::configureToolbars()
{
    K_D(KXmlGuiWindow);
    KConfigGroup cg(KGlobal::config(), QString());
    saveMainWindowSettings(cg);
    if (!d->toolBarEditor) {
      d->toolBarEditor = new KEditToolBar(guiFactory(), this);
      d->toolBarEditor->setAttribute(Qt::WA_DeleteOnClose);
      connect(d->toolBarEditor, SIGNAL(newToolBarConfig()), SLOT(saveNewToolbarConfig()));
    }
    d->toolBarEditor->show();
}

void KXmlGuiWindow::saveNewToolbarConfig()
{
    // createGUI(xmlFile()); // this loses any plugged-in guiclients, so we use remove+add instead.

    guiFactory()->removeClient(this);
    guiFactory()->addClient(this);

    KConfigGroup cg(KGlobal::config(), QString());
    applyMainWindowSettings(cg);
}

void KXmlGuiWindow::setupGUI( StandardWindowOptions options, const QString & xmlfile ) {
    setupGUI(QSize(), options, xmlfile);
}

void KXmlGuiWindow::setupGUI( const QSize & defaultSize, StandardWindowOptions options, const QString & xmlfile ) {
    K_D(KXmlGuiWindow);

    if( options & Keys ){
        KStandardAction::keyBindings(guiFactory(),
                    SLOT(configureShortcuts()), actionCollection());
    }

    if( (options & StatusBar) && statusBar() ){
        createStandardStatusBarAction();
    }

    if( options & ToolBar ){
        setStandardToolBarMenuEnabled( true );
        KStandardAction::configureToolbars(this,
                      SLOT(configureToolbars()), actionCollection());
    }

    d->defaultSize = defaultSize;

    if( options & Create ){
        createGUI(xmlfile);
    }

    if (initialGeometrySet()) {
        // Do nothing...
    }
    else if (d->defaultSize.isValid()) {
        resize(d->defaultSize);
    }
    else if (isHidden()) {
        adjustSize();
    }

    if( options & Save ){
        const KConfigGroup cg(autoSaveConfigGroup());
        if (cg.isValid()) {
            setAutoSaveSettings(cg);
        } else {
            setAutoSaveSettings();
        }
    }
}
void KXmlGuiWindow::createGUI( const QString &xmlfile )
{
    K_D(KXmlGuiWindow);
    // disabling the updates prevents unnecessary redraws
    //setUpdatesEnabled( false );

    // just in case we are rebuilding, let's remove our old client
    guiFactory()->removeClient( this );

    // make sure to have an empty GUI
    QMenuBar* mb = menuBar();
    if ( mb )
        mb->clear();

    qDeleteAll( toolBars() ); // delete all toolbars

    // don't build a help menu unless the user ask for it
    if (d->showHelpMenu) {
        delete d->helpMenu;
        // we always want a help menu
        d->helpMenu = new KHelpMenu(this, componentData().aboutData(), true, actionCollection());
    }

    const QString windowXmlFile = xmlfile.isNull() ? componentData().componentName() + "ui.rc" : xmlfile;

    // Help beginners who call setXMLFile and then setupGUI...
    if (!xmlFile().isEmpty() && xmlFile() != windowXmlFile) {
        kWarning() << "You called setXMLFile(" << xmlFile() << ") and then createGUI or setupGUI,"
                   << "which also calls setXMLFile and will overwrite the file you have previously set.\n"
                   << "You should call createGUI("<<xmlFile()<<") or setupGUI(<options>,"<<xmlFile()<<") instead.";
    }

    // we always want to load in our global standards file
    loadStandardsXmlFile();

    // now, merge in our local xml file.
    setXMLFile(windowXmlFile, true);

    // make sure we don't have any state saved already
    setXMLGUIBuildDocument( QDomDocument() );

    // do the actual GUI building
    guiFactory()->reset();
    guiFactory()->addClient( this );

    //  setUpdatesEnabled( true );
}

void KXmlGuiWindow::slotStateChanged(const QString &newstate)
{
  stateChanged(newstate, KXMLGUIClient::StateNoReverse);
}

void KXmlGuiWindow::slotStateChanged(const QString &newstate,
                                   bool reverse)
{
  stateChanged(newstate,
               reverse ? KXMLGUIClient::StateReverse : KXMLGUIClient::StateNoReverse);
}

void KXmlGuiWindow::setStandardToolBarMenuEnabled( bool enable )
{
    K_D(KXmlGuiWindow);
    if ( enable ) {
        if ( d->toolBarHandler )
            return;

        d->toolBarHandler = new KDEPrivate::ToolBarHandler( this );

        if ( factory() )
            factory()->addClient( d->toolBarHandler );
    } else {
        if ( !d->toolBarHandler )
            return;

        if ( factory() )
            factory()->removeClient( d->toolBarHandler );

        delete d->toolBarHandler;
        d->toolBarHandler = 0;
    }
}

bool KXmlGuiWindow::isStandardToolBarMenuEnabled() const
{
    K_D(const KXmlGuiWindow);
    return ( d->toolBarHandler );
}

void KXmlGuiWindow::createStandardStatusBarAction(){
    K_D(KXmlGuiWindow);
    if(!d->showStatusBarAction){
        d->showStatusBarAction = KStandardAction::showStatusbar(this, SLOT(setSettingsDirty()), actionCollection());
        KStatusBar *sb = statusBar(); // Creates statusbar if it doesn't exist already.
        connect(d->showStatusBarAction, SIGNAL(toggled(bool)), sb, SLOT(setVisible(bool)));
        d->showStatusBarAction->setChecked(sb->isHidden());
    } else {
        // If the language has changed, we'll need to grab the new text and whatsThis
        KAction *tmpStatusBar = KStandardAction::showStatusbar(NULL, NULL, NULL);
        d->showStatusBarAction->setText(tmpStatusBar->text());
        d->showStatusBarAction->setWhatsThis(tmpStatusBar->whatsThis());
        delete tmpStatusBar;
    }
}

void KXmlGuiWindow::finalizeGUI( KXMLGUIClient *client )
{
    Q_UNUSED(client);
    if (autoSaveSettings() && autoSaveConfigGroup().isValid()) {
        applyMainWindowSettings(autoSaveConfigGroup());
    }
}

void KXmlGuiWindow::applyMainWindowSettings(const KConfigGroup &config)
{
    K_D(KXmlGuiWindow);
    KMainWindow::applyMainWindowSettings(config);
    KStatusBar *sb = this->findChild<KStatusBar*>();
    if (sb && d->showStatusBarAction)
        d->showStatusBarAction->setChecked(!sb->isHidden());
}

#include "moc_kxmlguiwindow.cpp"

