// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ContentLine();
ContentLine(const ContentLine&);
ContentLine(const TQCString&);
ContentLine & operator = (ContentLine&);
ContentLine & operator = (const TQCString&);
bool operator ==(ContentLine&);
bool operator !=(ContentLine& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {ContentLine a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~ContentLine();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "ContentLine"; }

// End of automatically generated code           //
