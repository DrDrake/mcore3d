#include "Pch.h"
#include "Max3dsLoader.h"
#include "../Render/Material.h"
#include "../Render/MeshBuilder.h"
#include "../Render/MeshBuilderUtility.h"
#include "../Render/Mesh.h"
#include "../Render/Texture.h"
#include "../Render/TangentSpaceBuilder.h"
#include "../Core/Entity/Prefab.h"
#include "../Core/Math/Mat44.h"
#include "../Core/System/Log.h"
#include "../Core/System/MemoryProfiler.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/ResourceManager.h"
#include "../Core/System/Stream.h"
#include "../Core/System/StrUtility.h"
#include "../Core/System/Utility.h"
#include "../../3Party/lib3ds/file.h"
#include "../../3Party/lib3ds/io.h"

#ifdef MCD_VC
#	ifdef NDEBUG
#		pragma comment(lib, "lib3ds")
#	else
#		pragma comment(lib, "lib3dsd")
#	endif
#endif	// MCD_VC

namespace MCD {

static Lib3dsBool fileio_error_func(void* self)
{
//	FILE *f = (FILE*)self;
//	return(ferror(f)!=0);
	return false;
}

static long fileio_seek_func(void* self, long offset, Lib3dsIoSeek origin)
{
	std::istream* is = reinterpret_cast<std::istream*>(self);
	std::ios_base::seekdir o;
	switch (origin) {
	case LIB3DS_SEEK_SET:
		o = std::ios_base::beg;
		break;
	case LIB3DS_SEEK_CUR:
		o = std::ios_base::cur;
		break;
	case LIB3DS_SEEK_END:
		o = std::ios_base::end;
		break;
	default:
		MCD_ASSERT(false);
		return(0);
	}
	is->seekg(offset, o);
	return is->tellg();
}

static long fileio_tell_func(void* self)
{
	std::istream* is = reinterpret_cast<std::istream*>(self);
	return is->tellg();
}


static size_t fileio_read_func(void* self, void *buffer, size_t size)
{
	std::istream* is = reinterpret_cast<std::istream*>(self);
	is->read((char*)buffer, size);
	return is->gcount();
}

class Max3dsLoader::Impl
{
public:
	Impl();

	~Impl();

	IResourceLoader::LoadingState load(std::istream* is, const Path* fileId, const char* args);

	void commit(Resource& resource);

	Lib3dsFile* file;
	Lib3dsIo* io;
};	// Impl

static const int cIndexAttId = 0, cPositionAttId = 1, cNormalAttId = 2;

Max3dsLoader::Impl::Impl()
{
	file = lib3ds_file_new();
	MCD_ASSERT(file);

	io = nullptr;
}

Max3dsLoader::Impl::~Impl()
{
	lib3ds_file_free(file);
	lib3ds_io_free(io);
}

IResourceLoader::LoadingState Max3dsLoader::Impl::load(std::istream* is, const Path* fileId, const char* args)
{
	if(!is)
		return Aborted;

	io = lib3ds_io_new(
		is,
		fileio_error_func,
		fileio_seek_func,
		fileio_tell_func,
		fileio_read_func,
		nullptr	// fileio_write_func
	);

	if(!io)
		return Aborted;

	if(!lib3ds_file_read(file, io))
		return Aborted;

	return Loaded;
}

void Max3dsLoader::Impl::commit(Resource& resource)
{

}

Max3dsLoader::Max3dsLoader()
	: mImpl(*new Impl)
{
}

Max3dsLoader::~Max3dsLoader()
{
	delete &mImpl;
}

IResourceLoader::LoadingState Max3dsLoader::load(std::istream* is, const Path* fileId, const char* args)
{
	MemoryProfiler::Scope scope("Max3dsLoader::load");
	return mImpl.load(is, fileId, args);
}

void Max3dsLoader::commit(Resource& resource)
{
	MemoryProfiler::Scope scope("Max3dsLoader::commit");
	mImpl.commit(resource);
}

ResourcePtr Max3dsLoaderFactory::createResource(const Path& fileId, const char* args)
{
	if(strCaseCmp(fileId.getExtension().c_str(), "3ds") == 0)
		return new Prefab(fileId);
	return nullptr;
}

IResourceLoaderPtr Max3dsLoaderFactory::createLoader()
{
	return new Max3dsLoader;
}

}	// namespace MCD
