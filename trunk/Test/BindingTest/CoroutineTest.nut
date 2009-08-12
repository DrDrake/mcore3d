timer <- Timer();

// A simple implementation of sleep()
function sleep(seconds) {
	local begin = timer.get();
	while(true) {
		local now = timer.get();
		if(now > begin + seconds)
			break;
		::suspend(null);
	}
}

function routine(a, b) {
	sleep(0.1);
}

// Create the co-routine
local co = newthread(routine);

// Start the co-routine
local susparam = co.call(1, 2);

// Keep waking up the co-rountine until it is ended
// In reality, there are many co-routines to choose from,
// arrange them in a meaningfull order can reduce overhead.
while(co.getstatus() == "suspended") {
	susparam = co.wakeup(null);
}
