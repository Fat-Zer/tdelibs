// XXX Automatically generated. DO NOT EDIT! XXX //

public:
PhoneNumberValue();
PhoneNumberValue(const PhoneNumberValue&);
PhoneNumberValue(const TQCString&);
PhoneNumberValue & operator = (PhoneNumberValue&);
PhoneNumberValue & operator = (const TQCString&);
bool operator ==(PhoneNumberValue&);
bool operator !=(PhoneNumberValue& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {PhoneNumberValue a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~PhoneNumberValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "PhoneNumberValue"; }

// End of automatically generated code           //
