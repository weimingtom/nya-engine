//https://code.google.com/p/nya-engine/

#include "animation.h"

namespace
{

template<typename t_map,typename t_data> int add_bone_curve(const char *name,t_map &map,t_data &data)
{
    if(!name)
        return -1;

    const int idx=(int)data.size();
    std::pair<typename t_map::iterator,bool> ret=
    map.insert(std::pair<std::string,int>(name,idx));

    if(ret.second==false)
        return ret.first->second;

    map[name]=idx;
    data.resize(idx+1);
    data.back().name.assign(name);

    return idx;
}

template<typename t_list,typename t_frame> void add_frame(t_list &seq,const t_frame &f,
                                                          unsigned int time,unsigned int &duration)
{
    if(time>duration)
        duration=time;

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

template<typename t_map> int get_idx(const char *name,t_map &map)
{
    if(!name)
        return -1;

    typename t_map::const_iterator it=map.find(name);
    if(it==map.end())
        return -1;

    return it->second;
}

template<typename t_value,typename t_data,typename t_frame> t_value get_value(int idx,
                  unsigned int time,bool looped,const std::vector<t_data> &data,unsigned int duration)
{
    if(idx<0 || idx>=(int)data.size())
        return t_value();

    if(time>duration)
    {
        if(looped)
        {
            if(duration)
                time=time%duration;
            else
                time=0;
        }
        else
            time=duration;
    }
    
    //ToDo: faster frame search, start with non-first frame
    // via additional frames map with constant quantity
    
    const t_data &seq=data[idx];
    
    const unsigned int frames_count=(unsigned int)seq.frames.size();
    for(unsigned int i=frames_count;i>0;--i)
    {
        const t_frame &prev=seq.frames[i-1];
        if(prev.time<=time)
        {
            if(i==frames_count)
                return prev.value;

            const t_frame &next=seq.frames[i];
            const int time_diff=next.time-prev.time;
            
            if(time_diff==0)
                return next.value;

            return next.interpolate(prev,float(time-prev.time)/time_diff);
        }
    }

    if(frames_count)
        return seq.frames[0].value;

    return t_value();
}


}

namespace nya_render
{

int animation::get_bone_idx(const char *name) const { return get_idx(name,m_bones_map); }

animation::bone animation::get_bone(int idx,unsigned int time,bool looped) const
{
    return get_value<bone,sequence,frame>(idx,time,looped,m_bones,m_duration);
}

animation::bone animation::frame::interpolate(const frame &prev,float k) const
{
    bone b;

    float lerp=inter.pos_x.get(k);
    b.pos.x=value.pos.x*lerp+prev.value.pos.x*(1.0f-lerp);

    lerp=inter.pos_y.get(k);
    b.pos.y=value.pos.y*lerp+prev.value.pos.y*(1.0f-lerp);

    lerp=inter.pos_z.get(k);
    b.pos.z=value.pos.z*lerp+prev.value.pos.z*(1.0f-lerp);

    lerp=inter.rot.get(k);
    b.rot=nya_math::quat::slerp(prev.value.rot,value.rot,lerp);

    return b;
}

float animation::curve_frame::interpolate(const curve_frame &prev,float k) const
{
    return value*k+prev.value*(1.0f-k);
}

const char *animation::get_bone_name(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return 0;

    return m_bones[idx].name.c_str();
}

int animation::get_curve_idx(const char *name) const { return get_idx(name,m_curves_map); }

float animation::get_curve(int idx,unsigned int time,bool looped) const
{
    return get_value<float,curve_sequence,curve_frame>(idx,time,looped,m_curves,m_duration);
}

const char *animation::get_curve_name(int idx) const
{
    if(idx<0 || idx>=(int)m_curves.size())
        return 0;

    return m_curves[idx].name.c_str();
}

int animation::add_bone(const char *name) { return add_bone_curve(name,m_bones_map,m_bones); }

void animation::add_bone_frame(int idx,unsigned int time,bone &b)
{
    interpolation i;
    add_bone_frame(idx,time,b,i);
}

void animation::add_bone_frame(int idx,unsigned int time,bone &b,interpolation &i)
{
    if(idx<0 || idx>=(int)m_bones.size())
        return;

    frame f;
    f.time=time;
    f.value=b;
    f.inter=i;

    add_frame(m_bones[idx],f,time,m_duration);
}

int animation::add_curve(const char *name) { return add_bone_curve(name,m_curves_map,m_curves); }

void animation::add_curve_frame(int idx,unsigned int time,float value)
{
    if(idx<0 || idx>=(int)m_curves.size())
        return;

    curve_frame f;
    f.time=time;
    f.value=value;

    add_frame(m_curves[idx],f,time,m_duration);
}

void animation::release()
{
    m_bones_map.clear();
    m_bones.clear();
    m_curves_map.clear();
    m_curves.clear();
    m_duration=0;
}

}
