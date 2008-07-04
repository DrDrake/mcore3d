#ifndef TEST_MACROS_H
#define TEST_MACROS_H

//----------------------------
#define TEST(Name) \
	class Test##Name : public CppTestHarness::Test \
	{ \
	public: \
		Test##Name() : Test(#Name, __FILE__, __LINE__) {} \
	private: \
		virtual void RunImpl(CppTestHarness::TestResults& testResults_); \
	}; \
	CppTestHarness::TypedTestLauncher< Test##Name > \
		staticInitTest##Name##Creator(#Name); \
	\
	void Test##Name::RunImpl(CppTestHarness::TestResults& testResults_)

//----------------------------
#define TEST_FIXTURE(Fixture, Name) \
	class Test##Name : public CppTestHarness::Test, public Fixture \
	{ \
	public: \
		Test##Name() : Test(#Fixture "::" #Name, __FILE__, __LINE__) {} \
	private: \
		virtual void RunImpl(CppTestHarness::TestResults& testResults_); \
	}; \
	CppTestHarness::TypedTestLauncher< Test##Name > \
		staticInitTest##Name##Creator(#Fixture "::" #Name); \
	\
	void Test##Name::RunImpl(CppTestHarness::TestResults& testResults_)

//----------------------------
#define TEST_FIXTURE_CTOR(Fixture, CtorParams, Name) \
	class Test##Name : public CppTestHarness::Test, public Fixture \
	{ \
	public: \
		Test##Name() : Test(#Fixture "::" #Name, __FILE__, __LINE__), Fixture CtorParams {} \
	private: \
		virtual void RunImpl(CppTestHarness::TestResults& testResults_); \
	}; \
	CppTestHarness::TypedTestLauncher< Test##Name > \
		staticInitTest##Name##Creator(#Fixture "::" #Name); \
	\
	void Test##Name::RunImpl(CppTestHarness::TestResults& testResults_)

//----------------------------
#define TEST_FIXTURE_EX(Fixture, Name) \
	class Test##Name : public CppTestHarness::Test, public Fixture \
	{ \
	public: \
		Test##Name() : Test(#Fixture "::" #Name, __FILE__, __LINE__) {} \
	private: \
		virtual void RunImpl(CppTestHarness::TestResults& testResults_); \
	}; \
	CppTestHarness::TypedTestLauncherEx< Test##Name > \
		staticInitTest##Name##Creator(#Fixture "::" #Name); \
	\
	void Test##Name::RunImpl(CppTestHarness::TestResults& testResults_)

#endif

