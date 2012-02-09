//https://code.google.com/p/nya-engine/

#include "render.h"

namespace render
{

log::log *render_log=0;

void set_log(log::log *l)
{
	render_log = l;
}

log::log &get_log()
{
	if(!render_log)
		return log::no_log();

	return *render_log;
}

}

