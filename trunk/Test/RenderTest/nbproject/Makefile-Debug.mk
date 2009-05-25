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
	${OBJECTDIR}/TextureLoaderTest.o \
	${OBJECTDIR}/PickComponentTest.o \
	${OBJECTDIR}/Max3dsLoaderTest.o \
	${OBJECTDIR}/ShadowMapTest.o \
	${OBJECTDIR}/ASSAOTest.o \
	${OBJECTDIR}/BasicGlWindow.o \
	${OBJECTDIR}/FrustumTest.o \
	${OBJECTDIR}/ChamferBox.o \
	${OBJECTDIR}/Common.o \
	${OBJECTDIR}/MultipleRenderTargetTest.o \
	${OBJECTDIR}/MaterialTest.o \
	${OBJECTDIR}/MeshTest.o \
	${OBJECTDIR}/MeshComponentTest.o \
	${OBJECTDIR}/DefaultResourceManager.o \
	${OBJECTDIR}/NormalMappingTest.o \
	${OBJECTDIR}/SSAOTest.o \
	${OBJECTDIR}/ProjectiveTextureTest.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/RenderTargetTest.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath ../../MCD/Render/Debug/GNU-Linux-x86 -L../../MCD/Render/Debug/GNU-Linux-x86 -lRender -Wl,-rpath ../../MCD/Core/Debug/GNU-Linux-x86 -L../../MCD/Core/Debug/GNU-Linux-x86 -lCore -Wl,-rpath ../../3Party/CppTestHarness/Debug/GNU-Linux-x86 -L../../3Party/CppTestHarness/Debug/GNU-Linux-x86 -lCppTestHarness

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk Debug/${PLATFORM}/rendertest

Debug/${PLATFORM}/rendertest: ../../MCD/Render/Debug/GNU-Linux-x86/libRender.so

Debug/${PLATFORM}/rendertest: ../../MCD/Core/Debug/GNU-Linux-x86/libCore.so

Debug/${PLATFORM}/rendertest: ../../3Party/CppTestHarness/Debug/GNU-Linux-x86/libCppTestHarness.so

Debug/${PLATFORM}/rendertest: ${OBJECTFILES}
	${MKDIR} -p Debug/${PLATFORM}
	${LINK.cc} -o Debug/${PLATFORM}/rendertest ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/TextureLoaderTest.o: TextureLoaderTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/TextureLoaderTest.o TextureLoaderTest.cpp

${OBJECTDIR}/PickComponentTest.o: PickComponentTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/PickComponentTest.o PickComponentTest.cpp

${OBJECTDIR}/Max3dsLoaderTest.o: Max3dsLoaderTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Max3dsLoaderTest.o Max3dsLoaderTest.cpp

${OBJECTDIR}/ShadowMapTest.o: ShadowMapTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ShadowMapTest.o ShadowMapTest.cpp

${OBJECTDIR}/ASSAOTest.o: ASSAOTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ASSAOTest.o ASSAOTest.cpp

${OBJECTDIR}/BasicGlWindow.o: BasicGlWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/BasicGlWindow.o BasicGlWindow.cpp

${OBJECTDIR}/FrustumTest.o: FrustumTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/FrustumTest.o FrustumTest.cpp

${OBJECTDIR}/ChamferBox.o: ChamferBox.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ChamferBox.o ChamferBox.cpp

${OBJECTDIR}/Common.o: Common.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Common.o Common.cpp

${OBJECTDIR}/MultipleRenderTargetTest.o: MultipleRenderTargetTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/MultipleRenderTargetTest.o MultipleRenderTargetTest.cpp

${OBJECTDIR}/MaterialTest.o: MaterialTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/MaterialTest.o MaterialTest.cpp

${OBJECTDIR}/MeshTest.o: MeshTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/MeshTest.o MeshTest.cpp

${OBJECTDIR}/MeshComponentTest.o: MeshComponentTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/MeshComponentTest.o MeshComponentTest.cpp

${OBJECTDIR}/DefaultResourceManager.o: DefaultResourceManager.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/DefaultResourceManager.o DefaultResourceManager.cpp

${OBJECTDIR}/NormalMappingTest.o: NormalMappingTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/NormalMappingTest.o NormalMappingTest.cpp

${OBJECTDIR}/SSAOTest.o: SSAOTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/SSAOTest.o SSAOTest.cpp

${OBJECTDIR}/ProjectiveTextureTest.o: ProjectiveTextureTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/ProjectiveTextureTest.o ProjectiveTextureTest.cpp

${OBJECTDIR}/Main.o: Main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/Main.o Main.cpp

${OBJECTDIR}/RenderTargetTest.o: RenderTargetTest.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -MMD -MP -MF $@.d -o ${OBJECTDIR}/RenderTargetTest.o RenderTargetTest.cpp

# Subprojects
.build-subprojects:
	cd ../../MCD/Render && ${MAKE}  -f Makefile CONF=Debug
	cd ../../MCD/Core && ${MAKE}  -f Makefile CONF=Debug
	cd ../../3Party/CppTestHarness && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} Debug/${PLATFORM}/rendertest

# Subprojects
.clean-subprojects:
	cd ../../MCD/Render && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../MCD/Core && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../3Party/CppTestHarness && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
