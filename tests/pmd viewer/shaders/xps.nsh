@sampler base "diffuse"
@sampler bump "bump"
@sampler lightmap "lightmap"
@sampler spec "spec"
@sampler env "env"

@uniform alpha_test "alpha test"
@predefined cam_pos "nya camera position":local
@uniform light_dir "light dir":local_rot=-0.58,0.58,0.57
@uniform light_k "light k"=0.1,0.0,0.0,0.0

@all

varying vec3 pos;
varying vec2 tc;
varying mat3 tbn;

@vertex

void main()
{
    pos=gl_Vertex.xyz;
    tc=gl_MultiTexCoord0.xy;

    vec3 n=gl_Normal;
    vec3 t=gl_MultiTexCoord5.xyz;
    vec3 bt=gl_MultiTexCoord6.xyz;
    tbn=mat3(t,bt,n);

    gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;
}

@fragment

uniform sampler2D base;
uniform sampler2D bump;
uniform sampler2D lightmap;
uniform sampler2D spec;
uniform sampler2D env;

uniform vec4 alpha_test;
uniform vec4 cam_pos;
uniform vec4 light_dir;
uniform vec4 light_k;

void main()
{
    vec4 c=texture2D(base,tc);
    if(c.a*alpha_test.x+alpha_test.y>0.0)
        discard;

    vec3 normal=normalize(tbn*(texture2D(bump,tc).xyz*2.0-1.0));

    c.rgb*=light_k.x+light_k.y*max(0.0,dot(normal,light_dir.xyz));

    vec3 v=normalize(pos-cam_pos.xyz);
    vec3 lrn=reflect(light_dir.xyz,normal);

    c.rgb+=pow(max(0.0,dot(v,lrn)),10.0)*light_k.z*texture2D(spec,tc).rgb;
    
    c.rgb*=texture2D(lightmap,tc).rgb;

    vec3 r=normalize(reflect(v,normal));
	vec2 rtc = 0.5*r.xy/length(r)+0.5;
    c.rgb+=texture2D(env,rtc).rgb*0.3;

    gl_FragColor=c;
}
