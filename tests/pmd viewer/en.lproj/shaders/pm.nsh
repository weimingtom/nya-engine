@sampler base "diffuse"
@sampler toon "toon"
@sampler env_add "env add"
@sampler env_mult "env mult"

@uniform light_dir "light dir"

@all

varying vec2 tc;
varying vec2 env_tc;
varying vec3 normal;

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

@fragment

uniform sampler2D base;
uniform sampler2D toon;
uniform sampler2D env_add;
uniform sampler2D env_mult;

uniform vec4 light_dir;

void main()
{
    vec4 c=texture2D(base,tc);
    if(c.a<0.01)
        discard;

    float s=0.0;
    float l=0.5+0.5*dot(light_dir.xyz,normal);
    l=clamp(l,0.05,0.95); //ToDo: clamp to edge instead
    vec3 t=texture2D(toon,vec2(s,l)).rgb;

    c.rgb*=texture2D(env_mult,env_tc).rgb;
    c.rgb+=texture2D(env_add,env_tc).rgb;

    gl_FragColor=vec4(c.rgb*t,c.a);
}
