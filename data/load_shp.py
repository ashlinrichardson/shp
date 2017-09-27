'''20170922'''
from shp_typ_str import *
from ansicolor import *
from osgeo import ogr
import sys

def load_shp(shapes, records):
    # print kgrn + ('=' * 80)
    id =[]
    if len(shapes) != len(records):
        print "Err: mismatch", len(shapes), len(records)
        sys.exit(1)
    # type
    SHP_TYPE = shapes[0].shapeType
    # print (kred + "N_SHP"), '\t', kyel, len(shapes), '\t', kred, "SHP_TYP", kmag, shp_typ_str(SHP_TYPE), kgrn
    my_points = []  # return the point lists  
    # print kgrn + ('-' * 80) 
    for i in range(0, len(shapes)):  # len(shapes)):
        #if i==0:  print  (kgrn + 'shp:'), kblu, shapes[i].__dict__.keys(), kgrn, '\ndbf:', kblu, records[i]

        if shapes[i].shapeType != shapes[0].shapeType:
            print "Error: shape type mismatch"
            sys.exit(1)

        s, r = shapes[i], records[i]  # print s.__dict__.keys()  # ['z', 'points', 'parts', 'shapeType', 'bbox']
	# print "points", len(s.points)
	my_points.append(s.points)
        name = r[0] if records[i][1]==None else records[i][1] 

        #if i<6:  # kgrn, i
        #    print kyel, name, kgrn  # , s.parts, s.shapeType# , kred, "box", kblu, s.bbox, kgrn
        id.append(name)
    # print "id", id  # that would print (for the parks) a long list of names..
    #print kgrn + ('-' * 80) + knrm
    return id, my_points
