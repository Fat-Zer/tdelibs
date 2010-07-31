/*
 *
 * This file is part of the KDE project.
 * Copyright (C) 2001 Martin R. Jones <mjones@kde.org>
 *               2001 Carsten Pfeiffer <pfeiffer@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General Public
 * License. See the file "COPYING" for the exact licensing terms.
 */

#ifndef KIMAGEFILEPREVIEW_H
#define KIMAGEFILEPREVIEW_H

#include <tqpixmap.h>

#include <kurl.h>
#include <kpreviewwidgetbase.h>

class QCheckBox;
class QPushButton;
class QLabel;
class QTimer;

class KFileDialog;
class KFileItem;
namespace KIO { class Job; class PreviewJob; }

/**
 * Image preview widget for the file dialog.
 */
class KIO_EXPORT KImageFilePreview : public KPreviewWidgetBase
{
	Q_OBJECT

	public:
		KImageFilePreview(TQWidget *parent);
		~KImageFilePreview();

		virtual TQSize sizeHint() const;

	public slots:
		virtual void showPreview(const KURL &url);
		virtual void clearPreview();

	protected slots:
		void showPreview();
		void showPreview( const KURL& url, bool force );

		void toggleAuto(bool);
		virtual void gotPreview( const KFileItem*, const TQPixmap& );

	protected:
		virtual void resizeEvent(TQResizeEvent *e);
		virtual KIO::PreviewJob * createJob( const KURL& url,
                                                     int w, int h );

	private slots:
		void slotResult( KIO::Job * );
		virtual void slotFailed( const KFileItem* );

	private:
		bool autoMode;
		KURL currentURL;
		TQTimer *timer;
		TQLabel *imageLabel;
		TQLabel *infoLabel;
		TQCheckBox *autoPreview;
		TQPushButton *previewButton;
		KIO::PreviewJob *m_job;
        protected:
                virtual void virtual_hook( int id, void* data );
        private:
                class KImageFilePreviewPrivate;
                KImageFilePreviewPrivate *d;
};

#endif // KIMAGEFILEPREVIEW_H
