//https://code.google.com/p/nya-engine/

#include "quadtree.h"

namespace nya_math
{

namespace
{

struct quad
{
    int x,z,size_x,size_z;

    quad() {}
    quad(const aabb &box)
    {
        x=int(floorf(box.origin.x-box.delta.x));
        z=int(floorf(box.origin.z-box.delta.z));
        size_x=int(ceil(box.delta.x+box.delta.x));
        size_z=int(ceil(box.delta.z+box.delta.z));
    }
};

}

template<typename t> int add_object(const quad &obj,int obj_idx,float min_y,float max_y,const quad &leaf,int leaf_idx,std::vector<t> &leaves,int level)
{
    if(leaf_idx<0)
    {
        leaf_idx=int(leaves.size());
        leaves.resize(leaf_idx+1);
    }

    if(leaves[leaf_idx].min_y>min_y)
        leaves[leaf_idx].min_y=min_y;

    if(leaves[leaf_idx].max_y<max_y)
        leaves[leaf_idx].max_y=max_y;

    if(level<=0)
    {
        leaves[leaf_idx].objects.push_back(obj_idx);
        return leaf_idx;
    }

    --level;

    quad child;
    child.size_x=leaf.size_x/2;
    child.size_z=leaf.size_z/2;

    int center_x=leaf.x+child.size_x;
    int center_z=leaf.z+child.size_z;

    if(obj.x<=center_x)
    {
        child.x=leaf.x;

        if(obj.z<=center_z)
        {
            child.z=leaf.z;
            leaves[leaf_idx].leaves[0][0] = add_object(obj,obj_idx,min_y,max_y,child,leaves[leaf_idx].leaves[0][0],leaves,level);
        }

        if(obj.z+obj.size_z>center_z)
        {
            child.z=center_z;
            leaves[leaf_idx].leaves[0][1] = add_object(obj,obj_idx,min_y,max_y,child,leaves[leaf_idx].leaves[0][1],leaves,level);
        }
    }

    if(obj.x+obj.size_x>center_x)
    {
        child.x=center_x;

        if(obj.z<=center_z)
        {
            child.z=leaf.z;
            leaves[leaf_idx].leaves[1][0] = add_object(obj,obj_idx,min_y,max_y,child,leaves[leaf_idx].leaves[1][0],leaves,level);
        }

        if(obj.z+obj.size_z>center_z)
        {
            child.z=center_z;
            leaves[leaf_idx].leaves[1][1] = add_object(obj,obj_idx,min_y,max_y,child,leaves[leaf_idx].leaves[1][1],leaves,level);
        }
    }

    return leaf_idx;
}

void quadtree::add_object(const aabb &box,int idx)
{
    if(m_leaves.empty())
        return;

    objects_map::iterator it=m_objects.find(idx);
    if(it!=m_objects.end())
        remove_object(idx);

    m_objects[idx]=box;

    quad obj(box);
    quad leaf;leaf.x=m_x,leaf.z=m_z,leaf.size_x=m_size_x,leaf.size_z=m_size_z;
    int root=0;
    ::nya_math::add_object(quad(box),idx,box.origin.y-box.delta.y,box.origin.y+box.delta.y,leaf,root,m_leaves,m_max_level);
}

template<typename t> void remove_object(int obj_idx,int leaf_idx,std::vector<t> &leaves,int parent)
{
    if(leaf_idx<0)
        return;
    t &leaf=leaves[leaf_idx];
    remove_object(obj_idx,leaf.leaves[0][0],leaves,leaf_idx);
    remove_object(obj_idx,leaf.leaves[0][1],leaves,leaf_idx);
    remove_object(obj_idx,leaf.leaves[1][0],leaves,leaf_idx);
    remove_object(obj_idx,leaf.leaves[1][1],leaves,leaf_idx);

    for(int i=0;i<(int)leaf.objects.size();++i)
    {
        if(leaf.objects[i]!=obj_idx)
            continue;

        leaf.objects.erase(leaf.objects.begin()+i);
        break;
    }

    //ToDo: remove leaves
}

void quadtree::remove_object(int idx)
{
    objects_map::iterator it=m_objects.find(idx);
    if(it==m_objects.end())
        return;

    m_objects.erase(it);
    ::nya_math::remove_object(idx,0,m_leaves,-1);
}

const aabb &quadtree::get_object_aabb(int idx) const
{
    objects_map::const_iterator it=m_objects.find(idx);
    if(it==m_objects.end())
    {
        const static aabb invalid;
        return invalid;
    }

    return it->second;
}

bool sort_objects(bool previous_result,std::vector<int> &result)
{
    if(!previous_result)
        return false;

    std::sort(result.begin(),result.end());
    std::vector<int>::iterator last=std::unique(result.begin(),result.end());
    result.erase(last,result.end());
    return true;
}

template<typename t> bool get_objects(quad search,quad leaf,int leaf_idx,const std::vector<t> &leaves,std::vector<int> &result)
{
    if(leaf_idx<0)
        return false;

    const t &l=leaves[leaf_idx];
    if(!l.objects.empty())
    {
        result.insert(result.end(),l.objects.begin(),l.objects.end());
        return !result.empty();
    }

    quad child;
    child.size_x=leaf.size_x/2;
    child.size_z=leaf.size_z/2;

    int center_x=leaf.x+child.size_x;
    int center_z=leaf.z+child.size_z;

    if(search.x<=center_x)
    {
        child.x=leaf.x;

        if(search.z<=center_z)
        {
            child.z=leaf.z;
            get_objects(search,child,leaves[leaf_idx].leaves[0][0],leaves,result);
        }

        if(search.z+search.size_z>center_z)
        {
            child.z=center_z;
            get_objects(search,child,leaves[leaf_idx].leaves[0][1],leaves,result);
        }
    }

    if(search.x+search.size_x>center_x)
    {
        child.x=center_x;

        if(search.z<=center_z)
        {
            child.z=leaf.z;
            get_objects(search,child,leaves[leaf_idx].leaves[1][0],leaves,result);
        }

        if(search.z+search.size_z>center_z)
        {
            child.z=center_z;
            get_objects(search,child,leaves[leaf_idx].leaves[1][1],leaves,result);
        }
    }

    return !result.empty();
}

bool quadtree::get_objects(int x,int z, std::vector<int> &result) const
{
    if(m_leaves.empty())
        return false;

    result.clear();
    quad search;search.x=x;search.z=z;search.size_x=search.size_z=0;
    quad leaf;leaf.x=m_x,leaf.z=m_z,leaf.size_x=m_size_x,leaf.size_z=m_size_z;
    return sort_objects(::nya_math::get_objects(search,leaf,0,m_leaves,result),result);
}

bool quadtree::get_objects(int x,int z,int size_x,int size_z, std::vector<int> &result) const
{
    if(m_leaves.empty())
        return false;

    result.clear();
    quad search;search.x=x;search.z=z;search.size_x=size_x,search.size_z=size_z;
    quad leaf;leaf.x=m_x,leaf.z=m_z,leaf.size_x=m_size_x,leaf.size_z=m_size_z;
    return sort_objects(::nya_math::get_objects(search,leaf,0,m_leaves,result),result);
}

/*
bool quadtree::get_objects(const vec3 &v, std::vector<int> &result) const
{
    //ToDo
    return false;
}

bool quadtree::get_objects(const aabb &box, std::vector<int> &result) const
{
    //ToDo
    return false;
}

bool quadtree::get_objects(const frustum &f, std::vector<int> &result) const
{
    //ToDo
    return false;
}
*/

quadtree::quadtree(int x,int z,int size_x,int size_z,int max_level)
{
    m_x=x,m_z=z,m_size_x=size_x,m_size_z=size_z;
    m_max_level=max_level;
    m_leaves.resize(1);
}

}
