OBJS	=	newzpr.o	zprdemo.o
CC	=	g++
DEBUG	= # -g
CFLAGS	=	-O3 -c	-w	$(DEBUG)	-I/opt/local/include/	-I../boost_1_65_1/
LFLAGS	=	$(DEBUG)	-lm	-framework	Cocoa	-framework	GLUT	-framework	OpenGL
zpr:	$(OBJS)
	$(CC)	$(OBJS)	-o	run.exe	$(LFLAGS)
newzpr.o:	newzpr.cpp	newzpr.h
	$(CC)	$(CFLAGS)	newzpr.cpp
zprdemo.o:	zprdemo.cpp	newzpr.h	newzpr.cpp
	$(CC)	$(CFLAGS)	zprdemo.cpp
