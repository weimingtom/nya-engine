//https://code.google.com/p/nya-engine/

#pragma once

#ifdef min
    #undef min
#endif

#ifdef max
    #undef max
#endif

namespace nya_math
{

inline float max(float a,float b) { return a>b?a:b; }
inline float min(float a,float b) { return a<b?a:b; }
inline float clamp(float value,float from,float to) { return value<from?from:value>to?to:value; }
inline float lerp(float from,float to,float t) { return from*(1.0f-t)+to*t; }

}
