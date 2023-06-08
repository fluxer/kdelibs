/*  This file is part of the KDE libraries
    Copyright (C) 2023 Ivailo Monev <xakepa10@gmail.com>

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

#include "kspellbackgroundchecker.h"
#include "kspeller.h"
#include "kdebug.h"

#include <QThread>
#include <QTextBoundaryFinder>

class KSpellBackgroundCheckerThread : public QThread
{
    Q_OBJECT
public:
    KSpellBackgroundCheckerThread(KConfig *config, QObject *parent, const QString &lang, const QString &text);

    void run() final;

    void interrupt();

Q_SIGNALS:
    void misspelling(const QString &word, int start);

private:
    bool m_interrupt;
    QString m_text;
    KSpeller m_speller;
};

KSpellBackgroundCheckerThread::KSpellBackgroundCheckerThread(KConfig *config, QObject *parent, const QString &lang, const QString &text)
    : QThread(parent),
    m_interrupt(false),
    m_text(text),
    m_speller(config)
{
    // qDebug() << Q_FUNC_INFO << lang;
    m_speller.setDictionary(lang);
}

void KSpellBackgroundCheckerThread::run()
{
    // qDebug() << Q_FUNC_INFO << m_text.size() << m_text;
    QTextBoundaryFinder finder(QTextBoundaryFinder::Word, m_text);
    int wordstart = 0;
    while (finder.toNextBoundary() >= 0) {
        if (m_interrupt) {
            break;
        }
        QTextBoundaryFinder::BoundaryReasons boundary = finder.boundaryReasons();
        if (boundary & QTextBoundaryFinder::StartWord) {
            wordstart = finder.position();
        }
        if (boundary & QTextBoundaryFinder::EndWord) {
            QString word = m_text.mid(wordstart, finder.position() - wordstart);
            if (word.size() < 2) {
                continue;
            }
            if (word.at(word.size() - 1).isPunct()) {
                word = word.mid(0, word.size() - 1);
            }
            // qDebug() << Q_FUNC_INFO << boundary << wordstart << finder.position() << word;
            if (!m_speller.check(word)) {
                emit misspelling(word, wordstart);
            }
        }
    }
}

void KSpellBackgroundCheckerThread::interrupt()
{
    m_interrupt = true;
}

class KSpellBackgroundCheckerPrivate
{
public:
    KSpellBackgroundCheckerPrivate(KConfig *config);

    QString text;
    QString language;
    KSpellBackgroundCheckerThread* spellerthread;
    KConfig* spellerconfig;
};

KSpellBackgroundCheckerPrivate::KSpellBackgroundCheckerPrivate(KConfig *config)
    : spellerthread(nullptr),
    spellerconfig(config)
{
}

KSpellBackgroundChecker::KSpellBackgroundChecker(KConfig *config, QObject *parent)
    : QObject(parent),
    d(new KSpellBackgroundCheckerPrivate(config))
{
    d->language = KSpeller::defaultLanguage();
}

KSpellBackgroundChecker::~KSpellBackgroundChecker()
{
    delete d;
}

void KSpellBackgroundChecker::setText(const QString &text)
{
    d->text = text;
    start();
}

QString KSpellBackgroundChecker::text() const
{
    return d->text;
}

void KSpellBackgroundChecker::start()
{
    stop();
    // qDebug() << Q_FUNC_INFO;
    d->spellerthread = new KSpellBackgroundCheckerThread(d->spellerconfig, this, d->language, d->text);
    connect(
        d->spellerthread, SIGNAL(finished()),
        this, SIGNAL(done())
    );
    connect(
        d->spellerthread, SIGNAL(misspelling(QString,int)),
        this, SIGNAL(misspelling(QString,int))
    );
    d->spellerthread->start();
}

void KSpellBackgroundChecker::stop()
{
    // qDebug() << Q_FUNC_INFO << d->spellerthread;
    if (d->spellerthread) {
        d->spellerthread->interrupt();
        d->spellerthread->deleteLater();
        d->spellerthread = nullptr;
    }
}

void KSpellBackgroundChecker::changeLanguage(const QString &lang)
{
    d->language = lang;
    if (d->language.isEmpty()) {
        kWarning() << "Attempting to set language to empty";
        d->language = KSpeller::defaultLanguage();
    }
}

#include "moc_kspellbackgroundchecker.cpp"
#include "kspellbackgroundchecker.moc"
