#include "Pch.h"
#include "Stream.h"

using namespace std;

namespace MCD {

StreamProxy::StreamProxy()
{
	mStreamBuf = nullptr;
	mRawBufPtr = nullptr;
	mRawBufSize = 0;
}

bool StreamProxy::setbuf(char* buffer, size_t size, size_t initDataSize, StreamBuf* streamBuf)
{
	MCD_ASSERT(initDataSize <= size);
	if(initDataSize > size)
		return false;

	mRawBufPtr = buffer;
	mRawBufSize = size;
	mStreamBuf = streamBuf;

	if(streamBuf) {
		streamBuf->pubsetbuf(buffer, size);
		streamBuf->pbump(int(streamBuf->pbase() - streamBuf->pptr() + initDataSize));
	}

	return true;
}

size_t StreamProxy::write(const char*, size_t) {
	return 0;
}

size_t StreamProxy::read(char*, size_t) {
	return 0;
}

bool StreamProxy::unget(char) {
	return false;
}

bool StreamProxy::flush() {
	return true;
}

bool StreamProxy::seek(size_t, int) {
	return false;
}

long StreamProxy::tellp() {
	return -1;
}

char* StreamProxy::rawBufPtr() {
	return mRawBufPtr;
}

size_t StreamProxy::rawBufSize() const {
	return mRawBufSize;
}

StreamBuf::StreamBuf(IStreamProxy& proxy)
	: mProxy(&proxy)
{
	proxy.setbuf(proxy.rawBufPtr(), proxy.rawBufSize(), 0, this);
}

StreamBuf::~StreamBuf() {
	delete mProxy;
}

StreamBuf* StreamBuf::setbuf(char* buffer, streamsize size)
{
	// Get the current offset of the get/put area
	off_type pOff = pptr() - pbase();
	off_type gOff = gptr() - eback();

	// Clamp the range of the offsets
	pOff = pOff > size ? size : pOff;
	gOff = gOff > size ? size : gOff;

	setp(buffer, buffer + size);	// Why setp() didn't take 3 parameters just as setg()?
	pbump(pOff);
	setg(buffer, buffer + gOff, buffer + pOff);

	return this;
}

streamsize StreamBuf::xsputn(const char* data, streamsize size)
{
	if(pbase() == epptr())
		return mProxy->write(data, size);
	return Super::xsputn(data, size);
}

StreamBuf::int_type StreamBuf::overflow(int_type c)
{
	if(traits_type::eq_int_type(traits_type::eof(), c))
		return traits_type::not_eof(c);	// EOF, return success code

	if(pptr() < epptr())
	{	// Room in buffer, store it
		*pptr() = traits_type::to_char_type(c);
		pbump(1);
		return c;
	}

	{	// Write the data in the buffer, if any
		const size_t sizeToWrite = pptr() - pbase();
		if(sizeToWrite > 0 && mProxy->write(pbase(), sizeToWrite) != sizeToWrite)
			return traits_type::eof();
	}

	const char ch = traits_type::to_char_type(c);
	if(mProxy->write(&ch, sizeof(ch)) != 1)
		return traits_type::eof();

	return c;
}

streamsize StreamBuf::xsgetn(char* data, streamsize size)
{
	if(eback() == egptr())
		return mProxy->read(data, size);
	return Super::xsgetn(data, size);
}

StreamBuf::int_type StreamBuf::underflow()
{
	int_type c;
	if(gptr() < egptr())
		return traits_type::to_int_type(*gptr());	// Return buffered
	else if(traits_type::eq_int_type(traits_type::eof(), c = uflow()))
		return c;	// uflow failed, return EOF
	else
		pbackfail(c);

	return c;
}

StreamBuf::int_type StreamBuf::uflow()
{
	// Get an element from stream, point past it
	if(gptr() < egptr() || updateReadArea()) {
		int_type ret = traits_type::to_int_type(*gptr());
		gbump(1);
		return ret;
	}

	char c;
	return mProxy->read(&c, sizeof(c)) == sizeof(c) ?
		traits_type::to_int_type(c) : traits_type::eof();
}

StreamBuf::int_type StreamBuf::pbackfail(StreamBuf::int_type meta)
{
	char c = traits_type::to_char_type(meta);
	if(eback() < gptr())
	{
		gbump(-1);
		if(!traits_type::eq_int_type(traits_type::eof(), meta))
			*gptr() = c;
		return c;
	} else if(mProxy->unget(c))
		return meta;
	else
		return traits_type::eof();	// Nowhere to put back
}

StreamBuf::pos_type StreamBuf::seekoff(
	StreamBuf::off_type offset,
	ios_base::seekdir way,
	ios_base::openmode which)
{
	using namespace std;

	const off_type badOff = off_type(-1);

	updateReadArea();

	if(which & ios_base::in)
	{
		{	// Transform offset base on ios_base::beg
			if(way == ios_base::cur && !(which & ios_base::out))
				offset += off_type(gptr() - eback());
			else if(way == ios_base::end)
				offset += off_type(egptr() - eback());
			else if(way != ios_base::beg)
				offset = badOff;
		}

		if(offset >= 0 && offset <= egptr() - eback() && eback()) {
			gbump(int(eback() - gptr() + offset));

			if(which & ios_base::out) {
				setp(pbase(), epptr());
				pbump(gptr() - eback());
			}
		} else
			goto FSEEK;
	}
	else if(which & ios_base::out)
	{
		// NOTE: We use egptr() as the high water mark of the put area
		{	// Transform offset base on ios_base::beg
			if(way == ios_base::cur)
				offset += off_type(pptr() - pbase());
			else if(way == ios_base::end)
				offset += off_type(egptr() - eback());
			else if(way != ios_base::beg)
				offset = badOff;
		}

		if(offset >= 0 && offset <= egptr() - eback() && pbase()) {
			pbump(int(eback() - pptr() + offset));

			MCD_ASSERT(!(which & ios_base::in));
		} else
			goto FSEEK;
	}
	else
		goto FSEEK;

	return offset;

FSEEK:
	int way_ = way;
	switch(way) {
		case ios_base::cur: way_ = SEEK_CUR; break;
		case ios_base::beg: way_ = SEEK_SET; break;
		case ios_base::end: way_ = SEEK_END; break;
		default: break;
	}

	bool success = mProxy->seek(offset, way_);
	long pos = mProxy->tellp();
	return pos == -1 || !success ? badOff : pos;
}

int StreamBuf::sync()
{
	updateReadArea();
	return traits_type::eq_int_type(traits_type::eof(), overflow())
		|| mProxy->flush() ? 0 : -1;
}

bool StreamBuf::updateReadArea()
{
	if(pptr() > egptr()) {
		setg(eback(), gptr(), pptr());
		return true;
	}
	return false;
}

Stream::Stream(IStreamProxy& StreamProxy)
	: basic_iostream<char>(&mBuffer), mBuffer(StreamProxy)
{
}

StreamBuf* Stream::rdbuf() const {
	return const_cast<StreamBuf*>(&mBuffer);
}

}	// namespace MCD
