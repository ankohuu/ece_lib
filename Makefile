# target, subdir, objects in current dir  
OBJECTS=
SUBDIRS=${LIBDIRS} ${APPDIRS}

TARGET=ece
LIBS=libece.a
LIBDIRS=src lib
APPDIRS=app srv

all:
	${CC} -o ${TARGET} $$(find ${APPDIRS} -name '*.o') ${LDFLAGS} ${LIBS}

.PHONY: mklib
mklib:
	$(AR) $(ARFLAGS) ${LIBS} $$(find ${LIBDIRS} -name '*.o')

# path of "make global scripts"  
# NOTE, use absolute path. export once, use in all subdirs  
export PROJECTPATH=${PWD}
export MAKEINCLUDE=${PROJECTPATH}/cfg/make.global
      
# include "make global scripts"  
include ${MAKEINCLUDE}  
all:mklib
