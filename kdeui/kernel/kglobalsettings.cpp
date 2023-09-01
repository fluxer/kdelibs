/* This file is part of the KDE libraries
   Copyright (C) 2000, 2006 David Faure <faure@kde.org>
   Copyright 2008 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "kglobalsettings.h"
#include <config.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kglobal.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kprotocolinfo.h>
#include <kcolorscheme.h>
#include <kstyle.h>
#include <kapplication.h>
#include <kdebug.h>

#include <QtCore/QDir>
#include <QtCore/QStandardPaths>
#include <QtCore/QProcess>
#include <QtGui/QColor>
#include <QtGui/QCursor>
#include <QtGui/QDesktopWidget>
#include <QtGui/QFont>
#include <QtGui/QPixmap>
#include <QtGui/QPixmapCache>
#include <QtGui/QToolTip>
#include <QtGui/QWhatsThis>
#include <QtGui/QApplication>
#include <QtGui/QStyleFactory>
#include <QtGui/QGuiPlatformPlugin>
#include <QtDBus/QtDBus>
#include "qplatformdefs.h"

#ifdef Q_WS_X11
#  include <X11/Xlib.h>
#ifdef HAVE_XCURSOR
#  include <X11/Xcursor/Xcursor.h>
#endif
#  include "fixx11h.h"
#  include <QtGui/qx11info_x11.h>
#endif

#include <stdlib.h>

class KGlobalSettings::Private
{
    public:
        Private(KGlobalSettings *q)
            : q(q), activated(false), paletteCreated(false)
        {
        }

        QPalette createApplicationPalette(const KSharedConfigPtr &config);
        QPalette createNewApplicationPalette(const KSharedConfigPtr &config);
        void _k_slotNotifyChange(int, int);

        void kdisplaySetPalette();
        void kdisplaySetStyle();
        void kdisplaySetFont();

        KGlobalSettings *q;
        bool activated;
        bool paletteCreated;
        QPalette applicationPalette;
};

KGlobalSettings* KGlobalSettings::self()
{
    K_GLOBAL_STATIC(KGlobalSettings, s_self)
    return s_self;
}

KGlobalSettings::KGlobalSettings()
    : QObject(0),
    d(new Private(this))
{
}

KGlobalSettings::~KGlobalSettings()
{
    delete d;
}

void KGlobalSettings::activate(ActivateOptions options)
{
    if (!d->activated) {
        d->activated = true;

        if (options & ListenForChanges) {
            QDBusConnection::sessionBus().connect(QString(), "/KGlobalSettings", "org.kde.KGlobalSettings",
                                                  "notifyChange", this, SLOT(_k_slotNotifyChange(int,int)));
        }

        if (options & ApplySettings) {
            d->kdisplaySetStyle(); // implies palette setup
            d->kdisplaySetFont();
        }
    }
}

int KGlobalSettings::dndEventDelay()
{
    // NOTE: set by the platform plugin, see:
    // kde-workspace/qguiplatformplugin_kde/qguiplatformplugin_kde.cpp
    return QApplication::startDragDistance();
}

bool KGlobalSettings::singleClick()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
}

KGlobalSettings::TearOffHandle KGlobalSettings::insertTearOffHandle()
{
    bool effectsenabled = (KGlobalSettings::graphicEffectsLevel() > KGlobalSettings::NoEffects);
    KConfigGroup g( KGlobal::config(), "KDE");
    int tearoff = g.readEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return effectsenabled ? (TearOffHandle) tearoff : Disable;
}

bool KGlobalSettings::changeCursorOverIcon()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

int KGlobalSettings::autoSelectDelay()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

KGlobalSettings::Completion KGlobalSettings::completionMode()
{
    KConfigGroup g(KGlobal::config(), "General");
    int completion = g.readEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) || (completion > (int) CompletionPopupAuto)) {
        completion = (int) CompletionPopup; // Default
    }
    return (Completion) completion;
}

// NOTE: keep this in sync with:
// kde-workspace/kcontrol/colors/colorscm.cpp
QColor KGlobalSettings::inactiveTitleColor()
{
    KConfigGroup g(KGlobal::config(), "WM");
    return g.readEntry("inactiveBackground", QColor(224,223,222));
}

QColor KGlobalSettings::inactiveTextColor()
{
    KConfigGroup g(KGlobal::config(), "WM");
    return g.readEntry("inactiveForeground", QColor(75,71,67));
}

QColor KGlobalSettings::activeTitleColor()
{
    KConfigGroup g(KGlobal::config(), "WM");
    return g.readEntry("activeBackground", QColor(48,174,232));
}

QColor KGlobalSettings::activeTextColor()
{
    KConfigGroup g(KGlobal::config(), "WM");
    return g.readEntry("activeForeground", QColor(255,255,255));
}

int KGlobalSettings::contrast()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("contrast", 7);
}

qreal KGlobalSettings::contrastF(const KSharedConfigPtr &config)
{
    if (config) {
        KConfigGroup g(config, "KDE");
        return 0.1 * g.readEntry("contrast", 7);
    }
    return 0.1 * (qreal)contrast();
}

bool KGlobalSettings::shadeSortColumn()
{
    KConfigGroup g(KGlobal::config(), "General");
    return g.readEntry("shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN);
}

// NOTE: keep in sync with:
// kde-workspace/kcontrol/fonts/fonts.cpp
QFont KGlobalSettings::generalFont()
{
    return QApplication::font();
}

QFont KGlobalSettings::fixedFont()
{
    return QApplication::font("QTextEdit");
}

QFont KGlobalSettings::toolBarFont()
{
    return QApplication::font("QToolBar");
}

QFont KGlobalSettings::menuFont()
{
    return QApplication::font("QMenu");
}

QFont KGlobalSettings::windowTitleFont()
{
    static const QFont defaultFont(KDE_DEFAULT_FONT, 8);
    KConfigGroup g(KGlobal::config(), "WM");
    return g.readEntry("activeFont", defaultFont);
}

QFont KGlobalSettings::taskbarFont()
{
    static const QFont defaultFont(KDE_DEFAULT_FONT, 9);
    KConfigGroup g(KGlobal::config(), "General");
    return g.readEntry("taskbarFont", defaultFont);
}

QFont KGlobalSettings::smallestReadableFont()
{
    static const QFont defaultFont(KDE_DEFAULT_FONT, 8);
    KConfigGroup g(KGlobal::config(), "General");
    return g.readEntry("smallestReadableFont", defaultFont);
}

QFont KGlobalSettings::largeFont()
{
    QFont largeFont = generalFont();
    largeFont.setPointSize(48);
    return largeFont;
}

KGlobalSettings::Mouse KGlobalSettings::mouseButtonMapping()
{
    KGlobalSettings::Mouse handed = KGlobalSettings::RightHanded;

    KConfig config("kcminputrc");
    KConfigGroup g = config.group("Mouse");
    QString setting = g.readEntry("MouseButtonMapping");
    if (setting == "RightHanded") {
        handed = KGlobalSettings::RightHanded;
    } else if (setting == "LeftHanded") {
        handed = KGlobalSettings::LeftHanded;
    } else {
        // get settings from X server
        // This is a simplified version of the code in input/mouse.cpp
        // Keep in sync !
        handed = KGlobalSettings::RightHanded;
        unsigned char map[20];
        ::memset(map, 0, sizeof(map) * sizeof(unsigned char));
        int num_buttons = XGetPointerMapping(QX11Info::display(), map, 20);
        if (num_buttons == 2) {
            if ((int)map[0] == 1 && (int)map[1] == 2) {
                handed = KGlobalSettings::RightHanded;
            } else if ((int)map[0] == 2 && (int)map[1] == 1) {
                handed = KGlobalSettings::LeftHanded;
            }
        } else if (num_buttons >= 3) {
            if ((int)map[0] == 1 && (int)map[2] == 3) {
                handed = KGlobalSettings::RightHanded;
            } else if ((int)map[0] == 3 && (int)map[2] == 1) {
                handed = KGlobalSettings::LeftHanded;
            }
        }
    }

    return handed;
}

QString KGlobalSettings::desktopPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    return (path.isEmpty() ? QDir::homePath() : path);
}

QString KGlobalSettings::documentPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    return (path.isEmpty() ? QDir::homePath() : path);
}

QString KGlobalSettings::downloadPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::DownloadsLocation);
    return (path.isEmpty() ? QDir::homePath() + "/Downloads" : path);
}

QString KGlobalSettings::videosPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::VideosLocation);
    return (path.isEmpty() ? QDir::homePath() : path);
}

QString KGlobalSettings::picturesPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    return (path.isEmpty() ? QDir::homePath() : path);
}

QString KGlobalSettings::musicPath()
{
    QString path = QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
    return (path.isEmpty() ? QDir::homePath() : path);
}

bool KGlobalSettings::wheelMouseZooms()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM);
}

QRect KGlobalSettings::splashScreenDesktopGeometry()
{
    QDesktopWidget *dw = QApplication::desktop();
    return dw->screenGeometry(dw->screenNumber(QCursor::pos()));
}

bool KGlobalSettings::showIconsOnPushButtons()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("ShowIconsOnPushButtons", KDE_DEFAULT_ICON_ON_PUSHBUTTON);
}

bool KGlobalSettings::naturalSorting()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("NaturalSorting", KDE_DEFAULT_NATURAL_SORTING);
}

KGlobalSettings::GraphicEffects KGlobalSettings::graphicEffectsLevel()
{
    KConfigGroup g(KGlobal::config(), "KDE-Global GUI Settings");
    int graphicEffects = g.readEntry("GraphicEffectsLevel", int(KGlobalSettings::graphicEffectsLevelDefault()));
    return GraphicEffects(graphicEffects);
}

KGlobalSettings::GraphicEffects KGlobalSettings::graphicEffectsLevelDefault()
{
    KGlobalSettings::GraphicEffects result = KGlobalSettings::SimpleAnimationEffects;
    // NOTE: Katie's fade effect requires compositor and it is enabled if complex effects are
    if (QX11Info::isCompositingManagerRunning()) {
        result |= KGlobalSettings::ComplexAnimationEffects;
    }
    return result;
}

bool KGlobalSettings::showFilePreview(const KUrl &url)
{
    KConfigGroup g(KGlobal::config(), "PreviewSettings");
    QString protocol = url.protocol();
    bool defaultSetting = KProtocolInfo::showFilePreview(protocol);
    return g.readEntry(protocol, defaultSetting);
}

bool KGlobalSettings::opaqueResize()
{
    KConfigGroup g(KGlobal::config(), "KDE");
    return g.readEntry("OpaqueResize", KDE_DEFAULT_OPAQUE_RESIZE);
}

#ifdef Q_WS_X11
QT_BEGIN_NAMESPACE
extern void qt_x11_apply_settings_in_all_apps();
QT_END_NAMESPACE
#endif

void KGlobalSettings::emitChange(ChangeType changeType, int arg)
{
    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings", "org.kde.KGlobalSettings", "notifyChange");
    QList<QVariant> args;
    args.append(static_cast<int>(changeType));
    args.append(arg);
    message.setArguments(args);
    QDBusConnection::sessionBus().send(message);
#ifdef Q_WS_X11
    if (qApp && qApp->type() != QApplication::Tty) {
        //notify non-kde qt applications of the change
        qt_x11_apply_settings_in_all_apps();
    }
#endif
}

void KGlobalSettings::Private::_k_slotNotifyChange(int changeType, int arg)
{
    switch(changeType) {
        case SettingsChanged: {
            KGlobal::config()->reparseConfiguration();
            emit q->settingsChanged();
            break;
        }
        case StyleChanged: {
            if (activated) {
                KGlobal::config()->reparseConfiguration();
                kdisplaySetStyle();
            }
            break;
        }
        case ToolbarStyleChanged: {
            KGlobal::config()->reparseConfiguration();
            emit q->toolbarAppearanceChanged(arg);
            break;
        }
        case PaletteChanged: {
            if (activated) {
                KGlobal::config()->reparseConfiguration();
                paletteCreated = false;
                kdisplaySetPalette();
            }
            break;
        }
        case FontChanged: {
            KGlobal::config()->reparseConfiguration();
            if (activated) {
                kdisplaySetFont();
            }
            break;
        }
        case LocaleChanged: {
            KGlobal::config()->reparseConfiguration();
            KGlobal::locale()->reparseConfiguration();
            // KLocale is reponsible for both so event for locale and language change
            // is send
            QEvent localeevent(QEvent::LocaleChange);
            QApplication::sendEvent(qApp, &localeevent);
            QEvent languageevent(QEvent::LanguageChange);
            QApplication::sendEvent(qApp, &languageevent);
            emit q->localeChanged();
            break;
        }
        case IconChanged: {
            QPixmapCache::clear();
            KGlobal::config()->reparseConfiguration();
            emit q->iconChanged(arg);
            break;
        }
        case MouseChanged: {
            KGlobal::config()->reparseConfiguration();
            emit q->mouseChanged();
            break;
        }
        case NaturalSortingChanged: {
            KGlobal::config()->reparseConfiguration();
            emit q->naturalSortingChanged();
            break;
        }
        case PathsChanged: {
            emit q->pathsChanged();
            break;
        }
        case ShortcutsChanged: {
             emit q->shortcutsChanged();
            break;
        }
        case BlockShortcuts: {
            // NOTE: KGlobalAccel connects to this signal
            emit q->blockShortcuts(arg); // see kwin
            break;
        }
        default: {
            kWarning() << "Unknown type of change in KGlobalSettings::slotNotifyChange: " << changeType;
            break;
        }
    }
}

QPalette KGlobalSettings::createApplicationPalette(const KSharedConfigPtr &config)
{
    return self()->d->createApplicationPalette(config);
}

QPalette KGlobalSettings::createNewApplicationPalette(const KSharedConfigPtr &config)
{
    return self()->d->createNewApplicationPalette(config);
}

QPalette KGlobalSettings::Private::createApplicationPalette(const KSharedConfigPtr &config)
{
    // This method is typically called once by KQGuiPlatformPlugin::palette and once again
    // by kdisplaySetPalette(), so we cache the palette to save time.
    if (config == KGlobal::config() && paletteCreated) {
        return applicationPalette;
    }
    return createNewApplicationPalette(config);
}

QPalette KGlobalSettings::Private::createNewApplicationPalette(const KSharedConfigPtr &config)
{
    QPalette palette;

    QPalette::ColorGroup states[3] = {
        QPalette::Active,
        QPalette::Inactive,
        QPalette::Disabled
    };

    // TT thinks tooltips shouldn't use active, so we use our active colors for all states
    KColorScheme schemeTooltip(QPalette::Active, KColorScheme::Tooltip, config);

    for (int i = 0; i < 3; i++) {
        QPalette::ColorGroup state = states[i];
        KColorScheme schemeView(state, KColorScheme::View, config);
        KColorScheme schemeWindow(state, KColorScheme::Window, config);
        KColorScheme schemeButton(state, KColorScheme::Button, config);
        KColorScheme schemeSelection(state, KColorScheme::Selection, config);

        palette.setBrush(state, QPalette::WindowText, schemeWindow.foreground());
        palette.setBrush(state, QPalette::Window, schemeWindow.background());
        palette.setBrush(state, QPalette::Base, schemeView.background());
        palette.setBrush(state, QPalette::Text, schemeView.foreground());
        palette.setBrush(state, QPalette::Button, schemeButton.background());
        palette.setBrush(state, QPalette::ButtonText, schemeButton.foreground());
        palette.setBrush(state, QPalette::Highlight, schemeSelection.background());
        palette.setBrush(state, QPalette::HighlightedText, schemeSelection.foreground());
        palette.setBrush(state, QPalette::ToolTipBase, schemeTooltip.background());
        palette.setBrush(state, QPalette::ToolTipText, schemeTooltip.foreground());

        palette.setColor(state, QPalette::Light, schemeWindow.shade(KColorScheme::LightShade));
        palette.setColor(state, QPalette::Midlight, schemeWindow.shade(KColorScheme::MidlightShade));
        palette.setColor(state, QPalette::Mid, schemeWindow.shade(KColorScheme::MidShade));
        palette.setColor(state, QPalette::Dark, schemeWindow.shade(KColorScheme::DarkShade));
        palette.setColor(state, QPalette::Shadow, schemeWindow.shade(KColorScheme::ShadowShade));

        palette.setBrush(state, QPalette::AlternateBase, schemeView.background(KColorScheme::AlternateBackground));
        palette.setBrush(state, QPalette::Link, schemeView.foreground(KColorScheme::LinkText));
        palette.setBrush(state, QPalette::LinkVisited, schemeView.foreground(KColorScheme::VisitedText));
    }

    if (config == KGlobal::config()) {
        paletteCreated = true;
        applicationPalette = palette;
    }

    return palette;
}

void KGlobalSettings::Private::kdisplaySetPalette()
{
    // full session or not KColorScheme and even KGlobalSettings itself read settings from configs
    // thus if appearance settings are set in said configs the palette has to be applied here
    if (qApp->type() == KAPPLICATION_GUI_TYPE) {
        QApplication::setPalette(q->createApplicationPalette());
    }
    emit q->kdisplayPaletteChanged();
    emit q->appearanceChanged();
}

void KGlobalSettings::Private::kdisplaySetFont()
{
    if (qApp->type() == KAPPLICATION_GUI_TYPE) {
        QFont defaultFont(KDE_DEFAULT_FONT, 9);
        KConfigGroup g(KGlobal::config(), "General");
        const QFont generalFont = g.readEntry("font", defaultFont);
        const QFont menuFont = g.readEntry("menuFont", defaultFont);
        defaultFont = QFont(KDE_DEFAULT_FIXED_FONT, 8);
        const QFont toolBarFont = g.readEntry("toolBarFont", defaultFont);
        defaultFont = QFont(KDE_DEFAULT_FIXED_FONT, 9);
        const QFont fixedFont = g.readEntry("fixed", defaultFont);

        QApplication::setFont(generalFont);
        QApplication::setFont(menuFont, "QMenuBar");
        QApplication::setFont(menuFont, "QMenu");
        QApplication::setFont(toolBarFont, "QToolBar");
        QApplication::setFont(fixedFont, "QTextEdit");
        QApplication::setFont(fixedFont, "QPlainTextEdit");
    }
    emit q->kdisplayFontChanged();
    emit q->appearanceChanged();
}

void KGlobalSettings::Private::kdisplaySetStyle()
{
    if (qApp->type() == KAPPLICATION_GUI_TYPE) {
        emit q->kdisplayStyleChanged();

        // Reread palette from config file.
        kdisplaySetPalette();
    }
}

#include "moc_kglobalsettings.cpp"
