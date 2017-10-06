#!/usr/bin/env python
'''write C++ code to map orc to park-name 2017106'''
import sys
from xml.dom import minidom
xmldoc = minidom.parse('parkInfo.xml')
itemlist = xmldoc.getElementsByTagName('park')

# print("number of park entries " + str(len(itemlist)))
d = {}
for i in range(0, len(itemlist)):
    park =  itemlist[i]
    orc_i, nam_i, nodes = -1, -1, park.childNodes
    nam_i = -1
    for ni in range(0, len(nodes)):
      n = nodes[ni]
      if str(n.nodeName.strip()) == str(u'orc'):
        orc_i = ni
      if str(n.nodeName.strip()) == str(u'parkName'):
        nam_i = ni
    # print "\torc_i", orc_i, "nam_i", nam_i
    n = nodes[orc_i]
    # get all the methods of an object:
    #    print [method for method in dir(n) if callable(getattr(n, method))]

    orc = int((n.childNodes[0]).nodeValue)
    
    n = nodes[nam_i]
    nam =  n.childNodes[0].nodeValue

    # use ascii encoding
    d[orc] = nam.encode('ascii', 'ignore')

keys = sorted(d.iterkeys())
mk = 0 # max index?
for k in keys:
  #print k, d[k]
  k = int(k)
  if k > mk:
    mk = k 

print "long int mk = " + str(mk + 1) + ";"
print "long int nb = sizeof(std::string *) * (mk + 1);"
print "std::string * orc_to_name = (std::string *)(void *)malloc(nb);"
print "memset(orc_to_name, '\\0', nb);"
for k in keys:
  print "orc_to_name["+str(k)+'] = std::string("' + str(d[k]) + '");'
