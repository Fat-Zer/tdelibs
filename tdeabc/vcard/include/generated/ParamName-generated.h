// XXX Automatically generated. DO NOT EDIT! XXX //

public:
V_ParamName();
V_ParamName(const V_ParamName&);
V_ParamName(const TQCString&);
V_ParamName & operator = (V_ParamName&);
V_ParamName & operator = (const TQCString&);
bool operator ==(V_ParamName&);
bool operator !=(V_ParamName& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {V_ParamName a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~V_ParamName();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();

// End of automatically generated code           //
