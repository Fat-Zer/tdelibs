// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ClassValue();
ClassValue(const ClassValue&);
ClassValue(const TQCString&);
ClassValue & operator = (ClassValue&);
ClassValue & operator = (const TQCString&);
bool operator ==(ClassValue&);
bool operator !=(ClassValue& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {ClassValue a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~ClassValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "ClassValue"; }

// End of automatically generated code           //
