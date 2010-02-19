#include "Pch.h"
#include "Stream.h"

using namespace std;

namespace MCD {

StreamProxy::StreamProxy()
{
	mStreamBuf = nullptr;
	mRawBufPtr = nullptr;
	mRawBufSize = 0;
	mInitBufSize = 0;
}

bool StreamProxy::setbuf(char* buffer, size_t size, size_t initDataSize, StreamBuf* streamBuf)
{
	MCD_ASSERT(initDataSize <= size);
	if(initDataSize > size)
		return false;

	mRawBufPtr = buffer;
	mRawBufSize = size;
	mStreamBuf = streamBuf;
	mInitBufSize = initDataSize;

	if(streamBuf) {
		streamBuf->pubsetbuf(buffer, size);
		streamBuf->setg(streamBuf->eback(), streamBuf->gptr(), streamBuf->eback() + initDataSize);
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

long StreamProxy::seek(size_t, std::ios_base::seekdir, std::ios_base::openmode) {
	return -1;
}

char* StreamProxy::rawBufPtr() {
	return mRawBufPtr;
}

size_t StreamProxy::rawBufSize() const {
	return mRawBufSize;
}

size_t StreamProxy::initBufSize() const {
	return mInitBufSize;
}

StreamBuf::StreamBuf(IStreamProxy& proxy)
	: mProxy(&proxy)
{
	proxy.setbuf(proxy.rawBufPtr(), proxy.rawBufSize(), proxy.initBufSize(), this);
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
	ios_base::seekdir origin,
	ios_base::openmode which)
{
	using namespace std;

	const off_type badOff = off_type(-1);

	updateReadArea();

	if(which & ios_base::in)
	{
		{	// Transform offset base on ios_base::beg
			if(origin == ios_base::cur && !(which & ios_base::out))
				offset += off_type(gptr() - eback());
			else if(origin == ios_base::end)
				offset += off_type(egptr() - eback());
			else if(origin != ios_base::beg)
				offset = badOff;
		}

		if(offset >= 0 && offset <= egptr() - eback() && eback()) {
			gbump(int(eback() - gptr() + offset));

			if(which & ios_base::out) {
				setp(pbase(), epptr());
				pbump(gptr() - eback());
			}
		} else
			return mProxy->seek(offset, origin, which);
	}
	else if(which & ios_base::out)
	{
		// NOTE: We use egptr() as the high water mark of the put area
		{	// Transform offset base on ios_base::beg
			if(origin == ios_base::cur)
				offset += off_type(pptr() - pbase());
			else if(origin == ios_base::end)
				offset += off_type(egptr() - eback());
			else if(origin != ios_base::beg)
				offset = badOff;
		}

		if(offset >= 0 && offset <= egptr() - eback() && pbase()) {
			pbump(int(eback() - pptr() + offset));

			MCD_ASSERT(!(which & ios_base::in));
		} else
			return mProxy->seek(offset, origin, which);
	}
	else
	{
		MCD_ASSERT(false && "incorrect parameter 'which'");
		return badOff;
	}

	return offset;
}

StreamBuf::pos_type StreamBuf::seekpos(pos_type position, ios_base::openmode which)
{
	return seekoff(position, ios_base::cur, which);
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

void write(std::ostream& os, bool val) {
	write(os, static_cast<char>(val));
}

void write(std::ostream& os, char val) {
	os.write(&val, sizeof(val));
}

void write(std::ostream& os, uint16_t val) {
	os.write(reinterpret_cast<const char*>(&val), sizeof(val));
}

void write(std::ostream& os, uint32_t val) {
	os.write(reinterpret_cast<const char*>(&val), sizeof(val));
}

void write(std::ostream& os, float val) {
	os.write(reinterpret_cast<const char*>(&val), sizeof(val));
}

void writeString(std::ostream& os, const char* val, size_t len)
{
	// NOTE: We assume uint32_t as the string length
	uint32_t len_ = static_cast<uint32_t>(len == 0 ? ::strlen(val) : len);
	write(os, len_);
	os.write(val, len_);
}

void write(std::ostream& os, const void* val, std::streamsize size) {
	os.rdbuf()->sputn(reinterpret_cast<const char*>(val), size);
}

bool read(std::istream& is, bool& val)
{
	char val_;
	if(!read(is, val_)) return false;
	val = val_ > 0;
	return true;
}

bool read(std::istream& is, char& val) {
	return is.rdbuf()->sgetn(&val, sizeof(val)) == sizeof(val);
}

bool read(std::istream& is, uint16_t& val) {
	return is.rdbuf()->sgetn(reinterpret_cast<char*>(&val), sizeof(val)) == sizeof(val);
}

bool read(std::istream& is, uint32_t& val) {
	return is.rdbuf()->sgetn(reinterpret_cast<char*>(&val), sizeof(val)) == sizeof(val);
}

bool read(std::istream& is, float& val) {
	return is.rdbuf()->sgetn(reinterpret_cast<char*>(&val), sizeof(val)) == sizeof(val);
}

bool readString(std::istream& is, std::string& val)
{
	// NOTE: We assume uint32_t as the string length
	uint32_t len_;
	if(!read(is, len_))
		return false;
	val.resize(len_);
	return static_cast<size_t>(is.rdbuf()->sgetn(&val[0], len_)) == len_;
}

size_t readString(std::istream& is, char* buf, size_t bufLen)
{
	// NOTE: We assume uint32_t as the string length
	uint32_t len_;
	read(is, len_);
	if(len_ >= bufLen) len_ = bufLen - 1;	// Check bufLen and reserve room for '\0'
	std::streamsize readCnt = is.rdbuf()->sgetn(buf, len_);
	buf[readCnt] = '\0';
	return static_cast<size_t>(readCnt);
}

size_t readString(std::istream& is, char* buf, size_t bufLen, char seperator)
{
	is.getline(buf, bufLen, seperator);
	return static_cast<size_t>(is.gcount());
}

std::streamsize read(std::istream& is, void* val, std::streamsize size) {
	return is.rdbuf()->sgetn(reinterpret_cast<char*>(val), size);
}

}	// namespace MCD
