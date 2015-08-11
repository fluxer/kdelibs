/**
* QImageIO Routines to read/write EPS images.
* copyright (c) 1998 Dirk Schoenberger <dirk.schoenberger@freenet.de>
*
* This library is distributed under the conditions of the GNU LGPL.
*/
#ifndef EPS_H
#define EPS_H


#include <QtGui/qimageiohandler.h>

class EPSHandler : public QImageIOHandler
{
public:
    EPSHandler();

    bool canRead() const;
    bool read(QImage *image);
    bool write(const QImage &image);

    QByteArray name() const;

    static bool canRead(QIODevice *device);
};

#endif

