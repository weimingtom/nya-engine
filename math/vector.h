//https://code.google.com/p/nya-engine/

#pragma once

#include <math.h>

namespace nya_math
{

struct vec2
{
    float x;
    float y;

    vec2(): x(0),y(0) {}
    vec2(float x,float y) { this->x=x; this->y=y; }
    vec2(const float *v) { x=v[0]; y=v[1]; }

    vec2 operator + (const vec2 &v) const { return vec2(x+v.x,y+v.y); }
    vec2 operator - (const vec2 &v) const { return vec2(x-v.x,y-v.y); }
    float operator * (const vec2 &v) const { return x*v.x+y*v.y; }

    vec2 operator - () const { return vec2(-x,-y); }

    vec2 operator *= (const float a) { x*=a; y*=a; return *this; }
    vec2 operator /= (const float a) { x/=a; y/=a; return *this; }

    vec2 operator += (const vec2 &v) { x+=v.x; y+=v.y; return *this; }
    vec2 operator -= (const vec2 &v) { x-=v.x; y-=v.y; return *this; }

    float length() const { return sqrtf(x*x+y*y); }

    vec2 &abs() { x=fabsf(x); y=fabsf(y); return *this; }

    vec2 &normalize()
    {
        float len=length();
        if(len<0.00001f)
            return *this;

        *this *=(1.0f/len);
        return *this;
    }
};

inline vec2 operator * ( float a, const vec2& v ) { return vec2(v.x*a,v.y*a); }
inline vec2 operator * ( const vec2& v, float a ) { return vec2(v.x*a,v.y*a); }
inline vec2 operator / ( const vec2& v, float a ) { return vec2(v.x/a,v.y/a); }

struct vec3
{
    float x;
    float y;
    float z;

    vec3(): x(0),y(0),z(0) {}
    vec3(float x,float y,float z) { this->x=x; this->y=y; this->z=z; }
    vec3(const float *v) { x=v[0]; y=v[1]; z=v[2]; }

    vec3 operator + (const vec3 &v) const { return vec3(x+v.x,y+v.y,z+v.z); }
    vec3 operator - (const vec3 &v) const { return vec3(x-v.x,y-v.y,z-v.z); }
    float operator * (const vec3 &v) const { return x*v.x+y*v.y+z*v.z; }

    vec3 operator - () const { return vec3(-x,-y,-z); }

    vec3 operator *= (const float a) { x*=a; y*=a; z*=a; return *this; }
    vec3 operator /= (const float a) { x/=a; y/=a; z/=a; return *this; }

    vec3 operator += (const vec3 &v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    vec3 operator -= (const vec3 &v) { x-=v.x; y-=v.y; z-=v.z; return *this; }

    float length() const { return sqrtf(x*x+y*y+z*z); }

    vec3 &abs() { x=fabsf(x); y=fabsf(y); z=fabsf(z); return *this; }

    vec3 &normalize()
    {
        float len=length();
        if(len<0.00001f)
            return *this;

        *this *=(1.0f/len);
        return *this;
    }

    static vec3 cross(vec3 a,vec3 b)
    {
        return vec3(a.y*b.z - a.z*b.y,
                    a.z*b.x - a.x*b.z,
                    a.x*b.y - a.y*b.x);
    }
};

inline vec3 operator * ( float a, const vec3& v ) { return vec3(v.x*a,v.y*a,v.z*a); }
inline vec3 operator * ( const vec3& v, float a ) { return vec3(v.x*a,v.y*a,v.z*a); }
inline vec3 operator / ( const vec3& v, float a ) { return vec3(v.x/a,v.y/a,v.z/a); }

struct vec4
{
    float x;
    float y;
    float z;
    float w;

    vec4(): x(0),y(0),z(0),w(0) {}
    vec4(float x,float y,float z,float w) { this->x=x; this->y=y;
                                            this->z=z; this->w=w; }

    vec4(const float *v) { x=v[0]; y=v[1]; z=v[2]; w=v[3]; }

    vec4 operator + (const vec4 &v) const { return vec4(x+v.x,y+v.y,z+v.z,w+v.w); }

    vec4 operator - (const vec4 &v) const { return vec4(x-v.x,y-v.y,z-v.z,w-v.w); }
    float operator * (const vec4 &v) const { return x*v.x+y*v.y+z*v.z+w*v.w; }

    vec4 operator - () const { return vec4(-x,-y,-z,-w); }

    vec4 operator *= (const float a) { x*=a; y*=a; z*=a; w*=a; return *this; }
    vec4 operator /= (const float a) { x/=a; y/=a; z/=a; w/=a; return *this; }

    vec4 operator += (const vec4 &v) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }
    vec4 operator -= (const vec4 &v) { x-=v.x; y-=v.y; z-=v.z; w+=v.w; return *this; }

    float length() const { return sqrtf(x*x+y*y+z*z+w*w); }

    vec4 &abs() { x=fabsf(x); y=fabsf(y); z=fabsf(z); w=fabsf(w); return *this; }

    vec4 &normalize()
    {
        float len=length();
        if(len<0.00001f)
            return *this;

        *this *=(1.0f/len);
        return *this;
    }

    vec3 xyz() const { return vec3(x,y,z); }
};

inline vec4 operator * ( float a, const vec4& v ) { return vec4(v.x*a,v.y*a,v.z*a,v.w*a); }
inline vec4 operator * ( const vec4& v, float a ) { return vec4(v.x*a,v.y*a,v.z*a,v.w*a); }
inline vec4 operator / ( const vec4& v, float a ) { return vec4(v.x/a,v.y/a,v.z/a,v.w/a); }

}

