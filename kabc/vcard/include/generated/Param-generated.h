// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Param();
Param(const Param&);
Param(const TQCString&);
Param & operator = (Param&);
Param & operator = (const TQCString&);
bool operator ==(Param&);
bool operator !=(Param& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {Param a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~Param();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "Param"; }

// End of automatically generated code           //
