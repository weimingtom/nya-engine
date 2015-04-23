//https://code.google.com/p/nya-engine/

#pragma once

#include "frustum.h"
#include <map>
#include <vector>

namespace nya_math
{

class quadtree
{
public:
    void add_object(const aabb &box,int idx);
    void remove_object(int idx);

public:
    bool get_objects(int x,int z, std::vector<int> &result) const;
    bool get_objects(int x,int z,int size_x,int size_z, std::vector<int> &result) const;
/*
    bool get_objects(const vec3 &v, std::vector<int> &result) const;
    bool get_objects(const aabb &box, std::vector<int> &result) const;
    bool get_objects(const frustum &f, std::vector<int> &result) const;
*/
public:
    const aabb &get_object_aabb(int idx) const;

public:
    quadtree() {}
    quadtree(int x,int z,int size_x,int size_z,int max_level);

private:
    struct leaf
    {
        int leaves[2][2];
        std::vector<int> objects;
        float min_y;
        float max_y;

        leaf() { leaves[0][0]=leaves[0][1]=leaves[1][0]=leaves[1][1]=-1; min_y=INFINITY; max_y=-INFINITY; }
    };

    int m_x,m_z,m_size_x,m_size_z;
    int m_max_level;
    std::vector<leaf> m_leaves;
    typedef std::map<int,aabb> objects_map;
    objects_map m_objects;
};

}
