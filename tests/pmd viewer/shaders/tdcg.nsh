@sampler base "diffuse"

@all

varying vec2 tc;
varying vec3 normal;

@vertex

void main()
{
    tc=gl_MultiTexCoord0.xy;
    normal=gl_Normal;
    gl_Position=gl_ModelViewProjectionMatrix*gl_Vertex;
}

@fragment

uniform sampler2D base;

void main()
{
    gl_FragColor=texture2D(base,tc);
}
