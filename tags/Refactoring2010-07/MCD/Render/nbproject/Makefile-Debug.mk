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
	${OBJECTDIR}/TgaLoader.o \
	${OBJECTDIR}/Color.o \
	${OBJECTDIR}/ChamferBox.o \
	${OBJECTDIR}/Shader.o \
	${OBJECTDIR}/EditableMesh.o \
	${OBJECTDIR}/BackRenderBuffer.o \
	${OBJECTDIR}/MeshBuilder.o \
	${OBJECTDIR}/ModelPod.o \
	${OBJECTDIR}/SemanticMap.o \
	${OBJECTDIR}/ProjectiveTexture.o \
	${OBJECTDIR}/RenderBuffer.o \
	${OBJECTDIR}/Texture.o \
	${OBJECTDIR}/ShaderLoader.o \
	${OBJECTDIR}/RenderTarget.o \
	${OBJECTDIR}/Max3dsLoader.o \
	${OBJECTDIR}/MaterialProperty.o \
	${OBJECTDIR}/Camera.o \
	${OBJECTDIR}/TextureLoaderBase.o \
	${OBJECTDIR}/Model.o \
	${OBJECTDIR}/Mesh.o \
	${OBJECTDIR}/ResourceLoaderFactory.o \
	${OBJECTDIR}/Frustum.o \
	${OBJECTDIR}/GlWindow.o \
	${OBJECTDIR}/JpegLoader.o \
	${OBJECTDIR}/EffectLoader.o \
	${OBJECTDIR}/OgreMeshLoader.o \
	${OBJECTDIR}/DdsLoader.o \
	${OBJECTDIR}/Effect.o \
	${OBJECTDIR}/BitmapLoader.o \
	${OBJECTDIR}/TextureRenderBuffer.o \
	${OBJECTDIR}/CubemapLoader.o \
	${OBJECTDIR}/TangentSpaceBuilder.o \
	${OBJECTDIR}/Material.o \
	${OBJECTDIR}/PngLoader.o \
	${OBJECTDIR}/ShaderProgram.o

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
LDLIBSOPTIONS=-Wl,-rpath ../Core/dist/Debug/GNU-Linux-x86 -L../Core/dist/Debug/GNU-Linux-x86 -lCore -Wl,-rpath ../../3Party/PowerVR/dist/Debug/GNU-Linux-x86 -L../../3Party/PowerVR/dist/Debug/GNU-Linux-x86 -lPowerVR -Wl,-rpath ../../3Party/SmallJpeg/dist/Debug/GNU-Linux-x86 -L../../3Party/SmallJpeg/dist/Debug/GNU-Linux-x86 -lSmallJpeg -lGL -lGLU -lGLEW -lpng

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk dist/Debug/GNU-Linux-x86/libRender.so

dist/Debug/GNU-Linux-x86/libRender.so: ../Core/dist/Debug/GNU-Linux-x86/libCore.so

dist/Debug/GNU-Linux-x86/libRender.so: ../../3Party/PowerVR/dist/Debug/GNU-Linux-x86/libPowerVR.so

dist/Debug/GNU-Linux-x86/libRender.so: ../../3Party/SmallJpeg/dist/Debug/GNU-Linux-x86/libSmallJpeg.so

dist/Debug/GNU-Linux-x86/libRender.so: ${OBJECTFILES}
	${MKDIR} -p dist/Debug/GNU-Linux-x86
	${LINK.cc} -shared -o ${CND_DISTDIR}/${CND_CONF}/${CND_PLATFORM}/libRender.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/TgaLoader.o: nbproject/Makefile-${CND_CONF}.mk TgaLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TgaLoader.o TgaLoader.cpp

${OBJECTDIR}/Color.o: nbproject/Makefile-${CND_CONF}.mk Color.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Color.o Color.cpp

${OBJECTDIR}/ChamferBox.o: nbproject/Makefile-${CND_CONF}.mk ChamferBox.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ChamferBox.o ChamferBox.cpp

${OBJECTDIR}/Shader.o: nbproject/Makefile-${CND_CONF}.mk Shader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Shader.o Shader.cpp

${OBJECTDIR}/EditableMesh.o: nbproject/Makefile-${CND_CONF}.mk EditableMesh.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/EditableMesh.o EditableMesh.cpp

${OBJECTDIR}/BackRenderBuffer.o: nbproject/Makefile-${CND_CONF}.mk BackRenderBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/BackRenderBuffer.o BackRenderBuffer.cpp

${OBJECTDIR}/MeshBuilder.o: nbproject/Makefile-${CND_CONF}.mk MeshBuilder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/MeshBuilder.o MeshBuilder.cpp

${OBJECTDIR}/ModelPod.o: nbproject/Makefile-${CND_CONF}.mk ModelPod.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ModelPod.o ModelPod.cpp

${OBJECTDIR}/SemanticMap.o: nbproject/Makefile-${CND_CONF}.mk SemanticMap.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/SemanticMap.o SemanticMap.cpp

${OBJECTDIR}/ProjectiveTexture.o: nbproject/Makefile-${CND_CONF}.mk ProjectiveTexture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ProjectiveTexture.o ProjectiveTexture.cpp

${OBJECTDIR}/RenderBuffer.o: nbproject/Makefile-${CND_CONF}.mk RenderBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/RenderBuffer.o RenderBuffer.cpp

${OBJECTDIR}/Texture.o: nbproject/Makefile-${CND_CONF}.mk Texture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Texture.o Texture.cpp

${OBJECTDIR}/ShaderLoader.o: nbproject/Makefile-${CND_CONF}.mk ShaderLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ShaderLoader.o ShaderLoader.cpp

${OBJECTDIR}/RenderTarget.o: nbproject/Makefile-${CND_CONF}.mk RenderTarget.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/RenderTarget.o RenderTarget.cpp

${OBJECTDIR}/Max3dsLoader.o: nbproject/Makefile-${CND_CONF}.mk Max3dsLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Max3dsLoader.o Max3dsLoader.cpp

${OBJECTDIR}/MaterialProperty.o: nbproject/Makefile-${CND_CONF}.mk MaterialProperty.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/MaterialProperty.o MaterialProperty.cpp

${OBJECTDIR}/Camera.o: nbproject/Makefile-${CND_CONF}.mk Camera.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Camera.o Camera.cpp

${OBJECTDIR}/TextureLoaderBase.o: nbproject/Makefile-${CND_CONF}.mk TextureLoaderBase.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TextureLoaderBase.o TextureLoaderBase.cpp

${OBJECTDIR}/Model.o: nbproject/Makefile-${CND_CONF}.mk Model.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Model.o Model.cpp

${OBJECTDIR}/Mesh.o: nbproject/Makefile-${CND_CONF}.mk Mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Mesh.o Mesh.cpp

${OBJECTDIR}/ResourceLoaderFactory.o: nbproject/Makefile-${CND_CONF}.mk ResourceLoaderFactory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ResourceLoaderFactory.o ResourceLoaderFactory.cpp

${OBJECTDIR}/Frustum.o: nbproject/Makefile-${CND_CONF}.mk Frustum.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Frustum.o Frustum.cpp

${OBJECTDIR}/GlWindow.o: nbproject/Makefile-${CND_CONF}.mk GlWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/GlWindow.o GlWindow.cpp

${OBJECTDIR}/JpegLoader.o: nbproject/Makefile-${CND_CONF}.mk JpegLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/JpegLoader.o JpegLoader.cpp

${OBJECTDIR}/EffectLoader.o: nbproject/Makefile-${CND_CONF}.mk EffectLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/EffectLoader.o EffectLoader.cpp

${OBJECTDIR}/OgreMeshLoader.o: nbproject/Makefile-${CND_CONF}.mk OgreMeshLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/OgreMeshLoader.o OgreMeshLoader.cpp

${OBJECTDIR}/DdsLoader.o: nbproject/Makefile-${CND_CONF}.mk DdsLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/DdsLoader.o DdsLoader.cpp

${OBJECTDIR}/Effect.o: nbproject/Makefile-${CND_CONF}.mk Effect.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Effect.o Effect.cpp

${OBJECTDIR}/BitmapLoader.o: nbproject/Makefile-${CND_CONF}.mk BitmapLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/BitmapLoader.o BitmapLoader.cpp

${OBJECTDIR}/TextureRenderBuffer.o: nbproject/Makefile-${CND_CONF}.mk TextureRenderBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TextureRenderBuffer.o TextureRenderBuffer.cpp

${OBJECTDIR}/CubemapLoader.o: nbproject/Makefile-${CND_CONF}.mk CubemapLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/CubemapLoader.o CubemapLoader.cpp

${OBJECTDIR}/TangentSpaceBuilder.o: nbproject/Makefile-${CND_CONF}.mk TangentSpaceBuilder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TangentSpaceBuilder.o TangentSpaceBuilder.cpp

${OBJECTDIR}/Material.o: nbproject/Makefile-${CND_CONF}.mk Material.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Material.o Material.cpp

${OBJECTDIR}/PngLoader.o: nbproject/Makefile-${CND_CONF}.mk PngLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PngLoader.o PngLoader.cpp

${OBJECTDIR}/ShaderProgram.o: nbproject/Makefile-${CND_CONF}.mk ShaderProgram.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ShaderProgram.o ShaderProgram.cpp

# Subprojects
.build-subprojects:
	cd ../Core && ${MAKE}  -f Makefile CONF=Debug
	cd ../../3Party/PowerVR && ${MAKE}  -f Makefile CONF=Debug
	cd ../../3Party/SmallJpeg && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} dist/Debug/GNU-Linux-x86/libRender.so

# Subprojects
.clean-subprojects:
	cd ../Core && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../3Party/PowerVR && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../3Party/SmallJpeg && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
