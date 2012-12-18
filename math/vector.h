//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_math
{

struct vec3
{
	float x;
	float y;
	float z;

	vec3(): x(0),y(0),z(0) {}
    vec3(float x,float y,float z) { this->x=x; this->y=y; this->z=z; }
    vec3(const float *v) { x=v[0]; y=v[1]; z=v[2]; }

	vec3 operator + (const vec3 &v) const { return vec3(x+v.x,y+v.y,z+v.z);	}
	vec3 operator - (const vec3 &v) const { return vec3(x-v.x,y-v.y,z-v.z);	}

	vec3 operator - () const { return vec3(-x,-y,-z); }
};

}

