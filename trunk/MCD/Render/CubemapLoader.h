#ifndef __MCD_RENDER_CUBEMAPLOADER__
#define __MCD_RENDER_CUBEMAPLOADER__

#include "TextureLoaderBase.h"

namespace MCD {

/*!	Load bitmap file.
 */
class MCD_RENDER_API CubemapLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	CubemapLoader();

    sal_override ~CubemapLoader();

    /*! The texture file command
    */
    static wchar_t* texFileCmd() { return L"cubemap:"; }

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

protected:
    sal_override void preUploadData();
    sal_override void uploadData();
    sal_override void postUploadData();
    sal_override int textureType() const;

};	// CubemapLoader

}	// namespace MCD

#endif	// __MCD_RENDER_CUBEMAPLOADER__
