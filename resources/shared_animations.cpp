//https://code.google.com/p/nya-engine/

#include "memory/tmp_buffer.h"
#include "memory/memory_mapper.h"
#include "resources.h"
#include "shared_animations.h"
#include "string.h"

namespace
{

#pragma pack(push,1)
    struct vmd_header
    {
        char sign[30];
        char model_name[20];
    };

    struct vmd_bone_frame
    {
        char name[15];
        unsigned int frame;

        float pos[3];
        float rot[4];

        char bezier_x[16];
        char bezier_y[16];
        char bezier_z[16];
        char bezier_rot[16];
    };
#pragma pack(pop)

}

namespace nya_resources
{

bool load_vmd(nya_render::animation &res,size_t data_size,void*data)
{
    if(!data_size||!data)
        return false;

    nya_memory::memory_mapper mapper(data,data_size);
    const vmd_header *header=mapper.read<vmd_header>();
    if(!header)
        return false;

    if(strncmp(header->sign,"Vocaloid Motion Data 0002",30)!=0)
        return false;

    nya_memory::array<vmd_bone_frame> bones=
                mapper.read_array<vmd_bone_frame,unsigned int>();

    for(size_t i=0;i<bones.get_count();++i)
    {
        const vmd_bone_frame &bone_frame=bones.get(i);

        const float c2f=1.0f/128.0f;

        unsigned int bone_idx=res.add_bone(bone_frame.name);
        if(!bone_idx)
            continue;

        unsigned int time=bone_frame.frame*60; //ToDo: adjust framerate

        nya_render::animation::bone bone;
        nya_render::animation::interpolation interpolation;

        bone.pos=nya_math::vec3(bone_frame.pos);
        bone.rot=nya_math::quat(bone_frame.rot);

        interpolation.pos_x=nya_math::bezier(
                bone_frame.bezier_x[0]*c2f,bone_frame.bezier_x[4]*c2f,
                bone_frame.bezier_x[8]*c2f,bone_frame.bezier_x[12]*c2f);

        interpolation.pos_y=nya_math::bezier(
                bone_frame.bezier_y[0]*c2f,bone_frame.bezier_y[4]*c2f,
                bone_frame.bezier_y[8]*c2f,bone_frame.bezier_y[12]*c2f);

        interpolation.pos_z=nya_math::bezier(
                bone_frame.bezier_z[0]*c2f,bone_frame.bezier_z[4]*c2f,
                bone_frame.bezier_z[8]*c2f,bone_frame.bezier_z[12]*c2f);

        interpolation.rot=nya_math::bezier(
                bone_frame.bezier_rot[0]*c2f,bone_frame.bezier_rot[4]*c2f,
                bone_frame.bezier_rot[8]*c2f,bone_frame.bezier_rot[12]*c2f);

        res.add_bone_frame(bone_idx,time,bone,interpolation);
    }

    get_log()<<"loaded vmd "<<bones.get_count()<<"\n";

    return true;
}

bool shared_animations_manager::fill_resource(const char *name,nya_render::animation &res)
{
    nya_resources::resource_data *file_data = nya_resources::get_resources_provider().access(name);
    if(!file_data)
    {
        get_log()<<"unable to load animation: unable to acess resource\n";
        return false;
    }

    const size_t data_size=file_data->get_size();
    nya_memory::tmp_buffer_scoped animation_data(data_size);
    file_data->read_all(animation_data.get_data());
    file_data->release();

    if(load_vmd(res,data_size,animation_data.get_data()))
        return true;

    return false;
}

bool shared_animations_manager::release_resource(nya_render::animation &res)
{
	return true;
}

shared_animations_manager &get_shared_animations()
{
    static shared_animations_manager manager;
    return manager;
}

}

