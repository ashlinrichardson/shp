#!/usr/bin/env python
import sys
from xml.dom import minidom
xmldoc = minidom.parse('parkInfo.xml')
itemlist = xmldoc.getElementsByTagName('park')
print("number of park entries " + str(len(itemlist)))

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

print d
