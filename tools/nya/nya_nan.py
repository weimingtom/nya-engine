#https://code.google.com/p/nya-engine/

from bin_data import *
from nya_math import *

class nan_animation:
    class pos_frame:
        def __init__( self ):
            self.time = 0
            self.pos = nya_vec3()

    class rot_frame:
        def __init__( self ):
            self.time = 0
            self.rot = nya_quat()

    class bone_frames:
        def __init__( self ):
            self.name = ""
            self.type = 0 #10 pos_vec3_linear, 20 rot_quat_linear
            self.frames = []

    def __init__( self ):
        self.bones = []

    def write(self,file_name):
        fo = open(file_name,"wb")
        if fo == 0:
            print "unable to open file ", file_name
            return

        out = bin_data()
        out.add_data("nya anim")
        out.add_uint(1) #version

        out.add_uint(len(self.bones))

        for b in self.bones:
            out.add_string(b.name)
            out.add_uchar(b.type)
            out.add_uint(len(b.frames))

            if b.type == 10:
                for f in b.frames:
                    out.add_uint(f.time)
                    out.add_float(f.pos.x)
                    out.add_float(f.pos.y)
                    out.add_float(f.pos.z)
            elif b.type == 20:
                for f in b.frames:
                    out.add_uint(f.time)
                    out.add_float(f.rot.v.x)
                    out.add_float(f.rot.v.y)
                    out.add_float(f.rot.v.z)
                    out.add_float(f.rot.w)

        fo.write(out.data)

        print "saved animation: ", file_name
        print "file size: ", len(out.data)/1024, "kb"
