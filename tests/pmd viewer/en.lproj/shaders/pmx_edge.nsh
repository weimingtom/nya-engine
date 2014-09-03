@uniform edge_offset "edge offset"=0.02,0.02,0.02,0.0
@uniform edge_color "edge color"=0.0,0.0,0.0,1.0

@include "skeleton.nsh"

@vertex
uniform vec4 edge_offset;

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
