#ifndef __MCD_FBX_FBXNODEADAPTORS__
#define __MCD_FBX_FBXNODEADAPTORS__

namespace MCD
{

class MCD_FBX_API FbxNodeAdaptor
{
public:
	FbxNodeAdaptor(KFbxNode * fbxnode);

	~FbxNodeAdaptor();

	KFbxNode* getKFbxNode() const;

	/*! Returns the associated KFbxNode's name.*/
	const char* getName() const;

	/*! Returns true if the associated KFbxNode is shown in scene.*/
	bool getVisible() const;

	/*! Geometry transform is similar to the pivot transform in 3dsmax */
	KFbxMatrix getGeometryTransform() const;

	/*! frame = -1 for default take
	*/
	KFbxMatrix getGlobalTransform(int frame) const;

private:
	class Impl;
	Impl* mImpl;
};


/*! Mapping a bone KFbxNode to an index
*/
class MCD_FBX_API FbxSkeletonAdaptor
{
public:
	FbxSkeletonAdaptor(KFbxScene* fbxscene);
	~FbxSkeletonAdaptor();

	int getBoneIndex(KFbxNode* bone) const;

private:
	class Impl;
	Impl* mImpl;
};

}


#endif  // __MCD_FBX_FBXNODEADAPTORS__