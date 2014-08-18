@sampler base "diffuse"
@sampler toon "toon"
@sampler env_add "env add"
@sampler env_mult "env mult"

@uniform light_dir "light dir"

@uniform amb_k "amb k"
@uniform diff_k "diff k"
@uniform spec_k "spec k"

@all

varying vec2 tc;
varying vec2 env_tc;
varying vec3 normal;

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
