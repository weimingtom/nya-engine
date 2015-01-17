//https://code.google.com/p/nya-engine/

#pragma once

#include <math.h>

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif

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
    vec2 operator * (const float a) const { return vec2(x*a,y*a); }
    vec2 operator / (const float a) const { return vec2(x/a,y/a); }

    vec2 operator - () const { return vec2(-x,-y); }

    vec2 operator *= (const float a) { x*=a; y*=a; return *this; }
    vec2 operator /= (const float a) { x/=a; y/=a; return *this; }

    vec2 operator += (const vec2 &v) { x+=v.x; y+=v.y; return *this; }
    vec2 operator -= (const vec2 &v) { x-=v.x; y-=v.y; return *this; }

    float length() const { return sqrtf(x*x+y*y); }

    vec2 &abs() { x=fabsf(x); y=fabsf(y); return *this; }

    vec2 &normalize() { return *this=normalize(*this); }

    static vec2 max(const vec2 &a,const vec2 &b) { return vec2(a.x>b.x?a.x:b.x, a.y>b.y?a.y:b.y); }
    static vec2 min(const vec2 &a,const vec2 &b) { return vec2(a.x<b.x?a.x:b.x, a.y<b.y?a.y:b.y); }

    static vec2 normalize(const vec2 &v) { float len=v.length(); return len<0.00001f? vec2(1.0f,0.0f): v*(1.0f/len); }

    static vec2 abs(const vec2 &v) { return vec2(fabsf(v.x),fabsf(v.y)); }

    static vec2 lerp(const vec2 &from,const vec2 &to,float t) { return from*(1.0-t)+to*t; }
};

inline vec2 operator * ( float a, const vec2& v ) { return vec2(v.x*a,v.y*a); }

struct vec3
{
    float x;
    float y;
    float z;

    vec3(): x(0),y(0),z(0) {}
    vec3(float x,float y,float z) { this->x=x; this->y=y; this->z=z; }
    explicit vec3(const float *v) { x=v[0]; y=v[1]; z=v[2]; }

    vec3 operator + (const vec3 &v) const { return vec3(x+v.x,y+v.y,z+v.z); }
    vec3 operator - (const vec3 &v) const { return vec3(x-v.x,y-v.y,z-v.z); }
    float operator * (const vec3 &v) const { return x*v.x+y*v.y+z*v.z; }
    vec3 operator * (const float a) const { return vec3(x*a,y*a,z*a); }
    vec3 operator / (const float a) const { return vec3(x/a,y/a,z/a); }

    vec3 operator - () const { return vec3(-x,-y,-z); }

    vec3 operator *= (const float a) { x*=a; y*=a; z*=a; return *this; }
    vec3 operator /= (const float a) { x/=a; y/=a; z/=a; return *this; }

    vec3 operator += (const vec3 &v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    vec3 operator -= (const vec3 &v) { x-=v.x; y-=v.y; z-=v.z; return *this; }

    float length() const { return sqrtf(x*x+y*y+z*z); }

    vec3 &abs() { x=fabsf(x); y=fabsf(y); z=fabsf(z); return *this; }

    vec3 &normalize() { return *this=normalize(*this); }

    const vec2 &xy() const { return *(vec2*)&x; }
    vec2 &xy() { return *(vec2*)&x; }

    static vec3 cross(const vec3 &a,const vec3 &b) { return vec3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x); }

    static vec3 max(const vec3 &a,const vec3 &b) { return vec3(a.x>b.x?a.x:b.x, a.y>b.y?a.y:b.y, a.z>b.z?a.z:b.z); }
    static vec3 min(const vec3 &a,const vec3 &b) { return vec3(a.x<b.x?a.x:b.x, a.y<b.y?a.y:b.y, a.z<b.z?a.z:b.z); }

    static vec3 normalize(const vec3 &v) { float len=v.length(); return len<0.00001f? vec3(1.0f,0.0f,0.0f): v*(1.0f/len); }

    static vec3 abs(const vec3 &v) { return vec3(fabsf(v.x),fabsf(v.y),fabsf(v.z)); }

    static vec3 lerp(const vec3 &from,const vec3 &to,float t) { return from*(1.0-t)+to*t; }
};

inline vec3 operator * ( float a, const vec3& v ) { return vec3(v.x*a,v.y*a,v.z*a); }

struct vec4
{
    float x;
    float y;
    float z;
    float w;

    vec4(): x(0),y(0),z(0),w(0) {}
    vec4(float x,float y,float z,float w) { this->x=x; this->y=y;
                                            this->z=z; this->w=w; }

    explicit vec4(const float *v) { x=v[0]; y=v[1]; z=v[2]; w=v[3]; }

    vec4 operator + (const vec4 &v) const { return vec4(x+v.x,y+v.y,z+v.z,w+v.w); }

    vec4 operator - (const vec4 &v) const { return vec4(x-v.x,y-v.y,z-v.z,w-v.w); }
    float operator * (const vec4 &v) const { return x*v.x+y*v.y+z*v.z+w*v.w; }
    vec4 operator * (const float a) const { return vec4(x*a,y*a,z*a,w*a); }
    vec4 operator / (const float a) const { return vec4(x/a,y/a,z/a,w/a); }

    vec4 operator - () const { return vec4(-x,-y,-z,-w); }

    vec4 operator *= (const float a) { x*=a; y*=a; z*=a; w*=a; return *this; }
    vec4 operator /= (const float a) { x/=a; y/=a; z/=a; w/=a; return *this; }

    vec4 operator += (const vec4 &v) { x+=v.x; y+=v.y; z+=v.z; w+=v.w; return *this; }
    vec4 operator -= (const vec4 &v) { x-=v.x; y-=v.y; z-=v.z; w+=v.w; return *this; }

    float length() const { return sqrtf(x*x+y*y+z*z+w*w); }

    vec4 &abs() { x=fabsf(x); y=fabsf(y); z=fabsf(z); w=fabsf(w); return *this; }

    vec4 &normalize() { return *this=normalize(*this); }

    const vec3 &xyz() const { return *(vec3*)&x; }
    vec3 &xyz() { return *(vec3*)&x; }
    const vec2 &xy() const { return *(vec2*)&x; }
    vec2 &xy() { return *(vec2*)&x; }
    const vec2 &zw() const { return *(vec2*)&z; }
    vec2 &zw() { return *(vec2*)&z; }

    static vec4 max(const vec4 &a,const vec4 &b) { return vec4(a.x>b.x?a.x:b.x, a.y>b.y?a.y:b.y, a.z>b.z?a.z:b.z, a.w>b.w?a.w:b.w); }
    static vec4 min(const vec4 &a,const vec4 &b) { return vec4(a.x<b.x?a.x:b.x, a.y<b.y?a.y:b.y, a.z<b.z?a.z:b.z, a.w<b.w?a.w:b.w); }

    static vec4 normalize(const vec4 &v) { float len=v.length(); return len<0.00001f? vec4(1.0f,0.0f,0.0f,0.0): v*(1.0f/len); }

    static vec4 abs(const vec4 &v) { return vec4(fabsf(v.x),fabsf(v.y),fabsf(v.z),fabsf(v.w)); }

    static vec4 lerp(const vec4 &from,const vec4 &to,float t) { return from*(1.0-t)+to*t; }
};

inline vec4 operator * ( float a, const vec4& v ) { return vec4(v.x*a,v.y*a,v.z*a,v.w*a); }

}

