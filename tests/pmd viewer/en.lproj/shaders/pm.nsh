@include "fragment.nsh"

@vertex

void main()
{
    tc=gl_MultiTexCoord0.xy;

    vec3 pos=gl_Vertex.xyz;
    normal=gl_Normal;

    vec3 r=normalize((gl_ModelViewProjectionMatrix * vec4(normal,0.0)).xyz);
    r.z-=1.0;
    env_tc=0.5*r.xy/length(r) + 0.5;

    gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;
}
