/* This file is part of the KDE libraries
   Copyright (C) 2004, 2010 Joseph Wenninger <jowenn@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KDELIBS_KTEXTEDITOR_TEMPLATEINTERFACE_H
#define KDELIBS_KTEXTEDITOR_TEMPLATEINTERFACE_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtGui/QImage>

#include <ktexteditor/ktexteditor_export.h>
#include <ktexteditor/cursor.h>

namespace KTextEditor
{

class Cursor;

/**
 * This is an interface for inserting template strings with user editable
 * fields into a document.
 * \ingroup kte_group_view_extensions
 */
class KTEXTEDITOR_EXPORT TemplateInterface //should be named AbstractTemplateInterface, but for consistency with the other classes it is not (for the 3.x release series)
{
  public:
    TemplateInterface();
    virtual ~TemplateInterface();

  public:

    /**
     * Inserts an interactive ediable template text at cursor position @p insertPosition.
     * \return true if inserting the string succeeded
     *
     * Use insertTemplateText(lines(), ...) to append text at end of document
     * Template  strings look like
     * "for( int i=0;i<10;i++) { %{cursor} };"
     * or "%{date}"
     *
     * This syntax is somewhat similar to the one found in the Eclipse editor or textmate.
     *
     * There are certain common placeholders (macros), which get assigned a
     * default initialValue.
     *
     * Placeholder names may only consist of a-zA-Z0-9_
     * 
     * Common placeholders and values are
     *
     * - loginname: The current users's loginname
     * - fullname: The current user's first and last name retrieved from kabc
     * - email: The current user's primary email address retrieved from kabc
     * - date: current date
     * - time: current time
     * - year: current year
     * - month: current month
     * - day: current day
     * - hostname: hostname of the computer
     * - selection: The implementation should set this to the selected text, if any
     * - cursor: The implementation should set the cursor position there, if any.
     *
     * If the editor supports some kind of smart indentation, the inserted code
     * should be layouted by the indenter.
     */
    bool insertTemplateText ( const Cursor &insertPosition, const QString &templateString, const QMap<QString,QString> &initialValues);

protected:
    /**
     * You must implement this, it is called by insertTemplateText, after all
     * default values are inserted. If you are implementing this interface,
     * this method should work as described in the documentation for
     * insertTemplateText above.
     * \return true if any text was inserted.
     */
    virtual bool insertTemplateTextImplementation ( const Cursor &insertPosition, const QString &templateString, const QMap<QString,QString> &initialValues)=0;
};

}

Q_DECLARE_INTERFACE(KTextEditor::TemplateInterface,
"org.kde.KTextEditor.TemplateInterface")

#endif
