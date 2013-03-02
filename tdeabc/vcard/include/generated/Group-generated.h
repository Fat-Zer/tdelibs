// XXX Automatically generated. DO NOT EDIT! XXX //

public:
Group();
Group(const Group&);
Group(const TQCString&);
Group & operator = (Group&);
Group & operator = (const TQCString&);
bool operator ==(Group&);
bool operator !=(Group& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {Group a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~Group();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "Group"; }

// End of automatically generated code           //
