//https://code.google.com/p/nya-engine/

#ifndef log_h
#define log_h

namespace log
{

enum message_type
{
	msgtype_normal,
	msgtype_file,
	msgtype_error,
	msgtype_warning,
	msgtype_name,
	msgtype_error_internal
};

class log
{
public:
    virtual log &operator << (message_type) { return *this; }
    virtual log &operator << (int) { return *this; }
    virtual log &operator << (const char *) { return *this; }

	virtual void level_inc() {}
	virtual void level_dec() {}
};

}
#endif

