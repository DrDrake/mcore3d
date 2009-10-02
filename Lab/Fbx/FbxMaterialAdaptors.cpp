#include "Pch.h"
#include "FbxMaterialAdaptors.h"

#include "../../MCD/Render/Color.h"

#include <fbxsdk.h>
#include <sstream>
#include <vector>
#include <algorithm>

namespace MCD
{

/*----------------------------------------------------------------------------*/
class FbxMaterialAdaptorImplBase
{
public:
	FbxMaterialAdaptorImplBase() : mFbxmaterial(nullptr) {}
	~FbxMaterialAdaptorImplBase() {}

	KFbxSurfaceMaterial* mFbxmaterial;

	std::string getMaterialName() const
	{
		// process material name
		std::string materialName = mFbxmaterial->GetName();

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
};

/*----------------------------------------------------------------------------*/
/*! FbxPhongMaterialAdaptor */
class FbxPhongMaterialAdaptor::Impl : public FbxMaterialAdaptorImplBase
{
public:
	KFbxSurfacePhong* mPhong;
};

FbxPhongMaterialAdaptor::FbxPhongMaterialAdaptor(KFbxSurfaceMaterial* fbxmaterial)
{
	mImpl = new Impl;
	mImpl->mFbxmaterial = fbxmaterial;
	mImpl->mPhong = KFbxCast<KFbxSurfacePhong>(mImpl->mFbxmaterial);
}

FbxPhongMaterialAdaptor::~FbxPhongMaterialAdaptor()
{
	delete mImpl;
}

bool FbxPhongMaterialAdaptor::valid() const
{
	return mImpl->mPhong != nullptr;
}

std::string FbxPhongMaterialAdaptor::getMaterialName() const
{
	return mImpl->getMaterialName();
}

ColorRGBAf FbxPhongMaterialAdaptor::getAmbientColor() const
{
	MCD_ASSUME(mImpl->mPhong);
	fbxDouble3 val = mImpl->mPhong->GetAmbientColor().Get();
	return ColorRGBAf(val[0], val[1], val[2], 1.0f);
}

ColorRGBAf FbxPhongMaterialAdaptor::getDiffuseColor() const
{
	MCD_ASSUME(mImpl->mPhong);
	fbxDouble3 val = mImpl->mPhong->GetDiffuseColor().Get();
	return ColorRGBAf(val[0], val[1], val[2], 1.0f);
}

ColorRGBAf FbxPhongMaterialAdaptor::getSpecularColor() const
{
	MCD_ASSUME(mImpl->mPhong);
	fbxDouble3 val = mImpl->mPhong->GetSpecularColor().Get();
	return ColorRGBAf(val[0], val[1], val[2], 1.0f);
}

float FbxPhongMaterialAdaptor::getShininess() const
{
	MCD_ASSUME(mImpl->mPhong);
	return mImpl->mPhong->GetShininess().Get();
}

bool FbxPhongMaterialAdaptor::hasDiffuseTexture() const
{
	return !getDiffuseTextureFilename().empty();
}

std::string FbxPhongMaterialAdaptor::getDiffuseTextureFilename() const
{
	KFbxProperty prop = mImpl->mPhong->FindProperty(KFbxSurfaceMaterial::sDiffuse);
	if(!prop.IsValid()) return "";

	KFbxTexture* text = KFbxCast <KFbxTexture>(prop.GetSrcObject(KFbxTexture::ClassId, 0));
	if(nullptr == text) return "";

	const char* texFilename = text->GetFileName();
	if(strlen(texFilename) == 0) return "";

	return text->GetFileName();
}

bool FbxPhongMaterialAdaptor::hasSpecularTexture() const
{
	return !getSpecularTextureFilename().empty();
}

std::string FbxPhongMaterialAdaptor::getSpecularTextureFilename() const
{
	KFbxProperty prop = mImpl->mPhong->FindProperty(KFbxSurfaceMaterial::sSpecular);
	if(!prop.IsValid()) return "";

	KFbxTexture* text = KFbxCast <KFbxTexture>(prop.GetSrcObject(KFbxTexture::ClassId, 0));
	if(nullptr == text) return "";

	const char* texFilename = text->GetFileName();
	if(strlen(texFilename) == 0) return "";

	return text->GetFileName();
}

bool FbxPhongMaterialAdaptor::hasBumpTexture() const
{
	return !getBumpTextureFilename().empty();
}

std::string FbxPhongMaterialAdaptor::getBumpTextureFilename() const
{
	KFbxProperty prop = mImpl->mPhong->FindProperty(KFbxSurfaceMaterial::sBump);
	if(!prop.IsValid()) return "";

	KFbxTexture* text = KFbxCast <KFbxTexture>(prop.GetSrcObject(KFbxTexture::ClassId, 0));
	if(nullptr == text) return "";

	const char* texFilename = text->GetFileName();
	if(strlen(texFilename) == 0) return "";

	return text->GetFileName();
}

bool FbxPhongMaterialAdaptor::hasSelfIllumTexture() const
{
	return !getSelfIllumTextureFilename().empty();
}

std::string FbxPhongMaterialAdaptor::getSelfIllumTextureFilename() const
{
	KFbxProperty prop = mImpl->mPhong->FindProperty(KFbxSurfaceMaterial::sEmissive);
	if(!prop.IsValid()) return "";

	KFbxTexture* text = KFbxCast <KFbxTexture>(prop.GetSrcObject(KFbxTexture::ClassId, 0));
	if(nullptr == text) return "";

	const char* texFilename = text->GetFileName();
	if(strlen(texFilename) == 0) return "";

	return text->GetFileName();
}

/*----------------------------------------------------------------------------*/
/*! FbxFxMaterialAdaptor */
class FbxFxMaterialAdaptor::Impl : public FbxMaterialAdaptorImplBase
{
public:
};

FbxFxMaterialAdaptor::FbxFxMaterialAdaptor(KFbxSurfaceMaterial* fbxmaterial)
{
	mImpl = new Impl;
	mImpl->mFbxmaterial = fbxmaterial;
}

FbxFxMaterialAdaptor::~FbxFxMaterialAdaptor()
{
	delete mImpl;
}

bool FbxFxMaterialAdaptor::valid() const
{
	if(nullptr == mImpl->mFbxmaterial)
		return false;

	const KFbxImplementation* fxImpl;
	
	// try HLSL
	fxImpl = GetImplementation(mImpl->mFbxmaterial, eImplHLSL);
	if (nullptr != fxImpl) return true;
	
	// try CgFx
	fxImpl = GetImplementation(mImpl->mFbxmaterial, eImplCGFX);
	if (nullptr != fxImpl) return true;

	return false;
}

std::string FbxFxMaterialAdaptor::getMaterialName() const
{
	// process material name
	return mImpl->getMaterialName();
}

std::string FbxFxMaterialAdaptor::getFxParams() const
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


}	// namespace MCD