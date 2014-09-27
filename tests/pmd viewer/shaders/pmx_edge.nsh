@uniform edge_offset "edge offset"=0.02,0.02,0.02,0.0
@uniform edge_color "edge color"=0.0,0.0,0.0,1.0

@include "skeleton.nsh"

@vertex
uniform vec4 edge_offset;

void main()
{
    vec3 v=gl_Vertex.xyz+gl_Normal*edge_offset.xyz;

    vec3 pos=tr(v,int(gl_MultiTexCoord1[0]))*gl_MultiTexCoord2[0];
    for(int i=1;i<4;++i)
        if(gl_MultiTexCoord2[i]>0.0)
            pos+=tr(v,int(gl_MultiTexCoord1[i]))*gl_MultiTexCoord2[i];

    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w);
}

@fragment
uniform vec4 edge_color;
void main() { gl_FragColor=edge_color; }
