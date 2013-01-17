//https://code.google.com/p/nya-engine/

#include "skeleton.h"
#include "math/constants.h"

namespace nya_render
{

int skeleton::add_bone(const char *name,const nya_math::vec3 &pos,int parent)
{
    if(!name)
        return -1;

    if(parent>=(int)m_bones.size())
        return -1;

    index_map::const_iterator it=m_bones_map.find(name);
    if(it!=m_bones_map.end())
        return it->second;

    int bone_idx=(int)m_bones.size();
    m_bones_map[name]=bone_idx;
    m_bones.resize(bone_idx+1);
    m_pos_tr.resize(bone_idx+1);
    m_rot_tr.resize(bone_idx+1);

    bone &b=m_bones[bone_idx];
    b.pos_org=pos;
    b.parent=parent;

    if(parent>=0)
    {
        const bone &p=m_bones[b.parent];
        b.offset=b.pos_org-p.pos_org;
    }
    else
        b.offset=pos;

    return bone_idx;
}

nya_math::vec3 skeleton::get_bone_pos(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::vec3();

    return m_bones[idx].pos;
}

nya_math::quat skeleton::get_bone_rot(int idx) const
{
    if(idx<0 || idx>=(int)m_bones.size())
        return nya_math::vec3();

    return m_bones[idx].rot;
}

int skeleton::add_ik(int target_bone_idx,int effect_bone_idx,int count,float fact)
{
    if(target_bone_idx<0 || target_bone_idx>=(int)m_bones.size())
        return -1;

    if(effect_bone_idx<0 || effect_bone_idx>=(int)m_bones.size())
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

void skeleton::add_ik_link(int ik_idx,int bone_idx,bool limit_angle)
{
    if(ik_idx<0 || ik_idx>=(int)m_iks.size())
        return;

    if(bone_idx<0 || bone_idx>=(int)m_bones.size())
        return;

    ik &k=m_iks[ik_idx];
    k.links.resize(k.links.size()+1);
    k.links.back().idx=bone_idx;
    k.links.back().limit=limit_angle;
}

void skeleton::set_bone_transform(int bone_idx,const nya_math::vec3 &pos,const nya_math::quat &rot)
{
    if(bone_idx<0 || bone_idx>=(int)m_bones.size())
        return;

    bone &b=m_bones[bone_idx];
    b.pos=pos;
    b.rot=rot;
}

void skeleton::update_bone(int idx)
{
    bone &b=m_bones[idx];
    if(b.parent<=0)
    {
        m_pos_tr[idx]=b.pos+b.offset;
        m_rot_tr[idx]=b.rot;
        return;
    }

    m_pos_tr[idx]=m_pos_tr[b.parent] + m_rot_tr[b.parent].rotate(b.pos+b.offset);
    m_rot_tr[idx]=b.rot*m_rot_tr[b.parent];
}

void skeleton::update()
{
    for(int i=0;i<(int)m_bones.size();++i)
        update_bone(i);

    for(int i=0;i<(int)m_iks.size();++i)
    {
        const ik &k=m_iks[i];
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

                const float eps=0.00001f;

                const nya_math::vec3 diff=eff_pos-target_pos;
                if(diff*diff<eps)
                    return;

                eff_pos.normalize();
                target_pos.normalize();

                float ang=acosf(eff_pos*target_pos);
                if(fabsf(ang)<eps)
                    continue;

                if(ang< -k.fact)
                    ang= -k.fact;
                else if(ang>k.fact)
                    ang=k.fact;

                nya_math::vec3 axis=nya_math::vec3::cross(eff_pos,target_pos);
                if(axis*axis<eps)
                    continue;

                axis.normalize();

                nya_math::quat rot(axis,ang);

                if(k.links[l].limit)
					rot.limit_angle(-nya_math::constants::pi,-0.002f);

                rot.normalize();

                lnk.rot=lnk.rot*rot;
                lnk.rot.normalize();

                for(int m=l;m>=0;--m)
                    update_bone(k.links[m].idx);

                update_bone(k.eff);
            }
        }
    }
}

nya_math::vec3 skeleton::transform(int bone_idx,nya_math::vec3 point) const
{
    if(bone_idx<0 || bone_idx>=(int)m_bones.size())
        return nya_math::vec3();

    return m_pos_tr[bone_idx]+m_rot_tr[bone_idx].rotate(point-m_bones[bone_idx].pos_org);
}

float *skeleton::get_pos_buffer()
{
    if(m_pos_tr.empty())
        return 0;

    return &m_pos_tr[0].x;
}

float *skeleton::get_rot_buffer()
{
    if(m_rot_tr.empty())
        return 0;

    return &m_rot_tr[0].v.x;
}

}
