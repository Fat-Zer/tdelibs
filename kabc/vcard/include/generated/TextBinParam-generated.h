// XXX Automatically generated. DO NOT EDIT! XXX //

public:
TextBinParam();
TextBinParam(const TextBinParam&);
TextBinParam(const TQCString&);
TextBinParam & operator = (TextBinParam&);
TextBinParam & operator = (const TQCString&);
bool operator ==(TextBinParam&);
bool operator !=(TextBinParam& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {TextBinParam a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~TextBinParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "TextBinParam"; }

// End of automatically generated code           //
