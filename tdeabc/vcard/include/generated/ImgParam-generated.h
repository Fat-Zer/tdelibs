// XXX Automatically generated. DO NOT EDIT! XXX //

public:
ImgParam();
ImgParam(const ImgParam&);
ImgParam(const TQCString&);
ImgParam & operator = (ImgParam&);
ImgParam & operator = (const TQCString&);
bool operator ==(ImgParam&);
bool operator !=(ImgParam& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {ImgParam a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~ImgParam();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
virtual const char * className() const { return "ImgParam"; }

// End of automatically generated code           //
