#ifndef __MCD_FBX_FBXMATERIALADAPTORS__
#define __MCD_FBX_FBXMATERIALADAPTORS__

namespace MCD
{
class ColorRGBAf;

class MCD_FBX_API FbxPhongMaterialAdaptor
{
public:
	FbxPhongMaterialAdaptor(KFbxSurfaceMaterial* fbxmaterial);

	~FbxPhongMaterialAdaptor();

	bool valid() const;

	std::string getMaterialName() const;

	ColorRGBAf getAmbientColor() const;
	ColorRGBAf getDiffuseColor() const;
	ColorRGBAf getSpecularColor() const;
	float getShininess() const;
	
	bool hasDiffuseTexture() const;
	std::string getDiffuseTextureFilename() const;

	bool hasSpecularTexture() const;
	std::string getSpecularTextureFilename() const;

	bool hasBumpTexture() const;
	std::string getBumpTextureFilename() const;

	bool hasSelfIllumTexture() const;
	std::string getSelfIllumTextureFilename() const;

private:
	class Impl;
	Impl* mImpl;

};  // class FbxPhongMaterialAdaptor

class MCD_FBX_API FbxFxMaterialAdaptor
{
public:
	FbxFxMaterialAdaptor(KFbxSurfaceMaterial* fbxmaterial);

	~FbxFxMaterialAdaptor();

	bool valid() const;

	std::string getMaterialName() const;

	std::string getFxParams() const;

private:
	class Impl;
	Impl* mImpl;

};  // class FbxFxMaterialAdaptor

}   // namespace MCD

#endif  // __MCD_FBX_FBXMATERIALADAPTORS__