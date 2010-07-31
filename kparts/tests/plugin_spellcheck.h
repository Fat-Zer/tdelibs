#ifndef __plugin_spellcheck_h
#define __plugin_spellcheck_h

#include <kparts/plugin.h>

class PluginSpellCheck : public KParts::Plugin
{
    Q_OBJECT
public:
    PluginSpellCheck( TQObject* parent = 0, const char* name = 0, 
                      const TQStringList& = TQStringList() );
    virtual ~PluginSpellCheck();

public slots:
    void slotSpellCheck();
};

#endif
