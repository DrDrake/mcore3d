#ifndef __MCD_FBX_FBXFILE__
#define __MCD_FBX_FBXFILE__

namespace MCD
{

/*! A utility for opening a fbx file.
	ExampleUsage:

\code
	FbxFile file("c:/test.fbx");
	ErrorList errors;
	if(!file.open(&errors))
	{
		// print errors here
	}
\endcode

*/
class MCD_FBX_API FbxFile
{
public:
	struct Version
	{
		int major;
		int minor;
		int revision;
	};

	FbxFile(const wchar_t* filepath);

	~FbxFile();
	
	bool open();

	KFbxSdkManager* sdkManager() { return mSdkManager; }
	KFbxScene* scene() { return mScene; }
	KFbxImporter* importer() { return mImporter; }

	const Version& sdkVersion() const { return mSdkVer; }
	const Version& fileVersion() const { return mFileVer; }
	const char* filePath() const { return mFilePath.c_str(); }

private:
	std::string		mFilePath;
	KFbxSdkManager* mSdkManager;
	KFbxScene*		mScene;
	KFbxImporter*	mImporter;
	DeltaTimer		mTimer;

	Version			mSdkVer;
	Version			mFileVer;
};

}	// namespace MCD

#endif  // #ifndef __MCD_FBX_FBXFILE__