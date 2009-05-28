#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=g++
CXX=g++
FC=

# Macros
PLATFORM=GNU-Linux-x86

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/Release/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/sqstdlib/sqstdblob.o \
	${OBJECTDIR}/squirrel/sqcompiler.o \
	${OBJECTDIR}/squirrel/sqbaselib.o \
	${OBJECTDIR}/sqstdlib/sqstdrex.o \
	${OBJECTDIR}/squirrel/sqlexer.o \
	${OBJECTDIR}/sqstdlib/sqstdio.o \
	${OBJECTDIR}/squirrel/sqclass.o \
	${OBJECTDIR}/sqstdlib/sqstdmath.o \
	${OBJECTDIR}/squirrel/sqtable.o \
	${OBJECTDIR}/squirrel/sqdebug.o \
	${OBJECTDIR}/sqstdlib/sqstdaux.o \
	${OBJECTDIR}/squirrel/sqfuncstate.o \
	${OBJECTDIR}/squirrel/sqobject.o \
	${OBJECTDIR}/squirrel/sqvm.o \
	${OBJECTDIR}/sqstdlib/sqstdstream.o \
	${OBJECTDIR}/squirrel/sqapi.o \
	${OBJECTDIR}/sqstdlib/sqstdstring.o \
	${OBJECTDIR}/squirrel/sqstate.o \
	${OBJECTDIR}/squirrel/sqmem.o \
	${OBJECTDIR}/sqstdlib/sqstdsystem.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Release.mk dist/Release/${PLATFORM}/libsquirrel.so

dist/Release/${PLATFORM}/libsquirrel.so: ${OBJECTFILES}
	${MKDIR} -p dist/Release/${PLATFORM}
	${LINK.cc} -shared -o dist/Release/${PLATFORM}/libsquirrel.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/sqstdlib/sqstdblob.o: sqstdlib/sqstdblob.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdblob.o sqstdlib/sqstdblob.cpp

${OBJECTDIR}/squirrel/sqcompiler.o: squirrel/sqcompiler.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqcompiler.o squirrel/sqcompiler.cpp

${OBJECTDIR}/squirrel/sqbaselib.o: squirrel/sqbaselib.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqbaselib.o squirrel/sqbaselib.cpp

${OBJECTDIR}/sqstdlib/sqstdrex.o: sqstdlib/sqstdrex.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdrex.o sqstdlib/sqstdrex.cpp

${OBJECTDIR}/squirrel/sqlexer.o: squirrel/sqlexer.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqlexer.o squirrel/sqlexer.cpp

${OBJECTDIR}/sqstdlib/sqstdio.o: sqstdlib/sqstdio.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdio.o sqstdlib/sqstdio.cpp

${OBJECTDIR}/squirrel/sqclass.o: squirrel/sqclass.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqclass.o squirrel/sqclass.cpp

${OBJECTDIR}/sqstdlib/sqstdmath.o: sqstdlib/sqstdmath.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdmath.o sqstdlib/sqstdmath.cpp

${OBJECTDIR}/squirrel/sqtable.o: squirrel/sqtable.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqtable.o squirrel/sqtable.cpp

${OBJECTDIR}/squirrel/sqdebug.o: squirrel/sqdebug.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqdebug.o squirrel/sqdebug.cpp

${OBJECTDIR}/sqstdlib/sqstdaux.o: sqstdlib/sqstdaux.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdaux.o sqstdlib/sqstdaux.cpp

${OBJECTDIR}/squirrel/sqfuncstate.o: squirrel/sqfuncstate.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqfuncstate.o squirrel/sqfuncstate.cpp

${OBJECTDIR}/squirrel/sqobject.o: squirrel/sqobject.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqobject.o squirrel/sqobject.cpp

${OBJECTDIR}/squirrel/sqvm.o: squirrel/sqvm.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqvm.o squirrel/sqvm.cpp

${OBJECTDIR}/sqstdlib/sqstdstream.o: sqstdlib/sqstdstream.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdstream.o sqstdlib/sqstdstream.cpp

${OBJECTDIR}/squirrel/sqapi.o: squirrel/sqapi.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqapi.o squirrel/sqapi.cpp

${OBJECTDIR}/sqstdlib/sqstdstring.o: sqstdlib/sqstdstring.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdstring.o sqstdlib/sqstdstring.cpp

${OBJECTDIR}/squirrel/sqstate.o: squirrel/sqstate.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqstate.o squirrel/sqstate.cpp

${OBJECTDIR}/squirrel/sqmem.o: squirrel/sqmem.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqmem.o squirrel/sqmem.cpp

${OBJECTDIR}/sqstdlib/sqstdsystem.o: sqstdlib/sqstdsystem.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdsystem.o sqstdlib/sqstdsystem.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Release
	${RM} dist/Release/${PLATFORM}/libsquirrel.so

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
