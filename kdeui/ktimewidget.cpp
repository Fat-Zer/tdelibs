#include <tqdatetimeedit.h>
#include <tqlayout.h>

#include "kdebug.h"
#include "kdialog.h"

#include "ktimewidget.h"

class KTimeWidget::KTimeWidgetPrivate
{
public:
  QTimeEdit * timeWidget;
};

KTimeWidget::KTimeWidget(TQWidget * parent, const char * name)
  : TQWidget(parent, name)
{
  init();
}

KTimeWidget::KTimeWidget(const TQTime & time,
                         TQWidget * parent, const char * name)
  : TQWidget(parent, name)
{
  init();

  setTime(time);
}

KTimeWidget::~KTimeWidget()
{
  delete d;
}

void KTimeWidget::init()
{
  d = new KTimeWidgetPrivate;

  TQHBoxLayout *layout = new TQHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);

  d->timeWidget = new QTimeEdit(this);

  connect(d->timeWidget, TQT_SIGNAL(valueChanged(const TQTime &)),
          TQT_SIGNAL(valueChanged(const TQTime &)));
}

void KTimeWidget::setTime(const TQTime & time)
{
  d->timeWidget->setTime(time);
}

TQTime KTimeWidget::time() const
{
  return d->timeWidget->time();
}

#include "ktimewidget.moc"
