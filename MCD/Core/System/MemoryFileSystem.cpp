#include "Pch.h"
#include "MemoryFileSystem.h"
#include "Stream.h"

namespace MCD {

namespace {

class MemoryFileStreamProxy : public StreamProxy
{
public:
	MemoryFileStreamProxy(char* buffer, size_t size) {
		setbuf(buffer, size, size, nullptr);
	}
};	// MemoryFileStreamProxy

}	// namespace

MemoryFileSystem::MemoryFile::MemoryFile(const Path& key, const void* ptr, size_t sz)
	: MapBase<Path>::Node<MemoryFile>(key)
	, fileData(ptr)
	, fileSize(sz)
{}

MemoryFileSystem::MemoryFileSystem(const Path& root)
	: mRoot(root)
{}

MemoryFileSystem::~MemoryFileSystem()
{}

bool MemoryFileSystem::isExists(const Path& path) const
{
	return mMemoryFiles.find(path) != nullptr;
}

uint64_t MemoryFileSystem::getSize(const Path& path) const
{
	if(const MemoryFile* file = mMemoryFiles.find(path))
		return file->fileSize;
	else
		return 0;
}

std::auto_ptr<std::istream> MemoryFileSystem::openRead(const Path& path) const
{
	std::auto_ptr<std::istream> stream;
	if(const MemoryFile* file = mMemoryFiles.find(path))
		stream.reset(new Stream(*new MemoryFileStreamProxy((char*)file->fileData, file->fileSize)));

	return stream;
}

bool MemoryFileSystem::add(const Path& fileId, const void* fileData, size_t fileSize)
{
	MCD_ASSUME(fileData && fileSize > 0);
	const Path key = mRoot / fileId;
	if(mMemoryFiles.find(key) != nullptr)
		return false;

	mMemoryFiles.insert(*new MemoryFile(key, fileData, fileSize));

	return true;
}

}	// namespace MCD
