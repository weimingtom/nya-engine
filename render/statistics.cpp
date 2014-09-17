//https://code.google.com/p/nya-engine/

#include "statistics.h"
#include "platform_specific_gl.h"

namespace nya_render
{

namespace { statistics stats; bool stats_enabled=false; }

void statistics::begin_frame() { stats=statistics(); stats_enabled=true; }
statistics &statistics::get() { return stats; }
bool statistics::enabled() { return stats_enabled; }

}
