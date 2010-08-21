#include "Pch.h"
#include "FntLoader.h"
#include "../Render/Font.h"
#include "../Render/Texture.h"
#include "../Core/System/ResourceManager.h"
#include <sstream>

namespace MCD {

FntLoader::FntLoader(IResourceManager* resourceManager)
	: mResourceManager(resourceManager)
{
	mTmp = new BmpFont("");
}

// The loading code was base on
// http://www.gamedev.net/community/forums/topic.asp?topic_id=330742
IResourceLoader::LoadingState FntLoader::load(std::istream* is, const Path* fileId, const char*)
{
	mLoadingState = is ? NotLoaded : Aborted;

	if(mLoadingState & Stopped)
		return mLoadingState;

	MCD_ASSUME(is);

	std::string line, token, key, value;
	while(!is->eof()) {
		std::stringstream lineStream;
		std::getline(*is, line);
		lineStream << line;
		lineStream >> token;

		if(token == "common")
		{
			// This holds common data
			while(!lineStream.eof()) {
				std::stringstream converter;
				lineStream >> token;
				size_t i = token.find('=');
				key = token.substr(0, i);
				value = token.substr(i + 1);

				// Assign the correct value
				converter << value;
				if(key == "lineHeight")		converter >> mTmp->charSet.lineHeight;
				else if(key == "base")		converter >> mTmp->charSet.base;
				else if(key == "scaleW")	converter >> mTmp->charSet.width;
				else if(key == "scaleH")	converter >> mTmp->charSet.height;
				else if(key == "pages")		converter >> mTmp->charSet.pages;
			}
		}	// token == "common"
		else if(token == "char")
		{
			BmpFont::CharDescriptor* desc = nullptr;
			while(!lineStream.eof()) {
				std::stringstream converter;
				lineStream >> token;
				size_t i = token.find('=');
				key = token.substr(0, i);
				value = token.substr(i + 1);

				size_t charId = 0;

				// Assign the correct value
				converter << value;
				if(key == "id")	{				converter >> charId; desc = &mTmp->charSet.chars[charId];	}
				else {
					if(!desc) return Aborted;
					if(key == "x")				converter >> desc->x;
					else if(key == "y")			converter >> desc->y;
					else if(key == "width")		converter >> desc->width;
					else if(key == "height")	converter >> desc->height;
					else if(key == "xoffset")	converter >> desc->xOffset;
					else if(key == "yoffset")	converter >> desc->yOffset;
					else if(key == "xadvance")	converter >> desc->xAdvance;
					else if(key == "page")		converter >> desc->page;
				}
			}
		}
		else if(token == "page")	// TODO: Support multiple pages
		{
			while(!lineStream.eof()) {
				std::stringstream converter;
				lineStream >> token;
				size_t i = token.find('=');
				key = token.substr(0, i);
				value = token.substr(i + 1);

				if(key == "file" && mResourceManager) {
					// Trim the \" character
					value.erase(value.begin());
					value.resize(value.size() - 1);

					// We assume the texture is relative to the pod file, if the texture file didn't has a root path.
					Path adjustedPath = fileId ? fileId->getBranchPath()/value : value;

					mTmp->texture = dynamic_cast<Texture*>(mResourceManager->load(adjustedPath).get());
				}
			}
		}
		else if(token == "kerning")
		{
			int amount = 0;
			uint16_t firstChar = 0, secondChar = 0;
			while(!lineStream.eof()) {
				std::stringstream converter;
				lineStream >> token;
				size_t i = token.find('=');
				key = token.substr(0, i);
				value = token.substr(i + 1);

				converter << value;
				if(key == "first")
					converter >> firstChar;
				else if(key == "second")
					converter >> secondChar;
				else if(key == "amount") {
					converter >> amount;
					const int key2 = (uint32_t(firstChar) << 16) + secondChar;
					mTmp->kerning[key2] = amount;
					lineStream >> token;	// To eat the traling space
				}
			}
		}
	}

	mLoadingState = Loaded;

	return mLoadingState;
}

void FntLoader::commit(Resource& resource)
{
	if(!mTmp)
		return;

	// Will throw exception if the resource is not of the type BmpFont
	BmpFont& font = dynamic_cast<BmpFont&>(resource);

	font.texture = mTmp->texture;
	font.charSet = mTmp->charSet;
	font.kerning = mTmp->kerning;
	++resource.commitCount;
}

IResourceLoader::LoadingState FntLoader::getLoadingState() const
{
	return mLoadingState;
}

}	// namespace MCD
