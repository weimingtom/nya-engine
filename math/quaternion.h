//https://code.google.com/p/nya-engine/

#pragma once

namespace nya_math
{

struct quat
{
	float x;
	float y;
	float z;
	float w;

	quat(): x(0),y(0),z(0),w(0) {}

	quat(float x,float y,float z,float w)
	{
	    this->x=x; this->y=y;
	    this->z=z; this->w=w;
    }

	quat(const float *q) { x=q[0]; y=q[1]; z=q[2]; w=q[3]; }

	quat operator - () const { return quat(-x,-y,-z,-w); }

public:
    static quat slerp(const quat &from,const quat &to,float t);
};

}

