//https://code.google.com/p/nya-engine/

#include "stdout_log.h"
#include <stdio.h>

namespace nya_log
{

void stdout_log::output(const char *str) { printf("%s", str?str:"NULL"); }

}
