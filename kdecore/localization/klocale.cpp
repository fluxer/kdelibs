/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "klocale.h"
#include "kcatalog_p.h"
#include "kglobal.h"
#include "kconfig.h"
#include "kconfiggroup.h"
#include "kstandarddirs.h"
#include "kdatetime.h"
#include "kdebug.h"
#include "common_helpers_p.h"

#include <QMutex>
#include <QFileInfo>
#include <QCoreApplication>
#include <qmath.h>

#include <array>

// #define KLOCALE_DUMP

enum KLocaleDuration
{
    KDurationMilisecond = 0,
    KDurationSecond = 1,
    KDurationMinute = 2,
    KDurationHour = 3
};

// catalogs from which each application can draw translations
static QStringList s_defaultcatalogs = QStringList()
    << QString::fromLatin1("kio4")
    << QString::fromLatin1("kdelibs4")
    << QString::fromLatin1("kdeqt")
    << QString::fromLatin1("solid_qt");

static const QLatin1String s_localenamec = QLatin1String("C");

static bool isDefaultLocale(const KLocale *locale)
{
    return (locale->language() == KLocale::defaultLanguage());
}

static QString getDuration(const KLocaleDuration which, const int duration)
{
    switch (which) {
        case KLocaleDuration::KDurationHour: {
            return i18ncp("@item:intext", "1 hour", "%1 hours", duration);
        }
        case KLocaleDuration::KDurationMinute: {
            return i18ncp("@item:intext", "1 minute", "%1 minutes", duration);
        }
        case KLocaleDuration::KDurationSecond: {
            return i18ncp("@item:intext", "1 second", "%1 seconds", duration);
        }
        case KLocaleDuration::KDurationMilisecond: {
            return i18ncp("@item:intext", "1 milisecond", "%1 miliseconds", duration);
        }
    }
    Q_ASSERT(false);
    return QString();
}

static QString getMultiDuration(const KLocaleDuration which, const int duration,
                                const KLocaleDuration which2, const int duration2)
{
    return i18nc("@item:intext", "%1 and %2", getDuration(which, duration), getDuration(which2, duration2));
}

static QString getLanguage(const KLocale *locale)
{
    // NOTE: QLocale::name() always return "foo_BAR", unless the locale is "C" but KLocale uses
    // KLocale::defaultLanguage() instead
    const QString language = locale->language();
    const int underscoreindex = language.indexOf(QLatin1Char('_'));
    Q_ASSERT(underscoreindex > 1);
    return language.mid(0, underscoreindex);
}

class KLocalePrivate
{
public:
    KLocalePrivate(const KLocalePrivate &other);
    KLocalePrivate(const QString &catalog, KSharedConfig::Ptr config);
    KLocalePrivate(const QString &catalog, const QString &language, KConfig *config);
    ~KLocalePrivate();

    KLocale::BinaryUnitDialect binaryUnitDialect;
    std::array<QString, 3> dateFormats;
    std::array<QString, 3> timeFormats;
    std::array<QString, 3> dateTimeFormats;
    QLocale::MeasurementSystem measurementSystem;
    QLocale locale;
    QString catalog;
    QStringList languagelist;
    QStringList manualcatalogs;
    QList<KCatalog> catalogs;
    KConfigGroup configgroup;
    QMutex *mutex;
};

#ifdef KLOCALE_DUMP
static void dumpKLocaleCatalogs(const KLocalePrivate *locale)
{
    qDebug() << "Catalogs for" << locale->locale.name() << locale->catalogs.size();
    foreach (const KCatalog &catalog, locale->catalogs) {
        qDebug() << "Catalog for" << locale->locale.name();
        qDebug() << "    -> name =" << catalog.name();
        qDebug() << "    -> language =" << catalog.language();
    }
}
#endif

KLocalePrivate::KLocalePrivate(const KLocalePrivate &other)
    : binaryUnitDialect(other.binaryUnitDialect),
    dateFormats(other.dateFormats),
    timeFormats(other.timeFormats),
    dateTimeFormats(other.dateTimeFormats),
    measurementSystem(other.measurementSystem),
    locale(other.locale),
    catalog(other.catalog),
    languagelist(other.languagelist),
    manualcatalogs(other.manualcatalogs),
    catalogs(other.catalogs),
    configgroup(other.configgroup),
    mutex(new QMutex())
{
}

KLocalePrivate::KLocalePrivate(const QString &_catalog, KSharedConfig::Ptr config)
    : binaryUnitDialect(KLocale::IECBinaryDialect),
    measurementSystem(QLocale::MetricSystem),
    catalog(_catalog),
    mutex(new QMutex())
{
    if (config) {
        configgroup = config->group("Locale");
    } else {
        configgroup = KGlobal::config()->group("Locale");
    }

    // locale from the config overrides everything (not Unix-like but that's how it should be)
    const QString configlanguage = configgroup.readEntry("Language", QString());
    locale = QLocale(configlanguage);
    // if no locale was specified or QLocale does not support the specified language use the system
    // locale
    if (locale.name() == s_localenamec) {
        locale = QLocale::system();
    }
    // finally, if the locale is C for compat fallback to KLocale::defaultLanguage()
    if (locale.name() == s_localenamec) {
        locale = QLocale(KLocale::defaultLanguage());
    }
}

KLocalePrivate::KLocalePrivate(const QString &_catalog, const QString &language, KConfig *config)
    : binaryUnitDialect(KLocale::IECBinaryDialect),
    measurementSystem(QLocale::MetricSystem),
    catalog(_catalog),
    mutex(new QMutex())
{
    locale = QLocale(language);
    // fallback to the default
    if (locale.name() == s_localenamec) {
        locale = QLocale(KLocale::defaultLanguage());
    }

    if (config) {
        configgroup = config->group("Locale");
    } else {
        configgroup = KGlobal::config()->group("Locale");
    }
}

KLocalePrivate::~KLocalePrivate()
{
    delete mutex;
}

KLocale::KLocale(const QString &catalog, KSharedConfig::Ptr config)
    : d(new KLocalePrivate(catalog, config))
{
    reparseConfiguration();
}

KLocale::KLocale(const QString &catalog, const QString &language, KConfig *config)
    : d(new KLocalePrivate(catalog, language, config))
{
    reparseConfiguration();
}

KLocale::~KLocale()
{
    delete d;
}

KLocale::KLocale(const KLocale &rhs)
    : d(new KLocalePrivate(*rhs.d))
{
}

KLocale & KLocale::operator=(const KLocale &rhs)
{
    // the assignment operator works here
    *d = *rhs.d;
    return *this;
}

QString KLocale::language() const
{
    return d->locale.name();
}

QStringList KLocale::languageList() const
{
    return d->languagelist;
}

KLocale::BinaryUnitDialect KLocale::binaryUnitDialect() const
{
    return d->binaryUnitDialect;
}

QString KLocale::dateFormat(QLocale::FormatType format) const
{
    return d->dateFormats[format];
}

QString KLocale::timeFormat(QLocale::FormatType format) const
{
    return d->timeFormats[format];
}

QString KLocale::dateTimeFormat(QLocale::FormatType format) const
{
    return d->dateTimeFormats[format];
}

QLocale::MeasurementSystem KLocale::measureSystem() const
{
    return d->measurementSystem;
}

QString KLocale::formatNumber(double num, int precision) const
{
    if (precision <= 0) {
        // NOTE: kcalc for example passes 0 for precision but the constants should no be round
        // otherwise the whole meaning of Pi (3.14) being non-rounded number is lost so instead of
        // forcing a number rounding the precision is automatically chosen here
        return d->locale.toString(num, 'f', qRound(num) == num ? 0 : 2);
    }
    return d->locale.toString(num, 'f', precision);
}

QString KLocale::formatNumber(const QString &numStr, bool round, int precision) const
{
    if (round) {
        return formatNumber(qRound(numStr.toDouble()), 0);
    }
    return formatNumber(numStr.toDouble(), precision);
}

QString KLocale::formatLong(long num) const
{
    return d->locale.toString(qlonglong(num));
}

QString KLocale::formatByteSize(double size, int precision,
                                KLocale::BinarySizeUnits specificUnit) const
{
    double sizeinunit = size;
    int sizeprecision = precision;
    KLocale::BinarySizeUnits sizeunit = specificUnit;

    double sizemultiplier = double(1024.0);
    if (d->binaryUnitDialect == KLocale::MetricBinaryDialect) {
        sizemultiplier = double(1000.0);
    }

    if (sizeunit == KLocale::DefaultBinaryUnits) {
        Q_ASSERT(int(KLocale::UnitByte) == 0);
        int counter = 0;
        static const int s_lastunitasint = static_cast<int>(KLocale::LastBinaryUnit);
        while (qAbs(sizeinunit) >= sizemultiplier && counter < s_lastunitasint) {
            sizeinunit = (sizeinunit / sizemultiplier);
            counter++;
        }
        sizeunit = static_cast<KLocale::BinarySizeUnits>(counter);
    } else if (sizeunit != KLocale::UnitByte) {
        sizeinunit = (sizeinunit / qPow(sizemultiplier, static_cast<int>(sizeunit)));
    }

    if (sizeinunit == double(0.0)) {
        // 0.0 is zero
        sizeprecision = 0;
    }

    QString sizetranslation;
    switch (d->binaryUnitDialect) {
        case KLocale::MetricBinaryDialect: {
            switch (sizeunit) {
                case KLocale::UnitByte: {
                    translateRaw("size in bytes", "%1 B", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitKiloByte: {
                    translateRaw("size in 1000 bytes", "%1 kB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitMegaByte: {
                    translateRaw("size in 10^6 bytes", "%1 MB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitGigaByte: {
                    translateRaw("size in 10^9 bytes", "%1 GB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitTeraByte: {
                    translateRaw("size in 10^12 bytes", "%1 TB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitPetaByte: {
                    translateRaw("size in 10^15 bytes", "%1 PB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitExaByte: {
                    translateRaw("size in 10^18 bytes", "%1 EB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitZettaByte: {
                    translateRaw("size in 10^21 bytes", "%1 ZB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitYottaByte: {
                    translateRaw("size in 10^24 bytes", "%1 YB", nullptr, &sizetranslation);
                    break;
                }
                default: {
                    Q_ASSERT(false);
                    break;
                }
            }
        }
        case KLocale::JEDECBinaryDialect: {
            switch (sizeunit) {
                case KLocale::UnitByte: {
                    translateRaw("size in bytes", "%1 B", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitKiloByte: {
                    translateRaw("memory size in 1024 bytes", "%1 KB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitMegaByte: {
                    translateRaw("memory size in 2^20 bytes", "%1 MB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitGigaByte: {
                    translateRaw("memory size in 2^30 bytes", "%1 GB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitTeraByte: {
                    translateRaw("memory size in 2^40 bytes", "%1 TB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitPetaByte: {
                    translateRaw("memory size in 2^50 bytes", "%1 PB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitExaByte: {
                    translateRaw("memory size in 2^60 bytes", "%1 EB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitZettaByte: {
                    translateRaw("memory size in 2^70 bytes", "%1 ZB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitYottaByte: {
                    translateRaw("memory size in 2^80 bytes", "%1 YB", nullptr, &sizetranslation);
                    break;
                }
                default: {
                    Q_ASSERT(false);
                    break;
                }
            }
        }
        case KLocale::IECBinaryDialect:
        default: {
            switch (sizeunit) {
                case KLocale::UnitByte: {
                    translateRaw("size in bytes", "%1 B", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitKiloByte: {
                    translateRaw("size in 1024 bytes", "%1 KiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitMegaByte: {
                    translateRaw("size in 2^20 bytes", "%1 MiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitGigaByte: {
                    translateRaw("size in 2^30 bytes", "%1 GiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitTeraByte: {
                    translateRaw("size in 2^40 bytes", "%1 TiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitPetaByte: {
                    translateRaw("size in 2^50 bytes", "%1 PiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitExaByte: {
                    translateRaw("size in 2^60 bytes", "%1 EiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitZettaByte: {
                    translateRaw("size in 2^70 bytes", "%1 ZiB", nullptr, &sizetranslation);
                    break;
                }
                case KLocale::UnitYottaByte: {
                    translateRaw("size in 2^80 bytes", "%1 YiB", nullptr, &sizetranslation);
                    break;
                }
                default: {
                    Q_ASSERT(false);
                    break;
                }
            }
        }
    }
    return sizetranslation.arg(formatNumber(sizeinunit, sizeprecision));
}

QString KLocale::formatByteSize(double size) const
{
    return formatByteSize(size, 1, KLocale::DefaultBinaryUnits);
}

QString KLocale::formatDuration(unsigned long mSec) const
{
    QTime durationtime;
    durationtime = durationtime.addMSecs(mSec);
    const int hours = durationtime.hour();
    const int minutes = durationtime.minute();
    const int seconds = durationtime.second();
    const int miliseconds = durationtime.msec();

    if (hours && minutes) {
        return getMultiDuration(
            KLocaleDuration::KDurationHour, hours,
            KLocaleDuration::KDurationMinute, minutes
        );
    } else if (hours) {
        return getDuration(
            KLocaleDuration::KDurationHour, hours
        );
    } else if (minutes && seconds) {
        return getMultiDuration(
            KLocaleDuration::KDurationMinute, minutes,
            KLocaleDuration::KDurationSecond, seconds
        );
    } else if (minutes) {
        return getDuration(
            KLocaleDuration::KDurationMinute, minutes
        );
    } else if (seconds && miliseconds) {
        return getMultiDuration(
            KLocaleDuration::KDurationSecond, seconds,
            KLocaleDuration::KDurationMilisecond, miliseconds
        );
    } else if (seconds) {
        return getDuration(
            KLocaleDuration::KDurationSecond, seconds
        );
    }
    return getDuration(
        KLocaleDuration::KDurationMilisecond, miliseconds
    );
}

QString KLocale::formatDate(const QDate &date, QLocale::FormatType format) const
{
    return d->locale.toString(date, d->dateFormats[format]);
}

QString KLocale::formatTime(const QTime &time, QLocale::FormatType format) const
{
    return d->locale.toString(time, d->timeFormats[format]);
}

QString KLocale::formatDateTime(const QDateTime &dateTime, QLocale::FormatType format) const
{
    return d->locale.toString(dateTime, d->dateTimeFormats[format]);
}

double KLocale::readNumber(const QString &str, bool *ok) const
{
    return d->locale.toDouble(str, ok);
}

QDate KLocale::readDate(const QString &intstr, bool *ok) const
{
    if (!ok) {
        return d->locale.toDate(intstr);
    }
    const QDate result = d->locale.toDate(intstr);
    *ok = result.isValid();
    return result;
}

QTime KLocale::readTime(const QString &intstr, bool *ok) const
{
    if (!ok) {
        return d->locale.toTime(intstr);
    }
    const QTime result = d->locale.toTime(intstr);
    *ok = result.isValid();
    return result;
}

void KLocale::insertCatalog(const QString &catalog)
{
    QMutexLocker locker(d->mutex);
    const QStringList cataloglanguages = languageList();
    foreach (const QString &cataloglanguage, cataloglanguages) {
        if (KCatalog::hasCatalog(catalog, cataloglanguage)) {
            d->catalogs.append(KCatalog(catalog, cataloglanguage));
            if (!d->manualcatalogs.contains(catalog)) {
                d->manualcatalogs.append(catalog);
            }
        }
    }
#ifdef KLOCALE_DUMP
    dumpKLocaleCatalogs(d);
#endif
    KLocalizedString::notifyCatalogsUpdated(cataloglanguages);
}

void KLocale::removeCatalog(const QString &catalog)
{
    QMutexLocker locker(d->mutex);
    const QStringList cataloglanguages = languageList();
    QMutableListIterator<KCatalog> catalogsiter(d->catalogs);
    while (catalogsiter.hasNext()) {
        const QString catalogname = catalogsiter.next().name();
        if (catalogname == catalog) {
            catalogsiter.remove();
            d->manualcatalogs.removeAll(catalog);
        }
    }
#ifdef KLOCALE_DUMP
    dumpKLocaleCatalogs(d);
#endif
    KLocalizedString::notifyCatalogsUpdated(cataloglanguages);
}

void KLocale::setActiveCatalog(const QString &catalog)
{
    QMutexLocker locker(d->mutex);
    for (int i = 1; i < d->catalogs.size(); i++) {
        if (d->catalogs.at(i).name() == catalog) {
            d->catalogs.move(i, 0);
            i = 1;
        }
    }
    KLocalizedString::notifyCatalogsUpdated(languageList());
}

void KLocale::translateRaw(const char *ctxt, const char *msg, QString *lang, QString *trans) const
{
    {
        QMutexLocker locker(d->mutex);
        foreach (const KCatalog &catalog, d->catalogs) {
            QString result = catalog.translateStrict(ctxt, msg);
            if (!result.isEmpty()) {
                *trans = result;
                if (lang) {
                    *lang = catalog.language();
                }
                return;
            }
        }
    }
    if (lang) {
        *lang = KLocale::defaultLanguage();
    }
    *trans = QString::fromUtf8(msg);
}

void KLocale::translateRaw(const char *ctxt, const char *singular, const char *plural,
                           unsigned long n, QString *lang, QString *trans) const
{
    {
        QMutexLocker locker(d->mutex);
        foreach (const KCatalog &catalog, d->catalogs) {
            QString result = catalog.translateStrict(ctxt, singular, plural, n);
            if (!result.isEmpty()) {
                *trans = result;
                if (lang) {
                    *lang = catalog.language();
                }
                return;
            }
        }
    }
    if (lang) {
        *lang = KLocale::defaultLanguage();
    }
    if (!plural || n == 1) {
        *trans = QString::fromUtf8(singular);
    } else {
        *trans = QString::fromUtf8(plural);
    }
}

QString KLocale::translateQt(const char *context, const char *sourceText) const
{
    // return empty according to Katie's expectations
    QString result;
    if (isDefaultLocale(this)) {
        return result;
    }
    QMutexLocker locker(d->mutex);
    foreach (const KCatalog &catalog, d->catalogs) {
        result = catalog.translateStrict(context, sourceText);
        if (!result.isEmpty()) {
            break;
        }
    }
    return result;
}

QString KLocale::languageCodeToName(const QString &language) const
{
    QString result;
    const QString entryfile = KStandardDirs::locate("locale", language + QLatin1String("/entry.desktop"));
    if (!entryfile.isEmpty()) {
        KConfig entryconfig(entryfile);
        entryconfig.setLocale(getLanguage(this));
        KConfigGroup entrygroup(&entryconfig, "KCM Locale");
        result = entrygroup.readEntry("Name");
    }
    if (result.isEmpty()) {
        // in case the language is not installed
        KConfig languagesconfig(QLatin1String("all_languages"), KConfig::NoGlobals, "locale");
        languagesconfig.setLocale(getLanguage(this));
        KConfigGroup languagegroup(&languagesconfig, language);
        result = languagegroup.readEntry("Name");
    }
    if (result.isEmpty()) {
        // not translated at all
        QLocale locale(language);
        result = QLocale::languageToString(locale.language());
    }
    return result;
}

QString KLocale::countryCodeToName(const QString &country) const
{
    QString result;
    const QString entryfile = KStandardDirs::locate("locale", QString::fromLatin1("l10n/") + country.toLower() + QLatin1String("/entry.desktop"));
    if (!entryfile.isEmpty()) {
        KConfig entryconfig(entryfile);
        entryconfig.setLocale(getLanguage(this));
        KConfigGroup entrygroup(&entryconfig, "KCM Locale");
        result = entrygroup.readEntry("Name");
    }
    if (result.isEmpty()) {
        // not translated at all (e.g. 001 - world)
        result = country;
    }
    return result;
}

void KLocale::copyCatalogsTo(KLocale *locale)
{
    QMutexLocker locker(d->mutex);
    d->catalogs = locale->d->catalogs;
    KLocalizedString::notifyCatalogsUpdated(languageList());
}

QString KLocale::localizedFilePath(const QString &filePath) const
{
    // Stop here if the default language is primary.
    if (isDefaultLocale(this)) {
        return filePath;
    }

    // Check if l10n sudir is present, stop if not.
    QFileInfo fileInfo(filePath);
    const QString locDirPath = fileInfo.path() + QLatin1String("/l10n");
    QFileInfo locDirInfo(locDirPath);
    if (!locDirInfo.isDir()) {
        return filePath;
    }

    // Go through possible localized paths by priority of languages, return first that exists.
    const QString fileName = fileInfo.fileName();
    foreach(const QString &lang, languageList()) {
        // Stop when the default language is reached.
        if (lang == KLocale::defaultLanguage()) {
            return filePath;
        }
        const QString locFilePath = locDirPath + QLatin1Char('/') + lang + QLatin1Char('/') + fileName;
        QFileInfo locFileInfo(locFilePath);
        if (locFileInfo.isFile() && locFileInfo.isReadable()) {
            return locFilePath;
        }
    }

    return filePath;
}

QString KLocale::removeAcceleratorMarker(const QString &label) const
{
    return ::removeAcceleratorMarker(label);
}

void KLocale::reparseConfiguration()
{
    QMutexLocker locker(d->mutex);

    d->configgroup.sync();

    d->binaryUnitDialect = static_cast<KLocale::BinaryUnitDialect>(
        d->configgroup.readEntry("BinaryUnitDialect", int(KLocale::IECBinaryDialect))
    );
    Q_ASSERT(int(QLocale::LongFormat) == 0);
    Q_ASSERT(int(QLocale::NarrowFormat) == 2);
    d->dateFormats[QLocale::LongFormat] = d->configgroup.readEntry("LongDateFormat", d->locale.dateFormat(QLocale::LongFormat));
    d->dateFormats[QLocale::ShortFormat] = d->configgroup.readEntry("ShortDateFormat", d->locale.dateFormat(QLocale::ShortFormat));
    d->dateFormats[QLocale::NarrowFormat] = d->configgroup.readEntry("NarrowDateFormat", d->locale.dateFormat(QLocale::NarrowFormat));
    d->timeFormats[QLocale::LongFormat] = d->configgroup.readEntry("LongTimeFormat", d->locale.timeFormat(QLocale::LongFormat));
    d->timeFormats[QLocale::ShortFormat] = d->configgroup.readEntry("ShortTimeFormat", d->locale.timeFormat(QLocale::ShortFormat));
    d->timeFormats[QLocale::NarrowFormat] = d->configgroup.readEntry("NarrowTimeFormat", d->locale.timeFormat(QLocale::NarrowFormat));
    d->dateTimeFormats[QLocale::LongFormat] = d->configgroup.readEntry("LongDateTimeFormat", d->locale.dateTimeFormat(QLocale::LongFormat));
    d->dateTimeFormats[QLocale::ShortFormat] = d->configgroup.readEntry("ShortDateTimeFormat", d->locale.dateTimeFormat(QLocale::ShortFormat));
    d->dateTimeFormats[QLocale::NarrowFormat] = d->configgroup.readEntry("NarrowDateTimeFormat", d->locale.dateTimeFormat(QLocale::NarrowFormat));
    d->measurementSystem = static_cast<QLocale::MeasurementSystem>(
        d->configgroup.readEntry("MeasurementSystem", int(d->locale.measurementSystem()))
    );

    d->languagelist.clear();
    // translation languages in the config
    const QStringList configtranslations = d->configgroup.readEntry("Translations", QStringList());
    if (!configtranslations.isEmpty()) {
        d->languagelist.append(configtranslations);
    }
    // the locale name itself (e.g. "en_US")
    const QString localename = d->locale.name();
    d->languagelist.append(localename);
    // the language only (e.g. "en")
    d->languagelist.append(getLanguage(this));
    // default as fallback, unless the locale language is the default
    if (localename != KLocale::defaultLanguage()) {
        d->languagelist.append(KLocale::defaultLanguage());
    }
    // qDebug() << Q_FUNC_INFO << d->languagelist;

    const QStringList cataloglanguages = languageList();
    d->catalogs.clear();
    foreach (const QString &cataloglanguage, cataloglanguages) {
        if (KCatalog::hasCatalog(d->catalog, cataloglanguage)) {
            d->catalogs.append(KCatalog(d->catalog, cataloglanguage));
        }
        foreach (const QString &defaultcatalog, s_defaultcatalogs) {
            if (KCatalog::hasCatalog(defaultcatalog, cataloglanguage)) {
                d->catalogs.append(KCatalog(defaultcatalog, cataloglanguage));
            }
        }
        foreach (const QString &manualcatalog, d->manualcatalogs) {
            // assume it has already been checked for by KLocale::insertCatalog()
            d->catalogs.append(KCatalog(manualcatalog, cataloglanguage));
        }
    }

#ifdef KLOCALE_DUMP
    dumpKLocaleCatalogs(d);
#endif

    KLocalizedString::notifyCatalogsUpdated(cataloglanguages);
}

QLocale KLocale::toLocale() const
{
    return d->locale;
}

QStringList KLocale::allLanguagesList()
{
    QStringList result;
    const QList<QLocale> alllocales = QLocale::matchingLocales(
        QLocale::AnyLanguage, QLocale::AnyScript, QLocale::AnyCountry
    );
    result.reserve(alllocales.size());
    foreach (const QLocale &locale, alllocales) {
        result.append(locale.name());
    }
    // NOTE: the locale name does not include the script (it includes the ISO 639 language code
    // and the ISO 3166 country code) so there are duplicates
    result.removeDuplicates();
    return result;
}

QStringList KLocale::installedLanguages()
{
    QStringList result;
    const QStringList paths = KGlobal::dirs()->findAllResources("locale", QLatin1String("*/entry.desktop"));
    foreach (const QString &path, paths) {
        const QString part = path.left(path.length() - 14);
        result.append(part.mid(part.lastIndexOf(QLatin1Char('/')) + 1));
    }
    result.sort();
    return result;
}

bool KLocale::isApplicationTranslatedInto(const QString &lang)
{
    if (lang.isEmpty()) {
        return false;
    }
    if (lang == KLocale::defaultLanguage()) {
        // default language is always "installed"
        return true;
    }
    // check if the application itself was translated
    if (KCatalog::hasCatalog(QCoreApplication::applicationName(), lang)) {
        return true;
    }
    // check for partial translations from one of the default catalogs
    foreach (const QString &defaultcatalog, s_defaultcatalogs) {
        if (KCatalog::hasCatalog(defaultcatalog, lang)) {
            return true;
        }
    }
    return false;
}

void KLocale::splitLocale(const QString &locale, QString &language, QString &country, QString &modifier,
                          QString &charset)
{
    QString localecopy = locale;

    language.clear();
    country.clear();
    modifier.clear();
    charset.clear();

    // In case there are several concatenated locale specifications,
    // truncate all but first.
    int f = localecopy.indexOf(QLatin1Char(':'));
    if (f >= 0) {
        localecopy.truncate(f);
    }

    f = localecopy.indexOf(QLatin1Char('.'));
    if (f >= 0) {
        charset = localecopy.mid(f + 1);
        localecopy.truncate(f);
    }

    f = localecopy.indexOf(QLatin1Char('@'));
    if (f >= 0) {
        modifier = localecopy.mid(f + 1);
        localecopy.truncate(f);
    }

    f = localecopy.indexOf(QLatin1Char('_'));
    if (f >= 0) {
        country = localecopy.mid(f + 1);
        localecopy.truncate(f);
    }

    language = localecopy;
}

QString KLocale::defaultLanguage()
{
    return QString::fromLatin1("en_US");
}
