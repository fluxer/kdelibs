/* This file is part of the KDE libraries
   Copyright (c) 2001 Hans Petter Bieker <bieker@kde.org>

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

#include "kcatalog_p.h"
#include "kstandarddirs.h"
#include <kdebug.h>

#include <QFile>
#include <QTranslator>

static QByteArray translationData(const QByteArray &localeDir, const QByteArray &language, const QByteArray &name)
{
    QByteArray translationpath = localeDir;
    translationpath.append('/');
    translationpath.append(language);
    translationpath.append('/');
    translationpath.append(name);
    translationpath.append(".tr");
    QFile translationfile(QFile::decodeName(translationpath));
    if (!translationfile.open(QFile::ReadOnly)) {
        return QByteArray();
    }
    return translationfile.readAll();
}

class KCatalogPrivate
{
public:
    KCatalogPrivate();
    ~KCatalogPrivate();

    QByteArray language;
    QByteArray name;
    QByteArray localeDir;

#ifndef QT_NO_TRANSLATION
    QTranslator* translator;
#endif
};

KCatalogPrivate::KCatalogPrivate()
    : translator(nullptr)
{
}

KCatalogPrivate::~KCatalogPrivate()
{
    delete translator;
}

QDebug operator<<(QDebug debug, const KCatalog &c)
{
    return debug << c.d->language << " " << c.d->name << " " << c.d->localeDir;
}

KCatalog::KCatalog(const QString &name, const QString &language)
    : d(new KCatalogPrivate())
{
    d->language = QFile::encodeName(language);
    d->name = QFile::encodeName(name);
    d->localeDir = QFile::encodeName(catalogLocaleDir(name, language));

#ifndef QT_NO_TRANSLATION
    d->translator = new QTranslator();
    d->translator->loadFromData(translationData(d->localeDir, d->language, d->name));
    // kDebug() << << name << language << localeDir;
#endif
}

KCatalog::KCatalog(const KCatalog &rhs)
    : d(new KCatalogPrivate())
{
    d->language = rhs.d->language;
    d->name = rhs.d->name;
    d->localeDir = rhs.d->localeDir;

#ifndef QT_NO_TRANSLATION
    d->translator = new QTranslator();
    d->translator->loadFromData(translationData(d->localeDir, d->language, d->name));
#endif
}

KCatalog & KCatalog::operator=(const KCatalog & rhs)
{
    d->language = rhs.d->language;
    d->name = rhs.d->name;
    d->localeDir = rhs.d->localeDir;

#ifndef QT_NO_TRANSLATION
    d->translator->loadFromData(translationData(d->localeDir, d->language, d->name));
#endif
    return *this;
}

KCatalog::~KCatalog()
{
    delete d;
}

QString KCatalog::catalogLocaleDir(const QString &name,
                                   const QString &language)
{
    QString relpath =  QString::fromLatin1("%1/%2.tr").arg(language).arg(name);
    return KGlobal::dirs()->findResourceDir("locale", relpath);
}

QString KCatalog::name() const
{
    return QFile::decodeName(d->name);
}

QString KCatalog::language() const
{
    return QFile::decodeName(d->language);
}

QString KCatalog::localeDir() const
{
  return QFile::decodeName(d->localeDir);
}

QString KCatalog::translate(const char * msgid) const
{
#ifndef QT_NO_TRANSLATION
    return d->translator->translate(nullptr, msgid);
#else
    return QString::fromUtf8(msgid);
#endif
}

QString KCatalog::translate(const char * msgctxt, const char * msgid) const
{
#ifndef QT_NO_TRANSLATION
    return d->translator->translate(msgctxt, msgid);
#else
    Q_UNUSED(msgctxt);
    return QString::fromUtf8(msgid);
#endif
}

QString KCatalog::translate(const char * msgid, const char * msgid_plural,
                            unsigned long n) const
{
#ifndef QT_NO_TRANSLATION
    return (n == 1 ? d->translator->translate(nullptr, msgid) : d->translator->translate(nullptr, msgid_plural));
#else
    return (n == 1 ? QString::fromUtf8(msgid) : QString::fromUtf8(msgid_plural));
#endif
}

QString KCatalog::translate(const char * msgctxt, const char * msgid,
                            const char * msgid_plural, unsigned long n) const
{
#ifndef QT_NO_TRANSLATION
    return (n == 1 ? d->translator->translate(msgctxt, msgid) : d->translator->translate(msgctxt, msgid_plural));
#else
    Q_UNUSED(msgctxt);
    return (n == 1 ? QString::fromUtf8(msgid) : QString::fromUtf8(msgid_plural));
#endif
}

QString KCatalog::translateStrict(const char * msgid) const
{
#ifndef QT_NO_TRANSLATION
    return d->translator->translateStrict(nullptr, msgid);
#else
    Q_UNUSED(msgid);
    return QString();
#endif
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid) const
{
#ifndef QT_NO_TRANSLATION
    return d->translator->translateStrict(msgctxt, msgid);
#else
    Q_UNUSED(msgctxt);
    Q_UNUSED(msgid);
    return QString();
#endif
}

QString KCatalog::translateStrict(const char * msgid, const char * msgid_plural,
                                  unsigned long n) const
{
#ifndef QT_NO_TRANSLATION
    return (n == 1 ? d->translator->translateStrict(nullptr, msgid) : d->translator->translateStrict(nullptr, msgid_plural));
#else
    Q_UNUSED(msgid);
    Q_UNUSED(msgid_plural);
    Q_UNUSED(n);
    return QString();
#endif
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid,
                                  const char * msgid_plural, unsigned long n) const
{
#ifndef QT_NO_TRANSLATION
    return (n == 1 ? d->translator->translateStrict(msgctxt, msgid) : d->translator->translateStrict(msgctxt, msgid_plural));
#else
    Q_UNUSED(msgctxt);
    Q_UNUSED(msgid);
    Q_UNUSED(msgid_plural);
    Q_UNUSED(n);
    return QString();
#endif
}

