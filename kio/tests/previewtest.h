
#include <tqwidget.h>
#include <kio/job.h>

class KLineEdit;
class TQLabel;
class KFileItem;

class PreviewTest : public TQWidget
{
    Q_OBJECT
public:
    PreviewTest();

private slots:
    void slotGenerate();
    void slotResult(TDEIO::Job *);
    void slotPreview( const KFileItem *, const TQPixmap & );
    void slotFailed();

private:
    KLineEdit *m_url;
    TQLabel *m_preview;
};

