//https://code.google.com/p/nya-engine/

#ifndef label_h
#define label_h

#include "ui/ui.h"

namespace nya_ui
{

class label: public widget
{
public:
    virtual void set_text(const char *text)
    {
        if(!text)
            return;

        m_text.assign(text);
    }

private:
    virtual void draw(layer &l);

private:
    std::string m_text;
};

}

#endif
