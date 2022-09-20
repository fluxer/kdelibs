/* vi: ts=8 sts=4 sw=4
 *
 * This file is part of the KDE project, module kdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * (C) 2007 Daniel M. Duley <daniel.duley@verizon.net>
 * with minor additions and based on ideas from
 * Torsten Rahn <torsten@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kiconeffect.h"

#include <config.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#include <QtCore/QDebug>
#include <QtGui/QApplication>
#include <QtGui/QPaintEngine>
#include <QtGui/QDesktopWidget>
#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include <QtGui/QBitmap>
#include <QtGui/QPixmap>
#include <QtGui/QImage>
#include <QtGui/QColor>
#include <QtGui/QWidget>
#include <QtGui/QPainter>
#include <QtGui/QPen>

#include <kdebug.h>
#include <kglobal.h>
#include <ksharedconfig.h>
#include <kglobalsettings.h>
#include <kcolorscheme.h>
#include <kicontheme.h>
#include <kconfiggroup.h>


class KIconEffectPrivate
{
public:
    int effect[6][3];
    float value[6][3];
    QColor color[6][3];
    bool trans[6][3];
    QString key[6][3];
    QColor color2[6][3];
};

KIconEffect::KIconEffect()
    :d(new KIconEffectPrivate)
{
    init();
}

KIconEffect::~KIconEffect()
{
    delete d;
}

void KIconEffect::init()
{
    KSharedConfig::Ptr config = KGlobal::config();

    int i, j, effect=-1;
    //FIXME: this really should be using KIconLoader::metaObject() to guarantee synchronization
    // performance wise it's also practically guaranteed to be faster
    QStringList groups;
    groups += "Desktop";
    groups += "Toolbar";
    groups += "MainToolbar";
    groups += "Small";
    groups += "Panel";
    groups += "Dialog";

    QStringList states;
    states += "Default";
    states += "Active";
    states += "Disabled";

    QStringList::ConstIterator it, it2;
    QString _togray("togray");
    QString _colorize("colorize");
    QString _desaturate("desaturate");
    QString _togamma("togamma");
    QString _none("none");
    QString _tomonochrome("tomonochrome");

    for (it=groups.constBegin(), i=0; it!=groups.constEnd(); ++it, ++i)
    {
	// Default effects
	d->effect[i][0] = NoEffect;
	d->effect[i][1] =  ((i==0)||(i==4)) ? ToGamma : NoEffect;
	d->effect[i][2] = ToGray;

	d->trans[i][0] = false;
	d->trans[i][1] = false;
	d->trans[i][2] = true;
        d->value[i][0] = 1.0;
        d->value[i][1] = ((i==0)||(i==4)) ? 0.7 : 1.0;
        d->value[i][2] = 1.0;
        d->color[i][0] = QColor(144,128,248);
        d->color[i][1] = QColor(169,156,255);
        d->color[i][2] = QColor(34,202,0);
        d->color2[i][0] = QColor(0,0,0);
        d->color2[i][1] = QColor(0,0,0);
        d->color2[i][2] = QColor(0,0,0);

	KConfigGroup cg(config, *it + "Icons");
	for (it2=states.constBegin(), j=0; it2!=states.constEnd(); ++it2, ++j)
	{
	    QString tmp = cg.readEntry(*it2 + "Effect", QString());
	    if (tmp == _togray)
		effect = ToGray;
	    else if (tmp == _colorize)
		effect = Colorize;
	    else if (tmp == _desaturate)
		effect = DeSaturate;
	    else if (tmp == _togamma)
		effect = ToGamma;
	    else if (tmp == _tomonochrome)
		effect = ToMonochrome;
            else if (tmp == _none)
		effect = NoEffect;
	    else
		continue;
	    if(effect != -1)
                d->effect[i][j] = effect;
	    d->value[i][j] = cg.readEntry(*it2 + "Value", 0.0);
	    d->color[i][j] = cg.readEntry(*it2 + "Color", QColor());
	    d->color2[i][j] = cg.readEntry(*it2 + "Color2", QColor());
	    d->trans[i][j] = cg.readEntry(*it2 + "SemiTransparent", false);

	}
    }
}

bool KIconEffect::hasEffect(int group, int state) const
{
    if (group < 0 || group >= KIconLoader::LastGroup ||
        state < 0 || state >= KIconLoader::LastState) {
        return false;
    }

    return d->effect[group][state] != NoEffect;
}

QString KIconEffect::fingerprint(int group, int state) const
{
    if (group < 0 || group >= KIconLoader::LastGroup ||
        state < 0 || state >= KIconLoader::LastState) {
        return QString();
    }

    QString cached = d->key[group][state];
    if (cached.isEmpty())
    {
        QString tmp;
        cached = tmp.setNum(d->effect[group][state]);
        cached += ':';
        cached += tmp.setNum(d->value[group][state]);
        cached += ':';
        cached += d->trans[group][state] ? QLatin1String("trans")
            : QLatin1String("notrans");
        if (d->effect[group][state] == Colorize || d->effect[group][state] == ToMonochrome)
        {
            cached += ':';
            cached += d->color[group][state].name();
        }
        if (d->effect[group][state] == ToMonochrome)
        {
            cached += ':';
            cached += d->color2[group][state].name();
        }

        d->key[group][state] = cached;
    }

    return cached;
}

QImage KIconEffect::apply(const QImage &image, int group, int state) const
{
    if (state >= KIconLoader::LastState)
    {
	kDebug(265) << "Illegal icon state: " << state << "\n";
	return image;
    }
    if (group >= KIconLoader::LastGroup)
    {
	kDebug(265) << "Illegal icon group: " << group << "\n";
	return image;
    }
    return apply(image, d->effect[group][state], d->value[group][state],
	    d->color[group][state], d->color2[group][state], d->trans[group][state]);
}

QImage KIconEffect::apply(const QImage &image, int effect, float value,
                          const QColor &col, bool trans) const
{
    return apply(image, effect, value, col,
                 KColorScheme(QPalette::Active, KColorScheme::View).background().color(), trans);
}

QImage KIconEffect::apply(const QImage &img, int effect, float value,
                          const QColor &col, const QColor &col2, bool trans) const
{
    QImage image = img;
    if (effect >= LastEffect )
    {
	kDebug(265) << "Illegal icon effect: " << effect << "\n";
	return image;
    }
    if (value > 1.0)
	value = 1.0;
    else if (value < 0.0)
	value = 0.0;
    switch (effect)
    {
    case ToGray:
	toGray(image, value);
	break;
    case DeSaturate:
	deSaturate(image, value);
	break;
    case Colorize:
        colorize(image, col, value);
        break;
    case ToGamma:
        toGamma(image, value);
        break;
    case ToMonochrome:
        toMonochrome(image, col, col2, value);
        break;
    }
    if (trans == true)
    {
	semiTransparent(image);
    }
    return image;
}

QPixmap KIconEffect::apply(const QPixmap &pixmap, int group, int state) const
{
    if (state >= KIconLoader::LastState)
    {
	kDebug(265) << "Illegal icon state: " << state << "\n";
	return pixmap;
    }
    if (group >= KIconLoader::LastGroup)
    {
	kDebug(265) << "Illegal icon group: " << group << "\n";
	return pixmap;
    }
    return apply(pixmap, d->effect[group][state], d->value[group][state],
	    d->color[group][state], d->color2[group][state], d->trans[group][state]);
}

QPixmap KIconEffect::apply(const QPixmap &pixmap, int effect, float value,
	const QColor &col, bool trans) const
{
    return apply(pixmap, effect, value, col,
                 KColorScheme(QPalette::Active, KColorScheme::View).background().color(), trans);
}

QPixmap KIconEffect::apply(const QPixmap &pixmap, int effect, float value,
	const QColor &col, const QColor &col2, bool trans) const
{
    QPixmap result;

    if (effect >= LastEffect )
    {
	kDebug(265) << "Illegal icon effect: " << effect << "\n";
	return result;
    }

    if ((trans == true) && (effect == NoEffect))
    {
        result = pixmap;
        semiTransparent(result);
    }
    else if ( effect != NoEffect )
    {
        QImage tmpImg = pixmap.toImage();
        tmpImg = apply(tmpImg, effect, value, col, col2, trans);
        result = QPixmap::fromImage(tmpImg);
    }
    else
        result = pixmap;

    return result;
}

struct KIEImgEdit
{
    QImage& img;
    QVector <QRgb> colors;
    unsigned int*  data;
    unsigned int   pixels;

    KIEImgEdit(QImage& _img):img(_img)
    {
	if (img.depth() > 8)
        {
            //Code using data and pixels assumes that the pixels are stored
            //in 32bit values and that the image is not premultiplied
            if ((img.format() != QImage::Format_ARGB32) &&
                (img.format() != QImage::Format_RGB32))
            {
                img = img.convertToFormat(QImage::Format_ARGB32);
            }
            data   = (unsigned int*)img.bits();
	    pixels = img.width()*img.height();
	}
	else
	{
	    pixels = img.colorCount();
	    colors = img.colorTable();
	    data   = (unsigned int*)colors.data();
	}
    }

    ~KIEImgEdit()
    {
	if (img.depth() == 1)
	    img.setColorTable(colors);
    }
};

static bool painterSupportsAntialiasing()
{
   QPaintEngine* const pe = QApplication::desktop()->paintEngine();
   return pe && pe->hasFeature(QPaintEngine::Antialiasing);
}

// Taken from KImageEffect. We don't want to link kdecore to kdeui! As long
// as this code is not too big, it doesn't seem much of a problem to me.

void KIconEffect::toGray(QImage &img, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    unsigned char gray;
    if(value == 1.0){
        while(data != end){
            gray = qGray(*data);
            *data = qRgba(gray, gray, gray, qAlpha(*data));
            ++data;
        }
    }
    else{
        unsigned char val = (unsigned char)(255.0*value);
        while(data != end){
            gray = qGray(*data);
            *data = qRgba((val*gray+(0xFF-val)*qRed(*data)) >> 8,
                          (val*gray+(0xFF-val)*qGreen(*data)) >> 8,
                          (val*gray+(0xFF-val)*qBlue(*data)) >> 8,
                          qAlpha(*data));
            ++data;
        }
    }
}

void KIconEffect::colorize(QImage &img, const QColor &col, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    float rcol = col.red(), gcol = col.green(), bcol = col.blue();
    unsigned char red, green, blue, gray;
    unsigned char val = (unsigned char)(255.0*value);
    while(data != end){
        gray = qGray(*data);
        if(gray < 128){
            red = static_cast<unsigned char>(rcol/128*gray);
            green = static_cast<unsigned char>(gcol/128*gray);
            blue = static_cast<unsigned char>(bcol/128*gray);
        }
        else if(gray > 128){
            red = static_cast<unsigned char>((gray-128)*(2-rcol/128)+rcol-1);
            green = static_cast<unsigned char>((gray-128)*(2-gcol/128)+gcol-1);
            blue = static_cast<unsigned char>((gray-128)*(2-bcol/128)+bcol-1);
        }
        else{
            red = static_cast<unsigned char>(rcol);
            green = static_cast<unsigned char>(gcol);
            blue = static_cast<unsigned char>(bcol);
        }

        *data = qRgba((val*red+(0xFF-val)*qRed(*data)) >> 8,
                      (val*green+(0xFF-val)*qGreen(*data)) >> 8,
                      (val*blue+(0xFF-val)*qBlue(*data)) >> 8,
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::toMonochrome(QImage &img, const QColor &black,
                               const QColor &white, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    // Step 1: determine the average brightness
    double values = 0.0, sum = 0.0;
    bool grayscale = true;
    while(data != end){
        sum += qGray(*data)*qAlpha(*data) + 255*(255-qAlpha(*data));
        values += 255;
        if((qRed(*data) != qGreen(*data) ) || (qGreen(*data) != qBlue(*data)))
            grayscale = false;
        ++data;
    }
    double medium = sum/values;

    // Step 2: Modify the image
    unsigned char val = (unsigned char)(255.0*value);
    int rw = white.red(), gw = white.green(), bw = white.blue();
    int rb = black.red(), gb = black.green(), bb = black.blue();
    data = ii.data;

    if(grayscale){
        while(data != end){
            if(qRed(*data) <= medium)
                *data = qRgba((val*rb+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gb+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bb+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            else
                *data = qRgba((val*rw+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gw+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bw+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            ++data;
        }
    }
    else{
        while(data != end){
            if(qGray(*data) <= medium) 
                *data = qRgba((val*rb+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gb+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bb+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            else
                *data = qRgba((val*rw+(0xFF-val)*qRed(*data)) >> 8,
                              (val*gw+(0xFF-val)*qGreen(*data)) >> 8,
                              (val*bw+(0xFF-val)*qBlue(*data)) >> 8,
                              qAlpha(*data));
            ++data;
        }
    }
}

void KIconEffect::deSaturate(QImage &img, float value)
{
    if(value == 0.0)
        return;

    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    QColor color;
    int h, s, v;
    while(data != end){
        color.setRgb(*data);
        color.getHsv(&h, &s, &v);
        color.setHsv(h, (int) (s * (1.0 - value) + 0.5), v);
	*data = qRgba(color.red(), color.green(), color.blue(),
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::toGamma(QImage &img, float value)
{
    KIEImgEdit ii(img);
    QRgb *data = ii.data;
    QRgb *end = data + ii.pixels;

    float gamma = 1/(2*value+0.5);
    while(data != end){
        *data = qRgba(static_cast<unsigned char>
                      (pow(static_cast<float>(qRed(*data))/255 , gamma)*255),
                      static_cast<unsigned char>
                      (pow(static_cast<float>(qGreen(*data))/255 , gamma)*255),
                      static_cast<unsigned char>
                      (pow(static_cast<float>(qBlue(*data))/255 , gamma)*255),
                      qAlpha(*data));
        ++data;
    }
}

void KIconEffect::semiTransparent(QImage &img)
{
    int x, y;
    if(img.depth() == 32){
        if(img.format() == QImage::Format_ARGB32_Premultiplied)
            img = img.convertToFormat(QImage::Format_ARGB32);
        int width  = img.width();
        int height = img.height();

        if(painterSupportsAntialiasing()){
            unsigned char *line;
            for(y=0; y<height; ++y){
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
                line = img.scanLine(y);
#else
                line = img.scanLine(y) + 3;
#endif
                for(x=0; x<width; ++x){
                    *line >>= 1;
                    line += 4;
                }
            }
        }
        else{
            for(y=0; y<height; ++y){
                QRgb* line = (QRgb*)img.scanLine(y);
                for(x=(y%2); x<width; x+=2)
                    line[x] &= 0x00ffffff;
            }
        }
    }
    else{
        // Insert transparent pixel into the clut.
        int transColor = -1;

        // search for a color that is already transparent
        for(x=0; x<img.colorCount(); ++x){
            // try to find already transparent pixel
            if(qAlpha(img.color(x)) < 127){
                transColor = x;
                break;
            }
        }

        // FIXME: image must have transparency
        if(transColor < 0 || transColor >= img.colorCount())
            return;

        img.setColor(transColor, 0);
        unsigned char *line;
        bool setOn = (transColor != 0);
        if (img.format() == QImage::Format_MonoLSB){
            for(y=0; y<img.height(); ++y){
                line = img.scanLine(y);
                for(x=(y%2); x<img.width(); x+=2){
                    if(!setOn)
                        *(line + (x >> 3)) &= ~(1 << (x & 7));
                    else
                        *(line + (x >> 3)) |= (1 << (x & 7));
                }
            }
        } else {
            for(y=0; y<img.height(); ++y){
                line = img.scanLine(y);
                for(x=(y%2); x<img.width(); x+=2){
                    if(!setOn)
                        *(line + (x >> 3)) &= ~(1 << (7-(x & 7)));
                    else
                        *(line + (x >> 3)) |= (1 << (7-(x & 7)));
                }
            }
        }
    }
}

void KIconEffect::semiTransparent(QPixmap &pix)
{
    if (painterSupportsAntialiasing()) {
        QImage img=pix.toImage();
        semiTransparent(img);
        pix = QPixmap::fromImage(img);
        return;
    }

    QImage img;
    if (!pix.mask().isNull())
      img = pix.mask().toImage();
    else
    {
        img = QImage(pix.size(), QImage::Format_Mono);
        img.fill(1);
    }

    for (int y=0; y<img.height(); y++)
    {
        QRgb* line = (QRgb*)img.scanLine(y);
        QRgb pattern = (y % 2) ? 0x55555555 : 0xaaaaaaaa;
        for (int x=0; x<(img.width()+31)/32; x++)
            line[x] &= pattern;
    }
    pix.setMask(QBitmap::fromImage(img));
}

QImage KIconEffect::doublePixels(const QImage &src) const
{
    int w = src.width();
    int h = src.height();

    QImage dst( w*2, h*2, src.format() );

    if (src.depth() == 1)
    {
	kDebug(265) << "image depth 1 not supported\n";
	return QImage();
    }

    int x, y;
    if (src.depth() == 32)
    {
	QRgb* l1, *l2;
	for (y=0; y<h; ++y)
	{
	    l1 = (QRgb*)src.scanLine(y);
	    l2 = (QRgb*)dst.scanLine(y*2);
	    for (x=0; x<w; ++x)
	    {
		l2[x*2] = l2[x*2+1] = l1[x];
	    }
	    memcpy(dst.scanLine(y*2+1), l2, dst.bytesPerLine());
	}
    } else
    {
	for (x=0; x<src.colorCount(); ++x)
	    dst.setColor(x, src.color(x));

	const unsigned char *l1;
	unsigned char *l2;
	for (y=0; y<h; ++y)
	{
	    l1 = src.scanLine(y);
	    l2 = dst.scanLine(y*2);
	    for (x=0; x<w; ++x)
	    {
		l2[x*2] = l1[x];
		l2[x*2+1] = l1[x];
	    }
	    memcpy(dst.scanLine(y*2+1), l2, dst.bytesPerLine());
	}
    }
    return dst;
}

void KIconEffect::overlay(QImage &src, QImage &overlay)
{
    if (src.depth() != overlay.depth())
    {
	kDebug(265) << "Image depth src (" << src.depth() << ") != overlay " << "(" << overlay.depth() << ")!\n";
	return;
    }
    if (src.size() != overlay.size())
    {
	kDebug(265) << "Image size src != overlay\n";
	return;
    }
    if (src.format() == QImage::Format_ARGB32_Premultiplied)
        src = src.convertToFormat(QImage::Format_ARGB32);

    if (overlay.format() == QImage::Format_RGB32)
    {
	kDebug(265) << "Overlay doesn't have alpha buffer!\n";
	return;
    }
    else if (overlay.format() == QImage::Format_ARGB32_Premultiplied)
        overlay = overlay.convertToFormat(QImage::Format_ARGB32);

    int i, j;

    // We don't do 1 bpp

    if (src.depth() == 1)
    {
	kDebug(265) << "1bpp not supported!\n";
	return;
    }

    // Overlay at 32 bpp does use alpha blending

    if (src.depth() == 32)
    {
	QRgb* oline, *sline;
	int r1, g1, b1, a1;
	int r2, g2, b2, a2;

	for (i=0; i<src.height(); ++i)
	{
	    oline = (QRgb*)overlay.scanLine(i);
	    sline = (QRgb*)src.scanLine(i);

	    for (j=0; j<src.width(); ++j)
	    {
		r1 = qRed(oline[j]);
		g1 = qGreen(oline[j]);
		b1 = qBlue(oline[j]);
		a1 = qAlpha(oline[j]);

		r2 = qRed(sline[j]);
		g2 = qGreen(sline[j]);
		b2 = qBlue(sline[j]);
		a2 = qAlpha(sline[j]);

		r2 = (a1 * r1 + (0xff - a1) * r2) >> 8;
		g2 = (a1 * g1 + (0xff - a1) * g2) >> 8;
		b2 = (a1 * b1 + (0xff - a1) * b2) >> 8;
		a2 = qMax(a1, a2);

		sline[j] = qRgba(r2, g2, b2, a2);
	    }
	}
    }

    return;
}


static const quint32 stack_blur8_mul[255] =
{
    512,512,456,512,328,456,335,512,405,328,271,456,388,335,292,512,
    454,405,364,328,298,271,496,456,420,388,360,335,312,292,273,512,
    482,454,428,405,383,364,345,328,312,298,284,271,259,496,475,456,
    437,420,404,388,374,360,347,335,323,312,302,292,282,273,265,512,
    497,482,468,454,441,428,417,405,394,383,373,364,354,345,337,328,
    320,312,305,298,291,284,278,271,265,259,507,496,485,475,465,456,
    446,437,428,420,412,404,396,388,381,374,367,360,354,347,341,335,
    329,323,318,312,307,302,297,292,287,282,278,273,269,265,261,512,
    505,497,489,482,475,468,461,454,447,441,435,428,422,417,411,405,
    399,394,389,383,378,373,368,364,359,354,350,345,341,337,332,328,
    324,320,316,312,309,305,301,298,294,291,287,284,281,278,274,271,
    268,265,262,259,257,507,501,496,491,485,480,475,470,465,460,456,
    451,446,442,437,433,428,424,420,416,412,408,404,400,396,392,388,
    385,381,377,374,370,367,363,360,357,354,350,347,344,341,338,335,
    332,329,326,323,320,318,315,312,310,307,304,302,299,297,294,292,
    289,287,285,282,280,278,275,273,271,269,267,265,263,261,259
};

static const quint32 stack_blur8_shr[255] =
{
    9, 11, 12, 13, 13, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16, 17,
    17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18, 18, 19,
    19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20,
    20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 20, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    23, 23, 23, 23, 23, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24
};

inline static void blurHorizontal(QImage &image, unsigned int *stack, int div, int radius)
{
    int stackindex;
    int stackstart;

    quint32 * const pixels = reinterpret_cast<quint32 *>(image.bits());
    quint32 pixel;

    int w = image.width();
    int h = image.height();
    int wm = w - 1;

    unsigned int mul_sum = stack_blur8_mul[radius];
    unsigned int shr_sum = stack_blur8_shr[radius];

    unsigned int sum, sum_in, sum_out;

    for (int y = 0; y < h; y++)
    {
        sum     = 0;
        sum_in  = 0;
        sum_out = 0;

        const int yw = y * w;
        pixel = pixels[yw];
        for (int i = 0; i <= radius; i++)
        {
            stack[i] = qAlpha(pixel);

            sum += stack[i] * (i + 1);
            sum_out += stack[i];
        }

        for (int i = 1; i <= radius; i++)
        {
            pixel = pixels[yw + qMin(i, wm)];

            unsigned int *stackpix = &stack[i + radius];
            *stackpix = qAlpha(pixel);

            sum    += *stackpix * (radius + 1 - i);
            sum_in += *stackpix;
        }

        stackindex = radius;
        for (int x = 0, i = yw; x < w; x++)
        {
            pixels[i++] = (((sum * mul_sum) >> shr_sum) << 24) & 0xff000000;

            sum -= sum_out;

            stackstart = stackindex + div - radius;
            if (stackstart >= div)
                stackstart -= div;

            unsigned int *stackpix = &stack[stackstart];

            sum_out -= *stackpix;

            pixel = pixels[yw + qMin(x + radius + 1, wm)];

            *stackpix = qAlpha(pixel);

            sum_in += *stackpix;
            sum    += sum_in;

            if (++stackindex >= div)
                stackindex = 0;

            stackpix = &stack[stackindex];

            sum_out += *stackpix;
            sum_in  -= *stackpix;
        } // for (x = 0, ...)
    } // for (y = 0, ...)
}

inline static void blurVertical(QImage &image, unsigned int *stack, int div, int radius)
{
    int stackindex;
    int stackstart;

    quint32 * const pixels = reinterpret_cast<quint32 *>(image.bits());
    quint32 pixel;

    int w = image.width();
    int h = image.height();
    int hm = h - 1;

    int mul_sum = stack_blur8_mul[radius];
    int shr_sum = stack_blur8_shr[radius];

    unsigned int sum, sum_in, sum_out;

    for (int x = 0; x < w; x++)
    {
        sum     = 0;
        sum_in  = 0;
        sum_out = 0;

        pixel = pixels[x];
        for (int i = 0; i <= radius; i++)
        {
            stack[i] = qAlpha(pixel);

            sum += stack[i] * (i + 1);
            sum_out += stack[i];
        }

        for (int i = 1; i <= radius; i++)
        {
            pixel = pixels[qMin(i, hm) * w + x];

            unsigned int *stackpix = &stack[i + radius];
            *stackpix = qAlpha(pixel);

            sum    += *stackpix * (radius + 1 - i);
            sum_in += *stackpix;
        }

        stackindex = radius;
        for (int y = 0, i = x; y < h; y++, i += w)
        {
            pixels[i] = (((sum * mul_sum) >> shr_sum) << 24) & 0xff000000;

            sum -= sum_out;

            stackstart = stackindex + div - radius;
            if (stackstart >= div)
                stackstart -= div;

            unsigned int *stackpix = &stack[stackstart];

            sum_out -= *stackpix;

            pixel = pixels[qMin(y + radius + 1, hm) * w + x];

            *stackpix = qAlpha(pixel);

            sum_in += *stackpix;
            sum    += sum_in;

            if (++stackindex >= div)
                stackindex = 0;

            stackpix = &stack[stackindex];

            sum_out += *stackpix;
            sum_in  -= *stackpix;
        } // for (y = 0, ...)
    } // for (x = 0, ...)
}

static void stackBlur(QImage &image, float radius)
{
    radius = qRound(radius);

    int div = int(radius * 2) + 1;
    unsigned int *stack  = new unsigned int[div];

    blurHorizontal(image, stack, div, radius);
    blurVertical(image, stack, div, radius);

    delete [] stack;
}

void KIconEffect::shadowBlur(QImage &image, float radius, const QColor &color)
{
    if (radius < 0)
        return;

    if (radius > 0)
        stackBlur(image, radius);

    // Correct the color and opacity of the shadow
    QPainter p(&image);
    p.setCompositionMode(QPainter::CompositionMode_SourceIn);
    p.fillRect(image.rect(), color);
}
