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
OBJECTDIR=build/Debug/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/PVRTModelPOD.o \
	${OBJECTDIR}/PVRTVertex.o \
	${OBJECTDIR}/PVRTQuaternionF.o \
	${OBJECTDIR}/PVRTError.o \
	${OBJECTDIR}/PVRTMatrixF.o

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
	${MAKE}  -f nbproject/Makefile-Debug.mk Debug/${PLATFORM}/libPowerVR.so

Debug/${PLATFORM}/libPowerVR.so: ${OBJECTFILES}
	${MKDIR} -p Debug/${PLATFORM}
	${LINK.cc} -shared -o Debug/${PLATFORM}/libPowerVR.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/PVRTModelPOD.o: PVRTModelPOD.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PVRTModelPOD.o PVRTModelPOD.cpp

${OBJECTDIR}/PVRTVertex.o: PVRTVertex.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PVRTVertex.o PVRTVertex.cpp

${OBJECTDIR}/PVRTQuaternionF.o: PVRTQuaternionF.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PVRTQuaternionF.o PVRTQuaternionF.cpp

${OBJECTDIR}/PVRTError.o: PVRTError.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PVRTError.o PVRTError.cpp

${OBJECTDIR}/PVRTMatrixF.o: PVRTMatrixF.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PVRTMatrixF.o PVRTMatrixF.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Debug
	${RM} Debug/${PLATFORM}/libPowerVR.so

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
