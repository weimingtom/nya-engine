//https://code.google.com/p/nya-engine/

#include "ui/ui.h"

namespace
{
	nya_log::log *ui_log=0;
}

namespace nya_ui
{
    void set_log(nya_log::log *l)
    {
        ui_log = l;
    }

    nya_log::log &get_log()
    {
        static const char *ui_log_tag="ui";
        if(!ui_log)
        {
            return nya_log::get_log(ui_log_tag);
        }

        ui_log->set_tag(ui_log_tag);
        return *ui_log;
    }
}
