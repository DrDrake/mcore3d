#include "PrintfTestReporter.h"
#include <cstdio>

namespace CppTestHarness {

void PrintfTestReporter::ReportFailure(char const* file, size_t const line, std::string const failure) {
	printf("%s(%d) : failure: %s\n", file, line, failure.c_str());
}

void PrintfTestReporter::ReportSingleResult(const std::string& /*testName*/, bool /*failed*/) {
	//empty
}

void PrintfTestReporter::ReportSummary(size_t const testCount, size_t const failureCount) {
	printf("%d tests run.\n", testCount);
	printf("%d failures.\n", failureCount);
}

}
