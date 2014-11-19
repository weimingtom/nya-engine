#https://code.google.com/p/nya-engine/

#maya script path on mac is ~/Library/Preferences/Autodesk/maya/scripts

import maya.cmds as cmds
from nya.nya_nan import *

start_dir = cmds.workspace( query=True, rootDirectory=True )

def export():
    mask = start_dir + '*.nan'
    file_name = cmds.fileDialog( mode=1, directoryMask=mask )
    if file_name == '':
        return

    anim = nan_animation()

    time = cmds.currentTime(q=1)
    cmds.currentTime(0)

    frames_count = 0
    pos_org = []

    pos_bones = []
    rot_bones = []

    joints = cmds.ls(type="joint")
    for j in joints:
        v = nya_vec3()
        p = cmds.xform(j,translation=1,q=1)
        v.x = float(p[0])
        v.y = float(p[1])
        v.z = float(p[2])
        pos_org.append(v)

        pf = nan_animation.bone_frames()
        pf.name = str(j)
        pf.type = 10
        pos_bones.append(pf);

        rf = nan_animation.bone_frames()
        rf.name = str(j)
        rf.type = 20
        rot_bones.append(rf);

        curves=cmds.listConnections(j,type="animCurve")
        if curves:
            for c in curves:
                t = cmds.findKeyframe(c,w="last")
                if t > frames_count:
                    frames_count = t

    if frames_count <= 0:
        print "error: no animation frames"
        return

    for t in range(frames_count):
        cmds.currentTime(t)

        for i in range(len(joints)):
            p = cmds.xform(joints[i],translation=1,q=1)
            pf = nan_animation.pos_frame()
            pf.time=t

            pf.pos.x = p[0] - pos_org[i].x
            pf.pos.y = p[1] - pos_org[i].y
            pf.pos.z = p[2] - pos_org[i].z

            #ToDo: sort similar

            pos_bones[i].frames.append(pf)

            r = cmds.xform(joints[i],rotation=1,q=1)
            rf = nan_animation.rot_frame()
            rf.time=t
            rf.rot.from_pyr(r[0],r[1],r[2])

            #ToDo: sort similar

            rot_bones[i].frames.append(rf)

    for p in pos_bones:
        if len(p.frames)<0:
            continue
        if len(p.frames)==1 and p.frames[0].pos==nya_vec3():
            continue
        anim.bones.append(p)

    for r in rot_bones:
        if len(r.frames)<0:
            continue
        if len(r.frames)==1 and r.frames[0].rot==nya_quat():
            continue
        anim.bones.append(r)

    cmds.currentTime(time)
    anim.write(file_name)

export()
