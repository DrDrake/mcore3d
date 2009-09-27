#ifndef __MCD_FBX_FBXMESHADAPTORS__
#define __MCD_FBX_FBXMESHADAPTORS__

namespace MCD
{

class MCD_FBX_API FbxMaterialAdaptor
{
public:
	FbxMaterialAdaptor(KFbxSurfaceMaterial* fbxmaterial);

	~FbxMaterialAdaptor();

	std::string getMaterialName() const;

	std::string getFxParams() const;

private:
	class Impl;
	Impl* mImpl;

};  // class FbxMaterialAdaptor

/*! A wrapper which hides the detials on how to access the mesh data inside a KFbxMesh.
	The mesh data are accessed using the both face and vertex index.
*/
class MCD_FBX_API FbxMeshAdaptor
{
public:
	FbxMeshAdaptor(sal_notnull KFbxMesh* fbxmesh);

	virtual ~FbxMeshAdaptor();

	KFbxMesh* getKFbxMesh() const;

	size_t getFaceCount() const;

	bool hasNormal() const;

	bool hasVertexColor() const;

	bool hasUV(size_t channel) const;

	size_t getUVChannelCount() const;

	int getMaterialId(size_t face);

	void getPosition
		( size_t face, size_t vertex
		, double& x, double& y, double& z);

	void getPosition
		( size_t face, size_t vertex
		, const KFbxMatrix& transform
		, double& x, double& y, double& z);

	void getNormal
		( size_t face, size_t vertex
		, double& x, double& y, double& z);

	void getNormal
		( size_t face, size_t vertex
		, const KFbxMatrix& transform
		, double& x, double& y, double& z);

	void getVertexColor
		( size_t face, size_t vertex
		, double& r, double& g, double& b, double& a);

	void getUV
		( size_t face, size_t vertex, size_t channel
		, double& u, double& v);

private:
	class Impl;
	Impl* mImpl;

};  // class FbxMeshAdaptor

/*! A wrapper which hides the detials on how to access the skinning data inside a KFbxMesh.
*/
class MCD_FBX_API FbxSkinnedMeshAdaptor
	: public FbxMeshAdaptor
{
public:
	struct BoneAssignment
	{
		BoneAssignment();
		void Add(sal_notnull KFbxNode* bone, double weight);
		
		enum { MAX_ASSIGNMENT = 4 };

		int Count;
		KFbxNode* Bone[MAX_ASSIGNMENT];
		const char* BoneName[MAX_ASSIGNMENT];
		double Weight[MAX_ASSIGNMENT];
	};

public:
	FbxSkinnedMeshAdaptor(KFbxMesh* fbxmesh);

	~FbxSkinnedMeshAdaptor();

	void getBoneAssignment
		( size_t face, size_t vertex
		, BoneAssignment& boneAssig );

private:
	class Impl;
	Impl* mImpl;
};  // class FbxSkinnedMeshAdaptor


/*! For internal uses only
*/
struct SkinnedVertexDebugInfo
{
	int controlPointIndex;
	int boneIndex;
	double boneWeight;
	const char* boneName;

	bool operator < (const SkinnedVertexDebugInfo & rhs) const
	{
		if( controlPointIndex < rhs.controlPointIndex )
			return true;
		else if( controlPointIndex == rhs.controlPointIndex && boneIndex < rhs.boneIndex )
			return true;

		return false;
	}

	bool operator == (const SkinnedVertexDebugInfo & rhs) const
	{
		return (controlPointIndex == rhs.controlPointIndex)
			&& (boneIndex == rhs.boneIndex)
			&& ( fabsf(boneWeight - rhs.boneWeight) < 1e-5f );
	}

	static void ProcessAndSaveToTextFile(
		const char* filename
		, SkinnedVertexDebugInfo* debugInfo
		, size_t cnt
		);

};  // struct SkinnedVertexDebugInfo

}   // namespace MCD

#endif  // __MCD_FBX_FBXMESHADAPTORS__