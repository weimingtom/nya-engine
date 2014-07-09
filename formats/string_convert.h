//https://code.google.com/p/nya-engine/

#pragma once

#include "render/render.h"
#include "math/vector.h"
#include <string>

namespace nya_formats
{

bool bool_from_string(const std::string &s);
std::string string_from_bool(bool value);
nya_math::vec4 vec4_from_string(const std::string &s);
std::string string_from_vec4(const nya_math::vec4 &value,int precision = -1);
nya_render::blend::mode blend_mode_from_string(const std::string &s);
std::string string_from_blend_mode(nya_render::blend::mode value);
bool blend_mode_from_string(const std::string &s,nya_render::blend::mode &src_out,nya_render::blend::mode &dst_out);
std::string string_from_blend_mode(bool blend,nya_render::blend::mode src,nya_render::blend::mode dst);
bool cull_face_from_string(const std::string &s,nya_render::cull_face::order &order_out);
std::string string_from_cull_face(bool cull,nya_render::cull_face::order order);

}
