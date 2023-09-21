#specify your compiler
CC      = g++

# auxiliary flags
CFLAGS	= -g -std=c++11

#first set up the platform dependent variables
ifeq ("$(shell uname)", "Darwin")
  LDFLAGS     = -framework Foundation -framework GLUT -framework OpenGL -lOpenImageIO -lm
else
  ifeq ("$(shell uname)", "Linux")
    LDFLAGS     = -L /usr/lib64/ -lglut -lGL -lGLU -lOpenImageIO -lm
  endif
endif

#this will be the name of your executable
PROJECT = convolve

# list a .o file for each .cpp file that you will compile
# this makefile will compile each cpp separately before linking
OBJECTS = convolve.o

# this does the linking step
${PROJECT} : ${OBJECTS}
	${CC} ${CFLAGS} -o ${PROJECT} ${OBJECTS} ${LDFLAGS}

#this generically compiles each .cpp to a .o file
# %.o: %.cpp
# 	 ${CC} -c ${CFLAGS} $<

#it does not check for .h files dependencies, but you could add that, e.g.
#somfile.o    : somefile.cpp someheader.h
# %.o: %.cpp
# 	${CC} ${CFLAGS} $<


#this will clean up all temporary files created by make all
clean:
	rm -f core.* *.o *~ ${PROJECT}
