#ifndef __MCD_RENDER_EFFECTLOADER__
#define __MCD_RENDER_EFFECTLOADER__

#include "ShareLib.h"
#include "../Core/System/NonCopyable.h"
#include "../Core/System/Path.h"
#include "../Core/System/ResourceLoader.h"

namespace MCD {

class IMaterial;
class IResourceManager;
class XmlParser;

class MCD_RENDER_API EffectLoader : public IResourceLoader, private Noncopyable
{
public:
	class MCD_ABSTRACT_CLASS ILoader
	{
	public:
		struct Context
		{
			//! The current parsing render pass
			size_t pass;

			/*!	The directory where the effect file is stored.
				For example, a texture path as seen in the effect file will
				combine with this basePath and then use the resulting path
				to query about the resourceManager.
			 */
			Path basePath;

			void* userData;
			IResourceManager& resourceManager;
		};	// Context

		virtual ~ILoader() {}
		//! Returns the xml element name that this loader response to.
		virtual const wchar_t* name() const = 0;
		virtual bool load(XmlParser& parser, IMaterial& material, Context& context) = 0;
	};	// ILoader

	explicit EffectLoader(IResourceManager& resourceManager);

	sal_override ~EffectLoader();

	sal_override LoadingState load(
		sal_maybenull std::istream* is, sal_maybenull const Path* fileId=nullptr);

	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	class Impl;
	Impl* mImpl;
};	// EffectLoader

}	// namespace MCD

#endif	// __MCD_RENDER_EFFECTLOADER__
