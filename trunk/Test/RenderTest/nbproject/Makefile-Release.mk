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
CND_CONF=Release
CND_DISTDIR=dist

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/${CND_CONF}/${CND_PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/TextureLoaderTest.o \
	${OBJECTDIR}/MaterialTest.o \
	${OBJECTDIR}/ShadowMapTest.o \
	${OBJECTDIR}/MeshTest.o \
	${OBJECTDIR}/ASSAOTest.o \
	${OBJECTDIR}/BasicGlWindow.o \
	${OBJECTDIR}/DefaultResourceManager.o \
	${OBJECTDIR}/FrustumTest.o \
	${OBJECTDIR}/NormalMappingTest.o \
	${OBJECTDIR}/Common.o \
	${OBJECTDIR}/SSAOTest.o \
	${OBJECTDIR}/MultipleRenderTargetTest.o \
	${OBJECTDIR}/ProjectiveTextureTest.o \
	${OBJECTDIR}/RenderTargetTest.o \
	${OBJECTDIR}/Main.o

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
LDLIBSOPTIONS=-Wl,-rpath ../../MCD/Core/dist/Release/GNU-Linux-x86 -L../../MCD/Core/dist/Release/GNU-Linux-x86 -lCore -Wl,-rpath ../../MCD/Render/dist/Release/GNU-Linux-x86 -L../../MCD/Render/dist/Release/GNU-Linux-x86 -lRender -Wl,-rpath ../../3Party/CppTestHarness/dist/Release/GNU-Linux-x86 -L../../3Party/CppTestHarness/dist/Release/GNU-Linux-x86 -lCppTestHarness

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Release.mk dist/Release/GNU-Linux-x86/rendertest

dist/Release/GNU-Linux-x86/rendertest: ../../MCD/Core/dist/Release/GNU-Linux-x86/libCore.so

dist/Release/GNU-Linux-x86/rendertest: ../../MCD/Render/dist/Release/GNU-Linux-x86/libRender.so

dist/Release/GNU-Linux-x86/rendertest: ../../3Party/CppTestHarness/dist/Release/GNU-Linux-x86/libCppTestHarness.so

dist/Release/GNU-Linux-x86/rendertest: ${OBJECTFILES}
	${MKDIR} -p dist/Release/GNU-Linux-x86
	${LINK.cc} -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/rendertest ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/TextureLoaderTest.o: nbproject/Makefile-${CND_CONF}.mk TextureLoaderTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/TextureLoaderTest.o TextureLoaderTest.cpp

${OBJECTDIR}/MaterialTest.o: nbproject/Makefile-${CND_CONF}.mk MaterialTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/MaterialTest.o MaterialTest.cpp

${OBJECTDIR}/ShadowMapTest.o: nbproject/Makefile-${CND_CONF}.mk ShadowMapTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/ShadowMapTest.o ShadowMapTest.cpp

${OBJECTDIR}/MeshTest.o: nbproject/Makefile-${CND_CONF}.mk MeshTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/MeshTest.o MeshTest.cpp

${OBJECTDIR}/ASSAOTest.o: nbproject/Makefile-${CND_CONF}.mk ASSAOTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/ASSAOTest.o ASSAOTest.cpp

${OBJECTDIR}/BasicGlWindow.o: nbproject/Makefile-${CND_CONF}.mk BasicGlWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/BasicGlWindow.o BasicGlWindow.cpp

${OBJECTDIR}/DefaultResourceManager.o: nbproject/Makefile-${CND_CONF}.mk DefaultResourceManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/DefaultResourceManager.o DefaultResourceManager.cpp

${OBJECTDIR}/FrustumTest.o: nbproject/Makefile-${CND_CONF}.mk FrustumTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/FrustumTest.o FrustumTest.cpp

${OBJECTDIR}/NormalMappingTest.o: nbproject/Makefile-${CND_CONF}.mk NormalMappingTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/NormalMappingTest.o NormalMappingTest.cpp

${OBJECTDIR}/Common.o: nbproject/Makefile-${CND_CONF}.mk Common.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/Common.o Common.cpp

${OBJECTDIR}/SSAOTest.o: nbproject/Makefile-${CND_CONF}.mk SSAOTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/SSAOTest.o SSAOTest.cpp

${OBJECTDIR}/MultipleRenderTargetTest.o: nbproject/Makefile-${CND_CONF}.mk MultipleRenderTargetTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/MultipleRenderTargetTest.o MultipleRenderTargetTest.cpp

${OBJECTDIR}/ProjectiveTextureTest.o: nbproject/Makefile-${CND_CONF}.mk ProjectiveTextureTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/ProjectiveTextureTest.o ProjectiveTextureTest.cpp

${OBJECTDIR}/RenderTargetTest.o: nbproject/Makefile-${CND_CONF}.mk RenderTargetTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/RenderTargetTest.o RenderTargetTest.cpp

${OBJECTDIR}/Main.o: nbproject/Makefile-${CND_CONF}.mk Main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/Main.o Main.cpp

# Subprojects
.build-subprojects:
	cd ../../MCD/Core && ${MAKE}  -f Makefile CONF=Release
	cd ../../MCD/Render && ${MAKE}  -f Makefile CONF=Release
	cd ../../3Party/CppTestHarness && ${MAKE}  -f Makefile CONF=Release

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Release
	${RM} dist/Release/GNU-Linux-x86/rendertest

# Subprojects
.clean-subprojects:
	cd ../../MCD/Core && ${MAKE}  -f Makefile CONF=Release clean
	cd ../../MCD/Render && ${MAKE}  -f Makefile CONF=Release clean
	cd ../../3Party/CppTestHarness && ${MAKE}  -f Makefile CONF=Release clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
