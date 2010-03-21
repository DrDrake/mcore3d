#include "StdAfx.h"
#include "Global.h"
#include "Utility.h"
#include "../../MCD/Core/System/Log.h"
#include <sstream>
#include <gcroot.h>

#undef nullptr
using namespace MCD;

namespace Binding {

namespace {

ref class AppendOperation
{
public:
	System::Windows::Forms::TextBox^ textBox;
	System::String^ str;

	void execute() {
		textBox->AppendText(str);
	}
};

/*!	A simple STL stream buffer than redirect the inputting string into our text box
	\ref http://thecodeproject.com/debug/debugout.asp
 */
template<class CharT, class TraitsT=std::char_traits<CharT> >
class MyStreamBuf : public std::basic_stringbuf<CharT, TraitsT>
{
	typedef std::basic_stringbuf<CharT, TraitsT> super_type;
	typedef System::Windows::Forms::TextBox TextBox;

public:
	MyStreamBuf() : textBox(nullptr) {
	}

	virtual ~MyStreamBuf() {
		sync();
	}

	gcroot<TextBox^> textBox;

protected:
	virtual std::streamsize xsputn(const char_type* p, std::streamsize count);
	virtual int_type overflow(int_type c = super_type::traits_type::eof());
	virtual int sync();
};	// MyStreamBuf

template<class CharT, class TraitsT=std::char_traits<CharT> >
class MyOStream : public std::basic_ostream<CharT, TraitsT>
{
	typedef std::basic_ostream<CharT, TraitsT> super_type;
	typedef System::Windows::Forms::TextBox TextBox;
	MyStreamBuf<CharT, TraitsT>* mBuff;

public:
	explicit MyOStream(bool alwaysFlush=true)
		: super_type(nullptr), mBuff(new MyStreamBuf<CharT, TraitsT>())
	{
		super_type::init(mBuff);
		// Tell the stream to flush immediately
		if(alwaysFlush)
			super_type::setf(std::ios::unitbuf);
	}

	~MyOStream()
	{
		assert(mBuff == super_type::rdbuf());
		delete mBuff;
	}

	void setTextBox(TextBox^ textBox) {
		mBuff->textBox = textBox;
	}
};	// MyOStream

template<>
std::streamsize MyStreamBuf<char>::xsputn(const char_type* p, std::streamsize count)
{
	_Lock();
	std::streamsize result = super_type::xsputn(p, count);
	_Unlock();
	return result;
}

template<>
MyStreamBuf<char>::int_type MyStreamBuf<char>::overflow(int_type c)
{
	_Lock();
	int_type result = super_type::overflow(c);
	_Unlock();
	return result;
}

template<>
int MyStreamBuf<char>::sync()
{
	_Lock();

	// NOTE: Don't know why the textBox will be disposed even I use gcroot to reference it.
	if(textBox && !textBox->IsDisposed)
	{
		// Call textBox->BeginInvoke() to make this function .NET GUI thread safe
		AppendOperation^ op = gcnew AppendOperation;
		op->textBox = textBox;
		op->str = Utility::fromUtf8(str());

		textBox->BeginInvoke(gcnew System::Windows::Forms::MethodInvoker(op, &AppendOperation::execute));
	}

	str(std::basic_string<char>());	// Clear the string buffer
	int result = super_type::sync();
	_Unlock();

	return result;
}

}	// namespace

Global::Global(System::Windows::Forms::TextBox^ textBox)
{
	if(instance != nullptr)
		throw gcnew System::Exception("Global cannot be constructed more than once");
	instance = this;

	MyOStream<char>* os  = new MyOStream<char>();
	os->setTextBox(textBox);
	Log::start(os);
	Log::setLevel(Log::Level(Log::Error | Log::Warn | Log::Info));
}

Global::~Global() {
	this->!Global();
}

Global::!Global() {
	Log::stop();
}

}	// namespace Binding
