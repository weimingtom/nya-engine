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

class log_ref: public log
{
public:
    log_ref &operator << (message_type a) { if(!ref) return *this; *ref << a; return *this; }
    log_ref &operator << (int a) { if(!ref) return *this; *ref << a; return *this; }
    log_ref &operator << (const char *a) { if(!ref) return *this; *ref << a; return *this; }

    void level_inc() { if(!ref) return; ref->level_inc(); }
    void level_dec() { if(!ref) return; ref->level_dec(); }
    
    bool is_valid() { return ref !=0; }

    log_ref(): ref(0) {}
    
protected:
    log *ref;
};

}
#endif

