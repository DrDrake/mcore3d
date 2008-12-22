#include "Pch.h"
#include "../../../MCD/Core/System/Stream.h"
#include <fstream>
#include <sstream>
#include <iterator>

using namespace MCD;

//! A class for applying varous operation randomly on 2 streams for comparison.
class StreamTester
{
public:
	StreamTester(std::istream& referenceStream, Stream& testStream) :
		mIStream(&referenceStream), mOStream(nullptr),
		mTestStream(testStream)
	{}

	StreamTester(std::ostream& referenceStream, Stream& testStream) :
		mIStream(nullptr), mOStream(&referenceStream),
		mTestStream(testStream)
	{}

	StreamTester(std::iostream& referenceStream, Stream& testStream) :
		mIStream(&referenceStream), mOStream(&referenceStream),
		mTestStream(testStream)
	{}

	/*!	Return code: 0 - ok, 1 - both streams fail, negative number for the
		corresponding test performed which the 2 streams give different results.
	 */
	int singleTest()
	{
		static const size_t maxTestCase = 7;
		int testIndex = rand() % maxTestCase;

		std::string tmpStr;

		switch(testIndex)
		{
		case 0:		// Write
			genRandomText(tmpStr);
			if(mOStream) {
				*mOStream << tmpStr;
				mTestStream << tmpStr;
			}
			break;

		case 1:		// Read
			if(mIStream) {
				*mIStream >> tmpStr;
				std::string tmpStr2;
				mTestStream >> tmpStr2;

				if(tmpStr != tmpStr2 || mIStream->gcount() != mTestStream.gcount())
					return -testIndex;
			}
			break;

		case 2:		// Put back
			if(mIStream) {
				const char c = tmpStr.size() > 0 ? tmpStr[tmpStr.size()-1] : ' ';
				mIStream->putback(c);
				mTestStream.putback(c);
			}
			break;

		case 3:		// Flush
			if(mOStream) {
				mOStream->flush();
				mTestStream.flush();
			}
			break;

		case 4:		// Tell position
			if(mOStream) {
				std::ios::pos_type p1 = mOStream->tellp();
				std::ios::pos_type p2 = mTestStream.tellp();
				if(p1 != p2)
					return -testIndex;
			}
			break;

		case 5:		// Seek position
			{
				int off = (rand() % 512) - 256;
				std::ios_base::seekdir dir = genRandomSeekDir();

				if(mOStream) {
					mOStream->seekp(off, dir);
					mTestStream.seekp(off, dir);
				}
				if(mIStream) {
					mIStream->seekg(off, dir);
					mTestStream.seekg(off, dir);
				}
			}
			break;

		case 6:		// Get previous read count
			if(mIStream && mIStream->gcount() != mTestStream.gcount())
				return -testIndex;
			break;

		default:
			break;
		}

		bool refFail = mOStream ? mOStream->fail() : (mIStream ? mIStream->fail() : true);
		bool testFail = mTestStream.fail();

		// Clear all fail bits
		if(mIStream)
			mIStream->clear();
		if(mOStream)
			mOStream->clear();
		mTestStream.clear();

		if(refFail == testFail && testFail == true)
			return 1;
		else if(refFail != testFail)
			return -testIndex;
		else
			return 0;
	}

	void genRandomText(std::string& str)
	{
		static const size_t maxLength = 128;
		size_t len = rand() % maxLength;
		str.resize(len);
		for(;len--;)
			str[len] = char(rand() % (127 - ' ') + ' ');
	}

	std::ios_base::seekdir genRandomSeekDir()
	{
		int i = rand() % 3;
		switch(i) {
		case 0: return std::ios_base::beg;
		case 1: return std::ios_base::cur;
		case 2: return std::ios_base::end;
		default: return std::ios_base::seekdir(0);
		}
	}

	std::istream* mIStream;
	std::ostream* mOStream;
	Stream& mTestStream;
};	// StreamTester

#include <algorithm>
bool compareStream(std::istream& s1, std::istream& s2)
{
	typedef std::istream_iterator<char> itr;
	return std::equal(itr(s1), itr(), itr(s2));
}

// Similar to stdiobuf: http://www.digitalmars.com/rtl/iostream.html#stdiobuf
class FileStreamProxy : public IStreamProxy
{
	friend class std::auto_ptr<FileStreamProxy>;

protected:
	sal_override ~FileStreamProxy() {
		::fclose(mFile);
	}

	sal_override bool setbuf(char*, size_t, size_t, StreamBuf*) {
		return false;
	}

	size_t initBufSize() const {
		return 0;
	}

public:
	FileStreamProxy(const char* fileName, const char* openMode="w+") {
		mFile = fopen(fileName, openMode);
	}

	sal_override size_t write(const char* data, size_t size) {
		return fwrite(data, 1, size, mFile);
	}

	sal_override size_t read(char* data, size_t size) {
		return fread(data, 1, size, mFile);
	}

	sal_override bool unget(char c) {
		return ::ungetc(c, mFile) != EOF;
	}

	sal_override bool flush() {
		// Reference: http://www.cplusplus.com/reference/clibrary/cstdio/fflush.html
		return ::fflush(mFile) != EOF;
	}

	sal_override long seek(size_t offset, std::ios_base::seekdir origin, std::ios_base::openmode) {
		int ori = origin;
		switch(origin) {
			case std::ios_base::cur: ori = SEEK_CUR; break;
			case std::ios_base::beg: ori = SEEK_SET; break;
			case std::ios_base::end: ori = SEEK_END; break;
			default: break;
		}

		// Reference: http://www.cplusplus.com/reference/clibrary/cstdio/fseek.html
		bool ok = ::fseek(mFile, offset, ori) == 0;
		return ok ? ::ftell(mFile) : -1;
	}

	sal_override char* rawBufPtr() {
		return nullptr;
	}

	sal_override size_t rawBufSize() const {
		return 0;
	}

	FILE* mFile;
};	// FileStreamProxy

TEST(File_StreamTest)
{
	static const char referenceFile[] = "referende.txt";
	static const char testFile[] = "test.txt";

	{	// Perform write
		std::auto_ptr<FileStreamProxy> p(new FileStreamProxy(testFile, "w"));
		Stream os(*p);
		p.release();
		std::ofstream ofs(referenceFile);

		StreamTester tester(ofs, os);
		for(size_t i=0; i<1000; ++i) {
			int ret = tester.singleTest();
			if(ret != 1) CHECK_EQUAL(0, ret);
		}
	}

	{	// Perform read
		// NOTE: Gcc cygwin have problems in putback and seek
		std::auto_ptr<FileStreamProxy> p(new FileStreamProxy(testFile, "r"));
		Stream is(*p);
		p.release();
		std::ifstream ifs(referenceFile);

		StreamTester tester(ifs, is);
		for(size_t i=0; i<1000; ++i) {
			int ret = tester.singleTest();
			if(ret != 1) CHECK_EQUAL(0, ret);
		}
	}

	{	// Compare the content of 2 streams
		Stream is(*(new FileStreamProxy(testFile, "r")));
		std::ifstream ifs(referenceFile);
		CHECK(compareStream(is, ifs));
	}
}

class FixBufferStreamProxy : public StreamProxy
{
protected:
	sal_override ~FixBufferStreamProxy()
	{
		if(mDelete)
			mDelete(mRawBufPtr);
	}

public:
	typedef void (*Delete)(const void* p);

	FixBufferStreamProxy(char* buffer, size_t size, size_t initialReadableSize=0, Delete functionPtr=nullptr)
		: mDelete(functionPtr)
	{
		setbuf(buffer, size, initialReadableSize, nullptr);
	}

	Delete mDelete;
};	// FixBufferStreamProxy

TEST(FixBuffer_StreamTest)
{
	std::string buf;
	buf.resize(65536);

	for(size_t i=0; i<100; ++i) {
		Stream s(*(new FixBufferStreamProxy(const_cast<char*>(buf.c_str()), buf.size())));
		std::stringstream s2;

		s2 << "g";
		s << "g";

		StreamTester tester(s2, s);
		for(size_t j=0; j<100; ++j) {
			int ret = tester.singleTest();
			if(ret != 1) CHECK_EQUAL(0, ret);
		}
	}
}

class StringStreamProxy : public StreamProxy
{
protected:
	sal_override ~StringStreamProxy()
	{
		if(mRealloc)
			mRealloc(mRawBufPtr, 0);
	}

public:
	typedef void* (*Realloc)(void* p, size_t size);

	StringStreamProxy(Realloc functionPtr=&::realloc)
		: mRealloc(functionPtr)
	{
#ifdef MCD_GCC
		// GCC stl will allocate a buffer at init while VC will not
		static const size_t initSize = 512;
		char* buf = (char*)mRealloc(nullptr, initSize);
		setbuf(buf, initSize, 0, nullptr);
#endif
	}

	sal_override size_t write(const char* data, size_t size)
	{
		// Skip the operation if the source is the internal buffer itself
		if(data == mRawBufPtr)
			return size;

		size_t oldSize = mRawBufSize;
		size_t newSize = oldSize * 2;
		newSize = newSize < oldSize + size ? oldSize + size : newSize;

		char* newBuf = (char*)mRealloc(mRawBufPtr, newSize);
		memcpy(newBuf + oldSize, data, size);

		setbuf(newBuf, newSize, oldSize + size, mStreamBuf);
		mStreamBuf->pubseekoff(size, std::ios_base::cur, std::ios_base::out);

		return size;
	}

	Realloc mRealloc;
};	// StringStreamProxy

TEST(StringBuffer_StreamTest)
{
	for(size_t i=0; i<100; ++i) {
		Stream s(*(new StringStreamProxy()));
		std::stringstream s2;

		StreamTester tester(s2, s);
		for(size_t j=0; j<100; ++j) {
			int ret = tester.singleTest();
			if(ret != 1) CHECK_EQUAL(0, ret);
		}
	}
}
