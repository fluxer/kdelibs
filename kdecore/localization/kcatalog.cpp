/*  This file is part of the KDE libraries
    Copyright (C) 2022 Ivailo Monev <xakepa10@gmail.com>

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

#include "kcatalog_p.h"
#include "kstandarddirs.h"
#include <kdebug.h>

#include <QFile>

class KCatalogPrivate
{
public:
    KCatalogPrivate();
    ~KCatalogPrivate();

    QString language;
    QString name;

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
    return debug << c.d->language << " " << c.d->name;
}

KCatalog::KCatalog(const QString &name, const QString &language)
    : d(new KCatalogPrivate())
{
    d->language = language;
    d->name = name;

#ifndef QT_NO_TRANSLATION
    d->translator = new QTranslator();
    KCatalog::loadCatalog(d->name, d->language, d->translator);
    // kDebug() << << name << language;
#endif
}

KCatalog::KCatalog(const KCatalog &rhs)
    : d(new KCatalogPrivate())
{
    d->language = rhs.d->language;
    d->name = rhs.d->name;

#ifndef QT_NO_TRANSLATION
    d->translator = new QTranslator();
    KCatalog::loadCatalog(d->name, d->language, d->translator);
#endif
}

KCatalog & KCatalog::operator=(const KCatalog &rhs)
{
    d->language = rhs.d->language;
    d->name = rhs.d->name;

#ifndef QT_NO_TRANSLATION
    KCatalog::loadCatalog(d->name, d->language, d->translator);
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
    const QString relpath =  QString::fromLatin1("%1/%2.tr").arg(language).arg(name);
    return KGlobal::dirs()->findResourceDir("locale", relpath);
}

bool KCatalog::loadCatalog(const QString &name, const QString &language, QTranslator *translator)
{
    const QString relpath =  QString::fromLatin1("%1/%2.tr").arg(language).arg(name);
    const QString translationpath = KGlobal::dirs()->locate("locale", relpath);
    if (translationpath.isEmpty()) {
        return false;
    }
    QFile translationfile(translationpath);
    if (!translationfile.open(QFile::ReadOnly)) {
        return false;
    }
    return translator->loadFromData(translationfile.readAll());
}


QString KCatalog::name() const
{
    return d->name;
}

QString KCatalog::language() const
{
    return d->language;
}

QString KCatalog::translate(const char *msgctxt, const char *msgid) const
{
#ifndef QT_NO_TRANSLATION
    return d->translator->translate(msgctxt, msgid);
#else
    Q_UNUSED(msgctxt);
    return QString::fromUtf8(msgid);
#endif
}

QString KCatalog::translate(const char *msgctxt, const char *msgid,
                            const char *msgid_plural, unsigned long n) const
{
#ifndef QT_NO_TRANSLATION
    return (n == 1 ? d->translator->translate(msgctxt, msgid) : d->translator->translate(msgctxt, msgid_plural));
#else
    Q_UNUSED(msgctxt);
    return (n == 1 ? QString::fromUtf8(msgid) : QString::fromUtf8(msgid_plural));
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

QString KCatalog::translateStrict(const char *msgctxt, const char *msgid,
                                  const char *msgid_plural, unsigned long n) const
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

