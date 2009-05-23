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

function compositionTest()
{
	{	local foo = Foo();
		local bar = Bar();
		foo.addBar(bar);
	}

	{	// Give ownership from script to C++
		local foo = Foo();
		local bar = Bar();	// Script hold the ownership
		foo.addBar(bar);	// Script transfer ownership to C++
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
		print(bar)
		foo.addBar(bar);
		bar = null;	// Nothing in the script will have linkage with bar anymore
		local bar2 = foo.getBar();	// But we can get back bar from foo
		bar2.printMe();
		print(bar2);
		foo = null;
		assert(bar2 == null);
	}
}

createDestroyTest();
referenceTest();
cppDestroyTest();
compositionTest();
