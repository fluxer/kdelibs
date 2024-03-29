/*
 * This file is part of the KDE project
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *               2008 Rafael Fernández López <ereslibre@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#include "kimagefilepreview.h"

#include <QtGui/QLayout>
#include <QtGui/QLabel>
#include <QtGui/QPainter>
#include <QtGui/QCheckBox>
#include <QtGui/qevent.h>

#include <kglobalsettings.h>
#include <kconfig.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <klocale.h>
#include <kfileitem.h>
#include <kpixmapsequenceoverlaypainter.h>
#include <kio/previewjob.h>
#include <kpixmapwidget.h>
#include <kconfiggroup.h>

#include <config-kfile.h>


static KIO::PreviewJob* createJob(const KUrl &url, int w, int h)
{
    if (url.isValid()) {
        KFileItemList items;
        items.append(KFileItem(KFileItem::Unknown, KFileItem::Unknown, url, true));
        static const QStringList plugins = KIO::PreviewJob::availablePlugins();

        KIO::PreviewJob *previewJob = KIO::filePreview(items, QSize(w, h), &plugins);
        previewJob->setScaleType(KIO::PreviewJob::Scaled);
        return previewJob;
    }
    return nullptr;
}


/**** KImageFilePreviewPrivate ****/

class KImageFilePreviewPrivate
{
public:
    KImageFilePreviewPrivate();
    ~KImageFilePreviewPrivate();

    void _k_slotResult(KJob* job);
    void _k_slotFailed(const KFileItem &item);

    KUrl lastShownURL;
    KPixmapWidget *pixmapWidget;
    KPixmapSequenceOverlayPainter *busyPainter;
    KIO::PreviewJob *m_job;
};

KImageFilePreviewPrivate::KImageFilePreviewPrivate()
    : pixmapWidget(nullptr),
    busyPainter(nullptr),
    m_job(nullptr)
{
}

KImageFilePreviewPrivate::~KImageFilePreviewPrivate()
{
}

void KImageFilePreviewPrivate::_k_slotFailed(const KFileItem &item)
{
    busyPainter->stop();
    if (item.isDir()) {
        pixmapWidget->setPixmap(
            DesktopIcon("inode-directory", KIconLoader::SizeEnormous, KIconLoader::DisabledState)
        );
    } else {
        pixmapWidget->setPixmap(
            SmallIcon("image-missing", KIconLoader::SizeEnormous, KIconLoader::DisabledState)
        );
    }
}

void KImageFilePreviewPrivate::_k_slotResult(KJob *job)
{
    if (job == m_job) {
        m_job = nullptr;
    }
    busyPainter->stop();
}

/**** KImageFilePreview****/

KImageFilePreview::KImageFilePreview(QWidget *parent)
    : KPreviewWidgetBase(parent),
    d(new KImageFilePreviewPrivate())
{
    QVBoxLayout *vb = new QVBoxLayout(this);
    vb->setMargin(0);

    d->pixmapWidget = new KPixmapWidget(this);
    d->pixmapWidget->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding));
    d->pixmapWidget->setDragEnabled(true);
    vb->addWidget(d->pixmapWidget);

    d->busyPainter = new KPixmapSequenceOverlayPainter(this);
    d->busyPainter->setWidget(d->pixmapWidget);
    d->busyPainter->stop();

    setSupportedMimeTypes(KIO::PreviewJob::supportedMimeTypes());
    setMinimumWidth(50);
}

KImageFilePreview::~KImageFilePreview()
{
    if (d->m_job) {
        d->m_job->kill();
    }
    delete d;
}

// called via KPreviewWidgetBase interface
void KImageFilePreview::showPreview(const KUrl& url)
{
    if (!url.isValid()) {
        return;
    }

    d->lastShownURL = url;

    int w = d->pixmapWidget->contentsRect().width() - 4;
    int h = d->pixmapWidget->contentsRect().height() - 4;

    if (d->m_job) {
        disconnect(
            d->m_job, SIGNAL(result(KJob*)),
            this, SLOT(_k_slotResult(KJob*))
        );
        disconnect(
            d->m_job, SIGNAL(gotPreview(const KFileItem&, const QPixmap& )),
            this, SLOT(gotPreview(KFileItem,QPixmap))
        );

        disconnect(
            d->m_job, SIGNAL(failed(KFileItem)),
            this, SLOT(_k_slotFailed(KFileItem))
        );

        d->m_job->kill();
    }

    d->busyPainter->start();

    d->m_job = createJob(url, w, h);

    connect(
        d->m_job, SIGNAL(result(KJob*)),
        this, SLOT(_k_slotResult(KJob*))
    );
    connect(
        d->m_job, SIGNAL(gotPreview(const KFileItem&,const QPixmap&)),
        this, SLOT(gotPreview(KFileItem,QPixmap))
    );

    connect(
        d->m_job, SIGNAL(failed(KFileItem)),
        this, SLOT(_k_slotFailed(KFileItem))
    );
}

void KImageFilePreview::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    clearPreview();
    showPreview(d->lastShownURL);
}

QSize KImageFilePreview::sizeHint() const
{
    return QSize(100, 200);
}

void KImageFilePreview::gotPreview(const KFileItem &item, const QPixmap &pixmap)
{
    d->busyPainter->stop();
    d->pixmapWidget->setPixmap(pixmap);
}


void KImageFilePreview::clearPreview()
{
    if (d->m_job) {
        d->m_job->kill();
        d->m_job = nullptr;
    }
    d->pixmapWidget->setPixmap(QPixmap());
}

#include "moc_kimagefilepreview.cpp"
