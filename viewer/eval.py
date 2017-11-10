#!/usr/bin/env python

fc = {0:'Northwest',
      1:'Prince George',
      2:'Kamloops',
      3:'Coastal',
      4:'Southeast',
      5:'Cariboo'}

import os
import sys

lines = open("result.txt").readlines()

def maxi(a):
  s = []
  for i in range(0,len(a)):
    s.append([i, a[i]])
  t = sorted(s, key=lambda x: x[1], reverse=True)  # top value first (Decreasing order)
  return t[0][0], t[0][1]

ci = 0
for l in lines:
  ll = l.strip().split(",")
  if len(ll) != 14:
    print "error"
    sys.exit(1)
  park_name = ll[0]
  intr, bbox = ll[1:7], ll[8:]
  if len(intr) != 6 or len(bbox) != 6:
    print "error"
    sys.exit(1)
  
  # convert from string to float (double)
  for i in range(0,len(intr)):
    intr[i], bbox[i] = 0. if intr[i]=='' else float(intr[i]), 0. if bbox[i]=='' else float(bbox[i])
 

  maxi_intr, maxv_intr = maxi(intr)
  maxi_bbox, maxv_bbox = maxi(bbox)

  print ci, park_name + ',' + (str(fc[maxi_intr]) if maxv_intr > 0. else '') + "," + (str(fc[maxi_bbox]) if maxv_bbox > 0. else '')
  ci += 1

print 'should flag multiples (from either method) and if methods disagree'

