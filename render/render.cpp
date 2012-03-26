//https://code.google.com/p/nya-engine/

#include "render.h"

namespace nya_render
{

nya_log::log *render_log=0;

void set_log(nya_log::log *l)
{
	render_log = l;
}

nya_log::log &get_log()
{
	if(!render_log)
		return nya_log::no_log();

	return *render_log;
}

}

