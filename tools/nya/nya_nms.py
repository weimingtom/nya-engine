#https://code.google.com/p/nya-engine/

from bin_data import *

class nms_mesh:
    class nms_vertex_attribute:
        def __init__( self ):
            self.type = 0 #pos=0 normal=1 color=2 tc=100
            self.dimension = 0
            self.semantics = "" #optional

    class nms_group:
        def __init__( self ):
            self.name = ""
            self.mat_idx = 0
            self.offset = 0
            self.count = 0

    class nms_param:
        def __init__( self ):
            self.name = ""
            self.value = ""

    class nms_vec_param:
        def __init__( self ):
            self.name = ""
            self.x = 0.0
            self.y = 0.0
            self.z = 0.0
            self.w = 0.0

    class nms_material:
        def __init__( self ):
            self.name = ""
            self.textures = []
            self.params = []
            self.vec_params = []

    class nms_joint:
        def __init__( self ):
            self.name = ""
            self.parent = -1
            self.pos_x = 0.0
            self.pos_y = 0.0
            self.pos_z = 0.0

    def __init__( self ):
        self.verts_data = [] #floats 
        self.vcount = 0
        self.vert_attr = []
        self.indices = [] 
        self.groups = []
        self.materials = []
        self.joints = []

    def write(self,file_name):
        f = open(file_name,"wb")
        if f == 0:
            print "unable to open file ", file_name
            return

        out = bin_data()
        out.add_data("nya mesh")
        out.add_uint(1) #version ToDo: version 2
        chunks_count = 1

        mat_count = len(self.materials)
        jcount = len(self.joints)

        if mat_count:
            chunks_count += 1;
        if jcount:
            chunks_count += 1;

        out.add_uint(chunks_count)

        #---------------- mesh data -----------------

        buf = bin_data()

        for i in range(6): #ToDo: aabb
            buf.add_float(0)

        atr_count = len(self.vert_attr)
        buf.add_uchar(atr_count)
        for a in self.vert_attr:
            buf.add_uchar(a.type)
            buf.add_uchar(a.dimension)
            buf.add_string(a.semantics)

        buf.add_uint(self.vcount)
        buf.add_floats(self.verts_data)

        icount = len(self.indices)
        if icount>65535:
            buf.add_uchar(4) #uint indices
            buf.add_uint(icount)
            buf.add_uints(self.indices)
        elif icount>0:
            buf.add_uchar(2) #ushort indices
            buf.add_uint(icount)
            buf.add_ushorts(self.indices)
        else:
            buf.add_uchar(0) #no indices

        groups_count = len(self.groups)
        buf.add_ushort(1) #lods count
        buf.add_ushort(groups_count)
        for g in self.groups:
            buf.add_string(g.name)
            for j in range(6):
                buf.add_float(0)
            buf.add_ushort(g.mat_idx)
            buf.add_uint(g.offset)
            buf.add_uint(g.count)

        out.add_uint(0) #mesh
        out.add_uint(len(buf.data))
        out.data += buf.data

        #-------------- materials data ---------------

        if mat_count > 0:
            buf = bin_data()
            buf.add_ushort(mat_count)
            for m in self.materials:
                buf.add_string(m.name)
                buf.add_ushort(len(m.textures))
                for tex in m.textures:
                    buf.add_string(tex.name)
                    buf.add_string(tex.value)

                params=m.params
                buf.add_ushort(len(params))
                for p in params:
                    buf.add_string(p.name)
                    buf.add_string(p.value)

                vec_params=m.vec_params
                buf.add_ushort(len(vec_params))
                for p in vec_params:
                    buf.add_string(p.name)
                    buf.add_float(p.x)
                    buf.add_float(p.y)
                    buf.add_float(p.z)
                    buf.add_float(p.w)

                buf.add_ushort(0) #ToDo: integer params

            out.add_uint(2) #materials
            out.add_uint(len(buf.data))
            out.data += buf.data

        #-------------- skeleton data ---------------

        if jcount > 0:
            buf = bin_data()
            buf.add_uint(jcount)
            for i in range(jcount):
                buf.add_string(self.joints[i].name)

                buf.add_float(0.0) #ToDo: rot
                buf.add_float(0.0)
                buf.add_float(0.0)
                buf.add_float(1.0)

                buf.add_float(self.joints[i].pos_x)
                buf.add_float(self.joints[i].pos_y)
                buf.add_float(self.joints[i].pos_z)

                buf.add_int(self.joints[i].parent)

            out.add_uint(1) #skeleton
            out.add_uint(len(buf.data))
            out.data += buf.data

        f.write(out.data)
