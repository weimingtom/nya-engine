@predefined bones_pos "nya bones pos"
@predefined bones_rot "nya bones rot"

@vertex
uniform vec3 bones_pos[256];
uniform vec4 bones_rot[256];

vec3 tr(vec3 pos,int idx)
{
    vec4 q=bones_rot[idx];
    return bones_pos[idx]+pos+cross(q.xyz,cross(q.xyz,pos)+pos*q.w)*2.0;
}

void main()
{
    int bone0=int(gl_MultiTexCoord1.x);
    int bone1=int(gl_MultiTexCoord1.y);
    vec3 pos=mix(tr(gl_MultiTexCoord2.xyz,bone1),tr(gl_Vertex.xyz,bone0),gl_MultiTexCoord1.z);
    pos.xyz+=gl_Normal*0.01;
    gl_Position=gl_ModelViewProjectionMatrix*vec4(pos,1.0);
}

@fragment
void main() { gl_FragColor=vec4(0.0,0.0,0.0,1.0); }
