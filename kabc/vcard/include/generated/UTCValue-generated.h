// XXX Automatically generated. DO NOT EDIT! XXX //

public:
UTCValue();
UTCValue(const UTCValue&);
UTCValue(const TQCString&);
UTCValue & operator = (UTCValue&);
UTCValue & operator = (const TQCString&);
bool operator ==(UTCValue&);
bool operator !=(UTCValue& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {UTCValue a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~UTCValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "UTCValue"; }

// End of automatically generated code           //
