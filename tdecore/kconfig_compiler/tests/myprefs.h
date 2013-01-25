#include <kconfigskeleton.h>

class MyPrefs : public TDEConfigSkeleton
{
  public:
    MyPrefs( const TQString &a ) : TDEConfigSkeleton( a ) {}
};
