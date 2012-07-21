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
    const char *get_attrib(const char *key);
    void reset_attrib();
    void set_anim(const char *anim_name);
    const char *get_anim() { return m_anim_name.c_str(); }

    void draw(bool use_materials);

    void release();
    
    void copy_attributes(const character &from);

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

    typedef std::vector<part> parts_vector;
    parts_vector m_parts;
    typedef std::map<std::string,int> parts_map;
    parts_map m_parts_map;
    std::string m_anim_name;
};
#endif
