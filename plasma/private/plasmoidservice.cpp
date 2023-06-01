/*
 *   Copyright © 2009 Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#include "plasmoidservice_p.h"

#include "dataengineconsumer_p.h"
#include "dataengine_p.h"

#include <plasma/applet.h>
#include <plasma/packagemetadata.h>
#include <plasma/service.h>
#include <plasma/servicejob.h>

#include <kdebug.h>
#include <ktemporaryfile.h>

#include <QFile>
#include <QFileInfo>

namespace Plasma
{

PlasmoidServiceJob::PlasmoidServiceJob(const QString &destination,
                                       const QString &operation,
                                       const QMap<QString,QVariant>& parameters,
                                       PlasmoidService *service)
    : Plasma::ServiceJob(destination, operation, parameters,
                         static_cast<Plasma::Service*>(service)),
      m_service(service)
{
}

void PlasmoidServiceJob::start()
{
    if (operationName() == "GetMetaData") {
        KTemporaryFile tempFile;
        m_service->m_metadata.write(tempFile.fileName());
        QFile file(tempFile.fileName());
        setResult(file.readAll());
    } else if (operationName() == "DataEngine") {
        QString serviceName = "plasma-dataengine-" + parameters()["EngineName"].toString();
        setResult(serviceName);
    }
}


PlasmoidService::PlasmoidService(const QString &packageLocation)
    : Plasma::Service(0)
{
    setName("plasmoidservice");

    QString location(packageLocation);
    if (!location.endsWith('/')) {
        location.append('/');
    }

    m_metadata.read(location + "metadata.desktop");
    if (!m_metadata.isValid()) {
        kDebug() << "not a valid package";
    }
}

PlasmoidService::PlasmoidService(Applet *applet)
{
    setName("plasmoidservice");
    if (!applet->package() || !applet->package()->isValid()) {
        kDebug() << "not a valid package";
    }
}

PackageMetadata PlasmoidService::metadata() const
{
    return m_metadata;
}

Plasma::ServiceJob* PlasmoidService::createJob(const QString& operation,
                                               const QMap<QString,QVariant>& parameters)
{
    return new PlasmoidServiceJob(destination(), operation, parameters, this);
}

}

#include "moc_plasmoidservice_p.cpp"

