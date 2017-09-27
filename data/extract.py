#!/usr/bin/env python
'''Parks Polys       TA_PEP_SVW_polygon.dbf TA_PEP_SVW_polygon.prj TA_PEP_SVW_polygon.shp TA_PEP_SVW_polygon.shx
   Fire Centre Polys DRPMFFRCNT_polygon.dbf DRPMFFRCNT_polygon.prj DRPMFFRCNT_polygon.shp DRPMFFRCNT_polygon.shx

20170922-26'''
import os
import sys
import shapefile
from osgeo import ogr
from ansicolor import *
from load_shp import load_shp
from shp_typ_str import shp_typ_str

myfn = sys.argv[1]

def read_shp(fn):
    shp1, dbf1 = open(fn + ".shp"), open(fn + ".dbf")
    sr = shapefile.Reader(shp = open(fn + ".shp"),
                          dbf = open(fn + ".dbf"))
    return load_shp(sr.shapes(), sr.records())

pi, ps = read_shp(myfn)  # "TA_PEP_SVW_polygon")    
# fi, fp = read_shp("DRPMFFRCNT_polygon")


ci = 0
pn = {} # index for name
pp = {} # points for index
ppn = {}
for i in range(0, len(pi)):
    name = pi[i]
    if name not in pn:
        pn[name] = ci
        ci += 1
    if pn[name] not in pp:
        pp[pn[name]] = []
    for p in ps[i]:
        pp[pn[name]].append(p[0])
        pp[pn[name]].append(p[1])
        if pn[name] not in ppn:
            ppn[pn[name]] = 0
        ppn[pn[name]] += 1

print "FEATURE_NAME,FEATURE_ID,N_POINTS,POINTS"
for j in pn:
    pps =''
    pts =  pp[pn[j]]
    for i in range(0, len(pts)):
        if i > 0:
            pps += ','
        pps += str(pts[i])

    print str(j) + "," + str(pn[j]) + "," + str(ppn[pn[j]]) + "," + pps

