/* This file is part of the KDE libraries
    Copyright (C) 2005 Stephan Binner <binner@kde.org>

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

#ifndef KPREVIEWPROPS_H
#define KPREVIEWPROPS_H

#include <kpropertiesdialog.h>

class KImageFilePreview;

/*!
 * PreviewProps plugin
 * This plugin displays a preview of the given file
 */
class KIO_EXPORT KPreviewPropsPlugin : public KPropertiesDialogPlugin
{
    Q_OBJECT
public:
    KPreviewPropsPlugin(KPropertiesDialog *props);

    /**
     * Tests whether a preview for the first item should be shown
     */
    static bool supports(const KFileItemList &items);

private Q_SLOTS:
  void currentPageChanged(KPageWidgetItem *, KPageWidgetItem *);

private:
    KImageFilePreview* preview;
    void createLayout();
};

#endif // KPREVIEWPROPS_H
