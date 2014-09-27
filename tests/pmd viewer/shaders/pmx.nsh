@include "fragment.nsh"
@include "skeleton.nsh"

@vertex

void main()
{
    tc=gl_MultiTexCoord0.xy;

    vec3 pos=tr(gl_Vertex.xyz,int(gl_MultiTexCoord1[0]))*gl_MultiTexCoord2[0];
    normal=trn(gl_Normal,int(gl_MultiTexCoord1[0]))*gl_MultiTexCoord2[0];

    for(int i=1;i<4;++i)
    {
        if(gl_MultiTexCoord2[i]>0.0)
        {
            pos+=tr(gl_Vertex.xyz,int(gl_MultiTexCoord1[i]))*gl_MultiTexCoord2[i];
            normal+=trn(gl_Normal,int(gl_MultiTexCoord1[i]))*gl_MultiTexCoord2[i];
        }
    }

    vec3 r=normalize((gl_ModelViewProjectionMatrix * vec4(normal,0.0)).xyz);
    r.z-=1.0;
    env_tc=0.5*r.xy/length(r) + 0.5;

    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w);
}
