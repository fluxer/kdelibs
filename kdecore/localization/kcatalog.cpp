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

#include <config.h>

#include <QtCore/QFile>
#include <QtCore/qmutex.h>

#include <kdebug.h>

#include <stdlib.h>
#include <locale.h>

#ifdef HAVE_LIBINTL
#  include <libintl.h>
#endif

// = "LANGUAGE=" + 32 chars for language code + terminating zero
static const int langenvMaxlen = 42;

Q_GLOBAL_STATIC(QMutex, catalogLock)

// for reference:
// https://github.com/autotools-mirror/gettext/blob/master/gnulib-local/lib/gettext.h
static QByteArray gettextHack(const char* const msgctxt, const char* const msgid)
{
    QByteArray msgwithctx(msgctxt);
    msgwithctx.append('\004');
    msgwithctx.append(msgid);
    return msgwithctx;
}

class KCatalogPrivate
{
public:
    KCatalogPrivate()
#ifdef HAVE_LIBINTL
        : bindDone(false)
#endif
    {}

  QByteArray language;
  QByteArray name;
  QByteArray localeDir;

#ifdef HAVE_LIBINTL
  QByteArray systemLanguage;
  bool bindDone;

  static QByteArray currentLanguage;

  void setupGettextEnv ();
  void resetSystemLanguage ();
#endif
};

QDebug operator<<(QDebug debug, const KCatalog &c)
{
  return debug << c.d->language << " " << c.d->name << " " << c.d->localeDir;
}

#ifdef HAVE_LIBINTL
QByteArray KCatalogPrivate::currentLanguage;
#endif

KCatalog::KCatalog(const QString & name, const QString & language )
  : d( new KCatalogPrivate )
{
  setlocale(LC_ALL, "");

  // Find locale directory for this catalog.
  QString localeDir = catalogLocaleDir( name, language );

  d->language = QFile::encodeName( language );
  d->name = QFile::encodeName( name );
  d->localeDir = QFile::encodeName( localeDir );

#ifdef HAVE_LIBINTL
  // Always get translations in UTF-8, regardless of user's environment.
  bind_textdomain_codeset( d->name, "UTF-8" );

  // Invalidate current language, to trigger binding at next translate call.
  KCatalogPrivate::currentLanguage.clear();
#endif
}

KCatalog::KCatalog(const KCatalog & rhs)
  : d( new KCatalogPrivate )
{
  *this = rhs;
}

KCatalog & KCatalog::operator=(const KCatalog & rhs)
{
  *d = *rhs.d;

  return *this;
}

KCatalog::~KCatalog()
{
  delete d;
}

QString KCatalog::catalogLocaleDir( const QString &name,
                                    const QString &language )
{
  QString relpath =  QString::fromLatin1( "%1/LC_MESSAGES/%2.mo" )
                    .arg( language ).arg( name );
  return KGlobal::dirs()->findResourceDir( "locale", relpath );
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

#ifdef HAVE_LIBINTL
void KCatalogPrivate::setupGettextEnv ()
{
  // Point Gettext to current language, recording system value for recovery.
  systemLanguage = qgetenv("LANGUAGE");
  if (systemLanguage != language) {
    // it is enough to change the string set there.
    char langenv[langenvMaxlen];
    ::memset(langenv, 0, langenvMaxlen * sizeof(char));
    snprintf(langenv, langenvMaxlen, "LANGUAGE=%s", language.constData());
    putenv(strdup(langenv));
  }

  // Rebind text domain if language actually changed from the last time,
  // as locale directories may differ for different languages of same catalog.
  if (language != currentLanguage || !bindDone) {

    currentLanguage = language;
    bindDone = true;

    //kDebug() << "bindtextdomain" << name << localeDir;
    bindtextdomain(name, localeDir);

#ifdef __GLIBC__
    // Magic to make sure Gettext doesn't use stale cached translation
    // from previous language.
    extern int _nl_msg_cat_cntr;
    ++_nl_msg_cat_cntr;
#else
#warning is the catalog workaround needed?
#endif
  }
}

void KCatalogPrivate::resetSystemLanguage ()
{
  if (language != systemLanguage) {
    char langenv[langenvMaxlen];
    ::memset(langenv, 0, langenvMaxlen * sizeof(char));
    snprintf(langenv, langenvMaxlen, "LANGUAGE=%s", systemLanguage.constData());
    putenv(strdup(langenv));
  }
}
#endif

QString KCatalog::translate(const char * msgid) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = dgettext(d->name, msgid);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
#else
  return QString::fromUtf8(msgid);
#endif
}

QString KCatalog::translate(const char * msgctxt, const char * msgid) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = nullptr;
  if (msgctxt) {
    const QByteArray msgwithctx = gettextHack(msgctxt, msgid);
    msgstr = dgettext(d->name, msgwithctx.constData());
  } else {
    msgstr = dgettext(d->name, msgid);
  }
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
#else
  Q_UNUSED(msgctxt);
  return QString::fromUtf8(msgid);
#endif
}

QString KCatalog::translate(const char * msgid, const char * msgid_plural,
                            unsigned long n) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = dngettext(d->name, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
#else
  return (n == 1 ? QString::fromUtf8(msgid) : QString::fromUtf8(msgid_plural));
#endif
}

QString KCatalog::translate(const char * msgctxt, const char * msgid,
                            const char * msgid_plural, unsigned long n) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = nullptr;
  if (msgctxt) {
    const QByteArray msgwithctx = gettextHack(msgctxt, msgid);
    msgstr = dngettext(d->name, msgwithctx.constData(), msgid_plural, n);
  } else {
    msgstr = dngettext(d->name, msgid, msgid_plural, n);
  }
  d->resetSystemLanguage();
  return QString::fromUtf8(msgstr);
#else
  Q_UNUSED(msgctxt);
  return (n == 1 ? QString::fromUtf8(msgid) : QString::fromUtf8(msgid_plural));
#endif
}

QString KCatalog::translateStrict(const char * msgid) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = dgettext(d->name, msgid);
  d->resetSystemLanguage();
  return msgstr != msgid ? QString::fromUtf8(msgstr) : QString();
#else
  Q_UNUSED(msgid);
  return QString();
#endif
}

QString KCatalog::translateStrict(const char * msgctxt, const char * msgid) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = nullptr;
  bool msgstrict = false;
  if (msgctxt) {
    const QByteArray msgwithctx = gettextHack(msgctxt, msgid);
    msgstr = dgettext(d->name, msgwithctx.constData());
    msgstrict = (msgstr != msgwithctx);
  } else {
    msgstr = dgettext(d->name, msgid);
    msgstrict = (msgstr != msgid);
  }
  d->resetSystemLanguage();
  return msgstrict ? QString::fromUtf8(msgstr) : QString();
#else
  Q_UNUSED(msgctxt);
  Q_UNUSED(msgid);
  return QString();
#endif
}

QString KCatalog::translateStrict(const char * msgid, const char * msgid_plural,
                                  unsigned long n) const
{
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = dngettext(d->name, msgid, msgid_plural, n);
  d->resetSystemLanguage();
  return msgstr != msgid && msgstr != msgid_plural ? QString::fromUtf8(msgstr) : QString();
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
#ifdef HAVE_LIBINTL
  QMutexLocker locker(catalogLock());
  d->setupGettextEnv();
  const char *msgstr = nullptr;
  bool msgstrict = false;
  bool msgstrict2 = false;
  if (msgctxt) {
    const QByteArray msgwithctx = gettextHack(msgctxt, msgid);
    msgstr = dngettext(d->name, msgwithctx.constData(), msgid_plural, n);
    msgstrict = (msgstr != msgwithctx.constData());
    msgstrict2 = (msgstr != msgid_plural);
  } else {
    const QByteArray msgwithctx = gettextHack(msgctxt, msgid);
    msgstr = dngettext(d->name, msgwithctx.constData(), msgid_plural, n);
    msgstrict = (msgstr != msgid);
    msgstrict2 = (msgstr != msgid_plural);
  }
  d->resetSystemLanguage();
  return msgstrict && msgstrict2 ? QString::fromUtf8(msgstr) : QString();
#else
  Q_UNUSED(msgctxt);
  Q_UNUSED(msgid);
  Q_UNUSED(msgid_plural);
  Q_UNUSED(n);
  return QString();
#endif
}

