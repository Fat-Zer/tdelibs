// XXX Automatically generated. DO NOT EDIT! XXX //

public:
URIValue();
URIValue(const URIValue&);
URIValue(const TQCString&);
URIValue & operator = (URIValue&);
URIValue & operator = (const TQCString&);
bool operator ==(URIValue&);
bool operator !=(URIValue& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {URIValue a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~URIValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "URIValue"; }

// End of automatically generated code           //
