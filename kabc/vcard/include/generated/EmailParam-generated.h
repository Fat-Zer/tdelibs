// XXX Automatically generated. DO NOT EDIT! XXX //

public:
EmailParam();
EmailParam(const EmailParam&);
EmailParam(const TQCString&);
EmailParam & operator = (EmailParam&);
EmailParam & operator = (const TQCString&);
bool operator ==(EmailParam&);
bool operator !=(EmailParam& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {EmailParam a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~EmailParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "EmailParam"; }

// End of automatically generated code           //
