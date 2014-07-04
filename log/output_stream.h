//https://code.google.com/p/nya-engine/

#pragma once

#include <string>
#include <sstream>

namespace nya_log
{

class ostream_base
{
public:
    virtual ostream_base &operator << (long int) { return *this; }
    virtual ostream_base &operator << (unsigned long int) { return *this; }
    virtual ostream_base &operator << (float) { return *this; }
    virtual ostream_base &operator << (const char *) { return *this; }

    virtual ostream_base &operator << (int a) { return operator<<((long int)a); }
    virtual ostream_base &operator << (unsigned int a) { return operator<<((unsigned long int)a); }
    virtual ostream_base &operator << (short int a) { return operator<<((long int)a); }
    virtual ostream_base &operator << (unsigned short int a) { return operator<<((unsigned long int)a); }
    virtual ostream_base &operator << (const std::string &a) { return operator<<(a.c_str()); }

    virtual ~ostream_base() {}
};

class memory_ostream: public ostream_base
{
public:
    memory_ostream() {}
    memory_ostream(const memory_ostream &s) { m_sstream<<s.m_sstream.str(); }
    memory_ostream &operator=(const memory_ostream &s) {m_sstream.str(s.m_sstream.str()); return *this;}

    virtual ostream_base &operator << (long int a) { m_sstream<<a; return *this; }
    virtual ostream_base &operator << (unsigned long int a) { m_sstream<<a; return *this; }
    virtual ostream_base &operator << (float a) { m_sstream<<a; return *this; }
    virtual ostream_base &operator << (const char *a) { m_sstream<<(a?a:"NULL"); return *this; }

    const char *get_text() const { return m_sstream.str().c_str(); }
    void clear() { m_sstream.str(""); }

private:
    std::ostringstream m_sstream;
};

}
