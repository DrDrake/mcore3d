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
	${OBJECTDIR}/System.o \
	${OBJECTDIR}/Binding.o \
	${OBJECTDIR}/Render.o \
	${OBJECTDIR}/Math.o \
	${OBJECTDIR}/Entity.o \
	${OBJECTDIR}/Pch.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath ../3Party/squirrel/dist/Release/GNU-Linux-x86 -L../3Party/squirrel/dist/Release/GNU-Linux-x86 -lsquirrel

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Release.mk dist/Release/${PLATFORM}/libBinding.so

dist/Release/${PLATFORM}/libBinding.so: ../3Party/squirrel/dist/Release/GNU-Linux-x86/libsquirrel.so

dist/Release/${PLATFORM}/libBinding.so: ${OBJECTFILES}
	${MKDIR} -p dist/Release/${PLATFORM}
	${LINK.cc} -shared -o dist/Release/${PLATFORM}/libBinding.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/System.o: System.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System.o System.cpp

${OBJECTDIR}/Binding.o: Binding.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Binding.o Binding.cpp

${OBJECTDIR}/Render.o: Render.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Render.o Render.cpp

${OBJECTDIR}/Math.o: Math.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math.o Math.cpp

${OBJECTDIR}/Entity.o: Entity.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Entity.o Entity.cpp

${OBJECTDIR}/Pch.o: Pch.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -DUNICODE -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Pch.o Pch.cpp

# Subprojects
.build-subprojects:
	cd ../3Party/squirrel && ${MAKE}  -f Makefile CONF=Release

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release
	${RM} dist/Release/${PLATFORM}/libBinding.so

# Subprojects
.clean-subprojects:
	cd ../3Party/squirrel && ${MAKE}  -f Makefile CONF=Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
