#include "Pch.h"
#include "XmlParser.h"
#include "StrUtility.h"
#include <vector>

namespace MCD {

// Adopted from Irrlicht engine's xml parser
// http://www.ambiera.com/irrxml/

//! returns true if a character is whitespace
static inline bool isWhiteSpace(wchar_t c)
{
	return (c == L' ' || c == L'\t' || c == L'\n' || c == L'\r');
}

static const wchar_t* createString(const wchar_t* begin, wchar_t* end)
{
	*end = L'\0';
	return begin;
}

// Returns true if both string are equals.
// The string lhs should be null terminated while rhs need not to be.
static bool MyStrCmpLhsFixed(const wchar_t* lhs, const wchar_t* rhs)
{
	for(;*lhs != L'\0'; ++lhs, ++rhs)
		if(*lhs != *rhs)
			return false;
	return true;
}

static const wchar_t* escapeString(wchar_t* begin, wchar_t* end)
{
	// Characters that need to escape
	// Reference: http://www.w3schools.com/xml/xml_cdata.asp
	static const wchar_t cEscapeChar[5] = {L'&', L'<', L'>', L'\'', L'\"'};
	static const wchar_t* cEscapeString[5] = {L"&amp;", L"&lt;", L"&gt;", L"&apos;", L"&quot;"};
	static const size_t cEscapeStringLen[5] = {5, 4, 4, 6, 6};

	wchar_t* str = begin;

	// Scan for '&' character
	for(; str != end; ++str) {
		if(*str != L'&')
			continue;

		// Loop for all escape string
		for(size_t i = 0; i<5; ++i) {
			if(!MyStrCmpLhsFixed(cEscapeString[i], str))
				continue;

			// Replace with cEscapeChar
			*str = cEscapeChar[i];
			// Move the chars to the front
			const wchar_t* trailPos = str + cEscapeStringLen[i];
			::wmemcpy(str+1, trailPos, end - trailPos);
			// Adjust the new end
			end -= (cEscapeStringLen[i] - 1);
			break;
		}
	}

	*end = L'\0';

	return begin;
}

class XmlParser::Impl
{
	typedef XmlParser::Event Event;

	struct Attribute {
		const wchar_t* name;
		const wchar_t* value;
	};	// Attribute

	typedef std::vector<Attribute> Attrubutes;

public:
	Impl()
	{
		parse(nullptr);
	}

	void parse(wchar_t* source)
	{
		p = source;
		mElementName = L"";
		mText = L"";
		mHasBackupOpenTag = false;
		mCurrentNodeType = Event::Error;
		mAttrubutes.clear();
	}

	Event::Enum nextEvent()
	{
		if(!p)
			return mCurrentNodeType = Event::Error;

		wchar_t* start = p;
		wchar_t* backup = p;

		// More forward until '<' found
		if(!mHasBackupOpenTag) while(*p != L'<' && *p)
			++p;
		else
			*p = L'<';

		if(!*p)
			return mCurrentNodeType = Event::EndDocument;

		if(p - start > 0) {
			// We found some text, store it
			if(setText(start, p))
				return mCurrentNodeType;
		}

		++p;

		// Based on current token, parse and report next element
		switch(*p) {
		case L'/':
			parseClosingXMLElement(); 
			break;
		case L'?':
			ignoreDefinition();	
			break;
		case L'!':
			if(!parseCDATA())
				parseComment();	
			break;
		default:
			parseOpeningXMLElement();
			break;
		}

		if(mHasBackupOpenTag) {
			*backup = L'\0';
			mHasBackupOpenTag = false;
		}

		return mCurrentNodeType;
	}

	bool setText(wchar_t* start, wchar_t* end)
	{
		// Check if text is more than 2 characters, and if not, check if there is 
		// only white space, so that this text won't be reported
		if(end - start < 3) {
			const wchar_t* q = start;
			for(; q != end; ++q)
				if(!isWhiteSpace(*q))
					break;

			if(q == end)
				return false;
		}

		// Since we may lost the '<' character, we use a special tag to remember it
		mHasBackupOpenTag = (*end == L'<');

		// Set current text to the parsed text, and replace xml special characters
		mText = escapeString(start, end);

		// Current XML node type is text
		mCurrentNodeType = Event::Text;

		return true;
	}

	//! Ignores an xml definition like <?xml something />
	void ignoreDefinition()
	{
		mCurrentNodeType = Event::Unknown;

		// Move until end marked with '>' reached
		while(*p && *p != L'>')
			++p;
		++p;
	}

	//! parses a comment
	void parseComment()
	{
		mCurrentNodeType = Event::Comment;
		++p;

		const wchar_t* pCommentBegin = p;

		int count = 1;

		// Move until end of comment reached
		while(*p && count) {
			if(*p == L'>')
				--count;
			else if(*p == L'<')
				++count;
			++p;
		}

		mText = createString(pCommentBegin + 2, p - 3);
	}

	//! Parses an opening xml element and reads attributes
	void parseOpeningXMLElement()
	{
		mCurrentNodeType = Event::BeginElement;
		mIsEmptyElement = false;
		mAttrubutes.clear();

		// Find name
		const wchar_t* startName = p;

		// find end of element
		while(*p != L'>' && !isWhiteSpace(*p))
			++p;

		wchar_t* endName = p;

		// find mAttrubutes
		while(*p != L'>')
		{
			if(isWhiteSpace(*p))
				++p;
			else
			{
				if(*p != L'/')
				{
					// We've got an attribute

					// Read the attribute names
					const wchar_t* attributeNameBegin = p;

					while(!isWhiteSpace(*p) && *p != L'=')
						++p;

					wchar_t* attributeNameEnd = p;
					++p;

					// Read the attribute value, check for quotes and single quotes
					while((*p != L'\"') && (*p != L'\'') && *p) 
						++p;

					if(!*p) // Malformatted xml file
						return;

					const wchar_t attributeQuoteChar = *p;

					++p;
					wchar_t* attributeValueBegin = p;
					
					while(*p != attributeQuoteChar && *p)
						++p;

					if(!*p) // Malformatted xml file
						return;

					wchar_t* attributeValueEnd = p;
					++p;

					Attribute attr;
					attr.name = createString(attributeNameBegin, attributeNameEnd);
					attr.value = escapeString(attributeValueBegin, attributeValueEnd);
					mAttrubutes.push_back(attr);
				}
				else
				{
					// Tag is closed directly
					++p;
					mIsEmptyElement = true;
					break;
				}
			}
		}

		// Check if this tag is closing directly
		if(endName > startName && *(endName-1) == L'/')
		{
			// Directly closing tag
			mIsEmptyElement = true;
			--endName;
		}

		mElementName = createString(startName, endName);

		++p;
	}

	//! Parses an closing xml tag
	void parseClosingXMLElement()
	{
		mCurrentNodeType = Event::EndElement;
		mIsEmptyElement = false;
		mAttrubutes.clear();

		++p;
		wchar_t* pBeginClose = p;

		while(*p && *p != L'>')
			++p;

		if(!*p) {
			mCurrentNodeType = Event::Error;
			return;
		}

		mElementName = createString(pBeginClose, p);
		++p;
	}

	//! Parses a possible CDATA section, returns false if begin was not a CDATA section
	bool parseCDATA()
	{
		if(*(p+1) != L'[')
			return false;

		mCurrentNodeType = Event::CData;

		// Skip '<![CDATA['
		int count=0;
		while(*p && count < 8) {
			++p;
			++count;
		}

		if(!*p)
			return true;

		const wchar_t* cDataBegin = p;
		wchar_t* cDataEnd = nullptr;

		// Find end of CDATA
		while(*p && !cDataEnd) {
			if(*p == L'>' && (*(p-1) == L']') && (*(p-2) == L']'))
				cDataEnd = p - 2;
			++p;
		}

		if(cDataEnd)
			mText = createString(cDataBegin, cDataEnd);
		else
			mText = L"";

		return true;
	}

	const wchar_t* attributeValue(const wchar_t* name) const
	{
		if(!name)
			return nullptr;

		for(Attrubutes::const_iterator i=mAttrubutes.begin(); i!=mAttrubutes.end(); ++i)
			if(::wcscmp(name, i->name) == 0)
				return i->value;
		return nullptr;
	}

	const wchar_t* attributeValueIgnoreCase(const wchar_t* name) const
	{
		if(!name)
			return nullptr;

		for(Attrubutes::const_iterator i=mAttrubutes.begin(); i!=mAttrubutes.end(); ++i)
			if(wstrCaseCmp(name, i->name) == 0)
				return i->value;
		return nullptr;
	}

	wchar_t* p;	//!< The current pointer
	const wchar_t* mText;
	const wchar_t* mElementName;
	bool mIsEmptyElement;
	bool mHasBackupOpenTag;	//! Set to true when '\0' is assigned because of Text event.
	Event::Enum mCurrentNodeType;

	Attrubutes mAttrubutes;
};	// Impl

XmlParser::XmlParser()
{
	mImpl = new Impl();
}

XmlParser::~XmlParser()
{
	delete mImpl;
}

void XmlParser::parse(wchar_t* source)
{
	MCD_ASSUME(mImpl);
	mImpl->parse(source);
}

XmlParser::Event::Enum XmlParser::nextEvent()
{
	MCD_ASSUME(mImpl);
	return mImpl->nextEvent();
}

const wchar_t* XmlParser::elementName() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mElementName;
}

bool XmlParser::isEmptyElement() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mIsEmptyElement;
}

const wchar_t* XmlParser::textData() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mText;
}

size_t XmlParser::attributeCount() const
{
	MCD_ASSUME(mImpl);
	return mImpl->mAttrubutes.size();
}

const wchar_t* XmlParser::attributeName(size_t idx) const
{
	MCD_ASSUME(mImpl);
	return idx < mImpl->mAttrubutes.size() ? mImpl->mAttrubutes.at(idx).name : nullptr;
}

const wchar_t* XmlParser::attributeValue(size_t idx) const
{
	MCD_ASSUME(mImpl);
	return idx < mImpl->mAttrubutes.size() ? mImpl->mAttrubutes.at(idx).value : nullptr;
}

const wchar_t* XmlParser::attributeValue(const wchar_t* name) const
{
	MCD_ASSUME(mImpl);
	return mImpl->attributeValue(name);
}

const wchar_t* XmlParser::attributeValueIgnoreCase(const wchar_t* name) const
{
	MCD_ASSUME(mImpl);
	return mImpl->attributeValueIgnoreCase(name);
}

float XmlParser::attributeValueAsFloat(size_t idx, float defaultValue) const
{
	return stringToFloat(attributeValue(idx), defaultValue);
}

float XmlParser::attributeValueAsFloat(sal_in_z const wchar_t* name, float defaultValue) const
{
	return stringToFloat(attributeValue(name), defaultValue);
}

float XmlParser::attributeValueAsFloatIgnoreCase(sal_in_z const wchar_t* name, float defaultValue) const
{
	return stringToFloat(attributeValueIgnoreCase(name), defaultValue);
}

float XmlParser::stringToFloat(const wchar_t* str, float defaultValue)
{
	double value = defaultValue;
	if(!str || !wStr2Double(str, value))
		return defaultValue;
	return float(value);
}

}	// namespace MCD
