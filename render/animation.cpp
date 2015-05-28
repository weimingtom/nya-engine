//https://code.google.com/p/nya-engine/

#include "animation.h"

namespace
{

template<typename t_map,typename t_data,typename t_names_data> int add_bone_curve(const char *name,
                                                               t_map &map,t_data &data,t_names_data&names)
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
    names.resize(idx+1);
    names.back().assign(name);

    return idx;
}

template<typename t_list,typename t_frame> void add_frame(t_list &seq,const t_frame &f,
                                                          unsigned int time,unsigned int &duration)
{
    if(time>duration)
        duration=time;

    seq[time]=f;
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

    const t_data &seq=data[idx];

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

    typename t_data::const_iterator it_next=seq.lower_bound(time);
    if(it_next==seq.end())
        return seq.empty()?t_value():seq.rbegin()->second.value;

    if(it_next==seq.begin())
        return it_next->second.value;

    typename t_data::const_iterator it=it_next;
    --it;

    const int time_diff=it_next->first-it->first;

    if(time_diff==0)
        return it_next->second.value;

    return it_next->second.interpolate(it->second,float(time-it->first)/time_diff);
}


}

namespace nya_render
{

int animation::get_bone_idx(const char *name) const { return get_idx(name,m_bones_map); }

nya_math::vec3 animation::get_bone_pos(int idx,unsigned int time,bool looped) const
{
    return get_value<nya_math::vec3,pos_sequence,pos_frame>(idx,time,looped,m_pos_sequences,m_duration);
}

nya_math::quat animation::get_bone_rot(int idx,unsigned int time,bool looped) const
{
    return get_value<nya_math::quat,rot_sequence,rot_frame>(idx,time,looped,m_rot_sequences,m_duration);
}

nya_math::vec3 animation::pos_frame::interpolate(const pos_frame &prev,float k) const
{
    nya_math::vec3 p;

    float lerp=inter.x.get(k);
    p.x=value.x*lerp+prev.value.x*(1.0f-lerp);

    lerp=inter.y.get(k);
    p.y=value.y*lerp+prev.value.y*(1.0f-lerp);

    lerp=inter.z.get(k);
    p.z=value.z*lerp+prev.value.z*(1.0f-lerp);

    return p;
}

nya_math::quat animation::rot_frame::interpolate(const rot_frame &prev,float k) const
               { return nya_math::quat::slerp(prev.value,value,inter.get(k)); }

float animation::curve_frame::interpolate(const curve_frame &prev,float k) const
                                   { return value*k+prev.value*(1.0f-k); }

const char *animation::get_bone_name(int idx) const
{
    if(idx<0 || idx>=(int)m_bone_names.size())
        return 0;

    return m_bone_names[idx].c_str();
}

int animation::get_curve_idx(const char *name) const { return get_idx(name,m_curves_map); }

float animation::get_curve(int idx,unsigned int time,bool looped) const
{
    return get_value<float,curve_sequence,curve_frame>(idx,time,looped,m_curves,m_duration);
}

const char *animation::get_curve_name(int idx) const
{
    if(idx<0 || idx>=(int)m_curve_names.size())
        return 0;

    return m_curve_names[idx].c_str();
}

int animation::add_bone(const char *name)
{
    const int idx=add_bone_curve(name,m_bones_map,m_pos_sequences,m_bone_names);
    if(idx>=int(m_rot_sequences.size()))
        m_rot_sequences.resize(idx+1);

    return idx;
}

void animation::add_bone_pos_frame(int bone_idx,unsigned int time,const nya_math::vec3 &pos,const pos_interpolation &interpolation)
{
    if(bone_idx<0 || bone_idx>=(int)m_pos_sequences.size())
        return;

    pos_frame pf;
    pf.value=pos;
    pf.inter=interpolation;
    add_frame(m_pos_sequences[bone_idx],pf,time,m_duration);
}

void animation::add_bone_rot_frame(int bone_idx,unsigned int time,const nya_math::quat &rot,const nya_math::bezier &interpolation)
{
    if(bone_idx<0 || bone_idx>=(int)m_rot_sequences.size())
        return;

    rot_frame rf;
    rf.value=rot;
    rf.inter=interpolation;
    add_frame(m_rot_sequences[bone_idx],rf,time,m_duration);
}

int animation::add_curve(const char *name) { return add_bone_curve(name,m_curves_map,m_curves,m_curve_names); }

void animation::add_curve_frame(int idx,unsigned int time,float value)
{
    if(idx<0 || idx>=(int)m_curves.size())
        return;

    curve_frame f;
    f.value=value;

    add_frame(m_curves[idx],f,time,m_duration);
}

}
