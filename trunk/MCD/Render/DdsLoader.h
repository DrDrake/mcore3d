#ifndef __SGE_RENDER_DDSLOADER__
#define __SGE_RENDER_DDSLOADER__

#include "TextureLoaderBase.h"

namespace SGE {

/*!	Load dds file.
	This loader support background loading but not progressive loading.
 */
class SGE_RENDER_API DdsLoader : public TextureLoaderBase
{
	class LoaderImpl;

public:
	DdsLoader();

	/*!	Load data from stream.
		Block until all the data is read into it's internal buffer.
	 */
	sal_override LoadingState load(sal_maybenull std::istream* is);

protected:
	sal_override void uploadData();
};	// DdsLoader

}	// namespace SGE

#endif	// __SGE_RENDER_DDSLOADER__
