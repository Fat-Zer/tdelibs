// XXX Automatically generated. DO NOT EDIT! XXX //

public:
AdrParam();
AdrParam(const AdrParam&);
AdrParam(const TQCString&);
AdrParam & operator = (AdrParam&);
AdrParam & operator = (const TQCString&);
bool operator ==(AdrParam&);
bool operator !=(AdrParam& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {AdrParam a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~AdrParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "AdrParam"; }

// End of automatically generated code           //
