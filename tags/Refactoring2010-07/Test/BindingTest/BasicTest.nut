dofile("squnit.nut", true);

function println(str) {
	print(str);
	print("\n");
}

class TestBasicBinding
{
	function testCreateDestroy()
	{
		local foo = Foo();
		assert(foo instanceof Foo);
	}

	function testTypeof()
	{
		local foo = Foo();
		assert(typeof foo == "Foo");
	}

	function testReference()
	{
		local foo = Foo();
		local reference = foo;
	}

	// If the cpp object is destroyed, all reference will become null
	function testCppDestroy()
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

	function testReturnSameReference()
	{
		local foo = Foo();
		local bar = Bar();
		foo.addBar(bar);
		assert(bar == foo.getBar());
		assert(foo.getBar() == foo.getBar());
		bar = null;
		assert(foo.getBar() == foo.getBar());
	}

	function testComposition()
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
			foo.acceptNullBar(null);
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
//			collectgarbage();		// Even calling collectgarbage() didn't help... may I suggest something to the squirrel author?
//			assert(null == bar2);	//
			print("");				// But calling print did help
			assert(null == bar2);
		}
	}

	function testDestroyedCppObjInTable()
	{
		local foo = Foo();
		local bar = Bar();
		foo.addBar(bar);

		local t = {};
		t[bar] <- "haha";

		assert(t[bar] == "haha");
		assert(t.len() == 1);

		foo = null;	// bar will also deleted
		assert(null == bar);
		assert(t.len() == 1);

		// TODO: It's a know problem that a squirrel table key becomming null by
		// destruction from C++ will resulting a non-removable entry in the table.
		foreach(key, value in t) {
			assert(key == null);
			assert(value == "haha");
		}

		assert(t.len() == 1);
	}

	//! Any experimental feature
	function testPropertyConstructor()
	{
		local TestClass = class
		{
			constructor() { x = 0; }
			x = null;
			y = null;
		};

		//! Take a class as an input and transform it's constructor
		local makePropertyConstructor = function(yourClass)
		{
			local backup = yourClass.constructor;
			yourClass.constructor <- function(nameParamPairs) : (backup)
			{
				backup.call(this);	// Call the original constructor
				foreach(name, param in nameParamPairs) {
					if(name in this)
						this[name] = param;
				}
			}
		}

		makePropertyConstructor(TestClass);

		local f = TestClass({x=1.5, y=2.6});
		assertEquals(1.5, f.x);
		assertEquals(2.6, f.y);
	}
}

SqUnit().run();
