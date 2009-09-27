#include "Pch.h"
#include "FbxNodeAdaptors.h"

#include <fbxsdk.h>
#include <vector>

namespace MCD
{
//------------------------------------------------------------------------------
// FbxNodeAdaptor
//------------------------------------------------------------------------------
class FbxNodeAdaptor::Impl
{
public:
	Impl() {}
	~Impl() {}

	KFbxNode * mFbxnode;
};

FbxNodeAdaptor::FbxNodeAdaptor(KFbxNode * fbxnode)
	: mImpl(new Impl)
{
	mImpl->mFbxnode = fbxnode;
}

FbxNodeAdaptor::~FbxNodeAdaptor()
{
	delete mImpl;
}

KFbxNode* FbxNodeAdaptor::getKFbxNode() const
{
	return mImpl->mFbxnode;
}

const char* FbxNodeAdaptor::getName() const
{
	return mImpl->mFbxnode->GetName();
}

bool FbxNodeAdaptor::getVisible() const
{
	return mImpl->mFbxnode->Show.Get();
}

KFbxMatrix FbxNodeAdaptor::getGeometryTransform() const
{
	KFbxVector4 lT, lR, lS;
	KFbxXMatrix lGeometry;

	lT = mImpl->mFbxnode->GetGeometricTranslation(KFbxNode::eSOURCE_SET);
	lR = mImpl->mFbxnode->GetGeometricRotation(KFbxNode::eSOURCE_SET);
	lS = mImpl->mFbxnode->GetGeometricScaling(KFbxNode::eSOURCE_SET);

	lGeometry.SetT(lT);
	lGeometry.SetR(lR);
	lGeometry.SetS(lS);

	return lGeometry;
}

KFbxMatrix FbxNodeAdaptor::getGlobalTransform(int frame) const
{
	if(frame < 0)
	{
		return mImpl->mFbxnode->GetGlobalFromDefaultTake();
	}
	else
	{
		KTime time; time.SetTime( 0, 0, 0, frame );
		return mImpl->mFbxnode->GetGlobalFromCurrentTake(time);
	}
}

//------------------------------------------------------------------------------
// FbxSkeletonAdaptor
//------------------------------------------------------------------------------
class FbxSkeletonAdaptor::Impl
{
public:
	Impl() {}
	~Impl() {}

	std::vector<const char*> mBoneNames;

	static bool IsNubBone( KFbxNode * node );
};

bool FbxSkeletonAdaptor::Impl::IsNubBone( KFbxNode * node )
{
	if ( node->GetNodeAttribute() )
	{
		if ( node->GetNodeAttribute()->GetAttributeType() == KFbxNodeAttribute::eSKELETON )
		{
			std::string name = node->GetName();
			int length = name.length();
			if (name.substr(length-3) == "Nub")
				return true;
		}
	}

	return false;
}

FbxSkeletonAdaptor::FbxSkeletonAdaptor(KFbxScene* fbxscene)
	: mImpl(new Impl)
{
	KArrayTemplate<KFbxNode*> boneArray;
	fbxscene->FillNodeArray( boneArray, KFbxNodeAttribute::eSKELETON );

	const int cBoneCnt = boneArray.GetCount();

	//mImpl->mBones.reserve(cBoneCnt);
	mImpl->mBoneNames.reserve(cBoneCnt);

	for(int i = 0; i < cBoneCnt; ++i)
	{
		if( Impl::IsNubBone( boneArray.GetAt(i) ))
			continue;

		//mImpl->mBones.push_back( boneArray.GetAt(i) );
		mImpl->mBoneNames.push_back( boneArray.GetAt(i)->GetName() );
	}
}

FbxSkeletonAdaptor::~FbxSkeletonAdaptor()
{
	delete mImpl;
}

int FbxSkeletonAdaptor::getBoneIndex(KFbxNode* bone) const
{
	if(NULL == bone)
		return -1;

	const int cBoneCnt = (int)mImpl->mBoneNames.size();

	for(int i = 0; i < cBoneCnt; ++i)
	{
		if( 0 == strcmp( bone->GetName(), mImpl->mBoneNames[i] ) )
			return i;
	}

	return -1;
}

}   // namespace MCD