//https://code.google.com/p/nya-engine/

#ifndef log_h
#define log_h

namespace log
{

enum message_type
{
	normal,
	name,
	file,
	warning,
	error,
	error_internal
};

class log
{
public:
    virtual log &operator << (message_type) { return *this; }
    virtual log &operator << (int) { return *this; }
    virtual log &operator << (float) { return *this; }
    virtual log &operator << (const char *) { return *this; }

	virtual void scope_inc() {}
	virtual void scope_dec() {}
};

log &no_log();
    
}
#endif

