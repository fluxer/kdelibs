/*  This file is part of the KDE libraries
    Copyright (C) 2016 Ivailo Monev <xakepa10@gmail.com>

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

#include <QDebug>
#include <QApplication>
#include <QMainWindow>
#include <QFileDialog>

#include "kmediawidget.h"

int main(int argc, char **argv)
{
    QApplication *app = new QApplication(argc, argv);
    QMainWindow *window = new QMainWindow();

    KMediaWidget *widget = new KMediaWidget(window, KMediaWidget::AllOptions);
    window->setCentralWidget(widget);
    window->show();
    widget->open("http://video.webmfiles.org/big-buck-bunny_trailer.webm");

    // qDebug() << widget->player()->property("property-list");
    // qDebug() << widget->player()->property("audio-device-list");
    return app->exec();
}