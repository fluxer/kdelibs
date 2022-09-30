/*
   kcodecaction.h

    Copyright (c) 2003      Jason Keirstead   <jason@keirstead.org>
    Copyright (c) 2003-2006 Michel Hermier    <michel.hermier@gmail.com>
    Copyright (c) 2007      Nick Shaforostoff <shafff@ukr.net>

    ********************************************************************
    *                                                                  *
    * This library is free software; you can redistribute it and/or    *
    * modify it under the terms of the GNU Lesser General Public       *
    * License as published by the Free Software Foundation; either     *
    * version 2 of the License, or (at your option) any later version. *
    *                                                                  *
    * This library is distributed in the hope that it will be useful,  *
    * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
    * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
    * GNU Lesser General Public License for more details.              *
    *                                                                  *
    * You should have received a copy of the GNU Lesser General Public *
    * License along with this library; if not, write to the            *
    * Free Software Foundation, Inc., 51 Franklin Street,              *
    * Fifth Floor, Boston, MA  02110-1301  USA                         *
    *                                                                  *
    ********************************************************************
*/
#ifndef KCODECACTION_H
#define KCODECACTION_H

#include <QTextCodec>
#include <kselectaction.h>

/**
 *  @short Action for selecting one of several QTextCodec.
 *
 *  This action shows up a submenu with a list of the available codecs on the system.
 */
class KDEUI_EXPORT KCodecAction : public KSelectAction
{
    Q_OBJECT

    Q_PROPERTY(QString codecName READ currentCodecName WRITE setCurrentCodec)
    Q_PROPERTY(int codecMib READ currentCodecMib)

public:
    explicit KCodecAction(QObject *parent);
    KCodecAction(const QString &text, QObject *parent);
    KCodecAction(const KIcon &icon, const QString &text, QObject *parent);

    virtual ~KCodecAction();

public:
    int mibForName(const QString &codecName, bool *ok = 0) const;
    QTextCodec *codecForMib(int mib) const;

    QTextCodec *currentCodec() const;
    bool setCurrentCodec(QTextCodec *codec);

    QString currentCodecName() const;
    bool setCurrentCodec(const QString &codecName);

    int currentCodecMib() const;
    bool setCurrentCodec(int mib);

Q_SIGNALS:
    /**
     * Specific (proper) codec was selected
     *
     * Note that triggered(const QString&) is emitted too (as defined in KSelectAction)
     */
    void triggered(QTextCodec *codec);

protected:
    using KSelectAction::triggered;

private:
    class Private;
    Private* const d;

    Q_PRIVATE_SLOT( d, void _k_subActionTriggered(QAction*) )
};

#endif
