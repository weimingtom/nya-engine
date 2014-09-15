@include "fragment.nsh"
@include "skeleton.nsh"

@vertex

void main()
{
    tc=gl_MultiTexCoord0.xy;

    int bone0=int(gl_MultiTexCoord1.x);
    int bone1=int(gl_MultiTexCoord1.y);
    vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);
    normal=mix(trn(gl_Normal,bone1),trn(gl_Normal,bone0),gl_MultiTexCoord1.z);

    vec3 r=normalize((gl_ModelViewProjectionMatrix * vec4(normal,0.0)).xyz);
    r.z-=1.0;
    env_tc=0.5*r.xy/length(r) + 0.5;

    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,gl_Vertex.w);
}
