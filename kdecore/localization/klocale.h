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

#ifndef KLOCALE_H
#define KLOCALE_H

#include <klocalizedstring.h>
#include <ksharedconfig.h>

#include <QStringList>
#include <QLocale>
#include <QDateTime>

class KDateTime;
class KLocalePrivate;

/**
  *
  * KLocale provides support for country specific stuff like
  * the national language.
  *
  * KLocale supports translating, as well as specifying the format
  * for numbers, time, and date.
  *
  * Use KGlobal::locale() to get pointer to the global KLocale object,
  * containing the applications current locale settings.
  *
  * For example, to format the date May 17, 1995 in the current locale, use:
  *
  * \code
  *   QString date = KGlobal::locale()->formatDate(QDate(1995,5,17));
  * \endcode
  *
  * @author Stephan Kulow <coolo@kde.org>, Preston Brown <pbrown@kde.org>,
  * Hans Petter Bieker <bieker@kde.org>, Lukas Tinkl <lukas.tinkl@suse.cz>
  * @short class for supporting locale settings and national language
  */
class KDECORE_EXPORT KLocale
{
public:
    /**
     * Constructs a KLocale with the given catalog name
     *
     * The constructor looks for an entry Language in the group Locale in the configuration file.
     * If not specified in the configuration file or the one specified in the configuration fiel
     * is not valid the environment variables LC_ALL, LC_CTYPE and LANG will be checked (in that
     * order). If KLocale is not able to use any of the specified languages, the default language
     * (en_US) will be used.
     *
     * If you specify a configuration file, it has to be valid until the KLocale object is
     * destroyed because it may be synced and reparsed.
     *
     * @param catalog the name of the main language file
     * @param config  a configuration file with a Locale group detailing locale-related preferences
     *                (such as language and formatting options).
     */
    explicit KLocale(const QString &catalog, KSharedConfig::Ptr config = KSharedConfig::Ptr());

    /**
     * Constructs a KLocale with the given catalog name
     *
     * Allows you to override the language for this locale.
     *
     * @param catalog  the name of the main language file
     * @param language the complete name for the locale, e.g. "en" for English
     * @param config   a configuration file with a Locale group detailing locale-related
     *                 preferences (such as language and formatting options).
     */
    KLocale(const QString &catalog, const QString &language, KConfig *config = nullptr);

    /**
     * Copy constructor
     */
    KLocale(const KLocale &rhs);

    /**
     * Assignment operator
     */
    KLocale& operator=(const KLocale &rhs);

    /**
     * Destructor
     */
    ~KLocale();

    /**
     * Returns the language code used by this object. Note that this may be different from
     * the language used for translations.
     *
     * Use languageCodeToName(language) to get human readable, localized language name.
     *
     * @return the currently used language code
     *
     * @see languageCodeToName
     */
    QString language() const;

    /**
     * Returns the language codes selected by user, ordered by decreasing priority. Includes
     * additional languages such as the locale name (e.g. "en_US") and the locale language
     * (e.g. "en") for broader matches.
     *
     * Use languageCodeToName(language) to get human readable, localized language name.
     *
     * @return list of language codes
     *
     * @see languageCodeToName
     */
    QStringList languageList() const;

    /**
     * These binary units are used in KDE by the formatByteSize() functions.
     *
     * There are several different units standards:
     *  1) SI  (i.e. metric), powers-of-10.
     *  2) IEC, powers-of-2, with specific units KiB, MiB, etc.
     *  3) JEDEC, powers-of-2, used for solid state memory sizing which is why you see flash cards
     *     labels as e.g. 4GB.  These (ab)use the metric units.  Although JEDEC only defines KB,
     *     MB, GB, if JEDEC is selected all units will be powers-of-2 with metric prefixes for
     *     clarity in the event of sizes larger than 1024 GB.
     *
     * Although 3 different dialects are possible this enum only uses metric names since adding all
     * 3 different names of essentially the same unit would be pointless. Use BinaryUnitDialect to
     * control the exact units returned.
     *
     * @since 4.4
     * @see binaryUnitDialect
     */
    enum BinarySizeUnits {
        /// Auto-choose a unit such that the result is in the range [0, 1000 or 1024)
        DefaultBinaryUnits = -1,

        // The first real unit must be 0 for the current implementation!
        UnitByte      = 0, ///<  B         1 byte
        UnitKiloByte  = 1, ///<  KiB/KB/kB 1024/1000 bytes.
        UnitMegaByte  = 2, ///<  MiB/MB/MB 2^20/10^06 bytes.
        UnitGigaByte  = 3, ///<  GiB/GB/GB 2^30/10^09 bytes.
        UnitTeraByte  = 4, ///<  TiB/TB/TB 2^40/10^12 bytes.
        UnitPetaByte  = 5, ///<  PiB/PB/PB 2^50/10^15 bytes.
        UnitExaByte   = 6, ///<  EiB/EB/EB 2^60/10^18 bytes.
        UnitZettaByte = 7, ///<  ZiB/ZB/ZB 2^70/10^21 bytes.
        UnitYottaByte = 8, ///<  YiB/YB/YB 2^80/10^24 bytes.
        LastBinaryUnit = UnitYottaByte
    };

    /**
     * This enum chooses what dialect is used for binary units.
     *
     * On the other hand network transmission rates are typically in metric so Default, Metric, or
     * IEC (which is unambiguous) should be chosen.
     *
     * Although JEDEC abuses the metric prefixes and can therefore be confusing, it has been
     * used to describe *memory* sizes for quite some time and programs should therefore use either
     * Default, JEDEC, or IEC 60027-2 for memory sizes.
     *
     * @since 4.4
     * @see binaryUnitDialect
     */
    enum BinaryUnitDialect {
        IECBinaryDialect    = 0, ///< KDE Default, KiB, MiB, etc. 2^(10*n)
        JEDECBinaryDialect  = 1, ///< KDE 3.5 default, KB, MB, etc. 2^(10*n)
        MetricBinaryDialect = 2, ///< SI Units, kB, MB, etc. 10^(3*n)
        LastBinaryDialect = MetricBinaryDialect
    };

    /**
     * Returns the user's configured binary unit dialect. e.g. if MetricBinaryDialect is returned
     * then the values configured for how much a set of bytes are worth would be 10^(3*n) and KB
     * (1000 bytes == 1 KB), in this case.
     *
     * @since 4.4
     * @return User's configured binary unit dialect
     * @see BinaryUnitDialect
     */
    BinaryUnitDialect binaryUnitDialect() const;

    /**
     * @since 4.23
     *
     * Returns the currently selected date format.
     *
     * @return Current date format.
     */
    QString dateFormat(QLocale::FormatType format) const;

    /**
     * @since 4.23
     *
     * Returns the currently selected time format.
     *
     * @return Current time format.
     */
    QString timeFormat(QLocale::FormatType format) const;

    /**
     * @since 4.23
     *
     * Returns the currently selected date and time format.
     *
     * @return Current date and time format.
     */
    QString dateTimeFormat(QLocale::FormatType format) const;

    /**
     * Returns which measuring system we use.
     *
     * @return The preferred measuring system
     */
    QLocale::MeasurementSystem measureSystem() const;

    /**
     * Given a double, converts that to a numeric string containing the localized numeric
     * equivalent.
     *
     * Given 123456.78F, returns "123,456.78" (for some European country). If precision isn't
     * specified or is < 0, then the precision is automatically chosen based on the number. This
     * function is a wrapper that is provided for convenience.
     *
     * @param num The number to convert
     * @param precision Number of decimal places used.
     *
     * @return The number as a localized string
     * @see formatNumber(const QString, bool, int)
     */
    QString formatNumber(double num, int precision = -1) const;

    /**
     * Given a string representing a number, converts that to a numeric
     * string containing the localized numeric equivalent.
     *
     * e.g. given 123456.78F, return "123,456.78" (for some European country).
     *
     * If precision isn't specified or is < 0, then the precision is zero.
     *
     * @param numStr The number to format, as a string.
     * @param round Round fractional digits. (default true)
     * @param precision Number of fractional digits used for rounding. Unused if round=false.
     *
     * @return The number as a localized string
     */
    QString formatNumber(const QString &numStr, bool round = true, int precision = -1) const;

    /**
     * Given an integer, converts that to a numeric string containing the localized numeric
     * equivalent.
     *
     * Given 123456L, returns "123,456" (for some European country).
     *
     * @param num The number to convert
     *
     * @return The number as a localized string
     */
    QString formatLong(long num) const;

    /**
     * Converts @p size from bytes to the string representation using the
     * user's default binary unit dialect.  The default unit dialect is
     * IEC 60027-2.
     *
     * Example:
     * formatByteSize(1024) returns "1.0 KiB" by default.
     *
     * @param  size  size in bytes
     * @return converted size as a string - e.g. 123.4 KiB , 12.0 MiB
     * @see BinaryUnitDialect
     */
    QString formatByteSize(double size) const;

    /**
     * @since 4.4
     *
     * Converts @p size from bytes to the appropriate string representation
     * using the binary unit dialect @p dialect and the specific units @p specificUnit.
     *
     * Example:
     * formatByteSize(1000, unit, KLocale::UnitKiloByte) returns:
     *   for KLocale::MetricBinaryDialect, "1.0 kB",
     *   for KLocale::IECBinaryDialect,    "0.9 KiB",
     *   for KLocale::JEDECBinaryDialect,  "0.9 KB".
     *
     * @param size size in bytes
     * @param precision number of places after the decimal point to use.  KDE uses
     *        1 by default so when in doubt use 1.
     * @param specificUnit specific unit size to use in result.  Use
     *        DefaultBinaryUnits to automatically select a unit that will return
     *        a sanely-sized number.
     * @return converted size as a translated string including the units.
     *         E.g. "1.23 KiB", "2 GB" (JEDEC), "4.2 kB" (Metric).
     * @see BinaryUnitDialect
     */
    QString formatByteSize(double size, int precision,
                           BinarySizeUnits specificUnit = KLocale::DefaultBinaryUnits) const;

    /**
     * Given a number of milliseconds, converts that to a string containing the localized
     * equivalent.
     *
     * Given formatDuration(60000), returns "1.0 minutes"
     *
     * @param mSec Time duration in milliseconds
     * @return converted duration as a string - e.g. "5.5 seconds" "23.0 minutes"
     */
    QString formatDuration(unsigned long mSec) const;

    /**
     * Returns a string formatted to the current locale's conventions regarding dates.
     *
     * @param date the date to be formatted
     * @param format category of date format to use
     *
     * @return the date as a string
     */
    QString formatDate(const QDate &date, QLocale::FormatType format = QLocale::ShortFormat) const;

    /**
     *
     * Returns a string formatted to the current locale's conventions regarding times.
     *
     * @param time The time to be formatted.
     * @param format category of time format to use
     *
     * @return The time as a string
     */
    QString formatTime(const QTime &time, QLocale::FormatType format = QLocale::ShortFormat) const;

    /**
     *
     * Returns a string formatted to the current locale's conventions regarding dates and times.
     *
     * @param datetime The date and time to be formatted.
     * @param format category of time format to use
     *
     * @return The date and time as a string
     */
    QString formatDateTime(const QDateTime &datetime, QLocale::FormatType format = QLocale::ShortFormat) const;

    /**
     * Converts a localized numeric string to a double.
     *
     * @param numStr the string we want to convert.
     * @param ok the boolean that is set to false if it's not a number. If @p ok is null, it will
     *        be ignored
     *
     * @return The string converted to a double
     */
    double readNumber(const QString &numStr, bool *ok = nullptr) const;

    /**
     * Converts a localized date string to a QDate.
     *
     * The bool pointed by ok will be invalid if the date entered was not valid.
     *
     * @param str the string we want to convert.
     * @param ok the boolean that is set to false if it's not a valid date. If @p ok is null, it
     *        will be ignored
     *
     * @return The string converted to a QDate
     */
    QDate readDate(const QString &str, bool *ok = nullptr) const;

    /**
     * Converts a localized time string to a QTime.
     *
     * The bool pointed to by @p ok will be set to false if the time entered was not valid.
     *
     * @param str the string we want to convert.
     * @param ok the boolean that is set to false if it's not a valid time. If @p ok is null, it
     *        will be ignored
     *
     * @return The string converted to a QTime
     */
    QTime readTime(const QString &str, bool *ok = nullptr) const;

    /**
     * Adds another catalog to search for translation lookup. This function is useful for extern
     * libraries and/or code, that provide their own messages.
     *
     * If the catalog does not exist for the chosen language, it will be ignored and en_US will be
     * used.
     *
     * @param catalog The catalog to add.
     */
    void insertCatalog(const QString &catalog);

    /**
     * Removes a catalog for translation lookup.
     *
     * @param catalog The catalog to remove.
     * @see insertCatalog()
     */
    void removeCatalog(const QString &catalog);

    /**
     * Sets the active catalog for translation lookup.
     *
     * @param catalog The catalog to activate.
     */
    void setActiveCatalog(const QString &catalog);

    /**
     * @since 4.5
     *
     * Raw translation from all loaded catalogs, with given context. Context + message are used as
     * the lookup key in the catalog.
     *
     * Never use this directly to get message translations. See @p i18n and @p ki18n calls related
     * to KLocalizedString.
     *
     * @param ctxt the context. May be null. Must be UTF-8 encoded.
     * @param msg the message. Must not be null or empty. Must be UTF-8 encoded.
     * @param lang language in which the translation was found. If no translation was found,
     *             KLocale::defaultLanguage() is reported. If null, the language is not reported.
     * @param trans raw translation, or original if not found. If no translation was found,
     *              original message is reported. If null, the translation is not reported.
     *
     * @see KLocalizedString
     */
    void translateRaw(const char *ctxt, const char *msg, QString *lang, QString *trans) const;

    /**
     * @since 4.5
     *
     * Raw translation from all loaded catalogs, with given context and singular/plural form.
     * Context + singular form is used as the lookup key in the catalog.
     *
     * Never use this directly to get message translations. See @p i18n and @p ki18n calls related
     * to KLocalizedString.
     *
     * @param ctxt the context. May be null. Must be UTF-8 encoded.
     * @param singular the singular form. Must not be null or empty. Must be UTF-8 encoded.
     * @param plural the plural form. Must not be null. Must be UTF-8 encoded.
     * @param n number on which the forms are decided.
     * @param lang language in which the translation was found. If no translation was found,
     *             KLocale::defaultLanguage() is reported. If null, the language is not reported.
     * @param trans raw translation, or original if not found. If no translation was found,
     *              original message is reported (either plural or singular, as determined by
     *              @p n). If null, the translation is not reported.
     *
     * @see KLocalizedString
     */
    void translateRaw(const char *ctxt, const char *singular, const char *plural,
                      unsigned long n, QString *lang, QString *trans) const;

    /**
     * Translates a message as a QTranslator is supposed to. The parameters are similar to @p i18n,
     * but the result value has other semantics (it can be empty QString)
     */
    QString translateQt(const char *context, const char *sourceText) const;

    /**
     * Convert a known language code to a human readable, localized form. If an unknown language
     * code is supplied, empty string is returned; this will never happen if the code has been
     * obtained by one of the KLocale methods.
     *
     * @param language the language code
     *
     * @return the human readable and localized form if the code is known, empty otherwise
     *
     * @see language
     * @see languageList
     * @see allLanguagesList
     * @see installedLanguages
     */
    QString languageCodeToName(const QString &language) const;

    /**
     * Convert a known country code to a human readable, localized form.
     *
     * If an unknown country code is supplied, empty string is returned; this will never happen
     * if the code has been obtained by one of the KLocale methods.
     *
     * @param country the country code
     *
     * @return the human readable and localized form of the country name
     */
    QString countryCodeToName(const QString &country) const;

    /**
     * Copies the catalogs of this object to an other KLocale object.
     *
     * @param locale the destination KLocale object
     */
    void copyCatalogsTo(KLocale *locale);

    /**
     * @since 4.1
     *
     * Tries to find a path to the localized file for the given original path. This is intended
     * mainly for non-text resources (images, sounds, etc.), whereas text resources should be
     * handled in more specific ways.
     *
     * The possible localized paths are checked in turn by priority of set languages, in form of
     * dirname/l10n/ll/basename, where dirname and basename are those of the original path, and ll
     * is the language code.
     *
     * KDE core classes which resolve paths internally (e.g. KStandardDirs) will usually perform
     * this lookup behind the scene. In general, you should pipe resource paths through this method
     * only on explicit translators' request, or when a resource is an obvious candidate for
     * localization (e.g. a splash screen or a custom icon with some text drawn on it).
     *
     * @param filePath path to the original file
     *
     * @return path to the localized file if found, original path otherwise
     */
    QString localizedFilePath(const QString &filePath) const;

    /**
     * @since 4.2
     *
     * Removes accelerator marker from a UI text label.
     *
     * Accelerator marker is not always a plain ampersand (&), so it is not enough to just remove
     * it by @c QString::remove(). The label may contain escaped markers ("&&") which must be
     * resolved and skipped, as well as CJK-style markers ("Foo (&F)") where the whole parenthesis
     * construct should be removed. Therefore always use this function to remove accelerator marker
     * from UI labels.
     *
     * @param label UI label which may contain an accelerator marker
     * @return label without the accelerator marker
     */
    QString removeAcceleratorMarker(const QString &label) const;

    /**
     * @since 4.8
     *
     * Reparse locale configuration files for the current selected language.
     */
    void reparseConfiguration();

    /**
     * @since 4.23
     *
     * Converts the KLocale object to QLocale one.
     */
    QLocale toLocale() const;

    /**
     * Provides list of all known language codes.
     *
     * Use languageCodeToName(language) to get human readable, localized language names.
     *
     * @return list of all language codes
     *
     * @see languageCodeToName
     * @see installedLanguages
     */
    static QStringList allLanguagesList();

    /**
     * @since 4.6
     *
     * Provides list of all installed KDE Language Translations.
     *
     * Use languageCodeToName(language) to get human readable, localized language names.
     *
     * @return list of all installed language codes
     *
     * @see languageCodeToName
     */
    static QStringList installedLanguages();

    /**
     * Checks whether or not the active catalog is found for the given language.
     *
     * @param language language to check
     */
    static bool isApplicationTranslatedInto(const QString &language);

    /**
     * Parses locale string into distinct parts.
     *
     * The format of locale is language_COUNTRY@modifier.CHARSET
     *
     * @param locale the locale string to split
     * @param language set to the language part of the locale
     * @param country set to the country part of the locale
     * @param modifier set to the modifer part of the locale
     * @param charset set to the charset part of the locale
     */
    static void splitLocale(const QString &locale, QString &language, QString &country,
                            QString &modifier, QString &charset);

    /**
     * Returns the name of the internal language.
     *
     * @return Name of the default language
     */
    static QString defaultLanguage();

private:
    friend class KLocalePrivate;
    KLocalePrivate *d;
};

#endif // KLOCALE_H
