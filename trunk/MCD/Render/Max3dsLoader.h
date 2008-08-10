#ifndef __MCD_RENDER_MAX3DSLOADERLOADER__
#define __MCD_RENDER_MAX3DSLOADERLOADER__

#include "MeshBuilder.h"
#include "../Core/System/IntrusivePtr.h"
#include <list>

namespace MCD {

class Texture;
typedef IntrusivePtr<Texture> TexturePtr;

class ResourceManager;

struct MCD_RENDER_API Color
{
	float r, g, b;
};

class MCD_RENDER_API Material
{
public:
	void bind() const;

	Color mAmbient;
	Color mDiffuse;
	Color mSpecular;
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

// Compute vertex normals
// Reference: http://www.gamedev.net/community/forums/topic.asp?topic_id=313015
// Reference: http://www.devmaster.net/forums/showthread.php?t=414
void computeNormal(Vec3f* vertex, Vec3f* normal, uint16_t* index, size_t vertexCount, size_t indexCount);

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
	void readColor(Color& color);

	//! Read a 16 bit integer (why is call perc?).
	uint16_t readPercInt();

	size_t readString(std::wstring& str);

private:
	class Stream;
	Stream* mStream;

protected:
	ResourceManager* mResourceManager;

	struct MeshBuilderAndMaterial {
		MeshBuilder* meshBuilder;
		Material* material;
	};

	std::list<MeshBuilderAndMaterial> mMeshBuilders;
	MeshBuilder mMeshBuilder;

	class NamedMaterial;
	typedef std::list<NamedMaterial*> MaterialList;
	MaterialList mMaterials;
};	// Max3dsLoader

}	// namespace MCD

#endif	// __MCD_RENDER_MAX3DSLOADERLOADER__
