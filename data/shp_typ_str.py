'''20170922 asrichar'''
def shp_typ_str(s):
    t ={'NULL': 0, 'POINT': 1, 'POLYLINE': 3, 'POLYGON': 5, 'MULTIPOINT': 8, 'POINTZ': 11, 'POLYLINEZ': 13, 'POLYGONZ': 15, 'MULTIPOINTZ': 18, 'POINTM': 21, 'POLYLINEM': 23, 'POLYGONM': 25, 'MULTIPOINTM': 28, 'MULTIPATCH': 31}
    t2 = {}
    for k in t: t2[t[k]] = k
    return t2[s]
