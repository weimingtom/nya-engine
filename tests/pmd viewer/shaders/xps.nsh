@all

varying vec2 tc;
varying vec3 normal;

@uniform alpha_test "alpha test"

@vertex

void main()
{
    tc=gl_MultiTexCoord0.xy;
    normal=gl_Normal;
    gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;
}

@sampler base "diffuse"

@fragment

uniform sampler2D base;
uniform vec4 alpha_test;

void main()
{
    vec4 c=texture2D(base,tc);
    if(c.a*alpha_test.x+alpha_test.y>0.0)
        discard;

    gl_FragColor=c;
}
