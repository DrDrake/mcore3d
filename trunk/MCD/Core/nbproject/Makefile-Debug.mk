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
	${OBJECTDIR}/System/CondVar.o \
	${OBJECTDIR}/System/XmlParser.o \
	${OBJECTDIR}/System/CallstackProfiler.o \
	${OBJECTDIR}/Entity/Entity.o \
	${OBJECTDIR}/System/Path.o \
	${OBJECTDIR}/System/Timer.o \
	${OBJECTDIR}/System/ResourceManager.o \
	${OBJECTDIR}/System/ThreadPool.o \
	${OBJECTDIR}/System/Window.o \
	${OBJECTDIR}/System/FileSystemCollection.o \
	${OBJECTDIR}/System/Thread.o \
	${OBJECTDIR}/System/ErrorCode.o \
	${OBJECTDIR}/System/StrUtility.o \
	${OBJECTDIR}/Entity/BehaviourComponent.o \
	${OBJECTDIR}/System/Resource.o \
	${OBJECTDIR}/System/ZipFileSystem.o \
	${OBJECTDIR}/System/RawFileSystemMonitor.o \
	${OBJECTDIR}/System/ScriptOwnershipHandle.o \
	${OBJECTDIR}/System/RawFileSystem.o \
	${OBJECTDIR}/System/Exception.o \
	${OBJECTDIR}/System/Stream.o \
	${OBJECTDIR}/System/TaskPool.o \
	${OBJECTDIR}/System/Log.o \
	${OBJECTDIR}/System/Map.o \
	${OBJECTDIR}/System/LinkList.o \
	${OBJECTDIR}/Math/SrtTransform.o \
	${OBJECTDIR}/System/MemoryProfiler.o \
	${OBJECTDIR}/System/Utility.o \
	${OBJECTDIR}/System/Mutex.o \
	${OBJECTDIR}/Entity/Component.o \
	${OBJECTDIR}/System/CpuProfiler.o \
	${OBJECTDIR}/Math/Intersection.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=-Wl,-rpath ../../3Party/squirrel/dist/Debug/GNU-Linux-x86 -L../../3Party/squirrel/dist/Debug/GNU-Linux-x86 -lsquirrel -lpthread -lz -lX11

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk Debug/${PLATFORM}/libCore.so

Debug/${PLATFORM}/libCore.so: ../../3Party/squirrel/dist/Debug/GNU-Linux-x86/libsquirrel.so

Debug/${PLATFORM}/libCore.so: ${OBJECTFILES}
	${MKDIR} -p Debug/${PLATFORM}
	${LINK.cc} -shared -o Debug/${PLATFORM}/libCore.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/System/CondVar.o: System/CondVar.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/CondVar.o System/CondVar.cpp

${OBJECTDIR}/System/XmlParser.o: System/XmlParser.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/XmlParser.o System/XmlParser.cpp

${OBJECTDIR}/System/CallstackProfiler.o: System/CallstackProfiler.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/CallstackProfiler.o System/CallstackProfiler.cpp

${OBJECTDIR}/Entity/Entity.o: Entity/Entity.cpp 
	${MKDIR} -p ${OBJECTDIR}/Entity
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Entity/Entity.o Entity/Entity.cpp

${OBJECTDIR}/System/Path.o: System/Path.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Path.o System/Path.cpp

${OBJECTDIR}/System/Timer.o: System/Timer.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Timer.o System/Timer.cpp

${OBJECTDIR}/System/ResourceManager.o: System/ResourceManager.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ResourceManager.o System/ResourceManager.cpp

${OBJECTDIR}/System/ThreadPool.o: System/ThreadPool.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ThreadPool.o System/ThreadPool.cpp

${OBJECTDIR}/System/Window.o: System/Window.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Window.o System/Window.cpp

${OBJECTDIR}/System/FileSystemCollection.o: System/FileSystemCollection.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/FileSystemCollection.o System/FileSystemCollection.cpp

${OBJECTDIR}/System/Thread.o: System/Thread.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Thread.o System/Thread.cpp

${OBJECTDIR}/System/ErrorCode.o: System/ErrorCode.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ErrorCode.o System/ErrorCode.cpp

${OBJECTDIR}/System/StrUtility.o: System/StrUtility.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/StrUtility.o System/StrUtility.cpp

${OBJECTDIR}/Entity/BehaviourComponent.o: Entity/BehaviourComponent.cpp 
	${MKDIR} -p ${OBJECTDIR}/Entity
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Entity/BehaviourComponent.o Entity/BehaviourComponent.cpp

${OBJECTDIR}/System/Resource.o: System/Resource.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Resource.o System/Resource.cpp

${OBJECTDIR}/System/ZipFileSystem.o: System/ZipFileSystem.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ZipFileSystem.o System/ZipFileSystem.cpp

${OBJECTDIR}/System/RawFileSystemMonitor.o: System/RawFileSystemMonitor.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/RawFileSystemMonitor.o System/RawFileSystemMonitor.cpp

${OBJECTDIR}/System/ScriptOwnershipHandle.o: System/ScriptOwnershipHandle.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ScriptOwnershipHandle.o System/ScriptOwnershipHandle.cpp

${OBJECTDIR}/System/RawFileSystem.o: System/RawFileSystem.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/RawFileSystem.o System/RawFileSystem.cpp

${OBJECTDIR}/System/Exception.o: System/Exception.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Exception.o System/Exception.cpp

${OBJECTDIR}/System/Stream.o: System/Stream.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Stream.o System/Stream.cpp

${OBJECTDIR}/System/TaskPool.o: System/TaskPool.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/TaskPool.o System/TaskPool.cpp

${OBJECTDIR}/System/Log.o: System/Log.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Log.o System/Log.cpp

${OBJECTDIR}/System/Map.o: System/Map.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Map.o System/Map.cpp

${OBJECTDIR}/System/LinkList.o: System/LinkList.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/LinkList.o System/LinkList.cpp

${OBJECTDIR}/Math/SrtTransform.o: Math/SrtTransform.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/SrtTransform.o Math/SrtTransform.cpp

${OBJECTDIR}/System/MemoryProfiler.o: System/MemoryProfiler.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/MemoryProfiler.o System/MemoryProfiler.cpp

${OBJECTDIR}/System/Utility.o: System/Utility.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Utility.o System/Utility.cpp

${OBJECTDIR}/System/Mutex.o: System/Mutex.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/Mutex.o System/Mutex.cpp

${OBJECTDIR}/Entity/Component.o: Entity/Component.cpp 
	${MKDIR} -p ${OBJECTDIR}/Entity
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Entity/Component.o Entity/Component.cpp

${OBJECTDIR}/System/CpuProfiler.o: System/CpuProfiler.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/CpuProfiler.o System/CpuProfiler.cpp

${OBJECTDIR}/Math/Intersection.o: Math/Intersection.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/Intersection.o Math/Intersection.cpp

# Subprojects
.build-subprojects:
	cd ../../3Party/squirrel && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} Debug/${PLATFORM}/libCore.so

# Subprojects
.clean-subprojects:
	cd ../../3Party/squirrel && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
