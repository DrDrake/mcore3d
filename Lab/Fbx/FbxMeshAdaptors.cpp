#include "Pch.h"
#include "FbxMeshAdaptors.h"

#include <fbxsdk.h>
#include <sstream>
#include <vector>
#include <algorithm>

namespace MCD
{

//------------------------------------------------------------------------------
/*! FbxMeshAdaptor */
class FbxMaterialAdaptor::Impl
{
public:
	Impl() : mFbxmaterial(nullptr)
	{
	}

	~Impl()
	{
	}

	KFbxSurfaceMaterial* mFbxmaterial;
};

FbxMaterialAdaptor::FbxMaterialAdaptor(KFbxSurfaceMaterial* fbxmaterial)
{
	mImpl = new Impl;
	mImpl->mFbxmaterial = fbxmaterial;
}

FbxMaterialAdaptor::~FbxMaterialAdaptor()
{
	delete mImpl;
}

std::string FbxMaterialAdaptor::getMaterialName() const
{
	// process material name
	std::string materialName = mImpl->mFbxmaterial->GetName();

	for( size_t i = 0; i < materialName.length(); ++i)
	{
		if(materialName[i] == ' ')
		{
			materialName[i] = '_';
		}
	}

	for(size_t i=0; i<materialName.length(); ++i)
		materialName[i] = ::tolower(materialName[i]);

	return materialName;
}

std::string FbxMaterialAdaptor::getFxParams() const
{
	const KFbxImplementation* fxImpl = GetImplementation(mImpl->mFbxmaterial, eImplHLSL);

	// opps... not HLSL, try CgFx
	if (!fxImpl)
		fxImpl = GetImplementation(mImpl->mFbxmaterial, eImplCGFX);

	if (!fxImpl)
		return "";
	
	//Now we have a hardware shader, let's read it
	KFbxBindingTable const* fxRootTable = fxImpl->GetRootTable();

	std::ostringstream ss;
	
	// process Fx parameters
	ss << ";fx;" << fxRootTable->DescAbsoluteURL.Get().Buffer();
	ss << ";tech;" << fxRootTable->DescTAG.Get().Buffer(); 

	const size_t ENTRY_CNT = fxRootTable->GetEntryCount();

	for ( size_t ientry = 0; ientry < ENTRY_CNT; ++ientry )
	{
		const KFbxBindingTableEntry & fbxtableEntry = fxRootTable->GetEntry(ientry);

		KFbxProperty fbxprop;

		if ( strcmp( KFbxPropertyEntryView::sEntryType, fbxtableEntry.GetEntryType(true) ) == 0 )
		{   
			fbxprop = mImpl->mFbxmaterial->FindPropertyHierarchical(fbxtableEntry.GetSource()); 
			if(!fbxprop.IsValid())
				fbxprop = mImpl->mFbxmaterial->RootProperty.FindHierarchical(fbxtableEntry.GetSource());
		}

		if ( fbxprop.IsValid() )
		{
			KFbxDataType fbxdataType = fbxprop.GetPropertyDataType();

			if( fbxprop.GetSrcObjectCount( FBX_TYPE(KFbxTexture) ) > 0 )
			{
				// special handling to texture parameter
				const size_t TEX_CNT = (size_t)fbxprop.GetSrcObjectCount(FBX_TYPE(KFbxTexture));
				for(size_t itex = 0; itex < TEX_CNT; ++itex)
				{
					KFbxTexture * lTex = fbxprop.GetSrcObject(FBX_TYPE(KFbxTexture), itex);
					ss << ";" << fbxprop.GetName().Buffer() << ";" << lTex->GetFileName();
				}
			}
			else
			{
				// otherwise just convert it to string
				ss << ";" << fbxprop.GetName().Buffer() << ";" << KFbxGet<fbxString>(fbxprop).Buffer();
			}
		}
	}

	return ss.str();


}

//------------------------------------------------------------------------------
/*! FbxMeshAdaptor */
class FbxMeshAdaptor::Impl
{
public:
	Impl()
	: mElemMaterial(nullptr) 
	, mElemNormal(nullptr)
	, mElemVertexColor(nullptr)
	{
	}

	~Impl()
	{
	}

	KFbxMesh* mFbxmesh;

	KFbxLayerElementMaterial*           mElemMaterial;
	KFbxLayerElementNormal*             mElemNormal;
	KFbxLayerElementVertexColor*        mElemVertexColor;
	std::vector<KFbxLayerElementUV*>    mElemUV;

	void FindAttributes();

};

void FbxMeshAdaptor::Impl::FindAttributes()
{
	const int cLayerCnt = mFbxmesh->GetLayerCount();

	for(int i = 0; i < cLayerCnt; ++i)
	{
		if(nullptr != mFbxmesh->GetLayer(i)->GetMaterials())
		{
			mElemMaterial = mFbxmesh->GetLayer(i)->GetMaterials();
		}

		if(nullptr != mFbxmesh->GetLayer(i)->GetNormals())
		{
			mElemNormal = mFbxmesh->GetLayer(i)->GetNormals();
		}

		if(nullptr != mFbxmesh->GetLayer(i)->GetVertexColors())
		{
			mElemVertexColor = mFbxmesh->GetLayer(i)->GetVertexColors();
		}
		
		if(nullptr != mFbxmesh->GetLayer(i)->GetUVs())
		{
			mElemUV.push_back( mFbxmesh->GetLayer(i)->GetUVs() );
		}
	}
}

FbxMeshAdaptor::FbxMeshAdaptor(KFbxMesh* fbxmesh)
	: mImpl(new Impl)
{
	mImpl->mFbxmesh = fbxmesh;
	mImpl->FindAttributes();
}

FbxMeshAdaptor::~FbxMeshAdaptor()
{
	delete mImpl;
}

KFbxMesh* FbxMeshAdaptor::getKFbxMesh() const
{
	return mImpl->mFbxmesh;
}

bool FbxMeshAdaptor::hasNormal() const
{
	return mImpl->mElemNormal != nullptr;
}

bool FbxMeshAdaptor::hasVertexColor() const
{
	return mImpl->mElemVertexColor != nullptr;
}

bool FbxMeshAdaptor::hasUV(size_t channel) const
{
	return mImpl->mElemUV.size() > channel;
}

size_t FbxMeshAdaptor::getUVChannelCount() const
{
	return mImpl->mElemUV.size();
}

size_t FbxMeshAdaptor::getFaceCount() const
{
	return mImpl->mFbxmesh->GetPolygonCount();
}

int FbxMeshAdaptor::getMaterialId(size_t face)
{
	if(nullptr == mImpl->mElemMaterial)
	{
		return -1;
	}
	else
	{
		int matIdx = mImpl->mElemMaterial->GetIndexArray().GetAt(face);
		return matIdx;
	}
}

void FbxMeshAdaptor::getPosition
	( size_t face, size_t vertex
	, double& x, double& y, double& z
	)
{
	int index = mImpl->mFbxmesh->GetPolygonVertex(face, vertex);
	KFbxVector4& data = mImpl->mFbxmesh->GetControlPoints()[index];

	x = data[0];
	y = data[1];
	z = data[2];
}

void FbxMeshAdaptor::getPosition
	( size_t face, size_t vertex
	, const KFbxMatrix& transform
	, double& x, double& y, double& z
	)
{
	getPosition(face, vertex, x, y, z);

	KFbxVector4 data = transform.MultNormalize(KFbxVector4(x, y, z, 1));

	x = data[0];
	y = data[1];
	z = data[2];
}

void FbxMeshAdaptor::getNormal
	( size_t face, size_t vertex
	, double& x, double& y, double& z
	)
{
	if(!hasNormal())
	{
		KFbxMesh* fbxmesh = mImpl->mFbxmesh;

		// comput the face normal according to the positions
		KFbxVector4 v0, v1, v2;
		v0 = fbxmesh->GetControlPoints()[fbxmesh->GetPolygonVertex(face, 0)];
		v1 = fbxmesh->GetControlPoints()[fbxmesh->GetPolygonVertex(face, 1)];
		v2 = fbxmesh->GetControlPoints()[fbxmesh->GetPolygonVertex(face, 2)];

		KFbxVector4 v10 = v1 - v0;
		KFbxVector4 v20 = v2 - v0;
		KFbxVector4 n = v10.CrossProduct(v20);
		n.Normalize();

		x = n[0];
		y = n[1];
		z = n[2];
	}
	else
	{
		KFbxLayerElementNormal* elem = mImpl->mElemNormal;

		MCD_ASSUME(elem->GetMappingMode() == KFbxLayerElement::eBY_POLYGON_VERTEX);

		KFbxVector4 data;
		if(elem->GetReferenceMode() == KFbxLayerElement::eDIRECT)
		{
			// non indexed data
			data = elem->GetDirectArray().GetAt(face * 3 + vertex);
		}
		else
		{
			// indexed data
			int index = elem->GetIndexArray().GetAt(face * 3 + vertex);
			data = elem->GetDirectArray().GetAt(index);
		}

		x = data[0];
		y = data[1];
		z = data[2];
	}
}

void FbxMeshAdaptor::getNormal
	( size_t face, size_t vertex
	, const KFbxMatrix& transform
	, double& x, double& y, double& z
	)
{
	getNormal(face, vertex, x, y, z);

	KFbxVector4 n = transform.MultNormalize(KFbxVector4(x, y, z, 0));
	x = n[0];
	y = n[1];
	z = n[2];
}

void FbxMeshAdaptor::getVertexColor
	( size_t face, size_t vertex
	, double& r, double& g, double& b, double& a
	)
{
	if(!hasVertexColor())
	{
		r = g = b = a = 1.0f;
	}
	else
	{
		KFbxLayerElementVertexColor* elem = mImpl->mElemVertexColor;

		MCD_ASSUME(elem->GetMappingMode() == KFbxLayerElement::eBY_POLYGON_VERTEX);

		KFbxColor data;
		if(elem->GetReferenceMode() == KFbxLayerElement::eDIRECT)
		{
			// non indexed data
			data = elem->GetDirectArray().GetAt(face * 3 + vertex);
		}
		else
		{
			// indexed data
			int index = elem->GetIndexArray().GetAt(face * 3 + vertex);
			data = elem->GetDirectArray().GetAt(index);
		}

		r = data.mRed;
		g = data.mGreen;
		b = data.mBlue;
		a = data.mAlpha;
	}
}

void FbxMeshAdaptor::getUV
	( size_t face, size_t vertex, size_t channel
	, double& u, double& v
	)
{
	if(!hasUV(channel))
	{
		u = v = 0.0f;
	}
	else
	{
		KFbxLayerElementUV* elem = mImpl->mElemUV[channel];

		MCD_ASSUME(elem->GetMappingMode() == KFbxLayerElement::eBY_POLYGON_VERTEX);

		KFbxVector2 data;
		if(elem->GetReferenceMode() == KFbxLayerElement::eDIRECT)
		{
			// non indexed data
			data = elem->GetDirectArray().GetAt(face * 3 + vertex);
		}
		else
		{
			// indexed data
			int index = elem->GetIndexArray().GetAt(face * 3 + vertex);
			data = elem->GetDirectArray().GetAt(index);
		}

		u = data[0];
		v = data[1];
	}
}

//------------------------------------------------------------------------------
/*! FbxSkinnedMeshAdaptor::BoneAssignment */
FbxSkinnedMeshAdaptor::BoneAssignment::BoneAssignment()
	: Count(0)
{
	for(int i = 0; i < MAX_ASSIGNMENT; ++i)
	{
		Bone[i] = nullptr;
		BoneName[i] = nullptr;
		Weight[i] = 0;
	}
}

void FbxSkinnedMeshAdaptor::BoneAssignment::Add(KFbxNode* bone, double weight)
{
	if(Count < MAX_ASSIGNMENT)
	{
		// check for duplicate bone assignments
		for(int i = 0; i < Count; ++i)
		{
			if(strcmp(BoneName[i], bone->GetName()) == 0)
				return;
		}

		Bone[Count] = bone;
		BoneName[Count] = bone->GetName();
		Weight[Count] = weight;
		++Count;
	}
}

//------------------------------------------------------------------------------
/*! FbxSkinnedMeshAdaptor */
class FbxSkinnedMeshAdaptor::Impl
{
public:
	Impl()
	{
	}

	~Impl()
	{
	}

	std::vector<KFbxSkin*> mSkinDeformers;
	std::vector<BoneAssignment> mBoneAssignments;

	void FindDeformers(KFbxMesh* fbxmesh);
	void ReadBoneAssignments(KFbxMesh* fbxmesh);
};

void FbxSkinnedMeshAdaptor::Impl::FindDeformers(KFbxMesh* fbxmesh)
{
	const int cSkinCnt = fbxmesh->GetDeformerCount(KFbxDeformer::eSKIN);

	for(int i = 0; i < cSkinCnt; ++i)
	{
		mSkinDeformers.push_back( (KFbxSkin*)fbxmesh->GetDeformer(i, KFbxDeformer::eSKIN) );
	}
}

void FbxSkinnedMeshAdaptor::Impl::ReadBoneAssignments(KFbxMesh* fbxmesh)
{
	// allocate the bone assignments,
	// it should be equals to the # of ControlPoints (positions) in the fbxmesh
	if(!mBoneAssignments.empty())
		mBoneAssignments.clear();

	const int cVertexCnt = fbxmesh->GetControlPointsCount();
	mBoneAssignments.resize( cVertexCnt );

	// read them from the mSkinDeformers
	const int cSkinCnt = (int)mSkinDeformers.size();

	for(int iskin = 0; iskin < cSkinCnt; ++iskin)
	{
		KFbxSkin* fbxskin = mSkinDeformers[iskin];

		const int cClusterCnt = fbxskin->GetClusterCount();

		for(int icluster = 0; icluster < cClusterCnt; ++icluster)
		{
			// cluster store the assigned vertex indices & weights
			// of a bone ( fbxcluster->GetLink() )
			KFbxCluster* fbxcluster = fbxskin->GetCluster(icluster);
			KFbxNode* linkedBone = fbxcluster->GetLink();

			// sorry we do not support KFbxCluster::eADDITIVE
			if(fbxcluster->GetLinkMode() == KFbxCluster::eADDITIVE)
				continue;

			const int cIndexCnt = fbxcluster->GetControlPointIndicesCount();
			int* indices    = fbxcluster->GetControlPointIndices();
			double* weights = fbxcluster->GetControlPointWeights();

			for(int iindex = 0; iindex < cIndexCnt; ++iindex)
			{
				MCD_ASSUME(indices[iindex] < cVertexCnt);
				mBoneAssignments[indices[iindex]].Add( linkedBone, weights[iindex] );
			}
		}
	}

	for(int ivert = 0; ivert < cVertexCnt; ++ivert)
	{
		BoneAssignment& boneAssig = mBoneAssignments[ivert];

		if(boneAssig.Count > 0)
		{
			double weightSum = 0.0f;
			for(int i = 0; i < boneAssig.Count; ++i) { weightSum += boneAssig.Weight[i]; }

			if(weightSum > 0.0f)
			{
				weightSum = 1.0f / weightSum;
				for(int i = 0; i < boneAssig.Count; ++i) { boneAssig.Weight[i] *= weightSum; }
			}
		}

	}
}

FbxSkinnedMeshAdaptor::FbxSkinnedMeshAdaptor(KFbxMesh* fbxmesh)
	: FbxMeshAdaptor(fbxmesh)
	, mImpl(new Impl)
{
	mImpl->FindDeformers(fbxmesh);
	mImpl->ReadBoneAssignments(fbxmesh);
}

FbxSkinnedMeshAdaptor::~FbxSkinnedMeshAdaptor()
{
	delete mImpl;
}

void FbxSkinnedMeshAdaptor::getBoneAssignment
	( size_t face, size_t vertex
	, BoneAssignment& boneAssig )
{
	int index = getKFbxMesh()->GetPolygonVertex(face, vertex);
	boneAssig = mImpl->mBoneAssignments[index];
}

//------------------------------------------------------------------------------
/*! SkinnedVertexDebugInfo */
void SkinnedVertexDebugInfo::ProcessAndSaveToTextFile(
	const char* filename
	, SkinnedVertexDebugInfo* debugInfo
	, size_t cnt
	)
{
	FILE* fp = fopen(filename, "w");

	std::sort(&debugInfo[0], &debugInfo[cnt-1]);

	size_t uniqueCnt = std::distance
		( &debugInfo[0]
		, std::unique(&debugInfo[0], &debugInfo[cnt-1])
		);

	for(size_t i = 0; i < uniqueCnt; ++i)
	{
		const SkinnedVertexDebugInfo & dbgInfo = debugInfo[i];

		fprintf(fp, "v%04d; %2d; %8.3f; %s\n"
			, dbgInfo.controlPointIndex
			, dbgInfo.boneIndex
			, dbgInfo.boneWeight
			, dbgInfo.boneName
			);
	}

	fclose(fp);
}

}	// namespace MCD