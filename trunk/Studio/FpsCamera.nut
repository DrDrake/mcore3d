/*	An camera component that controls the camera like a FPS game
	WASD to move, left mouse click to look around.
 */
return class extends CameraComponent
{

translationSpeed = 10;
mouseSensitivity = 0.5 * 3.14159265 / 180;

_lastMouseAxis = Vec2(0);
_accumulateMouseDelta = Vec2(0);

//! If you want to make a constructor, read the comments carefully
/*constructor()
{
	// NOTE: This is very important to call the base class constructor,
	// otherwise null pointer dereference occur and crash!
	::CameraComponent.constructor();
}*/

function update()
{
/*	local sleep = function(seconds)
	{
		local begin = ::gFrameTimer.accumulateTime;
		while(true) {
			local now = ::gFrameTimer.accumulateTime;
			if(now > begin + seconds)
				break;
			if(!::suspend(null))
				return;
		}
	}*/

	// Make a short-cut first
	local localTransform = entity.localTransform;

	{	// Handling rotation
		local currentMouseAxis = ::Vec2(::gInput.getAxis("mouse x"), ::gInput.getAxis("mouse y"));

//		if(::gInput.getMouseButton(1))
//			sleep(5);

		if(::gInput.getMouseButton(0))
		{
			local delta = currentMouseAxis - _lastMouseAxis;
			delta.mulEqual(-mouseSensitivity);
			_accumulateMouseDelta += delta;

			local horizontalRotation = ::Mat44.makeAxisRotation(::Vec3(0, 1, 0), _accumulateMouseDelta.x);
			local verticalRotation = ::Mat44.makeAxisRotation(horizontalRotation.xBiasVector, _accumulateMouseDelta.y);

			local backupTranslation = localTransform.translation;
			localTransform = verticalRotation * horizontalRotation;
			localTransform.translateBy(backupTranslation);
		}
		_lastMouseAxis = currentMouseAxis;
	}

	{	// Handling translation
		local translation = ::Vec3();

		if(::gInput.getButton("s"))
			  translation += localTransform.zBiasVector;
		if(::gInput.getButton("w"))
			  translation -= localTransform.zBiasVector;
		if(::gInput.getButton("d"))
			  translation += localTransform.xBiasVector;
		if(::gInput.getButton("a"))
			  translation -= localTransform.xBiasVector;
		if(::gInput.getButton("PageUp"))
			  translation += localTransform.yBiasVector;
		if(::gInput.getButton("Next"))
			  translation -= localTransform.yBiasVector;

		translation.mulEqual(::gFrameTimer.frameTime * translationSpeed);
		localTransform.translateBy(translation);
	}

	entity.localTransform = localTransform;
}

}
