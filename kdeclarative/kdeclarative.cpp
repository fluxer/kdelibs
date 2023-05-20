/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "kdeclarative.h"
#include "bindings/i18n_p.h"
#include "private/kdeclarative_p.h"
#include "private/engineaccess_p.h"
#include "private/kiconprovider_p.h"

#include <QtDeclarative/QDeclarativeComponent>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeExpression>
#include <QtScript/QScriptEngine>
#include <QtScript/QScriptValueIterator>
#include <QtScript/QScriptEngineAgent>
#include <QtScript/QScriptContextInfo>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kconfiggroup.h>
#include <ksharedconfig.h>

void registerNonGuiMetaTypes(QScriptEngine *engine);
QScriptValue constructIconClass(QScriptEngine *engine);
QScriptValue constructKUrlClass(QScriptEngine *engine);

class KScriptEngineAgent : public QScriptEngineAgent
{
public:
    KScriptEngineAgent(QScriptEngine *engine);

    // reimplementation
    void exceptionThrow(qint64 scriptId, const QScriptValue &exception, bool hasHandler) final;

private:
    Q_DISABLE_COPY(KScriptEngineAgent);
};

KScriptEngineAgent::KScriptEngineAgent(QScriptEngine *engine)
    : QScriptEngineAgent(engine)
{
    engine->setAgent(this);
};

void KScriptEngineAgent::exceptionThrow(qint64 scriptId, const QScriptValue &exception, bool hasHandler)
{
    // qDebug() << Q_FUNC_INFO << scriptId;

    if (!hasHandler) {
        QScriptEngine* engine = QScriptEngineAgent::engine();
        if (Q_UNLIKELY(!engine)) {
            kWarning() << "No engine";
            return;
        }

        QScriptContext* engineContext = engine->currentContext();
        if (Q_UNLIKELY(!engineContext)) {
            kWarning() << "No engine context";
            return;
        }

        QScriptContextInfo contextInfo(engineContext);
        const QString infoFile = contextInfo.fileName();
        const QString infoLine = QString::number(contextInfo.lineNumber());
        const QString infoString = QString::fromLatin1("%1:%2").arg(infoFile, infoLine);
        kDebug() << infoString << exception.toString();
    }
}

KDeclarativePrivate::KDeclarativePrivate()
{
}

KDeclarative::KDeclarative()
    : d(new KDeclarativePrivate)
{
}

KDeclarative::~KDeclarative()
{
    delete d;
}


void KDeclarative::setDeclarativeEngine(QDeclarativeEngine *engine)
{
    if (d->declarativeEngine.data() == engine) {
        return;
    }
    d->declarativeEngine = engine;
}

QDeclarativeEngine *KDeclarative::declarativeEngine() const
{
    return d->declarativeEngine.data();
}

void KDeclarative::initialize()
{
    //Glorious hack:steal the engine
    //create the access object
    EngineAccess *engineAccess = new EngineAccess(this);
    d->declarativeEngine.data()->rootContext()->setContextProperty("__engineAccess", engineAccess);

    //make engineaccess set our d->scriptengine
    QDeclarativeExpression *expr = new QDeclarativeExpression(d->declarativeEngine.data()->rootContext(), d->declarativeEngine.data()->rootContext()->contextObject(), "__engineAccess.setEngine(this)");
    expr->evaluate();
    delete expr;

    //we don't need engineaccess anymore
    d->declarativeEngine.data()->rootContext()->setContextProperty("__engineAccess", 0);
    engineAccess->deleteLater();

    //fail?
    if (!d->scriptEngine) {
        kWarning() << "Failed to get the script engine";
        return;
    }

    //change the old globalobject with a new read/write copy
    QScriptValue originalGlobalObject = d->scriptEngine.data()->globalObject();

    QScriptValue newGlobalObject = d->scriptEngine.data()->newObject();

    QScriptValueIterator iter(originalGlobalObject);
    while (iter.hasNext()) {
        iter.next();

        if (iter.name() == QLatin1String("version")) {
            continue;
        }

        newGlobalObject.setProperty(iter.scriptName(), iter.value());
    }

    d->scriptEngine.data()->setGlobalObject(newGlobalObject);
}

void KDeclarative::setupBindings()
{
    QScriptEngine *engine = d->scriptEngine.data();
    if (!engine) {
        return;
    }

    /*tell the engine to search for import in the kde4 plugin dirs.
    addImportPath adds the path at the beginning, so to honour user's
    paths we need to traverse the list in reverse order*/

    const QStringList importPathList = KGlobal::dirs()->findDirs("module", "imports");
    QStringListIterator importPathIterator(importPathList);
    importPathIterator.toBack();
    while (importPathIterator.hasPrevious()) {
        d->declarativeEngine.data()->addImportPath(importPathIterator.previous());
    }

    QScriptValue global = engine->globalObject();

    //KConfig and KJob
    registerNonGuiMetaTypes(d->scriptEngine.data());

    // Stuff from Qt
    global.setProperty("QIcon", constructIconClass(engine));

    // Add stuff from KDE libs
    bindI18N(engine);
    qScriptRegisterSequenceMetaType<KUrl::List>(engine);
    global.setProperty("Url", constructKUrlClass(engine));

    // setup ImageProvider for KDE icons
    d->declarativeEngine.data()->addImageProvider(QString("icon"), new KIconProvider);

    // the engine should take ownership
    (void)new KScriptEngineAgent(engine);
}

QScriptEngine *KDeclarative::scriptEngine() const
{
    return d->scriptEngine.data();
}

QString KDeclarative::defaultComponentsTarget()
{
    return QLatin1String("desktop");
}

QString KDeclarative::componentsTarget()
{
    return defaultComponentsTarget();
}

QStringList KDeclarative::runtimePlatform()
{
    return QStringList();
}

