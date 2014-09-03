@include "fragment.nsh"
@include "skeleton.nsh"

@vertex

void main()
{
    tc=gl_MultiTexCoord0.xy;

    vec3 pos=tr(gl_Vertex.xyz,int(gl_MultiTexCoord1.x))*gl_MultiTexCoord2.x;
    normal=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.x))*gl_MultiTexCoord2.x;

    if(gl_MultiTexCoord2.y>0.0)
    {
        pos+=tr(gl_MultiTexCoord3.xyz,int(gl_MultiTexCoord1.y))*gl_MultiTexCoord2.y;
        normal+=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.y))*gl_MultiTexCoord2.y;
    }

    if(gl_MultiTexCoord2.z>0.0)
    {
        pos+=tr(gl_MultiTexCoord4.xyz,int(gl_MultiTexCoord1.z))*gl_MultiTexCoord2.z;
        normal+=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.z))*gl_MultiTexCoord2.z;
    }

    if(gl_MultiTexCoord2.w>0.0)
    {
        pos+=tr(gl_MultiTexCoord5.xyz,int(gl_MultiTexCoord1.w))*gl_MultiTexCoord2.w;
        normal+=trn(gl_Normal.xyz,int(gl_MultiTexCoord1.w))*gl_MultiTexCoord2.w;
    }

    vec3 r=normalize((gl_ModelViewProjectionMatrix * vec4(normal,0.0)).xyz);
    r.z-=1.0;
    env_tc=0.5*r.xy/length(r) + 0.5;

    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w);
}
