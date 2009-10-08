#include "Pch.h"
#include "../../MCD/Audio/OggLoader.h"
#include <fstream>

using namespace MCD;

TEST(OggTest)
{
	OggLoader loader;
	std::ifstream is("stereo.ogg", std::ios_base::binary);
//	bool ok = is.is_open();
//	(void)ok;
//	char buff[1024];
//	is.read(buff, sizeof(buff));
//	int a = is.gcount();
//	(void)a;

	loader.load(&is);
}
