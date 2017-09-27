#!/usr/bin/env python
'''  ansi color codes for posix terminal'''

color = {"knrm": "\\x1B[0m",  #normal
         "kblk": "\\x1B[30m", #black
         "kred": "\\x1B[31m", #red
         "kgrn": "\\x1B[32m", #green
         "kyel": "\\x1B[33m", #yellow
         "kblu": "\\x1B[34m", #blue
         "kmag": "\\x1B[35m", #magenta
         "kcyn": "\\x1B[36m", #cyan
         "kwht": "\\x1B[37m", #white
         "kbld": "\\x1B[1m",  #bold
         "kres": "\\x1B[0m",  #reset
         "kita": "\\x1B[3m",  #italics
         "kund": "\\x1B[4m",  #underline
         "kstr": "\\x1B[9m",  #strikethrough
         "kinv": "\\x1B[7m",  #inverseON
         "bblk": "\\x1B[30m", #black
         "bred": "\\x1B[31m", #red
         "bgrn": "\\x1B[32m", #green
         "byel": "\\x1B[33m", #yellow
         "bblu": "\\x1B[34m", #blue
         "bmag": "\\x1B[35m", #magenta
         "bcyn": "\\x1B[36m", #cyan
         "bwht": "\\x1B[37m"} #white

for c in color:
  exec(c + ' = "'  + color[c] + '"')

if __name__ == '__main__':
  print kmag + "ansicolor" + kyel + "." + kgrn + "py" + knrm

