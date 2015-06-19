//https://code.google.com/p/nya-engine/

#include "output_stream.h"
#include <stdio.h>

namespace nya_log
{

namespace { char buf[512]; }

ostream_base &ostream_base::operator << (long int a) { sprintf(buf,"%ld", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (unsigned long int a) { sprintf(buf,"%lu", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (float a) { sprintf(buf,"%f", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (const char *a) { output(a?a:"NULL"); return *this; }

ostream_base &ostream_base::operator << (int a) { sprintf(buf,"%d", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (unsigned int a) { sprintf(buf,"%u", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (short int a) { sprintf(buf,"%d", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (unsigned short int a) { sprintf(buf,"%u", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (long long int a) { sprintf(buf,"%lld", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (unsigned long long int a) { sprintf(buf,"%llu", a); output(buf); return *this; }
ostream_base &ostream_base::operator << (const std::string &a) { output(a.c_str()); return *this; }

}
