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
	${OBJECTDIR}/Components/CameraComponent.o \
	${OBJECTDIR}/TgaLoader.o \
	${OBJECTDIR}/Color.o \
	${OBJECTDIR}/Shader.o \
	${OBJECTDIR}/Components/PickComponent.o \
	${OBJECTDIR}/BackRenderBuffer.o \
	${OBJECTDIR}/MeshBuilder.o \
	${OBJECTDIR}/ModelPod.o \
	${OBJECTDIR}/Components/MeshComponent.o \
	${OBJECTDIR}/ProjectiveTexture.o \
	${OBJECTDIR}/RenderBuffer.o \
	${OBJECTDIR}/Texture.o \
	${OBJECTDIR}/ShaderLoader.o \
	${OBJECTDIR}/RenderTarget.o \
	${OBJECTDIR}/Max3dsLoader.o \
	${OBJECTDIR}/Components/RenderableComponent.o \
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
	${OBJECTDIR}/DdsLoader.o \
	${OBJECTDIR}/Effect.o \
	${OBJECTDIR}/BitmapLoader.o \
	${OBJECTDIR}/TextureRenderBuffer.o \
	${OBJECTDIR}/Material.o \
	${OBJECTDIR}/PngLoader.o \
	${OBJECTDIR}/Pch.o \
	${OBJECTDIR}/ShaderProgram.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath ../Core/Debug/GNU-Linux-x86 -L../Core/Debug/GNU-Linux-x86 -lCore -Wl,-rpath ../../3Party/SmallJpeg/Debug/GNU-Linux-x86 -L../../3Party/SmallJpeg/Debug/GNU-Linux-x86 -lSmallJpeg -Wl,-rpath ../../3Party/PowerVR/Debug/GNU-Linux-x86 -L../../3Party/PowerVR/Debug/GNU-Linux-x86 -lPowerVR -lGL -lGLU -lGLEW -lpng

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk Debug/${PLATFORM}/libRender.so

Debug/${PLATFORM}/libRender.so: ../Core/Debug/GNU-Linux-x86/libCore.so

Debug/${PLATFORM}/libRender.so: ../../3Party/SmallJpeg/Debug/GNU-Linux-x86/libSmallJpeg.so

Debug/${PLATFORM}/libRender.so: ../../3Party/PowerVR/Debug/GNU-Linux-x86/libPowerVR.so

Debug/${PLATFORM}/libRender.so: ${OBJECTFILES}
	${MKDIR} -p Debug/${PLATFORM}
	${LINK.cc} -shared -o Debug/${PLATFORM}/libRender.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/Components/CameraComponent.o: Components/CameraComponent.cpp 
	${MKDIR} -p ${OBJECTDIR}/Components
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Components/CameraComponent.o Components/CameraComponent.cpp

${OBJECTDIR}/TgaLoader.o: TgaLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TgaLoader.o TgaLoader.cpp

${OBJECTDIR}/Color.o: Color.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Color.o Color.cpp

${OBJECTDIR}/Shader.o: Shader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Shader.o Shader.cpp

${OBJECTDIR}/Components/PickComponent.o: Components/PickComponent.cpp 
	${MKDIR} -p ${OBJECTDIR}/Components
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Components/PickComponent.o Components/PickComponent.cpp

${OBJECTDIR}/BackRenderBuffer.o: BackRenderBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/BackRenderBuffer.o BackRenderBuffer.cpp

${OBJECTDIR}/MeshBuilder.o: MeshBuilder.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/MeshBuilder.o MeshBuilder.cpp

${OBJECTDIR}/ModelPod.o: ModelPod.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ModelPod.o ModelPod.cpp

${OBJECTDIR}/Components/MeshComponent.o: Components/MeshComponent.cpp 
	${MKDIR} -p ${OBJECTDIR}/Components
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Components/MeshComponent.o Components/MeshComponent.cpp

${OBJECTDIR}/ProjectiveTexture.o: ProjectiveTexture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ProjectiveTexture.o ProjectiveTexture.cpp

${OBJECTDIR}/RenderBuffer.o: RenderBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/RenderBuffer.o RenderBuffer.cpp

${OBJECTDIR}/Texture.o: Texture.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Texture.o Texture.cpp

${OBJECTDIR}/ShaderLoader.o: ShaderLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ShaderLoader.o ShaderLoader.cpp

${OBJECTDIR}/RenderTarget.o: RenderTarget.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/RenderTarget.o RenderTarget.cpp

${OBJECTDIR}/Max3dsLoader.o: Max3dsLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Max3dsLoader.o Max3dsLoader.cpp

${OBJECTDIR}/Components/RenderableComponent.o: Components/RenderableComponent.cpp 
	${MKDIR} -p ${OBJECTDIR}/Components
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Components/RenderableComponent.o Components/RenderableComponent.cpp

${OBJECTDIR}/MaterialProperty.o: MaterialProperty.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/MaterialProperty.o MaterialProperty.cpp

${OBJECTDIR}/Camera.o: Camera.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Camera.o Camera.cpp

${OBJECTDIR}/TextureLoaderBase.o: TextureLoaderBase.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TextureLoaderBase.o TextureLoaderBase.cpp

${OBJECTDIR}/Model.o: Model.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Model.o Model.cpp

${OBJECTDIR}/Mesh.o: Mesh.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Mesh.o Mesh.cpp

${OBJECTDIR}/ResourceLoaderFactory.o: ResourceLoaderFactory.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ResourceLoaderFactory.o ResourceLoaderFactory.cpp

${OBJECTDIR}/Frustum.o: Frustum.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Frustum.o Frustum.cpp

${OBJECTDIR}/GlWindow.o: GlWindow.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/GlWindow.o GlWindow.cpp

${OBJECTDIR}/JpegLoader.o: JpegLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/JpegLoader.o JpegLoader.cpp

${OBJECTDIR}/EffectLoader.o: EffectLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/EffectLoader.o EffectLoader.cpp

${OBJECTDIR}/DdsLoader.o: DdsLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/DdsLoader.o DdsLoader.cpp

${OBJECTDIR}/Effect.o: Effect.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Effect.o Effect.cpp

${OBJECTDIR}/BitmapLoader.o: BitmapLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/BitmapLoader.o BitmapLoader.cpp

${OBJECTDIR}/TextureRenderBuffer.o: TextureRenderBuffer.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TextureRenderBuffer.o TextureRenderBuffer.cpp

${OBJECTDIR}/Material.o: Material.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Material.o Material.cpp

${OBJECTDIR}/PngLoader.o: PngLoader.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PngLoader.o PngLoader.cpp

${OBJECTDIR}/Pch.o: Pch.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Pch.o Pch.cpp

${OBJECTDIR}/ShaderProgram.o: ShaderProgram.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ShaderProgram.o ShaderProgram.cpp

# Subprojects
.build-subprojects:
	cd ../Core && ${MAKE}  -f Makefile CONF=Debug
	cd ../../3Party/SmallJpeg && ${MAKE}  -f Makefile CONF=Debug
	cd ../../3Party/PowerVR && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} Debug/${PLATFORM}/libRender.so

# Subprojects
.clean-subprojects:
	cd ../Core && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../3Party/SmallJpeg && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../3Party/PowerVR && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
