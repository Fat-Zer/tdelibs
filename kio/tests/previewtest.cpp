
#include <tqlabel.h>
#include <layout.h>
#include <tqpushbutton.h>

#include <kapplication.h>
#include <kdebug.h>
#include <klineedit.h>

#include <kio/previewjob.h>

#include "previewtest.moc"

PreviewTest::PreviewTest()
    :TQWidget()
{
    TQGridLayout *layout = new TQGridLayout(this, 2, 2);
    m_url = new KLineEdit(this);
    m_url->setText("/home/malte/gore_bush.jpg");
    layout->addWidget(m_url, 0, 0);
    TQPushButton *btn = new TQPushButton("Generate", this);
    connect(btn, TQT_SIGNAL(clicked()), TQT_SLOT(slotGenerate()));
    layout->addWidget(btn, 0, 1);
    m_preview = new TQLabel(this);
    m_preview->setMinimumSize(400, 300);
    layout->addMultiCellWidget(m_preview, 1, 1, 0, 1);
}

void PreviewTest::slotGenerate()
{
    KURL::List urls;
    urls.append(m_url->text());
    KIO::PreviewJob *job = KIO::filePreview(urls, m_preview->width(), m_preview->height(), true, 48);
    connect(job, TQT_SIGNAL(result(KIO::Job*)), TQT_SLOT(slotResult(KIO::Job*)));
    connect(job, TQT_SIGNAL(gotPreview(const KFileItem *, const TQPixmap &)), TQT_SLOT(slotPreview(const KFileItem *, const TQPixmap &)));
    connect(job, TQT_SIGNAL(failed(const KFileItem *)), TQT_SLOT(slotFailed()));
}

void PreviewTest::slotResult(KIO::Job*)
{
    kdDebug() << "PreviewTest::slotResult(...)" << endl;
}

void PreviewTest::slotPreview(const KFileItem *, const TQPixmap &pix)
{
    kdDebug() << "PreviewTest::slotPreview()" << endl;
    m_preview->setPixmap(pix);
}

void PreviewTest::slotFailed()
{
    kdDebug() << "PreviewTest::slotFailed()" << endl;
    m_preview->setText("failed");
}

int main(int argc, char **argv)
{
    KApplication app(argc, argv, "previewtest");
    PreviewTest *w = new PreviewTest;
    w->show();
    app.setMainWidget(w);
    return app.exec();
}

