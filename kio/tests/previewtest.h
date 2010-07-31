
#include <tqwidget.h>
#include <kio/job.h>

class KLineEdit;
class QLabel;
class KFileItem;

class PreviewTest : public QWidget
{
    Q_OBJECT
public:
    PreviewTest();

private slots:
    void slotGenerate();
    void slotResult(KIO::Job *);
    void slotPreview( const KFileItem *, const TQPixmap & );
    void slotFailed();

private:
    KLineEdit *m_url;
    TQLabel *m_preview;
};

