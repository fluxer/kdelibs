/*  This file is part of the KDE libraries
    Copyright (C) 2006 Chusslove Illich <caslav.ilic@gmx.net>

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

#include <klocalizedstring.h>

#include <config.h>

#include <kglobal.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kuitsemantics_p.h>

#include <QtCore/qmutex.h>
#include <QStringList>
#include <QByteArray>
#include <QChar>
#include <QHash>
#include <QList>
#include <QVector>

// Truncates string, for output of long messages.
static QString shortenMessage(const QString &str)
{
    const int maxlen = 20;
    if (str.length() <= maxlen) {
        return str;
    }
    return str.left(maxlen).append(QLatin1String("..."));
}

typedef qulonglong pluraln;
typedef qlonglong intn;
typedef qulonglong uintn;
typedef double realn;

class KLocalizedStringPrivateStatics;

class KLocalizedStringPrivate
{
    friend class KLocalizedString;

    QStringList args;
    QList<QVariant> vals;
    bool numberSet;
    pluraln number;
    int numberOrd;
    QByteArray ctxt;
    QByteArray msg;
    QByteArray plural;

    QString toString(const KLocale *locale) const;
    QString selectForEnglish() const;
    QString substituteSimple(const QString &trans,
                             const QChar &plchar = QLatin1Char('%'),
                             bool partial = false) const;
    QString postFormat(const QString &text,
                       const QString &lang,
                       const QString &ctxt) const;
};

class KLocalizedStringPrivateStatics
{
public:
    QHash<QString, KuitSemantics*> formatters;

    ~KLocalizedStringPrivateStatics()
    {
        qDeleteAll(formatters);
    }
};
K_GLOBAL_STATIC(KLocalizedStringPrivateStatics, staticsKLSP)
Q_GLOBAL_STATIC(QMutex, staticsKLSPMutex)

KLocalizedString::KLocalizedString ()
    : d(new KLocalizedStringPrivate)
{
    d->numberSet = false;
    d->number = 0;
    d->numberOrd = 0;
}

KLocalizedString::KLocalizedString(const char *ctxt, const char *msg, const char *plural)
    : d(new KLocalizedStringPrivate)
{
    d->ctxt = ctxt;
    d->msg = msg;
    d->plural = plural;
    d->numberSet = false;
    d->number = 0;
    d->numberOrd = 0;
}

KLocalizedString::KLocalizedString(const KLocalizedString &rhs)
    : d(new KLocalizedStringPrivate(*rhs.d))
{
}

KLocalizedString& KLocalizedString::operator=(const KLocalizedString &rhs)
{
    if (&rhs != this) {
        *d = *rhs.d;
    }
    return *this;
}

KLocalizedString::~KLocalizedString()
{
    delete d;
}

bool KLocalizedString::isEmpty() const
{
    return d->msg.isEmpty();
}

QString KLocalizedString::toString() const
{
    return d->toString(KGlobal::locale());
}

QString KLocalizedString::toString(const KLocale *locale) const
{
    return d->toString(locale);
}

QString KLocalizedStringPrivate::toString(const KLocale *locale) const
{
    // Assure the message has been supplied.
    if (msg.isEmpty()) {
        kDebug(173) << "Trying to convert empty KLocalizedString to QString.";
#ifndef NDEBUG
        return QString::fromLatin1("(I18N_EMPTY_MESSAGE)");
#else
        return QString();
#endif
    }

    // Check whether plural argument has been supplied, if message has plural.
    if (!plural.isEmpty() && !numberSet)
        kDebug(173) << QString::fromLatin1("Plural argument to message {%1} not supplied before conversion.")
                              .arg(shortenMessage(QString::fromUtf8(msg)));

    // Get raw translation.
    QString rawtrans, lang;
    if (locale != NULL) {
        if (!ctxt.isEmpty() && !plural.isEmpty()) {
            locale->translateRaw(ctxt, msg, plural, number, &lang, &rawtrans);
        } else if (!plural.isEmpty()) {
            locale->translateRaw(nullptr, msg, plural, number, &lang, &rawtrans);
        } else if (!ctxt.isEmpty()) {
            locale->translateRaw(ctxt, msg, &lang, &rawtrans);
        } else {
            locale->translateRaw(nullptr, msg, &lang, &rawtrans);
        }
    } else {
        lang = KLocale::defaultLanguage();
        rawtrans = selectForEnglish();
    }

    // Substitute placeholders in ordinary translation.
    QString finalstr = substituteSimple(rawtrans);
    // Post-format ordinary translation.
    return postFormat(finalstr, lang, QString::fromLatin1(ctxt));
}

QString KLocalizedStringPrivate::selectForEnglish () const
{
    if (!plural.isEmpty()) {
        if (number == 1) {
            return QString::fromUtf8(msg);
        }
        return QString::fromUtf8(plural);
    }
    return QString::fromUtf8(msg);
}

QString KLocalizedStringPrivate::substituteSimple(const QString &trans,
                                                  const QChar &plchar,
                                                  bool partial) const
{
#ifdef NDEBUG
    Q_UNUSED(partial);
#endif

    QStringList tsegs; // text segments per placeholder occurrence
    QList<int> plords; // ordinal numbers per placeholder occurrence
#ifndef NDEBUG
    QVector<int> ords; // indicates which placeholders are present
#endif
    int slen = trans.length();
    int spos = 0;
    int tpos = trans.indexOf(plchar);
    while (tpos >= 0) {
        int ctpos = tpos;

        tpos++;
        if (tpos == slen)
            break;

        if (trans[tpos].digitValue() > 0) // %0 not considered a placeholder
        {
            // Get the placeholder ordinal.
            int plord = 0;
            while (tpos < slen && trans[tpos].digitValue() >= 0)
            {
                plord = 10 * plord + trans[tpos].digitValue();
                tpos++;
            }
            plord--; // ordinals are zero based

#ifndef NDEBUG
            // Perhaps enlarge storage for indicators.
            // Note that QVector<int> will initialize new elements to 0,
            // as they are supposed to be.
            if (plord >= ords.size()) {
                ords.resize(plord + 1);
            }

            // Indicate that placeholder with computed ordinal is present.
            ords[plord] = 1;
#endif

            // Store text segment prior to placeholder and placeholder number.
            tsegs.append(trans.mid(spos, ctpos - spos));
            plords.append(plord);

            // Position of next text segment.
            spos = tpos;
        }

        tpos = trans.indexOf(plchar, tpos);
    }
    // Store last text segment.
    tsegs.append(trans.mid(spos));

#ifndef NDEBUG
    // Perhaps enlarge storage for plural-number ordinal.
    if (!plural.isEmpty() && numberOrd >= ords.size()) {
        ords.resize(numberOrd + 1);
    }

    // Message might have plural but without plural placeholder, which is an
    // allowed state. To ease further logic, indicate that plural placeholder
    // is present anyway if message has plural.
    if (!plural.isEmpty()) {
        ords[numberOrd] = 1;
    }
#endif

    // Assemble the final string from text segments and arguments.
    QString finalstr;
    for (int i = 0; i < plords.size(); i++) {
        finalstr.append(tsegs.at(i));
        if (plords.at(i) >= args.size()) {
            // too little arguments, put back the placeholder
            finalstr.append(QLatin1Char('%') + QString::number(plords.at(i) + 1));
#ifndef NDEBUG
            if (!partial) {
                // spoof the message
                finalstr.append(QLatin1String("(I18N_ARGUMENT_MISSING)"));
            }
#endif
        } else {
            // just fine
            finalstr.append(args.at(plords.at(i)));
        }
    }
    finalstr.append(tsegs.last());

#ifndef NDEBUG
    if (!partial) {
        // Check that there are no gaps in numbering sequence of placeholders.
        bool gaps = false;
        for (int i = 0; i < ords.size(); i++)
            if (!ords.at(i)) {
                gaps = true;
                kDebug(173) << QString::fromLatin1("Placeholder %%1 skipped in message {%2}.")
                                      .arg(QString::number(i + 1), shortenMessage(trans));
            }
        // If no gaps, check for mismatch between number of unique placeholders and
        // actually supplied arguments.
        if (!gaps && ords.size() != args.size()) {
            kDebug(173) << QString::fromLatin1("%1 instead of %2 arguments to message {%3} supplied before conversion.")
                                  .arg(args.size()).arg(ords.size()).arg(shortenMessage(trans));
        }

        // Some spoofs.
        if (gaps) {
            finalstr.append(QLatin1String("(I18N_GAPS_IN_PLACEHOLDER_SEQUENCE)"));
        }
        if (ords.size() < args.size()) {
            finalstr.append(QLatin1String("(I18N_EXCESS_ARGUMENTS_SUPPLIED)"));
        }
        if (!plural.isEmpty() && !numberSet) {
            finalstr.append(QLatin1String("(I18N_PLURAL_ARGUMENT_MISSING)"));
        }
    }
#endif

    return finalstr;
}

QString KLocalizedStringPrivate::postFormat(const QString &text,
                                            const QString &lang,
                                            const QString &ctxt) const
{
    QMutexLocker lock(staticsKLSPMutex());
    const KLocalizedStringPrivateStatics *s = staticsKLSP;
    // Transform any semantic markup into visual formatting.
    if (s->formatters.contains(lang)) {
        return s->formatters[lang]->format(text, ctxt);
    }
    return text;
}

static QString wrapNum (const QString &tag, const QString &numstr,
                        int fieldWidth, const QChar &fillChar, const int precision)
{
    QString optag;
    if (fieldWidth != 0 || precision >= 0) {
        QString fillString = Qt::escape(fillChar);
        optag = QString::fromLatin1("<%1 width='%2' fill='%3' precision='%4'>")
                       .arg(tag, QString::number(fieldWidth), fillString, QString::number(precision));
    } else {
        optag = QString::fromLatin1("<%1>").arg(tag);
    }
    QString cltag = QString::fromLatin1("</%1>").arg(tag);
    return optag + numstr + cltag;
}

KLocalizedString KLocalizedString::subs(int a, int fieldWidth, int base,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(std::abs(a));
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar, -1));
    kls.d->vals.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(uint a, int fieldWidth, int base,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(a);
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar, -1));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(long a, int fieldWidth, int base,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(std::abs(a));
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar, -1));
    kls.d->vals.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(ulong a, int fieldWidth, int base,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(a);
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar, -1));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(qlonglong a, int fieldWidth, int base,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(qAbs(a));
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar, -1));
    kls.d->vals.append(static_cast<intn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(qulonglong a, int fieldWidth, int base,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    if (!kls.d->plural.isEmpty() && !kls.d->numberSet) {
        kls.d->number = static_cast<pluraln>(a);
        kls.d->numberSet = true;
        kls.d->numberOrd = d->args.size();
    }
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMINTG), QString::number(a, base),
                               fieldWidth, fillChar, -1));
    kls.d->vals.append(static_cast<uintn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(double a, int fieldWidth,
                                        char format, int precision,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(wrapNum(QString::fromLatin1(KUIT_NUMREAL),
                               QString::number(a, format, precision),
                               fieldWidth, fillChar, precision));
    kls.d->vals.append(static_cast<realn>(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(QChar a, int fieldWidth,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    kls.d->args.append(QString::fromLatin1("%1").arg(a, fieldWidth, fillChar));
    kls.d->vals.append(QString(a));
    return kls;
}

KLocalizedString KLocalizedString::subs(const QString &a, int fieldWidth,
                                        const QChar &fillChar) const
{
    KLocalizedString kls(*this);
    // if (!Qt::mightBeRichText(a)) { ...
    // Do not try to auto-escape non-rich-text alike arguments;
    // breaks compatibility with 4.0. Perhaps for KDE 5?
    // Perhaps bad idea alltogether (too much surprise)?
    kls.d->args.append(QString::fromLatin1("%1").arg(a, fieldWidth, fillChar));
    kls.d->vals.append(a);
    return kls;
}

KLocalizedString ki18n(const char* msg)
{
    return KLocalizedString(NULL, msg, NULL);
}

KLocalizedString ki18nc(const char* ctxt, const char *msg)
{
    return KLocalizedString(ctxt, msg, NULL);
}

KLocalizedString ki18np(const char* singular, const char* plural)
{
    return KLocalizedString(NULL, singular, plural);
}

KLocalizedString ki18ncp(const char* ctxt,
                         const char* singular, const char* plural)
{
    return KLocalizedString(ctxt, singular, plural);
}

void KLocalizedString::notifyCatalogsUpdated(const QStringList &languages)
{
    QMutexLocker lock(staticsKLSPMutex());
    if (staticsKLSP.isDestroyed()) {
        return;
    }
    KLocalizedStringPrivateStatics *s = staticsKLSP;
    // Create visual formatters for each new language.
    foreach (const QString &lang, languages) {
        if (!s->formatters.contains(lang)) {
            s->formatters.insert(lang, new KuitSemantics(lang));
        }
    }
}
