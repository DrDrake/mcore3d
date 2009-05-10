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
	${OBJECTDIR}/TestLauncher.o \
	${OBJECTDIR}/PrintfTestReporter.o \
	${OBJECTDIR}/Test.o \
	${OBJECTDIR}/AssertException.o \
	${OBJECTDIR}/ReportAssert.o \
	${OBJECTDIR}/TestRunner.o \
	${OBJECTDIR}/TestResults.o \
	${OBJECTDIR}/TestReporter.o \
	${OBJECTDIR}/Checks.o \
	${OBJECTDIR}/HTMLTestReporter.o

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
	${MAKE}  -f nbproject/Makefile-Release.mk Release/${PLATFORM}/libCppTestHarness.so

Release/${PLATFORM}/libCppTestHarness.so: ${OBJECTFILES}
	${MKDIR} -p Release/${PLATFORM}
	${LINK.cc} -shared -o Release/${PLATFORM}/libCppTestHarness.so -fPIC ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/TestLauncher.o: TestLauncher.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TestLauncher.o TestLauncher.cpp

${OBJECTDIR}/PrintfTestReporter.o: PrintfTestReporter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/PrintfTestReporter.o PrintfTestReporter.cpp

${OBJECTDIR}/Test.o: Test.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Test.o Test.cpp

${OBJECTDIR}/AssertException.o: AssertException.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/AssertException.o AssertException.cpp

${OBJECTDIR}/ReportAssert.o: ReportAssert.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/ReportAssert.o ReportAssert.cpp

${OBJECTDIR}/TestRunner.o: TestRunner.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TestRunner.o TestRunner.cpp

${OBJECTDIR}/TestResults.o: TestResults.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TestResults.o TestResults.cpp

${OBJECTDIR}/TestReporter.o: TestReporter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/TestReporter.o TestReporter.cpp

${OBJECTDIR}/Checks.o: Checks.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/Checks.o Checks.cpp

${OBJECTDIR}/HTMLTestReporter.o: HTMLTestReporter.cpp 
	${MKDIR} -p ${OBJECTDIR}
	${RM} $@.d
	$(COMPILE.cc) -O2 -fPIC  -MMD -MP -MF $@.d -o ${OBJECTDIR}/HTMLTestReporter.o HTMLTestReporter.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Release
	${RM} Release/${PLATFORM}/libCppTestHarness.so

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
