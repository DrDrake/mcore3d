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
	${OBJECTDIR}/jidctfst.o \
	${OBJECTDIR}/jpegdecoder.o \
	${OBJECTDIR}/idct.o \
	${OBJECTDIR}/H2v2.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=-msse2
CXXFLAGS=-msse2

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Release.mk Release/${PLATFORM}/libSmallJpeg.so

Release/${PLATFORM}/libSmallJpeg.so: ${OBJECTFILES}
	${MKDIR} -p Release/${PLATFORM}
	${LINK.cc} -shared -o Release/${PLATFORM}/libSmallJpeg.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/jidctfst.o: jidctfst.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jidctfst.o jidctfst.cpp

${OBJECTDIR}/jpegdecoder.o: jpegdecoder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/jpegdecoder.o jpegdecoder.cpp

${OBJECTDIR}/idct.o: idct.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/idct.o idct.cpp

${OBJECTDIR}/H2v2.o: H2v2.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/H2v2.o H2v2.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Release
	${RM} Release/${PLATFORM}/libSmallJpeg.so

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
