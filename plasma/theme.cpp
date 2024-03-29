/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "theme.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QList>
#include <QPair>
#include <QTimer>
#include <QCache>
#include <QBuffer>

#include <kcolorscheme.h>
#include <kcomponentdata.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>
#include <kwindowsystem.h>

#include "private/packages_p.h"
#include "windoweffects.h"

namespace Plasma
{

static const bool DEFAULT_THEME_CACHE = true;
static const int DEFAULT_THEME_CACHE_SIZE = 81920;
static const int DEFAULT_TOOLTIP_DELAY = 700;
static const int DEFAULT_WALLPAPER_WIDTH = 1920;
static const int DEFAULT_WALLPAPER_HEIGHT = 1200;
//NOTE: Default wallpaper can be set from the theme configuration
#define DEFAULT_WALLPAPER_THEME "default"
#define DEFAULT_WALLPAPER_SUFFIX ".png"

enum styles {
    DEFAULTSTYLE,
    SVGSTYLE
};

enum CacheType {
    NoCache = 0,
    PixmapCache = 1,
    SvgElementsCache = 2
};
Q_DECLARE_FLAGS(CacheTypes, CacheType)
Q_DECLARE_OPERATORS_FOR_FLAGS(CacheTypes)

class ThemePrivate
{
public:
    ThemePrivate(Theme *theme)
        : q(theme),
          colorScheme(QPalette::Active, KColorScheme::Window, KSharedConfigPtr(0)),
          buttonColorScheme(QPalette::Active, KColorScheme::Button, KSharedConfigPtr(0)),
          viewColorScheme(QPalette::Active, KColorScheme::View, KSharedConfigPtr(0)),
          defaultWallpaperTheme(DEFAULT_WALLPAPER_THEME),
          defaultWallpaperSuffix(DEFAULT_WALLPAPER_SUFFIX),
          defaultWallpaperWidth(DEFAULT_WALLPAPER_WIDTH),
          defaultWallpaperHeight(DEFAULT_WALLPAPER_HEIGHT),
          toolTipDelay(DEFAULT_TOOLTIP_DELAY),
          cachesToDiscard(NoCache),
          isDefault(false),
          useGlobal(true),
          hasWallpapers(false),
          cacheTheme(DEFAULT_THEME_CACHE),
          useNativeWidgetStyle(false)
    {
        generalFont = QApplication::font();

        pixmapCache = new QSharedPointer<QCache<QString, QPixmap> >(new QCache<QString, QPixmap>());

        saveTimer = new QTimer(q);
        saveTimer->setSingleShot(true);
        saveTimer->setInterval(600);
        QObject::connect(saveTimer, SIGNAL(timeout()), q, SLOT(scheduledCacheUpdate()));

        updateNotificationTimer = new QTimer(q);
        updateNotificationTimer->setSingleShot(true);
        updateNotificationTimer->setInterval(500);
        QObject::connect(updateNotificationTimer, SIGNAL(timeout()), q, SLOT(notifyOfChanged()));

        QObject::connect(KWindowSystem::self(), SIGNAL(compositingChanged(bool)), q, SLOT(compositingChanged(bool)));
    }

    ~ThemePrivate()
    {
       delete pixmapCache;
    }

    KConfigGroup &config()
    {
        if (!cfg.isValid()) {
            QString groupName = "Theme";

            if (!useGlobal) {
                QString app = KGlobal::mainComponent().componentName();

                if (!app.isEmpty()) {
                    kDebug() << "using theme for app" << app;
                    groupName.append("-").append(app);
                }
            }

            cfg = KConfigGroup(KSharedConfig::openConfig(themeRcFile), groupName);
        }

        return cfg;
    }

    QString findInTheme(const QString &image, const QString &theme, bool cache = true);
    void compositingChanged(bool active);
    void discardCache(CacheTypes caches);
    void scheduledCacheUpdate();
    void scheduleThemeChangeNotification(CacheTypes caches);
    void notifyOfChanged();
    void colorsChanged();
    bool useCache();
    void settingsFileChanged(const QString &);
    void setThemeName(const QString &themeName, bool writeSettings);
    void processWallpaperSettings(KConfigBase *metadata);

    QString processStyleSheet(const QString &css);

    static const char *defaultTheme;
    static const char *systemColorsTheme;
    static const char *themeRcFile;
    static PackageStructure::Ptr packageStructure;

    Theme *q;
    QString themeName;
    QList<QString> fallbackThemes;
    KSharedConfigPtr colors;
    KColorScheme colorScheme;
    KColorScheme buttonColorScheme;
    KColorScheme viewColorScheme;
    KConfigGroup cfg;
    QFont generalFont;
    QString defaultWallpaperTheme;
    QString defaultWallpaperSuffix;
    int defaultWallpaperWidth;
    int defaultWallpaperHeight;
    QSharedPointer<QCache<QString, QPixmap> > *pixmapCache;
    KSharedConfigPtr svgElementsCache;
    QHash<QString, QSet<QString> > invalidElements;
    QHash<QString, QPixmap> pixmapsToCache;
    QHash<QString, QString> keysToCache;
    QHash<QString, QString> idsToCache;
    QHash<styles, QString> cachedStyleSheets;
    QHash<QString, QString> discoveries;
    QTimer *saveTimer;
    QTimer *updateNotificationTimer;
    int toolTipDelay;
    CacheTypes cachesToDiscard;
    QString themeVersion;
    QString themeMetadataPath;

    bool isDefault;
    bool useGlobal;
    bool hasWallpapers;
    bool cacheTheme;
    bool useNativeWidgetStyle;
};

PackageStructure::Ptr ThemePrivate::packageStructure(0);
const char *ThemePrivate::defaultTheme = "default";

const char *ThemePrivate::themeRcFile = "plasmarc";
// the system colors theme is used to cache unthemed svgs with colorization needs
// these svgs do not follow the theme's colors, but rather the system colors
const char *ThemePrivate::systemColorsTheme = "internal-system-colors";

bool ThemePrivate::useCache()
{
    if (cacheTheme && !pixmapCache) {
        const bool isRegularTheme = themeName != systemColorsTheme;

        // clear any cached values from the previous theme cache
        themeVersion.clear();

        if (!themeMetadataPath.isEmpty()) {
            KDirWatch::self()->removeFile(themeMetadataPath);
        }
        themeMetadataPath = KStandardDirs::locate("data", "desktoptheme/" + themeName + "/metadata.desktop");

        if (isRegularTheme && !themeMetadataPath.isEmpty()) {
            // watch the metadata file for changes at runtime
            KDirWatch::self()->addFile(themeMetadataPath);
        }

        // TODO: discardCache(PixmapCache | SvgElementsCache); ?
    }

    if (cacheTheme && !svgElementsCache) {
        const QString svgElementsFileNameBase = "plasma-svgelements-" + themeName;
        QString svgElementsFileName = svgElementsFileNameBase;
        if (!themeVersion.isEmpty()) {
            svgElementsFileName += "_v" + themeVersion;
        }

        // now we check for (and remove) old caches
        foreach (const QString &file, KGlobal::dirs()->findAllResources("cache", svgElementsFileNameBase + "*")) {
            if (!file.endsWith(svgElementsFileName)) {
                QFile::remove(file);
            }
        }

        const QString svgElementsFile = KStandardDirs::locateLocal("cache", svgElementsFileName);
        svgElementsCache = KSharedConfig::openConfig(svgElementsFile);
    }

    return cacheTheme;
}

QString ThemePrivate::findInTheme(const QString &image, const QString &theme, bool cache)
{
    if (cache && discoveries.contains(image)) {
        return discoveries[image];
    }

    QString search;

    if (!q->windowTranslucencyEnabled()) {
        search = QLatin1String("desktoptheme/") + theme + QLatin1String("/opaque/") + image;
        search =  KStandardDirs::locate("data", search);
    } else {
        search = QLatin1String("desktoptheme/") + theme + QLatin1String("/translucent/") + image;
        search =  KStandardDirs::locate("data", search);
    }

    //not found or compositing enabled
    if (search.isEmpty()) {
        search = QLatin1String("desktoptheme/") + theme + QLatin1Char('/') + image;
        search =  KStandardDirs::locate("data", search);
    }

    if (cache && !search.isEmpty()) {
        discoveries.insert(image, search);
    }

    return search;
}

void ThemePrivate::compositingChanged(bool active)
{
#ifdef Q_WS_X11
    // kDebug() << QTime::currentTime();
    scheduleThemeChangeNotification(PixmapCache | SvgElementsCache);
#endif
}

void ThemePrivate::discardCache(CacheTypes caches)
{
    if (caches & PixmapCache) {
        pixmapsToCache.clear();
        saveTimer->stop();
    }
    // FIXME: this is always done to properly change theme but should be done only on demand
    if (pixmapCache) {
        pixmapCache->data()->clear();
    }

    cachedStyleSheets.clear();

    if (caches & SvgElementsCache) {
        discoveries.clear();
        invalidElements.clear();
        svgElementsCache = 0;
    }
}

void ThemePrivate::scheduledCacheUpdate()
{
    if (useCache()) {
        QHashIterator<QString, QPixmap> it(pixmapsToCache);
        while (it.hasNext()) {
            it.next();
            pixmapCache->data()->insert(idsToCache[it.key()], new QPixmap(it.value()));
        }
    }

    pixmapsToCache.clear();
    keysToCache.clear();
    idsToCache.clear();
}

void ThemePrivate::colorsChanged()
{
    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    scheduleThemeChangeNotification(PixmapCache);
}

void ThemePrivate::scheduleThemeChangeNotification(CacheTypes caches)
{
    cachesToDiscard |= caches;
    updateNotificationTimer->start();
}

void ThemePrivate::notifyOfChanged()
{
    //kDebug() << cachesToDiscard;
    discardCache(cachesToDiscard);
    cachesToDiscard = NoCache;
    emit q->themeChanged();
}

QString ThemePrivate::processStyleSheet(const QString &css)
{
    QString stylesheet;
    if (css.isEmpty()) {
        stylesheet = cachedStyleSheets.value(DEFAULTSTYLE);
        if (stylesheet.isEmpty()) {
            stylesheet = QString("\n\
                        body {\n\
                            color: %textcolor;\n\
                            font-size: %fontsize;\n\
                            font-family: %fontfamily;\n\
                        }\n\
                        a:active  { color: %activatedlink; }\n\
                        a:link    { color: %link; }\n\
                        a:visited { color: %visitedlink; }\n\
                        a:hover   { color: %hoveredlink; text-decoration: none; }\n\
                        ");
            stylesheet = processStyleSheet(stylesheet);
            cachedStyleSheets.insert(DEFAULTSTYLE, stylesheet);
        }

        return stylesheet;
    } else if (css == "SVG") {
        stylesheet = cachedStyleSheets.value(SVGSTYLE);
        if (stylesheet.isEmpty()) {
            QString skel = ".ColorScheme-%1{color:%2;}";

            stylesheet += skel.arg("Text","%textcolor");
            stylesheet += skel.arg("Background","%backgroundcolor");

            stylesheet += skel.arg("ButtonText","%buttontextcolor");
            stylesheet += skel.arg("ButtonBackground","%buttonbackgroundcolor");
            stylesheet += skel.arg("ButtonHover","%buttonhovercolor");
            stylesheet += skel.arg("ButtonFocus","%buttonfocuscolor");

            stylesheet += skel.arg("ViewText","%viewtextcolor");
            stylesheet += skel.arg("ViewBackground","%viewbackgroundcolor");
            stylesheet += skel.arg("ViewHover","%viewhovercolor");
            stylesheet += skel.arg("ViewFocus","%viewfocuscolor");

            stylesheet = processStyleSheet(stylesheet);
            cachedStyleSheets.insert(SVGSTYLE, stylesheet);
        }

        return stylesheet;
    } else {
        stylesheet = css;
    }

    QHash<QString, QString> elements;
    // If you add elements here, make sure their names are sufficiently unique to not cause
    // clashes between element keys
    elements["%textcolor"] = q->color(Theme::TextColor).name();
    elements["%backgroundcolor"] = q->color(Theme::BackgroundColor).name();
    elements["%visitedlink"] = q->color(Theme::VisitedLinkColor).name();
    elements["%activatedlink"] = q->color(Theme::HighlightColor).name();
    elements["%hoveredlink"] = q->color(Theme::HighlightColor).name();
    elements["%link"] = q->color(Theme::LinkColor).name();
    elements["%buttontextcolor"] = q->color(Theme::ButtonTextColor).name();
    elements["%buttonbackgroundcolor"] = q->color(Theme::ButtonBackgroundColor).name();
    elements["%buttonhovercolor"] = q->color(Theme::ButtonHoverColor).name();
    elements["%buttonfocuscolor"] = q->color(Theme::ButtonFocusColor).name();
    elements["%viewtextcolor"] = q->color(Theme::ViewTextColor).name();
    elements["%viewbackgroundcolor"] = q->color(Theme::ViewBackgroundColor).name();
    elements["%viewhovercolor"] = q->color(Theme::ViewHoverColor).name();
    elements["%viewfocuscolor"] = q->color(Theme::ViewFocusColor).name();

    QFont font = q->font(Theme::DefaultFont);
    elements["%fontsize"] = QString("%1pt").arg(font.pointSize());
    elements["%fontfamily"] = font.family().split('[').first();
    elements["%smallfontsize"] = QString("%1pt").arg(KGlobalSettings::smallestReadableFont().pointSize());

    QHashIterator<QString, QString> it(elements);
    while (it.hasNext()) {
        it.next();
        stylesheet.replace(it.key(), it.value());
    }
    return stylesheet;
}

class ThemeSingleton
{
public:
    ThemeSingleton()
    {
        self.d->isDefault = true;

        //FIXME: if/when kconfig gets change notification, this will be unnecessary
        KDirWatch::self()->addFile(KStandardDirs::locateLocal("config", ThemePrivate::themeRcFile));
        QObject::connect(KDirWatch::self(), SIGNAL(dirty(QString)),
                         &self, SLOT(settingsFileChanged(QString)));
    }

   Theme self;
};

K_GLOBAL_STATIC(ThemeSingleton, privateThemeSelf)

Theme *Theme::defaultTheme()
{
    return &privateThemeSelf->self;
}

Theme::Theme(QObject *parent)
    : QObject(parent),
      d(new ThemePrivate(this))
{
    settingsChanged();
}

Theme::Theme(const QString &themeName, QObject *parent)
    : QObject(parent),
      d(new ThemePrivate(this))
{
    // turn off caching so we don't accidently trigger unnecessary disk activity at this point
    bool useCache = d->cacheTheme;
    d->cacheTheme = false;
    setThemeName(themeName);
    d->cacheTheme = useCache;
}

Theme::~Theme()
{
    d->cacheTheme = false;

    d->pixmapsToCache.clear();
    delete d->pixmapCache;
    d->pixmapCache = 0;

    if (d->svgElementsCache) {
        QHashIterator<QString, QSet<QString> > it(d->invalidElements);
        while (it.hasNext()) {
            it.next();
            KConfigGroup imageGroup(d->svgElementsCache, it.key());
            imageGroup.writeEntry("invalidElements", it.value().toList()); //FIXME: add QSet support to KConfig
        }
    }

    delete d;
}

PackageStructure::Ptr Theme::packageStructure()
{
    if (!ThemePrivate::packageStructure) {
        ThemePrivate::packageStructure = new ThemePackage();
    }

    return ThemePrivate::packageStructure;
}

KPluginInfo::List Theme::listThemeInfo()
{
    const QStringList themes = KGlobal::dirs()->findAllResources("data", "desktoptheme/*/metadata.desktop",
                                                           KStandardDirs::NoDuplicates);
    return KPluginInfo::fromFiles(themes);
}

void ThemePrivate::settingsFileChanged(const QString &file)
{
    Q_UNUSED(file);
    if (!themeMetadataPath.isEmpty()) {
        const KPluginInfo pluginInfo(themeMetadataPath);
        if (themeVersion != pluginInfo.version()) {
            scheduleThemeChangeNotification(SvgElementsCache);
        }
    }
    config().config()->reparseConfiguration();
    q->settingsChanged();
}

void Theme::settingsChanged()
{
    KConfigGroup cg = d->config();
    d->setThemeName(cg.readEntry("name", ThemePrivate::defaultTheme), false);
    cg = KConfigGroup(cg.config(), "PlasmaToolTips");
    d->toolTipDelay = cg.readEntry("Delay", DEFAULT_TOOLTIP_DELAY);

    KConfigGroup cachegrp = KConfigGroup(KSharedConfig::openConfig(ThemePrivate::themeRcFile), "CachePolicies");
    d->cacheTheme = cachegrp.readEntry("CacheTheme", DEFAULT_THEME_CACHE);

    if (d->pixmapCache) {
        const int themeCacheKb = cachegrp.readEntry("ThemeCacheKb", DEFAULT_THEME_CACHE_SIZE);
        d->pixmapCache->data()->setMaxCost(themeCacheKb * 1024);
    }
}

void Theme::setThemeName(const QString &themeName)
{
    d->setThemeName(themeName, true);
}

void ThemePrivate::processWallpaperSettings(KConfigBase *metadata)
{
    if (!defaultWallpaperTheme.isEmpty() && defaultWallpaperTheme != DEFAULT_WALLPAPER_THEME) {
        return;
    }

    KConfigGroup cg;
    if (metadata->hasGroup("Wallpaper")) {
        // we have a theme color config, so let's also check to see if
        // there is a wallpaper defined in there.
        cg = KConfigGroup(metadata, "Wallpaper");
    } else {
        // since we didn't find an entry in the theme, let's look in the main
        // theme config
        cg = config();
    }

    defaultWallpaperTheme = cg.readEntry("defaultWallpaperTheme", DEFAULT_WALLPAPER_THEME);
    defaultWallpaperSuffix = cg.readEntry("defaultFileSuffix", DEFAULT_WALLPAPER_SUFFIX);
    defaultWallpaperWidth = cg.readEntry("defaultWidth", DEFAULT_WALLPAPER_WIDTH);
    defaultWallpaperHeight = cg.readEntry("defaultHeight", DEFAULT_WALLPAPER_HEIGHT);
}

void ThemePrivate::setThemeName(const QString &tempThemeName, bool writeSettings)
{
    //kDebug() << tempThemeName;
    QString theme = tempThemeName;
    if (theme.isEmpty() || theme == themeName) {
        // let's try and get the default theme at least
        if (themeName.isEmpty()) {
            theme = ThemePrivate::defaultTheme;
        } else {
            return;
        }
    }

    // we have one special theme: essentially a dummy theme used to cache things with
    // the system colors.
    bool realTheme = theme != systemColorsTheme;
    if (realTheme) {
        QString themePath = KStandardDirs::locate("data", QLatin1String("desktoptheme/") + theme + QLatin1Char('/'));
        if (themePath.isEmpty() && themeName.isEmpty()) {
            themePath = KStandardDirs::locate("data", "desktoptheme/default/");

            if (themePath.isEmpty()) {
                return;
            }

            theme = ThemePrivate::defaultTheme;
        }
    }

    // check again as ThemePrivate::defaultTheme might be empty
    if (themeName == theme) {
        return;
    }

    themeName = theme;

    // load the color scheme config
    const QString colorsFile = realTheme ? KStandardDirs::locate("data", QLatin1String("desktoptheme/") + theme + QLatin1String("/colors"))
                                         : QString();

    //kDebug() << "we're going for..." << colorsFile << "*******************";

    // load the wallpaper settings, if any
    if (realTheme) {
        const QString metadataPath(KStandardDirs::locate("data", QLatin1String("desktoptheme/") + theme + QLatin1String("/metadata.desktop")));
        KConfig metadata(metadataPath);

        processWallpaperSettings(&metadata);

        KConfigGroup cg(&metadata, "Settings");
        useNativeWidgetStyle = cg.readEntry("UseNativeWidgetStyle", false);
        QString fallback = cg.readEntry("FallbackTheme", QString());

        fallbackThemes.clear();
        while (!fallback.isEmpty() && !fallbackThemes.contains(fallback)) {
            fallbackThemes.append(fallback);

            QString metadataPath(KStandardDirs::locate("data", QLatin1String("desktoptheme/") + theme + QLatin1String("/metadata.desktop")));
            KConfig metadata(metadataPath);
            KConfigGroup cg(&metadata, "Settings");
            fallback = cg.readEntry("FallbackTheme", QString());
        }

        if (!fallbackThemes.contains(ThemePrivate::defaultTheme)) {
            fallbackThemes.append(ThemePrivate::defaultTheme);
        }

        foreach (const QString &theme, fallbackThemes) {
            QString metadataPath(KStandardDirs::locate("data", QLatin1String("desktoptheme/") + theme + QLatin1String("/metadata.desktop")));
            KConfig metadata(metadataPath);
            processWallpaperSettings(&metadata);
        }
    }

    if (colorsFile.isEmpty()) {
        colors = 0;
        QObject::connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                         q, SLOT(colorsChanged()));
    } else {
        QObject::disconnect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()),
                            q, SLOT(colorsChanged()));
        colors = KSharedConfig::openConfig(colorsFile);
    }

    colorScheme = KColorScheme(QPalette::Active, KColorScheme::Window, colors);
    buttonColorScheme = KColorScheme(QPalette::Active, KColorScheme::Button, colors);
    viewColorScheme = KColorScheme(QPalette::Active, KColorScheme::View, colors);
    hasWallpapers = KGlobal::dirs()->exists(KStandardDirs::locateLocal("data", QLatin1String("desktoptheme/") + theme + QLatin1String("/wallpapers/")));

    if (realTheme && isDefault && writeSettings) {
        // we're the default theme, let's save our state
        KConfigGroup &cg = config();
        if (ThemePrivate::defaultTheme == themeName) {
            cg.deleteEntry("name");
        } else {
            cg.writeEntry("name", themeName);
        }
        cg.sync();
    }

    scheduleThemeChangeNotification(SvgElementsCache);
}

QString Theme::themeName() const
{
    return d->themeName;
}

QString Theme::imagePath(const QString &name) const
{
    // look for a compressed svg file in the theme
    if (name.contains("../") || name.isEmpty()) {
        // we don't support relative paths
        //kDebug() << "Theme says: bad image path " << name;
        return QString();
    }

    const QString svgzName = name + QLatin1String(".svgz");
    QString path = d->findInTheme(svgzName, d->themeName);

    if (path.isEmpty()) {
        // try for an uncompressed svg file
        const QString svgName = name + QLatin1String(".svg");
        path = d->findInTheme(svgName, d->themeName);

        // search in fallback themes if necessary
        for (int i = 0; path.isEmpty() && i < d->fallbackThemes.count(); ++i) {
            if (d->themeName == d->fallbackThemes[i]) {
                continue;
            }

            // try a compressed svg file in the fallback theme
            path = d->findInTheme(svgzName, d->fallbackThemes[i]);

            if (path.isEmpty()) {
                // try an uncompressed svg file in the fallback theme
                path = d->findInTheme(svgName, d->fallbackThemes[i]);
            }
        }
    }

    /*
    if (path.isEmpty()) {
        kDebug() << "Theme says: bad image path " << name;
    }
    */

    return path;
}

QString Theme::styleSheet(const QString &css) const
{
    return d->processStyleSheet(css);
}

QString Theme::wallpaperPath(const QSize &size) const
{
    QString fullPath;
    QString image = d->defaultWallpaperTheme;

    image.append("/contents/images/%1x%2").append(d->defaultWallpaperSuffix);
    QString defaultImage = image.arg(d->defaultWallpaperWidth).arg(d->defaultWallpaperHeight);

    if (size.isValid()) {
        // try to customize the paper to the size requested
        //TODO: this should do better than just fallback to the default size.
        //      a "best fit" matching would be far better, so we don't end
        //      up returning a 1920x1200 wallpaper for a 640x480 request ;)
        image = image.arg(size.width()).arg(size.height());
    } else {
        image = defaultImage;
    }

    //TODO: the theme's wallpaper overrides regularly installed wallpapers.
    //      should it be possible for user installed (e.g. locateLocal) wallpapers
    //      to override the theme?
    if (d->hasWallpapers) {
        // check in the theme first
        fullPath = d->findInTheme(QLatin1String("wallpapers/") + image, d->themeName);

        if (fullPath.isEmpty()) {
            fullPath = d->findInTheme(QLatin1String("wallpapers/") + defaultImage, d->themeName);
        }
    }

    if (fullPath.isEmpty()) {
        // we failed to find it in the theme, so look in the standard directories
        //kDebug() << "looking for" << image;
        fullPath = KStandardDirs::locate("wallpaper", image);
    }

    if (fullPath.isEmpty()) {
        // we still failed to find it in the theme, so look for the default in
        // the standard directories
        //kDebug() << "looking for" << defaultImage;
        fullPath = KStandardDirs::locate("wallpaper", defaultImage);

        if (fullPath.isEmpty()) {
            kDebug() << "exhausted every effort to find a wallpaper.";
        }
    }

    return fullPath;
}

bool Theme::currentThemeHasImage(const QString &name) const
{
    if (name.contains("../")) {
        // we don't support relative paths
        return false;
    }

    return !(d->findInTheme(name + QLatin1String(".svgz"), d->themeName, false).isEmpty()) ||
           !(d->findInTheme(name + QLatin1String(".svg"), d->themeName, false).isEmpty());
}

KSharedConfigPtr Theme::colorScheme() const
{
    return d->colors;
}

QColor Theme::color(ColorRole role) const
{
    switch (role) {
        case TextColor:
            return d->colorScheme.foreground(KColorScheme::NormalText).color();

        case HighlightColor:
            return d->colorScheme.decoration(KColorScheme::HoverColor).color();

        case BackgroundColor:
            return d->colorScheme.background(KColorScheme::NormalBackground).color();

        case ButtonTextColor:
            return d->buttonColorScheme.foreground(KColorScheme::NormalText).color();

        case ButtonBackgroundColor:
            return d->buttonColorScheme.background(KColorScheme::NormalBackground).color();

        case ButtonHoverColor:
            return d->buttonColorScheme.decoration(KColorScheme::HoverColor).color();

        case ButtonFocusColor:
            return d->buttonColorScheme.decoration(KColorScheme::FocusColor).color();

        case ViewTextColor:
            return d->viewColorScheme.foreground(KColorScheme::NormalText).color();

        case ViewBackgroundColor:
            return d->viewColorScheme.background(KColorScheme::NormalBackground).color();

        case ViewHoverColor:
            return d->viewColorScheme.decoration(KColorScheme::HoverColor).color();

        case ViewFocusColor:
            return d->viewColorScheme.decoration(KColorScheme::FocusColor).color();

        case LinkColor:
            return d->viewColorScheme.foreground(KColorScheme::LinkText).color();

        case VisitedLinkColor:
            return d->viewColorScheme.foreground(KColorScheme::VisitedText).color();
    }

    return QColor();
}

void Theme::setFont(const QFont &font, FontRole role)
{
    Q_UNUSED(role)
    d->generalFont = font;
}

QFont Theme::font(FontRole role) const
{
    switch (role) {
        case DesktopFont: {
            KConfigGroup cg(KGlobal::config(), "General");
            return cg.readEntry("desktopFont", d->generalFont);
        }
        case SmallestFont: {
            return KGlobalSettings::smallestReadableFont();
        }
        case DefaultFont:
        default: {
            return d->generalFont;
        }
    }

    return d->generalFont;
}

QFontMetrics Theme::fontMetrics() const
{
    //TODO: allow this to be overridden with a plasma specific font?
    return QFontMetrics(d->generalFont);
}

bool Theme::windowTranslucencyEnabled() const
{
    return KWindowSystem::self()->compositingActive();
}

void Theme::setUseGlobalSettings(bool useGlobal)
{
    if (d->useGlobal == useGlobal) {
        return;
    }

    d->useGlobal = useGlobal;
    d->cfg = KConfigGroup();
    d->themeName.clear();
    settingsChanged();
}

bool Theme::useGlobalSettings() const
{
    return d->useGlobal;
}

bool Theme::useNativeWidgetStyle() const
{
    return d->useNativeWidgetStyle;
}

bool Theme::findInCache(const QString &key, QPixmap &pix)
{
    if (d->useCache()) {
        const QString id = d->keysToCache.value(key);
        if (d->pixmapsToCache.contains(id)) {
            pix = d->pixmapsToCache.value(id);
            return !pix.isNull();
        }

        QPixmap *temp = d->pixmapCache->data()->object(key);
        if (temp && !temp->isNull()) {
            pix = QPixmap::fromImage(temp->toImage());
            return !pix.isNull();
        }
    }

    return false;
}

void Theme::insertIntoCache(const QString& key, const QPixmap& pix)
{
    if (d->useCache()) {
        d->pixmapCache->data()->insert(key, new QPixmap(pix));
    }
}

void Theme::insertIntoCache(const QString& key, const QPixmap& pix, const QString& id)
{
    if (d->useCache()) {
        d->pixmapsToCache.insert(id, pix);

        if (d->idsToCache.contains(id)) {
            d->keysToCache.remove(d->idsToCache[id]);
        }

        d->keysToCache.insert(key, id);
        d->idsToCache.insert(id, key);
        d->saveTimer->start();
    }
}

bool Theme::findInRectsCache(const QString &image, const QString &element, QRectF &rect) const
{
    if (!d->useCache()) {
        return false;
    }

    KConfigGroup imageGroup(d->svgElementsCache, image);
    rect = imageGroup.readEntry(element + QLatin1String("Size"), QRectF());

    if (rect.isValid()) {
        return true;
    }

    //Name starting by _ means the element is empty and we're asked for the size of
    //the whole image, so the whole image is never invalid
    if (element.indexOf('_') <= 0) {
        return false;
    }

    bool invalid = false;

    QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
    if (it == d->invalidElements.end()) {
        QSet<QString> elements = imageGroup.readEntry("invalidElements", QStringList()).toSet();
        d->invalidElements.insert(image, elements);
        invalid = elements.contains(element);
    } else {
        invalid = it.value().contains(element);
    }

    return invalid;
}

QStringList Theme::listCachedRectKeys(const QString &image) const
{
    if (!d->useCache()) {
        return QStringList();
    }

    KConfigGroup imageGroup(d->svgElementsCache, image);
    QStringList keys = imageGroup.keyList();

    QMutableListIterator<QString> i(keys);
    while (i.hasNext()) {
        QString key = i.next();
        if (key.endsWith("Size")) {
            // The actual cache id used from outside doesn't end on "Size".
            key.resize(key.size() - 4);
            i.setValue(key);
        } else {
            i.remove();
        }
    }
    return keys;
}

void Theme::insertIntoRectsCache(const QString& image, const QString &element, const QRectF &rect)
{
    if (!d->useCache()) {
        return;
    }

    if (rect.isValid()) {
        KConfigGroup imageGroup(d->svgElementsCache, image);
        imageGroup.writeEntry(element + QLatin1String("Size"), rect);
    } else {
        QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
        if (it == d->invalidElements.end()) {
            d->invalidElements[image].insert(element);
        } else if (!it.value().contains(element)) {
            if (it.value().count() > 1000) {
                it.value().erase(it.value().begin());
            }

            it.value().insert(element);
        }
    }
}

void Theme::invalidateRectsCache(const QString& image)
{
    if (d->useCache()) {
        KConfigGroup imageGroup(d->svgElementsCache, image);
        imageGroup.deleteGroup();
    }

    d->invalidElements.remove(image);
}

void Theme::releaseRectsCache(const QString &image)
{
    QHash<QString, QSet<QString> >::iterator it = d->invalidElements.find(image);
    if (it != d->invalidElements.end()) {
        if (d->useCache()) {
            KConfigGroup imageGroup(d->svgElementsCache, it.key());
            imageGroup.writeEntry("invalidElements", it.value().toList());
        }

        d->invalidElements.erase(it);
    }
}

void Theme::setCacheLimit(int kbytes)
{
    Q_UNUSED(kbytes)
    if (d->useCache()) {
        ;
        // Too late for you bub.
        // d->pixmapCache->setCacheLimit(kbytes);
    }
}

KUrl Theme::homepage() const
{
    const QString metadataPath(KStandardDirs::locate("data", QLatin1String("desktoptheme/") + d->themeName + QLatin1String("/metadata.desktop")));
    KConfig metadata(metadataPath);
    KConfigGroup brandConfig(&metadata, "Branding");
    return brandConfig.readEntry("homepage", KUrl(QString::fromLatin1(KDE_HOME_URL)));
}

int Theme::toolTipDelay() const
{
    return d->toolTipDelay;
}

}

#include "moc_theme.cpp"
