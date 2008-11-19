#ifndef __MCD_CORE_SYSTEM_STREAM__
#define __MCD_CORE_SYSTEM_STREAM__

#include "../ShareLib.h"
#include "Platform.h"
#include <iostream>

namespace MCD {

class StreamBuf;

class MCD_ABSTRACT_CLASS IStreamProxy
{
protected:
	friend class StreamBuf;

	virtual ~IStreamProxy() {}

	/*!	Setup the internal raw buffer for the proxy.
		
	 */
	virtual bool setbuf(char* buffer, size_t size, size_t initDataSize=0, StreamBuf* streamBuf=nullptr) = 0;

	/*!	Write data to external shink.
		Invoked when no more rooms is in the internal buffer for write.
	 */
	virtual size_t write(const char* data, size_t size) = 0;

	/*!	Read data from external source.
		Invoked when no more data is in the internal buffer for read.
	 */
	virtual size_t read(char* data, size_t size) = 0;

	/*!	Put back one of the previous read character back to the stream.
		Invoked when no where to put back in the internal buffer.
	 */
	virtual bool unget(char c) = 0;

	//! Flush data to external data sink.
	virtual bool flush() = 0;

	/*!	
	 */
	virtual bool seek(size_t offset, int origin) = 0;

	virtual long tellp() = 0;

	//! Get the internal raw buffer pointer.
	virtual char* rawBufPtr() = 0;

	//! Get the internal raw buffer size.
	virtual size_t rawBufSize() const = 0;
};	// IStreamProxy

class MCD_CORE_API MCD_ABSTRACT_CLASS StreamProxy : public IStreamProxy
{
protected:
	StreamProxy();

	sal_override ~StreamProxy() {}

	sal_override bool setbuf(char* buffer, size_t size, size_t initDataSize, StreamBuf* streamBuf);

	sal_override size_t write(const char* data, size_t size);

	sal_override size_t read(char* data, size_t size);

	sal_override bool unget(char c);

	sal_override bool flush();

	sal_override bool seek(size_t offset, int origin);

	sal_override long tellp();

	sal_override char* rawBufPtr();

	sal_override size_t rawBufSize() const;

	StreamBuf* mStreamBuf;
	char* mRawBufPtr;
	size_t mRawBufSize;
};	// StreamProxy

class MCD_CORE_API StreamBuf : public std::basic_streambuf<char>
{
	friend class StreamProxy;
	typedef StreamBuf::traits_type traits_type;
	typedef std::basic_streambuf<char> Super;

public:
	typedef traits_type traits_type;

	// Will take over the ownership of proxy
	StreamBuf(IStreamProxy& proxy);

	sal_override ~StreamBuf();

protected:
	sal_override StreamBuf* StreamBuf::setbuf(char* buffer, std::streamsize size);

	sal_override std::streamsize xsputn(const char* data, std::streamsize size);

	sal_override int_type overflow(int_type c = traits_type::eof());

	sal_override std::streamsize xsgetn(char* data, std::streamsize size);

	sal_override int_type underflow();

	sal_override int_type uflow();

	sal_override int_type pbackfail(int_type c = traits_type::eof());

	sal_override pos_type seekoff(off_type offset,
		std::ios_base::seekdir way,
		std::ios_base::openmode which = std::ios_base::in | std::ios_base::out);

	sal_override int sync();

	bool updateReadArea();

	IStreamProxy* mProxy;
};	// StreamBuf

class MCD_CORE_API Stream : public std::basic_iostream<char>
{
public:
	explicit Stream(IStreamProxy& StreamProxy);

	StreamBuf* rdbuf() const;

private:
	StreamBuf mBuffer;
};	// Stream

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_STREAM__
