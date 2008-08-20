#ifndef __MCD_RENDER_MAX3DSLOADERLOADER__
#define __MCD_RENDER_MAX3DSLOADERLOADER__

#include "Color.h"
#include "MeshBuilder.h"
#include "../Core/System/IntrusivePtr.h"
#include <list>

namespace MCD {

class Texture;
typedef IntrusivePtr<Texture> TexturePtr;

class ResourceManager;

class MCD_RENDER_API Material
{
public:
	Material();

	void bind() const;

	ColorRGBf mAmbient, mDiffuse, mSpecular;
	uint8_t mShininess;
	TexturePtr mTexture;
};	// Material

class MCD_RENDER_API Model : Noncopyable
{
public:
	Model()
	{
	}

	void draw();

public:
	struct MeshAndMaterial {
		MeshPtr mesh;
		Material material;
	};

	typedef std::list<MeshAndMaterial> MeshList;
	MeshList mMeshes;
};	// Model

/*!
	\sa http://www.flipcode.com/archives/Another_3DS_LoaderViewer_Class.shtml
	\sa http://www.gamedev.net/community/forums/topic.asp?topic_id=382606
 */
class MCD_RENDER_API Max3dsLoader
{
public:
	Max3dsLoader(std::istream& is_, sal_maybenull ResourceManager* resourceManager);

	~Max3dsLoader();

	void commit(Model& model, MeshBuilder::StorageHint storageHint);

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
};	// Max3dsLoader

}	// namespace MCD

#endif	// __MCD_RENDER_MAX3DSLOADERLOADER__
