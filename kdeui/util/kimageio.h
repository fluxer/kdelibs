/*
* kimageio.h -- Declaration of interface to the KDE Image IO library.
* Copyright (c) 1998 Sirtaj Singh Kang <taj@kde.org>
*
* This library is distributed under the conditions of the GNU LGPL.
*/

#ifndef KIMAGEIO_H
#define KIMAGEIO_H

#include <QtCore/QStringList>

#include <kdeui_export.h>

/**
 * Methods to get information about image format names and
 * the corresponding mime type. Also, you can get information about supported
 * image types without loading all the imageformat plugins.
 *
 * The image processing backends are written as image handlers compatible
 * with the QImageIOHandler format. The backends are Katie imageformat plugins.
 * Each format can be identified by a unique type id string.
 *
 * \b Formats:
 *
 * Currently supported formats include:
 * @li WEBP    \<read\> \<write\>
 * @li JPEG    \<read\> \<write\>
 * @li ICO     \<read\> \<write\>
 * @li JP2     \<read\>
 * @li RAW     \<read\>
 * @li TIFF    \<read\>
 */
namespace KImageIO
{
  /**
   * Possible image file access modes.
   *
   * Used in various KImageIO static function.
   **/
  enum Mode { Reading, Writing };

  /**
   * Returns a list of patterns of all KImageIO supported formats.
   *
   * These patterns can be passed to KFileDialog::getOpenFileName()
   * or KFileDialog::getSaveFileName(), for example.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a space-separated list of file globs that describe the
   * supported formats
   */
  KDEUI_EXPORT QString pattern(Mode mode = Reading);

  /**
   * Returns the type of a MIME type.
   * @param mimeType the MIME type to search
   * @return type id(s) of the MIME type or QStringList() if the MIME type
   *         is not supported
   */
  KDEUI_EXPORT QStringList typeForMime(const QString& mimeType);
  /**
   * Returns a list of all KImageIO supported formats.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a list of the type ids
   */
  KDEUI_EXPORT QStringList types(Mode mode = Writing);
                   
  /**
   *  Returns a list of MIME types for all KImageIO supported formats.
   *
   * @param mode Tells whether to retrieve modes that can be read or written.
   * @return a list if MIME types of the supported formats
   */
  KDEUI_EXPORT QStringList mimeTypes(Mode mode = Writing);

  /**
   * Test to see whether a MIME type is supported to reading/writing.
   * @param _mimeType the MIME type to check
   * @param _mode Tells whether to check for reading or writing capabilities
   * @return true if the type is supported
   **/
  KDEUI_EXPORT bool isSupported(const QString& mimeType, Mode mode = Writing);
}

#endif // KIMAGEIO_H
