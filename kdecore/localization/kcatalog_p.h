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

#ifndef KCATALOG_H
#define KCATALOG_H

#include <QString>
#include <QTranslator>
#include <QDebug>

class KCatalogPrivate;

/**
 * This class abstracts a gettext message catalog. It will take care of
 * needed gettext bindings.
 *
 * @see KLocale
 * @internal
 */
//REVISED: hausmann
class KCatalog
{
public:
  /**
   * Constructor.
   *
   * @param name The name of the catalog
   * @param language The language of this catalog
   */
  KCatalog(const QString &name, const QString &language);

  /**
   * Copy constructor.
   */
  KCatalog(const KCatalog &rhs);

  /**
   * Assignment operator.
   */
  KCatalog& operator=(const KCatalog &rhs);

  /**
   * Destructor.
   */
  ~KCatalog();

  /**
   * Checks the locale directory for the given catalog in given language.
   *
   * @param name The name of the catalog
   * @param language The language of this catalog
   *
   * @return true if catalog was found, false otherwise.
   */
  static bool hasCatalog(const QString &name, const QString &language);

#ifndef QT_NO_TRANSLATION
  /**
   * Finds the catalog file for the given catalog in given language, reads it
   * and loads it into the QTranslator.
   *
   * @param name The name of the catalog
   * @param language The language of this catalog
   * @param translator The QTranslator to load data into
   *
   * @return True if catalog file is found and loaded, false otherwise.
   */
  static bool loadCatalog(const QString &name, const QString &language, QTranslator *translator);
#endif

  /**
   * Returns the name of the catalog.
   *
   * @return The name of the catalog
   */
  QString name() const;

  /**
   * Returns the language of the catalog.
   *
   * @return The language of the catalog
   */
  QString language() const;

  /**
   * Retrieves a translation of the specified message id with given context.
   *
   * Do not pass 0 or "" strings as message id.
   *
   * @param msgctxt The context
   * @param msgid The message id
   *
   * @return The translated message, or @p msgid if not found
   */
  QString translate(const char *msgctxt, const char *msgid) const;

  /**
   * Retrieves a proper plural form of translation for the specified English
   * singular and plural message ids, with given context.
   *
   * Do not pass 0 or "" strings as message id.
   *
   * @param msgctxt The context
   * @param msgid The singular message id
   * @param msgid_plural The plural message id
   * @param n The number to which the plural form applies
   *
   * @return The translated message, or proper English form if not found
   */
  QString translate(const char *msgctxt, const char *msgid,
                    const char *msgid_plural, unsigned long n) const;

  /**
   * Retrieves a translation of the specified message id with given context,
   * returning empty if the translation was not found.
   *
   * Do not pass 0 or "" strings as message id.
   *
   * @param msgctxt The context
   * @param msgid The message id
   *
   * @return The translated message, or QString() if not found
   */
  QString translateStrict(const char *msgctxt, const char *msgid) const;

  /**
   * Retrieves a proper plural form of translation for the specified English
   * singular and plural message ids, with given context,
   * returning empty if the translation was not found.
   *
   * Do not pass 0 or "" strings as message id.
   *
   * @param msgctxt The context
   * @param msgid The singular message id
   * @param msgid_plural The plural message id
   * @param n The number to which the plural form applies
   *
   * @return The translated message, or QString() if not found
   */
  QString translateStrict(const char *msgctxt, const char *msgid,
                          const char *msgid_plural, unsigned long n) const;

  friend QDebug operator<<(QDebug debug, const KCatalog &c);

private:
  KCatalogPrivate* const d;
};

QDebug operator<<(QDebug debug, const KCatalog &c);

#endif
