@predefined bones_pos "nya bones pos"
@predefined bones_rot "nya bones rot"

@vertex

uniform vec3 bones_pos[256];
uniform vec4 bones_rot[256];

vec3 tr(vec3 pos,int idx)
{
    vec4 q=bones_rot[idx];
    return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0;
}

vec3 trn(vec3 normal,int idx)
{
    vec4 q=bones_rot[idx];
    return normal+cross(q.xyz,cross(q.xyz,normal)+normal*q.w)*2.0;
}
