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
	${OBJECTDIR}/System/UtilityTest.o \
	${OBJECTDIR}/Entity/EntityTest.o \
	${OBJECTDIR}/System/ResourceLoaderTest.o \
	${OBJECTDIR}/Math/Vec3Test.o \
	${OBJECTDIR}/System/SharedPtrTest.o \
	${OBJECTDIR}/System/IntrusivePtrTest.o \
	${OBJECTDIR}/Math/Mat33Test.o \
	${OBJECTDIR}/System/TimerTest.o \
	${OBJECTDIR}/Math/IntersectionTest.o \
	${OBJECTDIR}/Math/TupleTest.o \
	${OBJECTDIR}/System/ResourceManagerTest.o \
	${OBJECTDIR}/System/XmlParserTest.o \
	${OBJECTDIR}/System/WindowTest.o \
	${OBJECTDIR}/System/StreamTest.o \
	${OBJECTDIR}/Math/Vec2Test.o \
	${OBJECTDIR}/System/StrUtilityTest.o \
	${OBJECTDIR}/System/TaskPoolTest.o \
	${OBJECTDIR}/Math/Mat44Test.o \
	${OBJECTDIR}/Entity/ComponentTest.o \
	${OBJECTDIR}/System/UserDataTest.o \
	${OBJECTDIR}/Math/QuaternionTest.o \
	${OBJECTDIR}/System/LinkListTest.o \
	${OBJECTDIR}/System/CondVarTest.o \
	${OBJECTDIR}/System/LogTest.o \
	${OBJECTDIR}/System/ThreadPoolTest.o \
	${OBJECTDIR}/System/PtrVectorTest.o \
	${OBJECTDIR}/System/FileSystemTest.o \
	${OBJECTDIR}/System/ThreadTest.o \
	${OBJECTDIR}/Main.o \
	${OBJECTDIR}/System/MapTest.o \
	${OBJECTDIR}/System/PathTest.o \
	${OBJECTDIR}/System/WeakPtrTest.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=../../3Party/CppTestHarness/../../Lib/libcpptestharnessd.a -Wl,-rpath ../../MCD/Core/Debug/GNU-Linux-x86 -L../../MCD/Core/Debug/GNU-Linux-x86 -lCore

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Debug.mk Debug/${PLATFORM}/coretest

Debug/${PLATFORM}/coretest: ../../3Party/CppTestHarness/../../Lib/libcpptestharnessd.a

Debug/${PLATFORM}/coretest: ../../MCD/Core/Debug/GNU-Linux-x86/libCore.so

Debug/${PLATFORM}/coretest: ${OBJECTFILES}
	${MKDIR} -p Debug/${PLATFORM}
	${LINK.cc} -o Debug/${PLATFORM}/coretest ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/System/UtilityTest.o: System/UtilityTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/UtilityTest.o System/UtilityTest.cpp

${OBJECTDIR}/Entity/EntityTest.o: Entity/EntityTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/Entity
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Entity/EntityTest.o Entity/EntityTest.cpp

${OBJECTDIR}/System/ResourceLoaderTest.o: System/ResourceLoaderTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ResourceLoaderTest.o System/ResourceLoaderTest.cpp

${OBJECTDIR}/Math/Vec3Test.o: Math/Vec3Test.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/Vec3Test.o Math/Vec3Test.cpp

${OBJECTDIR}/System/SharedPtrTest.o: System/SharedPtrTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/SharedPtrTest.o System/SharedPtrTest.cpp

${OBJECTDIR}/System/IntrusivePtrTest.o: System/IntrusivePtrTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/IntrusivePtrTest.o System/IntrusivePtrTest.cpp

${OBJECTDIR}/Math/Mat33Test.o: Math/Mat33Test.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/Mat33Test.o Math/Mat33Test.cpp

${OBJECTDIR}/System/TimerTest.o: System/TimerTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/TimerTest.o System/TimerTest.cpp

${OBJECTDIR}/Math/IntersectionTest.o: Math/IntersectionTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/IntersectionTest.o Math/IntersectionTest.cpp

${OBJECTDIR}/Math/TupleTest.o: Math/TupleTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/TupleTest.o Math/TupleTest.cpp

${OBJECTDIR}/System/ResourceManagerTest.o: System/ResourceManagerTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ResourceManagerTest.o System/ResourceManagerTest.cpp

${OBJECTDIR}/System/XmlParserTest.o: System/XmlParserTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/XmlParserTest.o System/XmlParserTest.cpp

${OBJECTDIR}/System/WindowTest.o: System/WindowTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/WindowTest.o System/WindowTest.cpp

${OBJECTDIR}/System/StreamTest.o: System/StreamTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/StreamTest.o System/StreamTest.cpp

${OBJECTDIR}/Math/Vec2Test.o: Math/Vec2Test.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/Vec2Test.o Math/Vec2Test.cpp

${OBJECTDIR}/System/StrUtilityTest.o: System/StrUtilityTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/StrUtilityTest.o System/StrUtilityTest.cpp

${OBJECTDIR}/System/TaskPoolTest.o: System/TaskPoolTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/TaskPoolTest.o System/TaskPoolTest.cpp

${OBJECTDIR}/Math/Mat44Test.o: Math/Mat44Test.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/Mat44Test.o Math/Mat44Test.cpp

${OBJECTDIR}/Entity/ComponentTest.o: Entity/ComponentTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/Entity
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Entity/ComponentTest.o Entity/ComponentTest.cpp

${OBJECTDIR}/System/UserDataTest.o: System/UserDataTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/UserDataTest.o System/UserDataTest.cpp

${OBJECTDIR}/Math/QuaternionTest.o: Math/QuaternionTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/Math
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Math/QuaternionTest.o Math/QuaternionTest.cpp

${OBJECTDIR}/System/LinkListTest.o: System/LinkListTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/LinkListTest.o System/LinkListTest.cpp

${OBJECTDIR}/System/CondVarTest.o: System/CondVarTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/CondVarTest.o System/CondVarTest.cpp

${OBJECTDIR}/System/LogTest.o: System/LogTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/LogTest.o System/LogTest.cpp

${OBJECTDIR}/System/ThreadPoolTest.o: System/ThreadPoolTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ThreadPoolTest.o System/ThreadPoolTest.cpp

${OBJECTDIR}/System/PtrVectorTest.o: System/PtrVectorTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/PtrVectorTest.o System/PtrVectorTest.cpp

${OBJECTDIR}/System/FileSystemTest.o: System/FileSystemTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/FileSystemTest.o System/FileSystemTest.cpp

${OBJECTDIR}/System/ThreadTest.o: System/ThreadTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/ThreadTest.o System/ThreadTest.cpp

${OBJECTDIR}/Main.o: Main.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/Main.o Main.cpp

${OBJECTDIR}/System/MapTest.o: System/MapTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/MapTest.o System/MapTest.cpp

${OBJECTDIR}/System/PathTest.o: System/PathTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/PathTest.o System/PathTest.cpp

${OBJECTDIR}/System/WeakPtrTest.o: System/WeakPtrTest.cpp 
	${MKDIR} -p ${OBJECTDIR}/System
	${RM} $@.d
	$(COMPILE.cc) -g -Wall -MMD -MP -MF $@.d -o ${OBJECTDIR}/System/WeakPtrTest.o System/WeakPtrTest.cpp

# Subprojects
.build-subprojects:
	cd ../../3Party/CppTestHarness && ${MAKE}  -f Makefile CONF=Debug
	cd ../../MCD/Core && ${MAKE}  -f Makefile CONF=Debug

# Clean Targets
.clean-conf: ${CLEAN_SUBPROJECTS}
	${RM} -r build/Debug
	${RM} Debug/${PLATFORM}/coretest

# Subprojects
.clean-subprojects:
	cd ../../3Party/CppTestHarness && ${MAKE}  -f Makefile CONF=Debug clean
	cd ../../MCD/Core && ${MAKE}  -f Makefile CONF=Debug clean

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
