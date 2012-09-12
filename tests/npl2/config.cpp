//https://code.google.com/p/nya-engine/

#include "config.h"
#include "attributes.h"
#include "string.h"
#include "stdlib.h"

void bool_from_str(bool &b,const char *str)
{
    if(!str)
        return;

    if(strcasecmp(str,"true")==0 || strcmp(str,"1")==0)
        b=true;

    if(strcasecmp(str,"false")==0 || strcmp(str,"0")==0)
        b=false;
}

void float_from_str(float &f,const char *str)
{
    if(!str)
        return;

    f=atof(str);
}

void int_from_str(int &i,const char *str)
{
    if(!str)
        return;

    i=atoi(str);
}

bool config::load(nya_resources::resource_data *data)
{
    if(!data)
        return false;

    attribute_parser parser;
    bool result=parser.load(data);

    bool_from_str(wireframe_outline_enabled,parser.get_value("wireframe_outline_enabled"));
    bool_from_str(specular_enabled,parser.get_value("specular_enabled"));
    float_from_str(specular_level,parser.get_value("specular_level"));

    return result;
}

config &get_config()
{
    static config cfg;
    return cfg;
}
