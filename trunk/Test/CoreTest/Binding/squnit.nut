/*!	squnit.nut
	Description: A unit testing framework for the squirrel language
	Homepage: http://mtlung.blogspot.com/2009/05/squirrel.html
	Author: Ricky Lung (http://mtlung.blogspot.com/)
	Version: 1.0
	The framework is base on luaunit v1.3 http://phil.freehackers.org/luaunit/

	License: X11 License

	This set of files is published under the X11 License. You can do
	more or less anything you want to do with it.

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE X
	CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
	ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
	WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*!	Turns a table into a human readable string.
	\ref http://squirrel-lang.org/forums/thread/477.aspx
 */
function tprint(table, margin=4, tabwidth=4)
{
	local indent = function(n)
	{
		local ret = "";
		for(local i=0; i<n; ++i)
			ret += " ";
		return ret;
	}

	local ret = "";
	foreach(key, value in table)
	{
		ret += indent(margin);
		ret += key;
		switch (type(value))
		{
			case "table":
				ret += " = {\n";
				ret += tprint(value, margin + tabwidth, tabwidth);
				ret += indent(margin);
				ret += "}";
				break
			case "array":
				ret += " = [\n";
				ret += tprint(value, margin + tabwidth, tabwidth);
				ret += indent(margin);
				ret += "]";
				break
			case "string":
				ret += " = \"";
				ret += value;
				ret += "\"";
				break;
			default:
				ret += " = ";
				ret += value;
				break;
		}
		ret += "\n";
	}

	return ret;
}

/*!	Raise an error with message and stack trace.
	\ref http://www.lua.org/manual/5.1/manual.html#pdf-error
 */
function error(message, level=1)
{
	// In some cases calling tprint will crash the VM because of null pointer dereference
	message = message;// + "\nstack trace:\n" + tprint(getstackinfos(level));
	throw message;
}

/*!	Assert that calling f with the arguments will raise an error.
	Example: assertError(f, obj, 1, 2) => obj.f(1, 2) should generate an error.

	\note
		If you think this function is not working as you expected, please ensure
		you have pass the "this" object right after the function. If the function is
		a global/static one, pass a null.
 */
function assertError(f, ...)
{
	// Since vargv cannot directly pass to acall, we need to create an array of parameters
	local args = array(0);
	for(local i=0; i<vargc; ++i)
		args.push(vargv[i]);

	try {
		f.acall(args);
	} catch(e) {
		//print(e + "\n");
		return;
	}

	throw("No error generated");
}

//! Assert that two values are equal and calls error else.
function assertEquals(actual, expected)
{
//	actual, expected = expected, actual;
	local tmp = actual;
	actual = expected;
	expected = tmp;

	if(actual != expected)
	{
		local errorMsg;
		if(type(expected) == "string")
			errorMsg = "\nexpected: " + expected + "\n" + "actual  : " + actual + "\n";
		else
			errorMsg = "expected: " + expected + ", actual: " + actual;

		throw(errorMsg);
	}
}

//! Assert that two values are close enough and calls error else.
function assertClose(actual, expected, tolerance=1.e-6)
{
	local diff = actual.tofloat() - expected.tofloat();
	local positiveTolerance = (tolerance >= 0) ? tolerance : -tolerance;

	if(diff > positiveTolerance || diff < -positiveTolerance)
		assertEquals(actual, expected);
}

/*!	Use me to wrap a set of functions into a Runnable test class:
	function f1() {}
	function f2() {}
	TestToto = wrapFunctions("f1", "f2");
	Now, TestToto will be picked up by SqUnit().run();
 */
function wrapFunctions(...)
{
	local testClass = {};
	for(local i=0; i<vargc; ++i) {
		if(typeof vargv[i] != "string" || !(vargv[i] in getroottable()))
			continue;
		testClass.rawset(vargv[i], getroottable().rawget(vargv[i]));
	}
	return testClass;
}

class UnitResult
{
	failureCount = 0;
	testCount = 0;
	errorList = {};
	currentClassName = "";
	currentTestName = "";
	testHasFailure = false;
	verbosity = 0;

	static function println(str)
	{
		print(str);
		print("\n");
	}

	function displayClassName() {
		println(">>>>>>>>> " + currentClassName);
	}

	function displayTestName()
	{
		if(verbosity > 0)
			println(">>> " + currentTestName);
	}

	function displayFailure(errorMsg)
	{
		if(verbosity == 0)
			print("F");
		else {
			println(errorMsg);
			println("Failed");
		}
	}

	function displaySuccess()
	{
		if(verbosity > 0)
			println("Ok");
		else
			print(".");
	}

	function displayOneFailedTest(testName, errorMsg)
	{
		println(">>> " + testName + " failed");
		println(errorMsg);
	}

	function displayFailedTests()
	{
		if(errorList.len() == 0)
			return;
		println("Failed tests:");
		println("-------------");
		foreach(currentTestName, errorMsg in errorList)
			displayOneFailedTest(currentTestName, errorMsg);
		println("");
	}

	function displayFinalResult()
	{
		println("=========================================================");
		displayFailedTests();
		local failurePercent, successCount;
		if(testCount == 0)
			failurePercent = 0;
		else
			failurePercent = 100 * failureCount / testCount;

		successCount = testCount - failureCount;
		println( format("Success : %d%% - %d / %d", 100-ceil(failurePercent), successCount, testCount) );
		return failureCount;
	}

	function startClass(className)
	{
		currentClassName = className;
		displayClassName();
	}

	function startTest(testName)
	{
		currentTestName = testName;
		displayTestName();
		testCount = testCount + 1;
		testHasFailure = false;
	}

	function addFailure(errorMsg)
	{
		failureCount = failureCount + 1;
		testHasFailure = true;
		// Use rawset to prevent currentTestName being evaluated
		errorList.rawset(currentTestName, errorMsg);
		displayFailure(errorMsg);
	}

	function endTest()
	{
		if(!testHasFailure)
			displaySuccess();
	}
}	// UnitResult

class SqUnit
{
	static result = UnitResult();

	static println = UnitResult.println;

	constructor(verbosity=0) {
		result.verbosity = verbosity;
	}

	function isFunction(aObject) {
		return "function" == typeof aObject;
	}

	//! Example: runTestMethod("TestToto.test1", TestToto, TestToto.testToto(this))
	function runTestMethod(aName, aClassInstance, aMethod)
	{
		local ok, errorMsg;
		result.startTest(aName);

		// Create an object instance of the class
		if(typeof aClassInstance == "class")
			aClassInstance = aClassInstance();

		// Run setUp first(if any)
		if("setUp" in aClassInstance && isFunction(aClassInstance.setUp))
			aClassInstance.setUp();

		// Run testMethod()
		try {
			aMethod.call(aClassInstance);
		} catch(errorMsg) {
			result.addFailure(errorMsg);
		}

		// Lastly, run tearDown(if any)
		if("tearDown" in aClassInstance && isFunction(aClassInstance.tearDown))
			 aClassInstance.tearDown();

		result.endTest();
	}

	//! Example: runTestMethodName("TestToto.testToto", TestToto)
	function runTestMethodName(methodName, classInstance)
	{
		local methodInstance = compilestring(methodName + "();");
		runTestMethod(methodName, classInstance, methodInstance);
	}

	//! Example: runTestMethodName("TestToto")
	function runTestClassByName(aClassName)
	{
		local hasMethod, methodName, classInstance;

		hasMethod = aClassName.find(".");
		if(hasMethod) {
			methodName = aClassName.slice(hasMethod+1);
			aClassName = aClassName.slice(0, hasMethod);
		}

		if(aClassName in getroottable())
			classInstance = getroottable()[aClassName];
		else
			println("No such class: " + aClassName);

		result.startClass(aClassName);

		if(hasMethod) {
			if(!(methodName in classInstance))
				println("No such method: " + methodName);
			runTestMethodName(aClassName + "." + methodName, classInstance);
		}
		else
		{
			// Run all test methods of the class
			foreach(methodName, method in classInstance)
			{
				if(isFunction(method) && methodName.len() >= 4 && methodName.slice(0, 4) == "test")
					runTestMethodName(aClassName + "." + methodName, classInstance);
			}
		}
		println("");
	}

	/*!	Run some specific test classes.
		If no arguments are passed, run the class names specified on the
		command line. If no class name is specified on the command line
		run all classes whose name starts with "Test"
	 */
	function run(...)
	{
		// If arguments are passed, they must be strings of the class names that you want to run
		if(vargc > 0)
		{
			for(local i=0; i<vargc; ++i)
				runTestClassByName(vargv[i]);
		}
		else
		{
			// Create the list before. If you do not do it now, you get
			// undefined result because you modify _G while iterating over it.
			local testClassList = {}
			foreach(key, val in getroottable()) {
				if(key.len() >= 4 && key.slice(0, 4) == "Test")
					testClassList.rawset(key, val);
			}
			foreach(key, val in testClassList)
				runTestClassByName(key);
		}

		return result.displayFinalResult();
	}
}	// SqUnit
