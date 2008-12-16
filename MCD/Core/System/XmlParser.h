#ifndef __MCD_CORE_SYSTEM_XMLPARSER__
#define __MCD_CORE_SYSTEM_XMLPARSER__

#include "../ShareLib.h"
#include "NonCopyable.h"

namespace MCD {

/*!	XmlParser is intended to be a high speed and easy-to-use XML Parser specialized for
	game development.

	Features:
	-It it fast as lighting and has very low memory usage.
	-Extremely easy to use.

	Limitations:
	-Forward-only, read-only.
	-The input xml file is not validated and assumed to be correct.

    The following code demonstrates the basic usage. A simple xml file like this is parsed:
    \code
	<?xml version="1.0"?>
	<config>
		<!-- This is a comment -->
		<model file="scene.3ds" type="mesh"/>
		<messageText caption="Hello world">
			Welcome to &quot;MCore 3D Engine&quot;.
		</messageText>
		<fragment>
		<![CDATA[
			main() {}
		]]>
		</fragment>
	</config>
	\endcode

	The code for parsing this file would look like this:
	\code
	// Get the xml document string
	std::wifstream ifs("test.xml");
	std::wstringstream ss;
	ifs >> ss.rdbuf();

	XmlParser parser;
	parser.parse(const_cast<wchar_t*>(ss.str().c_str()));

	std::wstring messageText;
	std::wstring fragmentCode;
	std::wstring fileName;

	// Parse the file until end of document or any error occurred.
	bool ended = false;
	while(!ended) {
		XmlParser::Event::Enum e = parser.nextEvent();

		switch(e) {
		// In this xml file, the only text which occurs is the "messageText"
		case XmlParser::Event::Text:
			messageText = parser.textData();
			break;

		// In this xml file, the only CDATA which occurs is the "fragment"
		case XmlParser::Event::CData:
			fragmentCode = parser.textData();
			break;

		case XmlParser::Event::BeginElement:
			if(wcscmp(parser.elementName(), L"model") == 0)
				fileName = parser.attributeValue(L"file");
			break;

		case XmlParser::Event::Error:
		case XmlParser::Event::EndDocument:
			ended = true;
			break;

		default:
			break;
		}
	}
	\endcode
 */
class MCD_CORE_API XmlParser : Noncopyable
{
public:
	struct Event { enum Enum {
		BeginElement,	//!< An xml element, like <foo>
		Text,			//!< Text within an xml element: <foo>This is the text.</foo>
		Comment,		//!< An xml comment like <!-- I am a comment --> or a DTD definition.
		CData,			//!< An xml cdata section like <![CDATA[ this is some CDATA ]]>
		EndElement,		//!< End of an xml element, like </foo>
		EndDocument,	//!< End of the document.
		Unknown,		//!< Unknown element.
		Error			//!< Error occurred during parsing.
	}; };	// Event

	XmlParser();

	~XmlParser();

	void parse(sal_in_z wchar_t* source);

	Event::Enum nextEvent();

	const wchar_t* elementName() const;

	bool isEmptyElement() const;

	const wchar_t* textData() const;

	size_t attributeCount() const;

	const wchar_t* attributeName(size_t idx) const;

	const wchar_t* attributeValue(size_t idx) const;

	const wchar_t* attributeValue(sal_in_z const wchar_t* name) const;

	const wchar_t* attributeValueIgnoreCase(sal_in_z const wchar_t* name) const;

	float attributeValueAsFloat(size_t idx, float defaultValue = 0.0f) const;

	float attributeValueAsFloat(sal_in_z const wchar_t* name, float defaultValue = 0.0f) const;

	float attributeValueAsFloatIgnoreCase(sal_in_z const wchar_t* name, float defaultValue = 0.0f) const;

	static float stringToFloat(sal_in_z const wchar_t* str, float defaultValue = 0.0f);

protected:
	class Impl;
	Impl* mImpl;
};	// XmlParser

}	// namespace MCD

#endif	// __MCD_CORE_SYSTEM_XMLPARSER__
