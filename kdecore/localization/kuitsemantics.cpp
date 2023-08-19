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

const QLatin1String KuitSemantics::s_numargs = QLatin1String(KUIT_NUMARGS);
const QLatin1String KuitSemantics::s_numintg = QLatin1String(KUIT_NUMINTG);
const QLatin1String KuitSemantics::s_numreal = QLatin1String(KUIT_NUMREAL);
const QLatin1String KuitSemantics::s_title = QLatin1String("title");

KuitSemantics::KuitSemantics(const QString &lang)
    : m_catalog(QString::fromLatin1("kdelibs4"), lang)
{
    KuitFormat format;
    format.tag = QString::fromLatin1("filename");
    format.plain = m_catalog.translate("@filename/plain", "‘%1’");
    format.rich = m_catalog.translate("@filename/rich", "<tt>%1</tt>");
    m_formats.append(format);
    format.tag = QString::fromLatin1("email");
    format.plain = m_catalog.translate("@email/plain", "&lt;%1&gt;");
    format.rich = m_catalog.translate("@email/rich", "&lt;<a href=\"mailto:%1\">%1</a>&gt;");
    m_formats.append(format);
    format.tag = QString::fromLatin1("title");
    format.plain = m_catalog.translate("@title/plain", "== %1 ==");
    format.rich = m_catalog.translate("@title/rich", "<h2>%1</h2>");
    m_formats.append(format);
    format.tag = QString::fromLatin1("warning");
    format.plain = m_catalog.translate("@warning/plain", "WARNING: %1");
    format.rich = m_catalog.translate("@warning/rich", "<b>Warning</b>: %1");
    m_formats.append(format);
    format.tag = QString::fromLatin1("command");
    format.plain = m_catalog.translate("@command/plain", "%1");
    format.rich = m_catalog.translate("@command/rich", "<tt>%1</tt>");
    m_formats.append(format);
    format.tag = QString::fromLatin1("resource");
    format.plain = m_catalog.translate("@resource/plain", "“%1”");
    format.rich = m_catalog.translate("@resource/rich", "“%1”");
    m_formats.append(format);
    format.tag = QString::fromLatin1("message");
    format.plain = m_catalog.translate("@message/plain", "/%1/");
    format.rich = m_catalog.translate("@message/rich", "<i>%1</i>");
    m_formats.append(format);
    // special cases
    format.tag = QString::fromLatin1(KUIT_NUMARGS);
    format.plain = QString();
    format.rich = QString();
    m_formats.append(format);
    format.tag = QString::fromLatin1(KUIT_NUMREAL);
    m_formats.append(format);
    format.tag = QString::fromLatin1(KUIT_NUMINTG);
    m_formats.append(format);
}

QString KuitSemantics::format(const QString &text, const QString &ctxt) const
{
    QString result = text;

    bool checkifrich = true;
    bool isrich = false;
    if (ctxt.startsWith(QLatin1Char('@'))) {
        if (ctxt.startsWith(QLatin1String("@info"))) {
            // rich by default (compat)
            isrich = true;
            checkifrich = false;
        }
        if (ctxt.contains(QLatin1String("/plain"))) {
            isrich = false;
            checkifrich = false;
        } else if (ctxt.contains(QLatin1String("/rich"))) {
            isrich = true;
            checkifrich = false;
        }
    }
    if (checkifrich) {
        isrich = Qt::mightBeRichText(result);
    }

    // qDebug() << Q_FUNC_INFO << "formatting" << ctxt << result << isrich;
    foreach (const KuitFormat &format, m_formats) {
        // exceptions
        if (format.tag == s_title || format.tag == s_numargs
            || format.tag == s_numintg || format.tag == s_numreal) {
            continue;
        }
        if (ctxt.startsWith(QLatin1String("@") + format.tag)) {
            if (isrich) {
                result = format.rich.arg(result);
            } else {
                result = format.plain.arg(result);
            }
            break;
        }
    }

    int numprec = -1;
    int numwidth = 0;
    QChar numfill = QLatin1Char(' ');
    foreach (const KuitFormat &format, m_formats) {
        const QString startformat = QLatin1String("<") + format.tag + QLatin1String(">");
        const QString endformat = QLatin1String("</") + format.tag + QLatin1String(">");
        int tagstartpos = result.indexOf(startformat);
        // qDebug() << Q_FUNC_INFO << format.tag << tagstartpos;
        while (tagstartpos >= 0) {
            const int tagendpos = result.indexOf(endformat, tagstartpos + startformat.size());
            if (Q_LIKELY(tagendpos >= tagstartpos)) {
                const QString tagvalue = result.mid(tagstartpos + startformat.size(), tagendpos - tagstartpos - startformat.size());
                // qDebug() << Q_FUNC_INFO << "tagvalue" << format.tag << tagvalue << tagstartpos << tagendpos;
                QString tagsubstitute;
                if (format.tag == s_numargs) {
                    // split alert!
                    const QStringList numargs = tagvalue.split(QLatin1Char(':'));
                    if (Q_LIKELY(numargs.size() == 3)) {
                        numprec = numargs.at(0).toInt();
                        numwidth = numargs.at(1).toInt();
                        numfill = numargs.at(2)[0];
                    } else {
                        kWarning() << "invalid number arguments tag" << format.tag;
                    }
                    tagsubstitute = QString();
                } else if (format.tag == s_numintg || format.tag == s_numreal) {
                    // qDebug() << Q_FUNC_INFO << "tagnum" << format.tag << tagvalue << numprec << numwidth << numfill;
                    tagsubstitute = QString::fromLatin1("%1").arg(
                        KGlobal::locale()->formatNumber(tagvalue, false, numprec),
                        numwidth, numfill
                    );
                    numprec = -1;
                    numwidth = 0;
                    numfill = QLatin1Char(' ');
                } else if (isrich) {
                    tagsubstitute = format.rich.arg(tagvalue);
                } else {
                    tagsubstitute = format.plain.arg(tagvalue);
                }
                // qDebug() << Q_FUNC_INFO << "replacing" << result.mid(tagstartpos, tagendpos - tagstartpos + endformat.size()) << tagsubstitute;
                result.replace(tagstartpos, tagendpos - tagstartpos + endformat.size(), tagsubstitute);
            } else {
                kWarning() << "found starting but no ending tag for" << format.tag;
                break;
            }

            tagstartpos = result.indexOf(startformat);
        }
    }

    return result;
}
