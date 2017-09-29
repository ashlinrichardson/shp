#!/usr/bin/env python
'''collate the intersection matrix 20170928'''
import os
import sys
lines = open("mat00.txt").readlines()

pi = { }

for line in lines:
  line = line.strip()
  f, p, a = line.split(",")
  f, p, a = int(f)-1, int(p)-1, float(a)
  #if( a>0.):
  #  print f, p, a
  if not p in pi:
    pi[p] = [None, None, None, None, None, None]
  pi[p][f] = a
  

for k in pi:
  s = str(k) 
  for i in range(0, len(pi[k])):
    s+= "," + str("%1.1e"% pi[k][i])
  print s




