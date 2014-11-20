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
    rot_org = []

    pos_bones = []
    rot_bones = []

    joints = cmds.ls(type="joint")
    for j in joints:
        p = cmds.xform(j,translation=1,q=1)
        pos_org.append(p)
        r = cmds.xform(j,rotation=1,q=1)
        rot_org.append(r)

        name = str(j).replace("FBXASC032", " ");

        pf = nan_animation.bone_frames()
        pf.name = name
        pf.type = 10
        pos_bones.append(pf);

        rf = nan_animation.bone_frames()
        rf.name = name
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

    maya_frame_time = 1000/24 #ToDo

    for t in range(int(frames_count)):
        cmds.currentTime(t)

        for i in range(len(joints)):
            p = cmds.xform(joints[i],translation=1,q=1)
            pf = nan_animation.pos_frame()
            pf.time=t*maya_frame_time

            pf.pos.x = p[0] - pos_org[i][0]
            pf.pos.y = p[1] - pos_org[i][1]
            pf.pos.z = p[2] - pos_org[i][2]

            #ToDo: sort similar

            pos_bones[i].frames.append(pf)

            r = cmds.xform(joints[i],rotation=1,q=1)
            rf = nan_animation.rot_frame()
            rf.time=t*maya_frame_time
            rf.rot.from_pyr(r[0]-rot_org[i][0],r[1]-rot_org[i][1],r[2]-rot_org[i][2])

            #ToDo: sort similar

            rot_bones[i].frames.append(rf)

    for p in pos_bones:
        if len(p.frames)==0:
            continue
        if len(p.frames)==1 and p.frames[0].pos==nya_vec3():
            continue
        anim.bones.append(p)

    for r in rot_bones:
        if len(r.frames)==0:
            continue
        if len(r.frames)==1 and r.frames[0].rot==nya_quat():
            continue
        anim.bones.append(r)

    cmds.currentTime(time)
    anim.write(file_name)

export()
