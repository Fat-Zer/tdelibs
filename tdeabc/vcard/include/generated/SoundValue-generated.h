// XXX Automatically generated. DO NOT EDIT! XXX //

public:
SoundValue();
SoundValue(const SoundValue&);
SoundValue(const TQCString&);
SoundValue & operator = (SoundValue&);
SoundValue & operator = (const TQCString&);
bool operator ==(SoundValue&);
bool operator !=(SoundValue& x) {return !(*this==x);}
bool operator ==(const TQCString& s) {SoundValue a(s);return(*this==a);} 
bool operator != (const TQCString& s) {return !(*this == s);}

virtual ~SoundValue();
void parse() {if(!parsed_) _parse();parsed_=true;assembled_=false;}

void assemble() {if(assembled_) return;parse();_assemble();assembled_=true;}

void _parse();
void _assemble();
const char * className() const { return "SoundValue"; }

// End of automatically generated code           //
