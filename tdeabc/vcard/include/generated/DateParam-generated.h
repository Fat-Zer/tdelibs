// XXX Automatically generated. DO NOT EDIT! XXX //

public:
DateParam();
DateParam(const DateParam&);
DateParam(const TQCString&);
DateParam & operator = (DateParam&);
DateParam & operator = (const TQCString&);
bool operator ==(DateParam&);
bool operator !=(DateParam& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {DateParam a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~DateParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "DateParam"; }

// End of automatically generated code           //
