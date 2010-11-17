#include "Pch.h"
#include "../../../MCD/Core/System/XmlParser.h"

using namespace MCD;

TEST(XmlParserTest)
{
	XmlParser parser;

	std::string xmlString("\
	<?xml version=\"1.0\"?>\
	<config>\
		<!-- This is a comment -->\
		<model file=\"scene.3ds\" type='mesh' />\
		<messageText caption=\"Hello world\">\
			Welcome to &quot;MCore 3D Engine&quot;.\
		</messageText>\
		<fragment>\
		<![CDATA[\
			main() {}\
		]]>\
		</fragment>\
	</config>\
	");

	// Note that the data in xmlString will be modified by the parser
	parser.parse(const_cast<char*>(xmlString.c_str()));

	bool ended = false;
	size_t eventCount = 0;
	static const size_t maxCount = 13;

	typedef XmlParser::Event Event;

	const Event::Enum expectedEvents[maxCount] = {
		Event::Unknown,
		Event::BeginElement,
		Event::Comment,
		Event::BeginElement,
		Event::EndElement,
		Event::BeginElement,
		Event::Text,
		Event::EndElement,
		Event::BeginElement,
		Event::CData,
		Event::EndElement,
		Event::EndElement,
		Event::EndDocument
	};

	while(!ended)
	{
		if(eventCount >= maxCount) {
			CHECK(false);
			return;
		}

		Event::Enum e = parser.nextEvent();

		switch(e)
		{
		case Event::BeginElement:
			if(strcmp(parser.elementName(), "config") == 0) {
				CHECK_EQUAL(0u, parser.attributeCount());
				CHECK(!parser.isEmptyElement());
			}
			else if(strcmp(parser.elementName(), "model") == 0) {
				CHECK_EQUAL(2u, parser.attributeCount());
				CHECK(parser.isEmptyElement());

				CHECK_EQUAL(std::string("scene.3ds"), parser.attributeValue("file"));
				CHECK_EQUAL(std::string("mesh"), parser.attributeValue("type"));
			}
			else if(strcmp(parser.elementName(), "messageText") == 0) {
				CHECK_EQUAL(1u, parser.attributeCount());
				CHECK(!parser.isEmptyElement());
			}
			else if(strcmp(parser.elementName(), "fragment") == 0) {
				CHECK_EQUAL(0u, parser.attributeCount());
				CHECK(!parser.isEmptyElement());
			}
			break;

		case Event::Comment:
			CHECK_EQUAL(std::string(" This is a comment "), parser.textData());
			break;

		case Event::Text:
			CHECK_EQUAL(std::string("\t\t\tWelcome to \"MCore 3D Engine\".\t\t"), parser.textData());
			break;

		case Event::CData:
			CHECK_EQUAL(std::string("\t\t\tmain() {}\t\t"), parser.textData());
			break;

		case Event::Error:
		case Event::EndDocument:
			ended = true;
			break;

		default:
			break;
		}

		CHECK_EQUAL(expectedEvents[eventCount], e);

		++eventCount;
	}

	// After the parsing is finished, further call to nextEvent() should return EndDocument
	CHECK_EQUAL(Event::EndDocument, parser.nextEvent());
}
