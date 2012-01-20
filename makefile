#!gmake 

#-----------------------------------------
#Basic Stuff -----------------------------
CC          = g++ 
cc          = gcc

#-----------------------------------------
#Optimization ----------------------------
OPT   = -O3

#-----------------------------------------
# X       --------------------------------
X_INC  =   -I/usr/X11R6/include -I/sw/include -I/usr/sww/include -I/usr/sww/pkg/Mesa/include
X_LIB  =   -L/usr/X11R6/lib 

#-----------------------------------------

#-----------------------------------------
# GL      --------------------------------
GL_LIB  =   -lglut -lGLU -lGL -lX11

#-----------------------------------------

TARGETS = 3d_workshop

OBJECTS = 3d_workshop.o

#-----------------------------------------

LIBS = $(X_LIB) $(GL_LIB)
INCS = $(X_INC)

CCOPTS = $(OPT) $(INCS)
LDOPTS = $(OPT) $(INCS) $(LIBS)

#-----------------------------------------
#-----------------------------------------

default: $(TARGETS)

clean:
	/bin/rm -f *.o $(TARGETS)

#-----------------------------------------
#-----------------------------------------

3d_workshop: 3d_workshop.o
	$(CC) 3d_workshop.o $(LDOPTS) -o 3d_workshop 

3d_workshop.o: 3d_workshop.cpp
	$(CC) 3d_workshop.cpp -c $(CCOPTS)


#export LD_LIBRARY_PATH="/usr/sww/pkg/Mesa/lib:/usr/lib:/usr/sww/lib"

