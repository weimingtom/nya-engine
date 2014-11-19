#https://code.google.com/p/nya-engine/

import math

class nya_vec3:
    def __init__( self ):
        self.x = 0.0
        self.y = 0.0
        self.z = 0.0

    def eq( a, b ):
        eps = 0.001
        if abs(a.x - b.x) > eps:
            return False
        if abs(a.y - b.y) > eps:
            return False
        if abs(a.z - b.z) > eps:
            return False
        return True

class nya_quat:
    def __init__( self ):
        self.v = nya_vec3()
        self.w = 1.0

    def from_pyr(self,pitch,yaw,roll):
        d2r = 3.14 / 180.0
        hp = pitch * 0.5 * d2r
        hy = yaw * 0.5 * d2r
        hr = roll * 0.5 * d2r

        sx = math.sin(hp)
        cx = math.cos(hp)

        sy = math.sin(hy)
        cy = math.cos(hy)

        sz = math.sin(hr)
        cz = math.cos(hr)

        self.v = nya_vec3()

        self.v.x = sx*cy*cz - cx*sy*sz
        self.v.y = cx*sy*cz + sx*cy*sz
        self.v.z = cx*cy*sz - sx*sy*cz
        self.w = cx*cy*cz + sx*sy*sz

    def eq( a, b ):
        eps = 0.001
        if abs(a.v.x - b.v.x) > eps:
            return False
        if abs(a.v.y - b.v.y) > eps:
            return False
        if abs(a.v.z - b.v.z) > eps:
            return False
        if abs(a.w - b.w) > eps:
            return False
        return True
