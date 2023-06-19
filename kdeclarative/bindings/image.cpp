/*
 *   Copyright (c) 2023 Ivailo Monev <xakepa10@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QtScript/QScriptValue>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptContext>

#include <QImage>
#include <QPixmap>

#include "backportglobal.h"

Q_DECLARE_METATYPE(QImage)
Q_DECLARE_METATYPE(QImage*)
Q_DECLARE_METATYPE(QPixmap)
Q_DECLARE_METATYPE(QPixmap*)

static QScriptValue imageCtor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 0) {
        QScriptValue v = ctx->argument(0);
        if (v.isString()) {
            return qScriptValueFromValue(eng, QImage(v.toString()));
        } else if (v.isVariant()) {
            QVariant variant = v.toVariant();
            QPixmap p = variant.value<QPixmap>();
            if (!p.isNull()) {
                return qScriptValueFromValue(eng, p.toImage());
            }
        }
    }
    return qScriptValueFromValue(eng, QImage());
}

static QScriptValue imageIsNull(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QImage, null);
    return self->isNull();
}

static QScriptValue imageWidth(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QImage, width);
    return self->width();
}

static QScriptValue imageHeight(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QImage, height);
    return self->height();
}

QScriptValue constructImageClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QImage());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    // QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("null", eng->newFunction(imageIsNull), getter);
    proto.setProperty("width", eng->newFunction(imageWidth), getter);
    proto.setProperty("height", eng->newFunction(imageHeight), getter);

    QScriptValue ctorFun = eng->newFunction(imageCtor, proto);

    eng->setDefaultPrototype(qMetaTypeId<QImage>(), proto);

    return ctorFun;
}
