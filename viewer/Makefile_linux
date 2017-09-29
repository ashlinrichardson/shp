OBJS = newzpr.o zprdemo.o
CC = g++  
DEBUG = #-g
CFLAGS =  -c -w $(DEBUG) -O3 
LFLAGS =  $(DEBUG) -lm -lGL -lGLU -lglut #-framework Cocoa -framework GLUT -framework OpenGL
zpr: $(OBJS)
	$(CC)   $(OBJS) -o  zpr.exe  $(LFLAGS)
newzpr.o: newzpr.cpp  newzpr.h
	$(CC) $(CFLAGS) newzpr.cpp
zprdemo.o:	zprdemo.cpp	newzpr.h  newzpr.cpp
	$(CC)	    $(CFLAGS)	    zprdemo.cpp
