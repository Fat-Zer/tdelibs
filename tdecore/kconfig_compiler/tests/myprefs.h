#include <kconfigskeleton.h>

class MyPrefs : public KConfigSkeleton
{
  public:
    MyPrefs( const TQString &a ) : KConfigSkeleton( a ) {}
};
