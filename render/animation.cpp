//https://code.google.com/p/nya-engine/

#include "animation.h"

namespace nya_render
{

int animation::get_bone_idx(const char *name) const
{
    if(!name)
        return -1;

    index_map::const_iterator it=m_bones_map.find(name);
    if(it==m_bones_map.end())
        return -1;

    return it->second;
}

animation::bone animation::get_bone(int idx,unsigned int time,bool looped) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return bone();

    if(m_duration)
        time=time%m_duration;
    else
        time=0;

    //ToDo: faster frame search, start with non-first frame
    // via additional frames map with constant quantity

    const sequence &seq=m_bones[idx];

    const unsigned int frames_count=(unsigned int)seq.frames.size();
    for(unsigned int i=frames_count;i>0;--i)
    {
        const frame &prev=seq.frames[i-1];
        if(prev.time<=time)
        {
            bone b;

            const frame *next;
            int time_diff;

            if(i==frames_count)
            {
                if(!looped || i==0)
                    return prev.pos_rot;

                next=&seq.frames[0];
                time_diff=m_duration-prev.time+next->time;
            }
            else
            {
                next=&seq.frames[i];
                time_diff=next->time-prev.time;
            }

            if(time_diff==0)
                return next->pos_rot;

            const float inter_arg=float(time-prev.time)/time_diff;

            const interpolation &inter=next->inter;

            float lerp=inter.pos_x.get(inter_arg);
            b.pos.x=next->pos_rot.pos.x*lerp+prev.pos_rot.pos.x*(1.0f-lerp);

            lerp=inter.pos_y.get(inter_arg);
            b.pos.y=next->pos_rot.pos.y*lerp+prev.pos_rot.pos.y*(1.0f-lerp);

            lerp=inter.pos_z.get(inter_arg);
            b.pos.z=next->pos_rot.pos.z*lerp+prev.pos_rot.pos.z*(1.0f-lerp);

            lerp=inter.rot.get(inter_arg);
            b.rot=nya_math::quat::slerp(prev.pos_rot.rot,next->pos_rot.rot,lerp);

            return b;
        }
    }

    return bone();
}

int animation::add_bone(const char *name)
{
    if(!name)
        return -1;

    index_map::const_iterator it=m_bones_map.find(name);
    if(it!=m_bones_map.end())
        return it->second;

    int bone_idx=(int)m_bones.size();
    m_bones_map[name]=bone_idx;
    m_bones.resize(bone_idx+1);

    return bone_idx;
}

void animation::add_bone_frame(int idx,unsigned int time,bone &b)
{
    interpolation i;
    add_bone_frame(idx,time,b,i);
}

void animation::add_bone_frame(int idx,unsigned int time,bone &b,interpolation &i)
{
    if(idx<0 || idx>=(int)m_bones.size())
        return;

    sequence &seq=m_bones[idx];

    frame f;
    f.time=time;
    f.pos_rot=b;
    f.inter=i;

    if(time>m_duration)
        m_duration=time;

    for(int i=(int)seq.frames.size()-1;i>=0;--i)
    {
        if(seq.frames[i].time<time)
        {
            seq.frames.insert(seq.frames.begin()+i+1,f);
            return;
        }
    }

    seq.frames.push_back(f);
}

void animation::release()
{
    m_bones_map.clear();
    m_bones.clear();
    m_duration=0;
}

}
