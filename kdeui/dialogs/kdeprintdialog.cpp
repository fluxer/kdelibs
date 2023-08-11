/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2007 Alex Merry <alex.merry@kdemail.net>
 *  Copyright (c) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (c) 2008 John Layt <john@layt.net>
 *
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
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kdeprintdialog.h"
#include "kdebug.h"
#include "kdialog.h"
#include "klocale.h"

#include <QPrintDialog>

QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                          PageSelectPolicy pageSelectPolicy,
                                          const QList<QWidget*> &customTabs,
                                          QWidget *parent)
{
    QPrintDialog *dialog = new QPrintDialog( printer, parent );
    // Windows and lpr don't support server side page range so default to not
    // showing print range in dialog, enable only for CUPS depending on Qt version.
    // Need to check OSX.
    if ( pageSelectPolicy == SystemSelectsPages ) {
        dialog->setOption( QAbstractPrintDialog::PrintPageRange, false);
    }
    dialog->setOptionTabs( customTabs );
    dialog->setWindowTitle( KDialog::makeStandardCaption( i18nc( "@title:window", "Print" ) ) );
    return dialog;
}

QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                          const QList<QWidget*> &customTabs,
                                          QWidget *parent)
{
    return KdePrint::createPrintDialog(printer, KdePrint::ApplicationSelectsPages, customTabs, parent);
}


QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                          PageSelectPolicy pageSelectPolicy,
                                          QWidget *parent)
{
    return KdePrint::createPrintDialog(printer, pageSelectPolicy, QList<QWidget*>(), parent);
}

QPrintDialog *KdePrint::createPrintDialog(QPrinter *printer,
                                          QWidget *parent)
{
    return KdePrint::createPrintDialog(printer, KdePrint::ApplicationSelectsPages, QList<QWidget*>(), parent);
}
