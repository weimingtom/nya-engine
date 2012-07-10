//https://code.google.com/p/nya-engine/

#ifndef tsb_anim_h
#define tsb_anim_h

#include "resources/resources.h"
#include "resources/shared_resources.h"
#include <vector>

class tsb_anim
{
public:
    bool load(nya_resources::resource_data *data);
    
    struct bone
    {
        float mat[4][4];
    };

    bone *get_bones(unsigned int frame)
    {
        if(!m_bones_count || frame>=m_frames_count)
            return 0;

        return &m_data[m_bones_count*(frame % m_frames_count)];
    }

    unsigned int get_bones_count() { return m_bones_count; }
    unsigned int get_frames_count() { return m_frames_count; }

    void release() { m_data.clear(); m_bones_count=m_frames_count=0; }

    tsb_anim(): m_bones_count(0), m_frames_count(0) {}

private:
    unsigned int m_bones_count;
    unsigned int m_frames_count;

private:
    std::vector<bone> m_data;
};

typedef nya_resources::shared_resources<tsb_anim,8> shared_anims;
typedef shared_anims::shared_resource_ref anim_ref;

class shared_anims_manager: public shared_anims
{
    bool fill_resource(const char *name,tsb_anim &res);
    bool release_resource(tsb_anim &res);
};

shared_anims_manager &get_shared_anims();

#endif
