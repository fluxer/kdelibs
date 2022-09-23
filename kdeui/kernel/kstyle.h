/**
 * KStyle for KDE4
 * Copyright (C) 2004-2005 Maksim Orlovich <maksim@kde.org>
 * Copyright (C) 2005,2006 Sandro Giessl <giessl@kde.org>
 *
 * Based in part on the following software:
 *  KStyle for KDE3
 *      Copyright (C) 2001-2002 Karol Szwed <gallium@kde.org>
 *      Portions  (C) 1998-2000 TrollTech AS
 *  Keramik for KDE3,
 *      Copyright (C) 2002      Malte Starostik   <malte@kde.org>
 *                (C) 2002-2003 Maksim Orlovich  <maksim@kde.org>
 *      Portions  (C) 2001-2002 Karol Szwed     <gallium@kde.org>
 *                (C) 2001-2002 Fredrik Höglund <fredrik@kde.org>
 *                (C) 2000 Daniel M. Duley       <mosfet@kde.org>
 *                (C) 2000 Dirk Mueller         <mueller@kde.org>
 *                (C) 2001 Martijn Klingens    <klingens@kde.org>
 *                (C) 2003 Sandro Giessl      <sandro@giessl.com>
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
 
#ifndef KSTYLE_H
#define KSTYLE_H

#include <kdeui_export.h>

#include <QtCore/QString>
#include <QtCore/QHash>
#include <QtGui/QStyle>

/**
 * Makes style coding more convenient.
 *
 * @todo and allows to style KDE specific widgets.
 *
 * KStyle strives to ease style development by implementing various QStyle
 * methods. These implementations are based on
 * -# the concept of Layout Properties. These properties can be set using
 *    setWidgetLayoutProp(). KStyle uses this information to respect various
 *    metrics (like space between primitives or margins around widget contents)
 *    or turn specific features on or off.
 * -# the concept of KStyle Primitives. These can be implemented by overriding
 *    drawKStylePrimitive() and providing drawing methods for specific
 *    primitives. Often, the drawing of more complex widgets consists of
 *    several primitives.
 *
 * @author Maksim Orlovich (maksim\@kde.org)
 * @author Sandro Giessl (giessl\@kde.org)
 */

class KDEUI_EXPORT KStyle
{
public:
    KStyle();

    /**
     * Returns the default widget style.
     */
    static QString defaultStyle();

    /**
     * Runtime element extension
     * This is just convenience and does /not/ require the using widgets style to inherit KStyle
     * (i.e. calling this while using cleanlooks won't segfault or so but just return 0)
     * Returns a unique id for an element string (e.g. "CE_CapacityBar")
     *
     * For simplicity, only StyleHints, ControlElements and their SubElements are supported
     * If you don't need extended SubElement functionality, just drop it
     * 
     * @param element The style element, represented as string.
     * Naming convention: "appname.(2-char-element-type)_element"
     * where the 2-char-element-type is of {SH, CE, SE}
     * (widgets in kdelibs don't have to pass the appname)
     * examples: "CE_CapacityBar", "amarok.CE_Analyzer"
     * @param widget Your widget ("this") passing this is mandatory, passing NULL will just return 0
     * @returns a unique id for the @p element string or 0, if the element is not supported by the
     * widgets current style
     *
     * Important notes:
     * 1) If your string lacks the matching "SH_", "CE_" or "SE_" token the element
     * request will be ignored (return is 0)
     * 2) Try to avoid custom elements and use default ones (if possible) to get better style support
     * and keep UI coherency
     * 3) If you cache this value (good idea, this requires a map lookup) don't (!) forget to catch
     * style changes in QWidget::changeEvent()
     */
     static QStyle::StyleHint customStyleHint(const QString &element, const QWidget *widget);
     static QStyle::ControlElement customControlElement(const QString &element, const QWidget *widget);
     static QStyle::SubElement customSubElement(const QString &element, const QWidget *widget);

protected:
    /**
    * Runtime element extension, allows inheriting styles to add support custom elements
    * merges supporting inherit chains
    * Supposed to be called e.g. in your constructor.
    *
    * NOTICE: in order to have this work, your style must provide
    * an "X-KDE-CustomElements" classinfo, i.e.
    * class MyStyle : public KStyle
    * {
    *       Q_OBJECT
    *       Q_CLASSINFO ("X-KDE-CustomElements", "true")
    *
    *   public:
    *       .....
    * }
    *
    * @param element The style element, represented as string.
    * Suggested naming convention: appname.(2-char-element-type)_element
    * where the 2-char-element-type is of {SH, CE, SE}
    * widgets in kdelibs don't have to pass the appname
    * examples: "CE_CapacityBar", "amarok.CE_Analyzer"
    *
    * Important notes:
    * 1) If your string lacks the matching "SH_", "CE_" or "SE_" token the element
    * request will be ignored (return is 0)
    * 2) To keep UI coherency, don't support any nonsense in your style, but convince app developers
    * to use standard elements - if available
    */
    QStyle::StyleHint newStyleHint(const QString &element);
    QStyle::ControlElement newControlElement(const QString &element);
    QStyle::SubElement newSubElement(const QString &element);

private:
    int hintCounter, controlCounter, subElementCounter;
    QHash<QString, int> styleElements;
};

#endif // KSTYLE_H
