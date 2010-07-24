class DerivedFoo extends Foo
{
	// Note that if we have defined a custom constructor
	// for the derived class, we MUST also call the base's
	// constructor, otherwise the program may CRASH.
	constructor() {
		::Foo.constructor();
	}
}

local f = DerivedFoo();
local b = Bar();

f.addBar(b);
assert(f.getBar() == b);

assert(f instanceof Foo);
assert(f instanceof DerivedFoo);
