//https://code.google.com/p/nya-engine/

#ifndef character_h
#define character_h

#include "tmb_model.h"

#include <string>
#include <map>
#include <list>

class character
{
public:
    void set_attrib(const char *key, const char *value,int num=-1);
    void reset_attrib();
    void set_anim(const char *anim_name);

    void draw(bool use_materials);

    void release();

    //CRAP
    float *get_buffer(unsigned int frame);
    unsigned int get_frames_count();
    unsigned int get_bones_count();

private:
    struct part
    {
        std::string value;
        std::vector<model_ref> models;
        
        void free_models()
        {
            for(unsigned int i=0;i<models.size();++i)
                models[i].free();

            models.clear();
        }
    };

    typedef std::map<std::string,part*> parts_map;
    std::map<std::string,part*> m_parts_map;
    typedef std::list<part> parts_list;
    parts_list m_parts;
};
#endif
