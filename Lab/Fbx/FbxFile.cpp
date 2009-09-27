#include "Pch.h"
#include "FbxFile.h"

namespace MCD
{

FbxFile::FbxFile(const wchar_t* filepath)
	: mSdkManager(NULL)
	, mScene(NULL)
	, mImporter(NULL)
{
	wStrToUtf8(filepath, wcslen(filepath), mFilePath);

	mTimer.getDelta().asSecond();

	mSdkManager = KFbxSdkManager::Create();
	Log::format( Log::Info, L"KFbxSdkManager::Create done in %f", mTimer.getDelta().asSecond() );

	mScene = KFbxScene::Create(mSdkManager,"");
	Log::format( Log::Info, L"KFbxScene::Create done in %f", mTimer.getDelta().asSecond() );
}

FbxFile::~FbxFile()
{
	Log::format( Log::Info, L"Convert done in %f", mTimer.getDelta().asSecond() );

	mImporter->Destroy();
	mSdkManager->Destroy();
}

bool FbxFile::open()
{
	// Get the file version number generate by the FBX SDK.
	KFbxIO::GetCurrentVersion(mSdkVer.major, mSdkVer.minor, mSdkVer.revision);

	// Create an importer.
	mImporter = KFbxImporter::Create(mSdkManager,"");

	int fbxFileFormat = -1;
	
	if (!mSdkManager->GetIOPluginRegistry()->DetectFileFormat(mFilePath.c_str(), fbxFileFormat))
	{
		// Unrecognizable file format. Try to fall back to native format.
		fbxFileFormat = mSdkManager->GetIOPluginRegistry()->GetNativeReaderFormat();
	}
	mImporter->SetFileFormat(fbxFileFormat);

	// Initialize the importer by providing a filename.
	const bool cImportStatus = mImporter->Initialize(mFilePath.c_str());
	mImporter->GetFileVersion(mFileVer.major, mFileVer.minor, mFileVer.revision);

	if(!cImportStatus)
	{
		Log::format
			( Log::Error
			, L"Call to KFbxImporter::Initialize() failed.\nError returned: %s\n\n"
			, mImporter->GetLastErrorString()
			);

		if (mImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_YET ||
			mImporter->GetLastErrorID() == KFbxIO::eFILE_VERSION_NOT_SUPPORTED_ANYMORE)
		{
			Log::format
				( Log::Error
				, L"FBX version number for this FBX SDK is %d.%d.%d\n"
				  L"FBX version number for file %s is %d.%d.%d\n"
				, mSdkVer.major, mSdkVer.minor, mSdkVer.revision
				, mFilePath.c_str(), mFileVer.major, mFileVer.minor, mFileVer.revision
				);
		}

		return false;
	}

	KFbxStreamOptionsFbxReader* fbxImportOptions = KFbxStreamOptionsFbxReader::Create(mSdkManager, "");

	if(mImporter->IsFBX())
	{
		/*
		// Set the import states. By default, the import states are always set to 
		// true. The code below shows how to change these states.
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_MATERIAL, true);
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_TEXTURE, true);
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_LINK, true);
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_SHAPE, true);
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_GOBO, true);
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_ANIMATION, true);
		fbxImportOptions->SetOption(KFBXSTREAMOPT_FBX_GLOBAL_SETTINGS, true);
		*/
		IOSREF.SetBoolProp(IMP_FBX_MATERIAL,        true);
        IOSREF.SetBoolProp(IMP_FBX_TEXTURE,         true);
        IOSREF.SetBoolProp(IMP_FBX_LINK,            true);
        IOSREF.SetBoolProp(IMP_FBX_SHAPE,           true);
        IOSREF.SetBoolProp(IMP_FBX_GOBO,            true);
        IOSREF.SetBoolProp(IMP_FBX_ANIMATION,       true);
        IOSREF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
	}

	Log::format( Log::Info, L"Importer Create done in %f", mTimer.getDelta().asSecond() );

	// Import the scene.
	const bool cImportResult = mImporter->Import(mScene, fbxImportOptions);

	if(cImportResult == false && mImporter->GetLastErrorID() == KFbxIO::ePASSWORD_ERROR)
	{
		Log::format
			( Log::Error
			, L"File %s is password protected! abort"
			, mFilePath.c_str()
			);
	}

	Log::format( Log::Info, L"Importer::Import done in %f", mTimer.getDelta().asSecond() );

	// convert to opengl coordinate system
	if(mScene->GetGlobalSettings().GetAxisSystem() != KFbxAxisSystem::OpenGL)
	{
		KFbxAxisSystem::OpenGL.ConvertScene(mScene);
	}

	// Destroy the importer
	fbxImportOptions->Destroy();

	return cImportResult;
}

}