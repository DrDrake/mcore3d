#ifndef __MCD_RENDER_MAX3DSLOADERLOADER__
#define __MCD_RENDER_MAX3DSLOADERLOADER__

#include "Color.h"
#include "MeshBuilder.h"
#include "../Core/System/IntrusivePtr.h"
#include "../Core/System/Mutex.h"
#include "../Core/System/ResourceLoader.h"
#include <list>

namespace MCD {

class Material;

class ResourceManager;

/*!
	\sa http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
	\sa http://www.gamedev.net/community/forums/topic.asp?topic_id=382606
 */
class MCD_RENDER_API Max3dsLoader : public IResourceLoader, private Noncopyable
{
public:
	Max3dsLoader(sal_maybenull ResourceManager* resourceManager);

	sal_override ~Max3dsLoader();

	sal_override LoadingState load(sal_maybenull std::istream* is);

	sal_override void commit(Resource& resource);

	sal_override LoadingState getLoadingState() const;

protected:
	void readColor(ColorRGBf& color);

	//! Read a 16 bit integer (why is call perc?).
	int16_t readPercentageAsInt();

	size_t readString(std::wstring& str);

private:
	class Stream;
	Stream* mStream;

protected:
	ResourceManager* mResourceManager;

	//! Represent which face the material is assigned to.
	struct MultiSubObject {
		Material* material;
		std::vector<uint16_t> mFaceIndex;	//! Index to the index buffer
	};

	struct ModelInfo {
		MeshBuilder* meshBuilder;	//! Contains vertex buffer only
		std::vector<uint16_t> index;//! The triangel index
		std::list<MultiSubObject> multiSubObject;
		std::vector<uint32_t> smoothingGroup;
	};

	std::list<ModelInfo> mModelInfo;
	MeshBuilder mMeshBuilder;

	class NamedMaterial;
	typedef std::list<NamedMaterial*> MaterialList;
	MaterialList mMaterials;

	volatile IResourceLoader::LoadingState mLoadingState;
	mutable Mutex mMutex;
};	// Max3dsLoader

}	// namespace MCD

#endif	// __MCD_RENDER_MAX3DSLOADERLOADER__
