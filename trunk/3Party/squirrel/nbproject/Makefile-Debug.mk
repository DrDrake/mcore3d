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
AS=as

# Macros
CND_PLATFORM=GNU-Linux-x86
CND_CONF=Debug
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

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

# Assembler Flags
ASFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/libsquirrel.so

dist/Debug/GNU-Linux-x86/libsquirrel.so: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.cc} -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libsquirrel.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/sqstdlib/sqstdblob.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdblob.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdblob.o sqstdlib/sqstdblob.cpp

${OBJECTDIR}/squirrel/sqcompiler.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqcompiler.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqcompiler.o squirrel/sqcompiler.cpp

${OBJECTDIR}/squirrel/sqbaselib.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqbaselib.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqbaselib.o squirrel/sqbaselib.cpp

${OBJECTDIR}/sqstdlib/sqstdrex.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdrex.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdrex.o sqstdlib/sqstdrex.cpp

${OBJECTDIR}/squirrel/sqlexer.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqlexer.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqlexer.o squirrel/sqlexer.cpp

${OBJECTDIR}/sqstdlib/sqstdio.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdio.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdio.o sqstdlib/sqstdio.cpp

${OBJECTDIR}/squirrel/sqclass.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqclass.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqclass.o squirrel/sqclass.cpp

${OBJECTDIR}/sqstdlib/sqstdmath.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdmath.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdmath.o sqstdlib/sqstdmath.cpp

${OBJECTDIR}/squirrel/sqtable.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqtable.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqtable.o squirrel/sqtable.cpp

${OBJECTDIR}/squirrel/sqdebug.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqdebug.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqdebug.o squirrel/sqdebug.cpp

${OBJECTDIR}/sqstdlib/sqstdaux.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdaux.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdaux.o sqstdlib/sqstdaux.cpp

${OBJECTDIR}/squirrel/sqfuncstate.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqfuncstate.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqfuncstate.o squirrel/sqfuncstate.cpp

${OBJECTDIR}/squirrel/sqobject.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqobject.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqobject.o squirrel/sqobject.cpp

${OBJECTDIR}/squirrel/sqvm.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqvm.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqvm.o squirrel/sqvm.cpp

${OBJECTDIR}/sqstdlib/sqstdstream.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdstream.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdstream.o sqstdlib/sqstdstream.cpp

${OBJECTDIR}/squirrel/sqapi.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqapi.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqapi.o squirrel/sqapi.cpp

${OBJECTDIR}/sqstdlib/sqstdstring.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdstring.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdstring.o sqstdlib/sqstdstring.cpp

${OBJECTDIR}/squirrel/sqstate.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqstate.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqstate.o squirrel/sqstate.cpp

${OBJECTDIR}/squirrel/sqmem.o: nbproject/Makefile-${CND_CONF}.mk squirrel/sqmem.cpp 
	${MKDIR} -p ${OBJECTDIR}/squirrel
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/squirrel/sqmem.o squirrel/sqmem.cpp

${OBJECTDIR}/sqstdlib/sqstdsystem.o: nbproject/Makefile-${CND_CONF}.mk sqstdlib/sqstdsystem.cpp 
	${MKDIR} -p ${OBJECTDIR}/sqstdlib
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/sqstdlib/sqstdsystem.o sqstdlib/sqstdsystem.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/libsquirrel.so

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
