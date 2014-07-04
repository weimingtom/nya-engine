//https://code.google.com/p/nya-engine/

#include "string_convert.h"
#include <algorithm>
#include <sstream>

namespace nya_formats
{

inline std::string fix_string(const std::string &s,const std::string whitespaces = " \t\r\n")
{
    std::string ss=s;
    std::transform(ss.begin(),ss.end(),ss.begin(),::tolower);
    const size_t start_idx=ss.find_first_not_of(whitespaces);
    if (start_idx==std::string::npos)
        return std::string("");

    const size_t end_idx=ss.find_last_not_of(whitespaces)+1;
    return ss.substr(start_idx,end_idx-start_idx);
}

bool bool_from_string(const std::string &s)
{
    const std::string ss=fix_string(s);
    return ss=="yes" || ss=="1" || ss=="true";
}

nya_math::vec4 vec4_from_string(const std::string &s)
{
    nya_math::vec4 v;
    std::string ss=s;
    std::replace(ss.begin(),ss.end(),',',' ');
    std::istringstream iss(ss);
    if(iss>>v.x)
        if(iss>>v.y)
            if(iss>>v.z)
                iss>>v.w;
    return v;
}

nya_render::blend::mode blend_mode_from_string(const std::string &s)
{
    const std::string ss=fix_string(s);
    if(ss=="src_alpha") return nya_render::blend::src_alpha;
    if(ss=="inv_src_alpha") return nya_render::blend::inv_src_alpha;
    if(ss=="src_color") return nya_render::blend::src_color;
    if(ss=="inv_src_color") return nya_render::blend::inv_src_color;
    if(ss=="dst_color") return nya_render::blend::dst_color;
    if(ss=="inv_dst_color") return nya_render::blend::inv_dst_color;
    if(ss=="dst_alpha") return nya_render::blend::dst_alpha;
    if(ss=="inv_dst_alpha") return nya_render::blend::inv_dst_alpha;
    if(ss=="zero") return nya_render::blend::zero;
    if(ss=="one") return nya_render::blend::one;

    return nya_render::blend::one;
}

bool blend_mode_from_string(const std::string &s,nya_render::blend::mode &src_out,nya_render::blend::mode &dst_out)
{
    const size_t div_idx=s.find(':');
    if(div_idx==std::string::npos)
    {
        src_out=nya_render::blend::one;
        dst_out=nya_render::blend::zero;
        return false;
    }

    std::string src_str=s;
    src_str.resize(div_idx);
    src_out=blend_mode_from_string(src_str);
    dst_out=blend_mode_from_string(s.substr(div_idx+1));
    return true;
}

bool cull_face_from_string(const std::string &s,nya_render::cull_face::order &order_out)
{
    const std::string ss=fix_string(s);
    if(ss=="cw")
    {
        order_out=nya_render::cull_face::cw;
        return true;
    }

    if(ss=="ccw")
    {
        order_out=nya_render::cull_face::ccw;
        return true;
    }

    order_out=nya_render::cull_face::ccw;
    return false;
}

}
