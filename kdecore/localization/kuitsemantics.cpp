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

#include "kuitsemantics_p.h"
#include "kglobal.h"
#include "klocale.h"
#include "kdebug.h"

KuitSemantics::KuitSemantics(const QString &lang)
    : m_catalog(QString::fromLatin1("kdelibs4"), lang)
{
    KuitFormat format;
    format.plain = m_catalog.translate("@filename/plain", "‘%1’");
    format.rich = m_catalog.translate("@filename/rich", "<tt>%1</tt>");
    m_patterns.insert(QString::fromLatin1("filename"), format);
    format.plain = m_catalog.translate("@emphasis/plain", "*%1*");
    format.rich = m_catalog.translate("@emphasis/rich", "<i>%1</i>");
    m_patterns.insert(QString::fromLatin1("emphasis"), format);
    format.plain = m_catalog.translate("@email/plain", "&lt;%1&gt;");
    format.rich = m_catalog.translate("@email/rich", "&lt;<a href=\"mailto:%1\">%1</a>&gt;");
    m_patterns.insert(QString::fromLatin1("email"), format);
    format.plain = m_catalog.translate("@title/plain", "== %1 ==");
    format.rich = m_catalog.translate("@title/rich", "<h2>%1</h2>");
    m_patterns.insert(QString::fromLatin1("title"), format);
    format.plain = m_catalog.translate("@para/plain", "%1");
    format.rich = m_catalog.translate("@para/rich", "<p>%1</p>");
    m_patterns.insert(QString::fromLatin1("para"), format);
    format.plain = m_catalog.translate("@warning/plain", "WARNING: %1");
    format.rich = m_catalog.translate("@warning/rich", "<b>Warning</b>: %1");
    m_patterns.insert(QString::fromLatin1("warning"), format);
    format.plain = m_catalog.translate("@command/plain", "%1");
    format.rich = m_catalog.translate("@command/rich", "<tt>%1</tt>");
    m_patterns.insert(QString::fromLatin1("command"), format);
    format.plain = m_catalog.translate("@resource/plain", "“%1”");
    format.rich = m_catalog.translate("@resource/rich", "“%1”");
    m_patterns.insert(QString::fromLatin1("resource"), format);
    format.plain = m_catalog.translate("@message/plain", "/%1/");
    format.rich = m_catalog.translate("@message/rich", "<i>%1</i>");
    m_patterns.insert(QString::fromLatin1("message"), format);
    format.plain = m_catalog.translate("@nl/plain", "%1\n");
    format.rich = m_catalog.translate("@nl/rich", "%1<br/>");
    m_patterns.insert(QString::fromLatin1("nl"), format);
    // strip the tags only
    format.plain = QString::fromLatin1("%1");
    format.rich = QString::fromLatin1("%1");
    m_patterns.insert(QString::fromLatin1("application"), format);
    m_patterns.insert(QString::fromLatin1("numid"), format);
    // special cases
    format.plain = QString();
    format.rich = QString();
    m_patterns.insert(QString::fromLatin1(KUIT_NUMINTG), format);
    m_patterns.insert(QString::fromLatin1(KUIT_NUMREAL), format);
    m_patterns.insert(QString::fromLatin1(KUIT_NUMPREC), format);
}

QString KuitSemantics::format(const QString &text, const QString &ctxt) const
{
    QString result = text;

    bool checkifrich = true;
    bool isrich = false;
    if (ctxt.startsWith(QLatin1String("@info"))) {
        // rich by default (compat)
        isrich = true;
        checkifrich = false;
    }
    if (ctxt.startsWith(QLatin1Char('@'))) {
        if (ctxt.contains(QLatin1String("plain"))) {
            isrich = false;
            checkifrich = false;
        } else if (ctxt.contains(QLatin1String("rich"))) {
            isrich = true;
            checkifrich = false;
        }
    }
    if (checkifrich) {
        isrich = Qt::mightBeRichText(result);
    }

    // qDebug() << Q_FUNC_INFO << "formatting" << ctxt << result << isrich;

    static const QStringList s_exceptions = QStringList()
        << QString::fromLatin1("title")
        << QString::fromLatin1("para");
    QMapIterator<QString,KuitFormat> patternsit(m_patterns);
    while (patternsit.hasNext()) {
        patternsit.next();
        const QString pattern = patternsit.key();
        if (s_exceptions.contains(pattern)) {
            continue;
        }
        if (ctxt.startsWith(QLatin1String("@") + pattern)) {
            if (isrich) {
                result = patternsit.value().rich.arg(result);
            } else {
                result = patternsit.value().plain.arg(result);
            }
        }
    }

    patternsit.toFront();
    int precision = -1;
    while (patternsit.hasNext()) {
        patternsit.next();
        const QString pattern = patternsit.key();
        const QString startpattern = QLatin1String("<") + pattern + QLatin1String(">");
        const QString endpattern = QLatin1String("</") + pattern + QLatin1String(">");
        int tagstartpos = result.indexOf(startpattern);
        // qDebug() << Q_FUNC_INFO << pattern << tagstartpos;
        while (tagstartpos >= 0) {
            const int tagendpos = result.indexOf(endpattern, tagstartpos);
            if (tagendpos >= tagstartpos) {
                const QString tagvalue = result.mid(tagstartpos + startpattern.size(), tagendpos - tagstartpos - startpattern.size());
                // qDebug() << Q_FUNC_INFO << "tagvalue" << pattern << tagvalue << tagstartpos << tagendpos;
                QString tagsubstitute;
                if (pattern == QLatin1String(KUIT_NUMPREC)) {
                    precision = tagvalue.toInt();
                    tagsubstitute = QString();
                } else if (pattern == QLatin1String(KUIT_NUMINTG) || pattern == QLatin1String(KUIT_NUMREAL)) {
                    tagsubstitute = KGlobal::locale()->formatNumber(tagvalue, false, precision);
                    precision = -1;
                } else if (isrich) {
                    tagsubstitute = patternsit.value().rich.arg(tagvalue);
                } else {
                    tagsubstitute = patternsit.value().plain.arg(tagvalue);
                }
                // qDebug() << Q_FUNC_INFO << "replacing" << result.mid(tagstartpos, tagendpos - tagstartpos + endpattern.size()) << tagsubstitute;
                result.replace(tagstartpos, tagendpos - tagstartpos + endpattern.size(), tagsubstitute);
            } else {
                kWarning() << "found starting but no ending markup tag for" << pattern;
            }

            tagstartpos = result.indexOf(startpattern);
        }
    }

    // TODO: maybe entities compat, see:
    // kdecore/tests/klocalizedstringtest.cpp
    return result;
}
