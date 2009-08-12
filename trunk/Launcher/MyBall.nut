return class extends RigidBodyComponent
{
	constructor(mass, shape)
	{
		::RigidBodyComponent.constructor(mass, shape);
	}

	function update()
	{
		local sleep = function(seconds)
		{
			local begin = ::gFrameTimer.accumulateTime;
			while(true) {
				local now = ::gFrameTimer.accumulateTime;
				if(now > begin + seconds)
					break;
				if(!::suspend(null))
					return;
			}
		}

		sleep(4);
//		local e = entity;
//		e.transform.translateBy(Vec3(0, 5, 0));
		::println("hi");
	}
}
