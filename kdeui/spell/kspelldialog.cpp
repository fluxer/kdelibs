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

#include "kspelldialog.h"
#include "kspeller.h"
#include "klocale.h"
#include "klistwidget.h"
#include "kpushbutton.h"
#include "kmessagebox.h"
#include "kdebug.h"

#include <QThread>
#include <QCoreApplication>
#include <QTextBoundaryFinder>
#include <QGridLayout>
#include <QLabel>

class KSpellDialogThread : public QThread
{
    Q_OBJECT
public:
    KSpellDialogThread(QObject *parent, KSpeller *speller, const QString &text);

    void run() final;

    void interrupt();

    void findNext();
    int position() const;

Q_SIGNALS:
    void foundWord(const QString &word);
    void atEnd();

private:
    bool m_interrupt;
    QString m_text;
    KSpeller* m_speller;
    QTextBoundaryFinder m_finder;
    bool m_findprevious;
    bool m_findnext;
    int m_wordstart;
};

KSpellDialogThread::KSpellDialogThread(QObject *parent, KSpeller *speller, const QString &text)
    : QThread(parent),
    m_interrupt(false),
    m_text(text),
    m_speller(speller),
    m_finder(QTextBoundaryFinder::Word, text),
    m_findnext(false),
    m_wordstart(0)
{
    // qDebug() << Q_FUNC_INFO << lang;
}

void KSpellDialogThread::run()
{
    while (!m_interrupt) {
        if (m_findnext) {
            // kDebug() << "Looking for the next word";
            const int finderresult = m_finder.toNextBoundary();
            if (finderresult < 0) {
                m_findnext = false;
                emit atEnd();
                continue;
            }

            QTextBoundaryFinder::BoundaryReasons boundary = m_finder.boundaryReasons();
            if (boundary & QTextBoundaryFinder::StartWord) {
                m_wordstart = m_finder.position();
            }
            if (boundary & QTextBoundaryFinder::EndWord) {
                QString word = m_text.mid(m_wordstart, m_finder.position() - m_wordstart);
                if (word.size() < 2) {
                    continue;
                }
                if (word.at(word.size() - 1).isPunct()) {
                    word = word.mid(0, word.size() - 1);
                }
                // qDebug() << Q_FUNC_INFO << boundary << m_wordstart << m_finder.position() << word;
                if (!m_speller->check(word)) {
                    m_findnext = false;
                    emit foundWord(word);
                }
            }
        } else {
            // kDebug() << "Busy loop for 200ms";
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
            QThread::msleep(100);
        }
    }
}

void KSpellDialogThread::interrupt()
{
    m_interrupt = true;
}

void KSpellDialogThread::findNext()
{
    m_findnext = true;
}

int KSpellDialogThread::position() const
{
    return m_finder.position();
}

class KSpellDialogPrivate
{
public:
    KSpellDialogPrivate(KConfig *config);

    QWidget* dialogwidget;
    QGridLayout* layout;
    QLabel* misspelledlabel;
    QLabel* wordlabel;
    KListWidget* suggestionswidget;
    KPushButton* correctbutton;
    KPushButton* nextbutton;
    bool showcompletionmessage;
    bool continueaftercorrect;
    QString text;
    KSpeller speller;
    KSpellDialogThread* spellerthread;
};

KSpellDialogPrivate::KSpellDialogPrivate(KConfig *config)
    : dialogwidget(nullptr),
    layout(nullptr),
    misspelledlabel(nullptr),
    wordlabel(nullptr),
    suggestionswidget(nullptr),
    correctbutton(nullptr),
    nextbutton(nullptr),
    showcompletionmessage(false),
    continueaftercorrect(false),
    speller(config),
    spellerthread(nullptr)
{
}

KSpellDialog::KSpellDialog(KConfig *config, QWidget *parent)
    : KDialog(parent),
    d(new KSpellDialogPrivate(config))
{
    d->dialogwidget = new QWidget(this);
    d->layout = new QGridLayout(d->dialogwidget);
    d->layout->setSpacing(KDialog::spacingHint());

    d->misspelledlabel = new QLabel(i18n("Misspelled word:"), this);
    d->layout->addWidget(d->misspelledlabel, 0, 0);
    d->wordlabel = new QLabel(QString::fromLatin1("..."), this);
    d->wordlabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    d->layout->addWidget(d->wordlabel, 0, 1);

    d->suggestionswidget = new KListWidget(this);
    d->layout->addWidget(d->suggestionswidget, 1, 0, 5, 1);

    d->correctbutton = new KPushButton(i18n("Correct"), this);
    d->correctbutton->setIcon(KIcon("edit-node")); // TODO: or tools-check-spelling?
    connect(
        d->correctbutton, SIGNAL(pressed()),
        this, SLOT(_correct())
    );
    d->layout->addWidget(d->correctbutton, 1, 1);

    d->nextbutton = new KPushButton(i18n("Next"), this);
    d->nextbutton->setIcon(KIcon("go-next"));
    connect(
        d->nextbutton, SIGNAL(pressed()),
        this, SLOT(_next())
    );
    d->layout->addWidget(d->nextbutton, 2, 1);

    setMainWidget(d->dialogwidget);
}

KSpellDialog::~KSpellDialog()
{
    if (d->spellerthread) {
        d->spellerthread->interrupt();
        d->spellerthread->wait();
        delete d->spellerthread;
    }
    delete d;
}

void KSpellDialog::showSpellCheckCompletionMessage(bool b)
{
    d->showcompletionmessage = b;
}

void KSpellDialog::setSpellCheckContinuedAfterReplacement(bool b)
{
    d->continueaftercorrect = b;
}

QString KSpellDialog::buffer() const
{
    return d->text;
}

void KSpellDialog::setBuffer(const QString &buffer)
{
     d->text = buffer;
     if (d->spellerthread) {
        d->spellerthread->interrupt();
        d->spellerthread->wait();
        delete d->spellerthread;
    }
    d->spellerthread = new KSpellDialogThread(this, &d->speller, d->text);
    connect(
        d->spellerthread, SIGNAL(foundWord(QString)),
        this, SLOT(_suggest(QString))
    );
    connect(
        d->spellerthread, SIGNAL(atEnd()),
        this, SLOT(_done())
    );
    d->spellerthread->start();
    _next();
}

void KSpellDialog::changeLanguage(const QString &lang)
{
    d->speller.setDictionary(lang);
    emit languageChanged(lang);
}

void KSpellDialog::slotButtonClicked(int button)
{
    if (d->spellerthread) {
        d->spellerthread->interrupt();
        d->spellerthread->wait();
        delete d->spellerthread;
        d->spellerthread = nullptr;
    }
    KDialog::slotButtonClicked(button);
}

void KSpellDialog::_correct()
{
    QListWidgetItem* suggestionitem = d->suggestionswidget->currentItem();
    Q_ASSERT(suggestionitem);
    const QString word = d->wordlabel->text();
    emit replace(word, d->spellerthread->position() - word.size(), suggestionitem->text());
    if (d->continueaftercorrect) {
        kDebug() << "Spell checking continues..";
        _next();
    }
}

void KSpellDialog::_next()
{
    Q_ASSERT(d->spellerthread);
    kDebug() << "Looking for the next word";
    d->spellerthread->findNext();
}

void KSpellDialog::_done()
{
    kDebug() << "Done checking";
    Q_ASSERT(d->spellerthread);
    d->spellerthread->interrupt();
    d->spellerthread->wait();
    delete d->spellerthread;
    d->spellerthread = nullptr;
    if (d->showcompletionmessage) {
        KMessageBox::information(this, i18n("Spell check complete."), i18nc("@title:window", "Check Spelling"));
    }
    KDialog::accept();
}

void KSpellDialog::_suggest(const QString &word)
{
    kDebug() << "Got a word from the checker" << word;
    d->wordlabel->setText(word);
    d->suggestionswidget->clear();
    const QStringList suggestions = d->speller.suggest(word);
    foreach (const QString &suggestion, suggestions) {
        d->suggestionswidget->addItem(suggestion);
    }
    if (suggestions.isEmpty()) {
        d->correctbutton->setEnabled(false);
    } else {
        d->correctbutton->setEnabled(true);
        d->suggestionswidget->setCurrentRow(0);
    }
    emit misspelling(word, d->spellerthread->position() - word.size());
}

#include "moc_kspelldialog.cpp"
#include "kspelldialog.moc"
