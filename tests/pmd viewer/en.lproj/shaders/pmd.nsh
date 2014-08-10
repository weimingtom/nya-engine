@sampler base "diffuse"
@sampler toon "toon"
@sampler env_add "env add"
@sampler env_mult "env mult"

@predefined bones_pos "nya bones pos"
@predefined bones_rot "nya bones rot"

@uniform light_dir "light dir"

@uniform amb_k "amb k"
@uniform diff_k "diff k"
@uniform spec_k "spec k"

@all

varying vec2 tc;
varying vec2 env_tc;
varying vec3 normal;

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

@fragment

uniform sampler2D base;
uniform sampler2D toon;
uniform sampler2D env_add;
uniform sampler2D env_mult;

uniform vec4 light_dir;

uniform vec4 amb_k;
uniform vec4 diff_k;
uniform vec4 spec_k;

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
