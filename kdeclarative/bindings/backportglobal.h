/****************************************************************************
**
** This file is part of the Qt Script Generator.
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact:  Nokia Corporation info@qt.nokia.com
**
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging of
** this file.  Please review the following information to ensure the GNU
** Lesser General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** Copyright (C)  2011 Nokia. All rights reserved
****************************************************************************/

#ifndef QTSCRIPTEXTENSIONS_GLOBAL_H
#define QTSCRIPTEXTENSIONS_GLOBAL_H

#include <QScriptValue>
#include <QScriptContext>
#include <QScriptEngine>

#define DECLARE_SELF(Class, __fn__) \
    Class* self = qscriptvalue_cast<Class*>(ctx->thisObject()); \
    if (!self) { \
        return ctx->throwError(QScriptContext::TypeError, \
            QString::fromLatin1("%0.prototype.%1: this object is not a %0") \
            .arg(#Class).arg(#__fn__)); \
    }

#define ADD_ENUM_VALUE(__c__, __ns__, __v__) \
    __c__.setProperty(#__v__, QScriptValue(__c__.engine(), __ns__::__v__))

#endif // QTSCRIPTEXTENSIONS_GLOBAL_H
