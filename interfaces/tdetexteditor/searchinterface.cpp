#include "searchinterface.h"
#include "searchdcopinterface.h"
#include "document.h"

using namespace KTextEditor;

namespace KTextEditor
{
	class PrivateSearchInterface
	{
	public:
		PrivateSearchInterface()
		{
		interface = 0;
		}
		~PrivateSearchInterface(){}
	// Data Members
	SearchDCOPInterface *interface;
	};

}

unsigned int SearchInterface::globalSearchInterfaceNumber = 0;

SearchInterface::SearchInterface()
{
	d = new PrivateSearchInterface();
	globalSearchInterfaceNumber++;
	mySearchInterfaceNumber=globalSearchInterfaceNumber;
        TQString name = "SearchInterface#" + TQString::number(mySearchInterfaceNumber);
	 d->interface = new SearchDCOPInterface(this, name.latin1());
}
SearchInterface::~SearchInterface()
{
  delete d->interface;
  delete d;
}

unsigned int SearchInterface::searchInterfaceNumber () const
{
  return mySearchInterfaceNumber;
}

void SearchInterface::setSearchInterfaceDCOPSuffix (const TQCString &suffix)
{
  d->interface->setObjId ("SearchInterface#"+suffix);
}

SearchInterface *KTextEditor::searchInterface (Document *doc)
{  
  if (!doc)
    return 0;

  return dynamic_cast<KTextEditor::SearchInterface*>(doc);
}
