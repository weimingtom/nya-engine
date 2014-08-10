@uniform edge_offset "edge offset"=0.02,0.02,0.02,0.0
@uniform edge_color "edge color"=0.0,0.0,0.0,1.0

@predefined bones_pos "nya bones pos"
@predefined bones_rot "nya bones rot"

@vertex
uniform vec3 bones_pos[256];
uniform vec4 bones_rot[256];
uniform vec4 edge_offset;

vec3 tr(vec3 pos,int idx)
{
    vec4 q=bones_rot[idx];
    return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0;
}

void main()
{
    vec3 offset=gl_Normal.xyz*edge_offset.xyz;

    vec3 pos=tr(gl_Vertex.xyz+offset,int(gl_MultiTexCoord1.x))*gl_MultiTexCoord2.x;
    if(gl_MultiTexCoord2.y>0.0) pos+=tr(gl_MultiTexCoord3.xyz+offset,int(gl_MultiTexCoord1.y))*gl_MultiTexCoord2.y;
    if(gl_MultiTexCoord2.z>0.0) pos+=tr(gl_MultiTexCoord4.xyz+offset,int(gl_MultiTexCoord1.z))*gl_MultiTexCoord2.z;
    if(gl_MultiTexCoord2.w>0.0) pos+=tr(gl_MultiTexCoord5.xyz+offset,int(gl_MultiTexCoord1.w))*gl_MultiTexCoord2.w;

    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w);
}

@fragment
uniform vec4 edge_color;
void main() { gl_FragColor=edge_color; }
