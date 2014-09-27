@include "skeleton.nsh"

@vertex

void main()
{
    int bone0=int(gl_MultiTexCoord1.x);
    int bone1=int(gl_MultiTexCoord1.y);
    vec3 v=gl_Vertex.xyz+gl_Normal*0.01;
    vec3 pos=mix(tr(v,bone1),tr(v,bone0),gl_MultiTexCoord1.z);
    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0);
}

@fragment
void main() { gl_FragColor=vec4(0.0,0.0,0.0,1.0); }
