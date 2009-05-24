function println(str) {
	print(str);
	print("\n");
}

function createDestroyTest()
{
	local foo = Foo();
}

function referenceTest()
{
	local foo = Foo();
	local reference = foo;
}

// If the cpp object is destroyed, all reference will become null
function cppDestroyTest()
{
	{	local foo = Foo();
		foo.destroy();
		assert(!foo);
	}

	{	local foo = Foo();
		local reference = foo;
		foo.destroy();
		assert(!foo);
		assert(!reference);
	}
}

function returnSameReferenceTest()
{
	local foo = Foo();
	local bar = Bar();
	foo.addBar(bar);
	assert(bar == foo.getBar());
	assert(foo.getBar() == foo.getBar());
	bar = null;
	assert(foo.getBar() == foo.getBar());
}

function compositionTest()
{
	{	local foo = Foo();
		local bar = Bar();
		foo.addBar(bar);
	}

	{	// Give ownership from script to C++
		local foo = Foo();
		local bar = Bar();	// Script hold the ownership of bar
		foo.addBar(bar);	// bar transfer it's ownership from script to C++
		bar.printMe();
		foo = null;			// Foo is destroyed and bar is also destroy by C++
		assert(bar == null);// All reference to bar will become null
	}

	{	local foo = Foo();
		local bar = Bar();
		foo.addBar(bar);
		bar = null;	// Nothing in the script will have linkage with bar anymore
	}

	{	local foo = Foo();
		local bar = Bar();
		foo.addBar(bar);
		bar = null;	// Nothing in the script will have linkage with bar anymore
		local bar2 = foo.getBar();	// But we can get back bar from foo
		bar2.printMe();
		assert(bar == null);	// The old reference of bar should keep null
		assert(bar2 == foo.getBar());
		foo = null;				// At this point, the Cpp foo object may stil alive
		collectgarbage();		// Even calling collectgarbage() didn't help... may I suggest something to the squirrel author?
//		assert(null == bar2);	//
		print("");				// But calling print did help
		assert(null == bar2);
	}
}

createDestroyTest();
referenceTest();
cppDestroyTest();
returnSameReferenceTest();
compositionTest();
