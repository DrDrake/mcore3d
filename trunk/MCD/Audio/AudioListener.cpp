#include "Pch.h"
#include "AudioListener.h"
#include "ALInclude.h"

namespace MCD {

float AudioListener::gain() const
{
	ALfloat value = 0;
	alGetListenerf(AL_GAIN, &value);
	return value;
}

void AudioListener::setGain(float value)
{
	alListenerf(AL_GAIN, value);
}

Vec3f AudioListener::position() const
{
	Vec3f ret;
	::alGetListenerfv(AL_POSITION, ret.data);
	return ret;
}

void AudioListener::setPosition(const Vec3f& p)
{
	::alListenerfv(AL_POSITION, p.data);
}

void AudioListener::orientation(Vec3f& lookAt, Vec3f& up) const
{
	Vec3f tmp[2];
	::alGetListenerfv(AL_ORIENTATION, (float*)tmp);
	lookAt = tmp[0];
	up = tmp[1];
}

void AudioListener::setOrientation(const Vec3f& lookAt, const Vec3f& up)
{
	const Vec3f tmp[2] = { lookAt, up };
	::alListenerfv(AL_ORIENTATION, (const float*)tmp);
}

Vec3f AudioListener::velocity() const
{
	Vec3f ret;
	::alGetListenerfv(AL_VELOCITY, ret.data);
	return ret;
}

void AudioListener::setVelocity(const Vec3f& v)
{
	::alListenerfv(AL_VELOCITY, v.data);
}

}	// namespace MCD
