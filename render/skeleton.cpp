//https://code.google.com/p/nya-engine/

#include "skeleton.h"

namespace nya_render
{

int skeleton::add_bone(const char *name,const nya_math::vec3 &pos,const nya_math::quat &rot,int parent,bool allow_doublicate)
{
    if(!name)
        return -1;

    if(parent>=(int)m_bones.size())
        return -1;

    int bone_idx=(int)m_bones.size();
    std::pair<index_map::iterator,bool> ret=
            m_bones_map.insert (std::pair<std::string,int>(name,bone_idx));

    if(!allow_doublicate && ret.second==false)
        return ret.first->second;

    m_bones.resize(bone_idx+1);
    m_pos_tr.resize(bone_idx+1);
    m_rot_tr.resize(bone_idx+1);

    if(!m_rot_org.empty() || rot.v.length_sq()>0.001f)
        m_rot_org.resize(m_bones.size());

    bone &b=m_bones[bone_idx];
    b.parent=parent;
    b.name.assign(name);

    b.pos_org=pos;
    if(parent>=0)
    {
        const bone &p=m_bones[parent];
        b.offset=pos-p.pos_org;
    }
    else
        b.offset=pos;

    if(!m_rot_org.empty())
    {
        m_rot_org[bone_idx].rot_org=rot;
        if(parent>=0)
        {
            nya_math::quat pq=m_rot_org[parent].rot_org;
            pq.v= -pq.v;
            m_rot_org[bone_idx].offset=pq*m_rot_org[bone_idx].rot_org;
            b.offset=pq.rotate(b.offset);
        }
        else
            m_rot_org[bone_idx].offset=m_rot_org[bone_idx].rot_org;
    }

    update_bone(bone_idx);

    return bone_idx;
}

void skeleton::update_bone(int idx,const nya_math::vec3 &pos,const nya_math::quat &rot)
{
    const bone &b=m_bones[idx];
    if(b.parent<0)
    {
        m_pos_tr[idx]=pos+b.offset;
        if(m_rot_org.empty())
            m_rot_tr[idx]=rot;
        else
            m_rot_tr[idx]=m_rot_org[idx].offset*rot;

        return;
    }

    m_pos_tr[idx]=m_pos_tr[b.parent] + m_rot_tr[b.parent].rotate(pos+b.offset);

    if(m_rot_org.empty())
        m_rot_tr[idx]=m_rot_tr[b.parent]*rot;
    else
        m_rot_tr[idx]=m_rot_tr[b.parent]*(m_rot_org[idx].offset*rot);
}

int skeleton::get_bone_idx(const char *name) const
{
    if(!name)
        return -1;

    index_map::const_iterator it=m_bones_map.find(name);
    if(it==m_bones_map.end())
        return -1;

    return (int)it->second;
}

int skeleton::get_bone_parent_idx(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return -1;

    return m_bones[idx].parent;
}

const char *skeleton::get_bone_name(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return 0;

    return m_bones[idx].name.c_str();
}

nya_math::vec3 skeleton::get_bone_pos(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::vec3();

    return m_pos_tr[idx];
}

nya_math::quat skeleton::get_bone_rot(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::quat();

    return m_rot_tr[idx];
}

nya_math::vec3 skeleton::get_bone_local_pos(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::vec3();

    return m_bones[idx].pos;
}

nya_math::quat skeleton::get_bone_local_rot(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::quat();

    return m_bones[idx].rot;
}

nya_math::vec3 skeleton::get_bone_original_pos(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::vec3();

    return m_bones[idx].pos_org;
}

nya_math::quat skeleton::get_bone_original_rot(int idx) const
{
    if(idx<0 || idx>=(int)m_rot_org.size())
        return nya_math::quat();

    return m_rot_org[idx].rot_org;
}

int skeleton::add_ik(int target_bone_idx,int effect_bone_idx,int count,float fact,bool allow_invalid)
{
    if(target_bone_idx<0 || (!allow_invalid && target_bone_idx>=(int)m_bones.size()))
        return -1;

    if(effect_bone_idx<0 || (!allow_invalid && effect_bone_idx>=(int)m_bones.size()))
        return -1;

    int ik_idx=(int)m_iks.size();
    m_iks.resize(ik_idx+1);

    ik &k=m_iks[ik_idx];
    k.target=target_bone_idx;
    k.eff=effect_bone_idx;
    k.count=count;
    k.fact=fact;

    return ik_idx;
}

bool skeleton::add_ik_link(int ik_idx,int bone_idx,bool allow_invalid)
{
    if(ik_idx<0 || ik_idx>=(int)m_iks.size())
        return false;

    if(bone_idx<0 || (!allow_invalid && bone_idx>=(int)m_bones.size()))
        return false;

    ik &k=m_iks[ik_idx];
    k.links.resize(k.links.size()+1);
    k.links.back().idx=bone_idx;
    k.links.back().limit=false;

    return true;
}

bool skeleton::add_ik_link(int ik_idx,int bone_idx,float limit_from,float limit_to,bool allow_invalid)
{
    if(ik_idx<0 || ik_idx>=(int)m_iks.size())
        return false;

    if(bone_idx<0 || (!allow_invalid && bone_idx>=(int)m_bones.size()))
        return false;

    ik &k=m_iks[ik_idx];
    k.links.resize(k.links.size()+1);
    k.links.back().idx=bone_idx;
    k.links.back().limit=true;
    k.links.back().limit_from=limit_from;
    k.links.back().limit_to=limit_to;

    return true;
}

bool skeleton::add_bound(int bone_idx,int target_bone_idx,float k,bool pos,bool rot,bool allow_invalid)
{
    if(bone_idx<0 || (!allow_invalid && bone_idx>=(int)m_bones.size()))
        return false;

    if(target_bone_idx<0 || (!allow_invalid && target_bone_idx>=(int)m_bones.size()))
        return false;

    if(!pos && !rot)
        return false;

    m_bounds.resize(m_bounds.size()+1);
    m_bounds.back().idx=bone_idx;
    m_bounds.back().target=target_bone_idx;
    m_bounds.back().k=k;
    m_bounds.back().pos=pos;
    m_bounds.back().rot=rot;

    return true;
}

void skeleton::set_bone_transform(int bone_idx,const nya_math::vec3 &pos,const nya_math::quat &rot)
{
    if(bone_idx<0 || bone_idx>=(int)m_bones.size())
        return;

    bone &b=m_bones[bone_idx];
    b.pos=pos;
    b.rot=rot;
}

void skeleton::update_bone_childs(int idx) //ToDo: better realisation
{
    for(int j=0;j<(int)m_bones.size();++j)
    {
        if(m_bones[j].parent==idx)
        {
            update_bone(j);
            update_bone_childs(j);
        }
    }
}

void skeleton::update_ik(int idx)
{
    const ik &k=m_iks[idx];
    const nya_math::vec3 target_pos_org=m_pos_tr[k.target];

    for(int j=0;j<k.count;++j)
    {
        for(int l=0;l<(int)k.links.size();++l)
        {
            const int lnk_idx=k.links[l].idx;
            bone &lnk=m_bones[lnk_idx];

            nya_math::vec3 target_pos=
            m_rot_tr[lnk_idx].rotate_inv(target_pos_org-m_pos_tr[lnk_idx]);

            nya_math::vec3 eff_pos=
            m_rot_tr[lnk_idx].rotate_inv(m_pos_tr[k.eff]-m_pos_tr[lnk_idx]);

            const float eps=0.0001f;

            const nya_math::vec3 diff=eff_pos-target_pos;
            if(diff.length_sq()<eps)
                return;

            eff_pos.normalize();
            target_pos.normalize();

            float ang=acosf(eff_pos.dot(target_pos));
            if(fabsf(ang)<eps)
                return;

            if(ang< -k.fact)
                ang= -k.fact;
            else if(ang>k.fact)
                ang=k.fact;

            nya_math::vec3 axis=nya_math::vec3::cross(eff_pos,target_pos);
            const float axis_len=axis.length();
            if(axis_len<0.001f)
                return;

            axis*=(1.0f/axis_len);

            nya_math::quat rot(axis,ang);

            if(k.links[l].limit)
                rot.limit_pitch(k.links[l].limit_from,k.links[l].limit_to);

            rot.normalize();

            lnk.rot=lnk.rot*rot;
            lnk.rot.normalize();

            for(int m=l;m>=0;--m)
                update_bone(k.links[m].idx);

            update_bone(k.eff);
        }
    }
}

void skeleton::update()
{
    for(int i=0;i<(int)m_bones.size();++i)
        update_bone(i);

    for(int i=0;i<(int)m_iks.size();++i)
        update_ik(i);

    for(int i=0;i<(int)m_bounds.size();++i)
    {
        const bound &b=m_bounds[i];
        const bone &f=m_bones[b.idx];
        bone &t=m_bones[b.target];

        nya_math::quat tmp=f.rot;
        if(b.rot)
            tmp.apply_weight(b.k);

        update_bone(b.target,b.pos?t.pos+f.pos*b.k:t.pos,b.rot?(t.rot*tmp).normalize():t.rot);
        update_bone_childs(b.target);
    }
}

nya_math::vec3 skeleton::transform(int bone_idx,const nya_math::vec3 &point) const
{
    if(bone_idx<0 || bone_idx>=(int)m_bones.size())
        return point;

    return m_pos_tr[bone_idx]+m_rot_tr[bone_idx].rotate(point);
}

const float *skeleton::get_pos_buffer() const
{
    if(m_pos_tr.empty())
        return 0;

    return &m_pos_tr[0].x;
}

const float *skeleton::get_rot_buffer() const
{
    if(m_rot_tr.empty())
        return 0;

    return &m_rot_tr[0].v.x;
}

}
