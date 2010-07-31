// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextNSParam();
TextNSParam(const TextNSParam&);
TextNSParam(const TQCString&);
TextNSParam & operator = (TextNSParam&);
TextNSParam & operator = (const TQCString&);
bool operator ==(TextNSParam&);
bool operator !=(TextNSParam& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {TextNSParam a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~TextNSParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextNSParam"; }

// End of automatically generated code           //
