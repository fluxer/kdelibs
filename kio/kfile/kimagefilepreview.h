/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *               2008 Rafael Fernández López <ereslibre@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef KIMAGEFILEPREVIEW_H
#define KIMAGEFILEPREVIEW_H

#include <QtGui/QPixmap>

#include <kurl.h>
#include <kjob.h>
#include <kfileitem.h>
#include <kpreviewwidgetbase.h>

class KImageFilePreviewPrivate;

/**
 * Image preview widget for the file dialog.
 */
class KIO_EXPORT KImageFilePreview : public KPreviewWidgetBase
{
    Q_OBJECT

public:
    /**
      * Creates a new image file preview.
      *
      * @param parent The parent widget.
      */
    explicit KImageFilePreview(QWidget *parent = 0);

    /**
      * Destroys the image file preview.
      */
    ~KImageFilePreview();

    /**
      * Returns the size hint for this widget.
      */
    virtual QSize sizeHint() const;

public Q_SLOTS:
    /**
     * Shows a preview for the given @p url.
     */
    virtual void showPreview(const KUrl &url);

    /**
      * Clears the preview.
      */
    virtual void clearPreview();

protected Q_SLOTS:
    void gotPreview(const KFileItem &item, const QPixmap &pixmap);

protected:
    virtual void resizeEvent(QResizeEvent *event);

private:
    KImageFilePreviewPrivate *const d;

    Q_DISABLE_COPY(KImageFilePreview)

    Q_PRIVATE_SLOT(d, void _k_slotResult(KJob *job))
    Q_PRIVATE_SLOT(d, void _k_slotFailed(const KFileItem &item))
};

#endif // KIMAGEFILEPREVIEW_H
