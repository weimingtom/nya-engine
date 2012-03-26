//https://code.google.com/p/nya-engine/

#include "log.h"

namespace nya_log
{

log &no_log()
{
	static log l;
	return l;
}

}
