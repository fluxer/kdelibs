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

#include <QLocale>

#include "backportglobal.h"

Q_DECLARE_METATYPE(QLocale)
Q_DECLARE_METATYPE(QLocale*)

static QScriptValue localeCtor(QScriptContext *ctx, QScriptEngine *eng)
{
    if (ctx->argumentCount() > 0) {
        QScriptValue v = ctx->argument(0);
        if (v.isString()) {
            return qScriptValueFromValue(eng, QLocale(v.toString()));
        }
    }
    return qScriptValueFromValue(eng, QLocale());
}

static QScriptValue localeName(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, name);
    return self->name();
}

static QScriptValue localeDecimalPoint(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, decimalPoint);
    return QString(self->decimalPoint());
}

static QScriptValue localeGroupSeparator(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, groupSeparator);
    return QString(self->groupSeparator());
}

static QScriptValue localePercent(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, percent);
    return QString(self->percent());
}

static QScriptValue localeZeroDigit(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, zeroDigit);
    return QString(self->zeroDigit());
}

static QScriptValue localeNegativeSign(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, negativeSign);
    return QString(self->negativeSign());
}

static QScriptValue localePositiveSign(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, positiveSign);
    return QString(self->positiveSign());
}

static QScriptValue localeExponential(QScriptContext *ctx, QScriptEngine *eng)
{
    Q_UNUSED(eng)
    DECLARE_SELF(QLocale, exponential);
    return QString(self->exponential());
}

QScriptValue constructLocaleClass(QScriptEngine *eng)
{
    QScriptValue proto = qScriptValueFromValue(eng, QLocale());
    QScriptValue::PropertyFlags getter = QScriptValue::PropertyGetter;
    // QScriptValue::PropertyFlags setter = QScriptValue::PropertySetter;
    proto.setProperty("name", eng->newFunction(localeName), getter);
    proto.setProperty("decimalPoint", eng->newFunction(localeDecimalPoint), getter);
    proto.setProperty("groupSeparator", eng->newFunction(localeGroupSeparator), getter);
    proto.setProperty("percent", eng->newFunction(localePercent), getter);
    proto.setProperty("zeroDigit", eng->newFunction(localeZeroDigit), getter);
    proto.setProperty("negativeSign", eng->newFunction(localeNegativeSign), getter);
    proto.setProperty("positiveSign", eng->newFunction(localePositiveSign), getter);
    proto.setProperty("exponential", eng->newFunction(localeExponential), getter);

    QScriptValue ctorFun = eng->newFunction(localeCtor, proto);

    eng->setDefaultPrototype(qMetaTypeId<QLocale>(), proto);

    return ctorFun;
}
