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

print "opening parks shp.."
shp_1, dbf_1 = open("TA_PEP_SVW_polygon.shp"), open("TA_PEP_SVW_polygon.dbf")
sr_1 = shapefile.Reader(shp=shp_1, dbf=dbf_1) 

print "opening firec shp.."
shp_2, dbf_2 = open("DRPMFFRCNT_polygon.shp"), open("DRPMFFRCNT_polygon.dbf")
sr_2 = shapefile.Reader(shp=shp_2, dbf=dbf_2)  

print "parse parks shp.."
shapes_1, records_1 = sr_1.shapes(), sr_1.records()

print("parse fire centre shp..")
shapes_2, records_2 = sr_2.shapes(), sr_2.records()

id_1, p_p = load_shp(shapes_1, records_1)
id_2, p_f = load_shp(shapes_2, records_2)
'''
print "len(poly_p)", len(poly_p)
print "len(poly_f)", len(poly_f)
'''
f = open("park_names.txt", "wb")
for i in id_1:
    f.write(str(i).strip() + "\n")
f.close()


# create an ogr polygon object, from a list of raw coords..
def create_poly(coords):
    ring = ogr.Geometry(ogr.wkbLinearRing)
    for c in coords:
        ring.AddPoint_2D(c[0], c[1])
    c0 = coords[0]
    cn = coords[len(coords)-1]
    if not (c0[0] == cn[0] and c0[1] == cn[1]):
        ring.AddPoint(c0[0], c0[1])

    poly = ogr.Geometry(ogr.wkbPolygon)
    poly.AddGeometry(ring)
    return poly   # .ExportToWkt()


import matplotlib.pyplot as plt

o_p = []
p1 = []
p2 = []

fcp = [ ] 
poly_p = p_f

outfile=open("out.txt", "ab")
outfile.write("park_i, park_name,")
for i in range(0, len(id_2)):
    outfile.write(id_2[i][0:-12] + ",")
outfile.write("fire_centre_index,fire_centre_name, multiple\n")


outfile.close()

for park_i in range(0, len(poly_p)):
    if True: #park_i < 10:  #  # park_i >= 272 and park_i < 279: # and park_272 i < 279:
        park_poly = create_poly(poly_p[park_i])
        fcp.append(park_poly)

poly_p = p_p
for park_i in range(0, len(poly_p)):
    if True: #park_i < 10:  #  # park_i >= 272 and park_i < 279: # and park_272 i < 279:
        '''
 273 STRATHCONA PARK
 274 STRATHCONA PARK
 275 STRATHCONA PARK
 276 STRATHCONA PARK
 277 STRATHCONA PARK
 278 STRATHCONA PARK
 279 STRATHCONA PARK
        '''
        park_poly = create_poly(poly_p[park_i])
        # print "park_poly", park_poly
        # sys.exit(1)
        p1 = []
        p2 = []
#        for p in poly_p[park_i]:
#            # print p[0], p[1]
#            p1.append(p[0])
#            p2.append(p[1])
#        plt.plot(p1, p2) #, label=str(id_1[park_i]))
        #print "parki", id_1[park_i], len(poly_p[park_i])#, park_poly
        #plt.legend()
        ipi = []        
        for ii in range(0,len(fcp)):
            #print "parki,ii", park_i, ii
            result = None
            try:
                result = fcp[ii].Intersection(park_poly).ExportToWkt()
            except:
                result = None
            if not result is None:
                ipi.append(len(result) if result!='GEOMETRYCOLLECTION EMPTY' else 0)
            else:
                ipi.append("")
        print "park_i", park_i, str(ipi)
        sss = (str(park_i) + "," + str(id_1[park_i]) + ",")
        maxi = -1
        maxn = 0
        for i in range(0,len(ipi)):
            sss += (str(ipi[i])+",")
            if ipi[i] > maxn:
                maxi = i
                maxn = ipi[i]
        sss += str(maxi)+","
        if(maxi >0):
            sss += str(id_2[maxi][0:-12])
        else:
            sss += str("N/A")
        np = 0
        for i in range(0,len(ipi)):
            if ipi[i] > 0:
                np += 1
        sss += ","
        sss += "True" if np > 1 else "False"
        print kgrn+sss+knrm
        outfile = open("out.txt", "ab")
        outfile.write(sss+ "\n")
        outfile.close()

