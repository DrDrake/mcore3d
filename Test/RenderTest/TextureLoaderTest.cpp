#include "Pch.h"
#include "../../MCD/Render/Texture.h"
#include "../../MCD/Render/BitmapLoader.h"
#include "../../MCD/Render/JpegLoader.h"
#include "../../MCD/Render/PngLoader.h"

#include "../../MCD/Core/System/CondVar.h"
#include "../../MCD/Core/System/RawFileSystem.h"
#include "../../MCD/Core/System/Thread.h"

using namespace MCD;

//! Create the corresponding texture loader from the file extension
static std::auto_ptr<IResourceLoader> createLoader(const wchar_t* file)
{
	Path path(file);
	Path::string_type ext = path.getExtension();

	if(ext == L"bmp")
		return std::auto_ptr<IResourceLoader>(new BitmapLoader);
	else if(ext == L"jpg")
		return std::auto_ptr<IResourceLoader>(new JpegLoader);
	else if(ext == L"png")
		return std::auto_ptr<IResourceLoader>(new PngLoader);
	else
		return std::auto_ptr<IResourceLoader>(nullptr);
}

struct TestStruct
{
	const wchar_t* path;
	size_t expectingWidth;
	size_t expectingHeight;
};

TestStruct gTestStruct[] = {
	{ L"4x4.bmp",					4,   4   },
	{ L"4x4.jpg",					4,   4   },
	{ L"4x4.png",					4,   4   },
	{ L"Gray512x512.jpg",			512, 512 },
	{ L"Normal512x512.jpg",			512, 512 },
	{ L"Progressive512x512.jpg",	512, 512 },
	{ L"Interlaced256x256.png",		256, 256 },
	{ L"InterlacedGray256x256.png",	256, 256 },
	{ L"InterlacedTrans256x256.png",256, 256 },
	{ L"normal128x128.png",			128, 128 },
};

TEST(SynPng_TextureLoaderTest)
{
	for(size_t i=0; i<sizeof(gTestStruct)/sizeof(TestStruct); ++i)
	{
		MCD::ResourcePtr resource = new Texture(gTestStruct[i].path);
		Texture& texture = static_cast<Texture&>(*resource);
		std::auto_ptr<IResourceLoader> loader = createLoader(gTestStruct[i].path);

		if(loader.get() == nullptr) {
			CHECK(false);
			continue;
		}

		CHECK_EQUAL(IResourceLoader::NotLoaded, loader->getLoadingState());

		RawFileSystem fs(L"./Media");
		std::auto_ptr<std::istream> is = fs.openRead(texture.fileId());

		// Assure we can read the file
		if(is.get() == nullptr) {
			CHECK(is.get() != nullptr);
			continue;
		}

		// Commit will do nothing since nothing is loaded yet
		loader->commit(texture);

		// Calling load with a null stream will do nothing
		CHECK_EQUAL(IResourceLoader::Aborted, loader->load(nullptr));

		while(!(loader->load(is.get()) & IResourceLoader::Stopped));

		loader->commit(texture);

		CHECK_EQUAL(IResourceLoader::Loaded, loader->getLoadingState());
		CHECK_EQUAL(gTestStruct[i].expectingWidth, texture.width());
		CHECK_EQUAL(gTestStruct[i].expectingHeight, texture.height());
	}
}

TEST(AsynPng_TextureLoaderTest)
{
	class Runnable : public Thread::IRunnable
	{
	public:
		Runnable(IResourceLoader& loader, std::istream& is)
			: mLoader(loader), mIStream(is)
		{
		}

		void wait() {
			mCondVar.wait();
		}

	protected:
		sal_override void run(Thread& thread) throw()
		{
			while(thread.keepRun()) {
				if(mLoader.load(&mIStream) & IResourceLoader::Stopped) {
					mCondVar.signal();
					return;
				}
				else
					mCondVar.signal();
			}
		}

	protected:
		IResourceLoader& mLoader;
		std::istream& mIStream;
		CondVar mCondVar;
	};	// Runnable

	for(size_t i=0; i<sizeof(gTestStruct)/sizeof(TestStruct); ++i)
	{
		MCD::ResourcePtr resource = new Texture(gTestStruct[i].path);
		Texture& texture = static_cast<Texture&>(*resource);
		std::auto_ptr<IResourceLoader> loader = createLoader(gTestStruct[i].path);

		if(loader.get() == nullptr) {
			CHECK(false);
			continue;
		}

		CHECK_EQUAL(IResourceLoader::NotLoaded, loader->getLoadingState());

		RawFileSystem fs(L"./Media");
		std::auto_ptr<std::istream> is = fs.openRead(texture.fileId());

		// Assure we can read the file
		if(is.get() == nullptr) {
			CHECK(is.get() != nullptr);
			return;
		}

		Runnable runnable(*loader, *is);
		Thread thread(runnable, false);

		while(!(loader->getLoadingState() & IResourceLoader::Stopped)) {
			runnable.wait();
			loader->commit(texture);
		}
		loader->commit(texture);

		CHECK_EQUAL(IResourceLoader::Loaded, loader->getLoadingState());
		CHECK_EQUAL(gTestStruct[i].expectingWidth, texture.width());
		CHECK_EQUAL(gTestStruct[i].expectingHeight, texture.height());
	}
}

TEST(Error_TextureLoaderTest)
{
	MCD::ResourcePtr resource = new Texture(L"error.png");
	Texture& texture = static_cast<Texture&>(*resource);
	PngLoader loader;

	CHECK_EQUAL(IResourceLoader::NotLoaded, loader.getLoadingState());

	RawFileSystem fs(L"./Media");
	std::auto_ptr<std::istream> is = fs.openRead(texture.fileId());

	// Assure we can read the file
	if(is.get() == nullptr) {
		CHECK(is.get() != nullptr);
		return;
	}

	CHECK_EQUAL(IResourceLoader::Aborted, loader.load(is.get()));

	// Commit will just do nothing
	loader.commit(texture);

	// Any error will result in an aborted state
	CHECK_EQUAL(IResourceLoader::Aborted, loader.getLoadingState());
	CHECK_EQUAL(0u, texture.width());
	CHECK_EQUAL(0u, texture.height());
}

#include "BasicGlWindow.h"

namespace RotatingBox {

class Runnable : public Thread::IRunnable
{
public:
	Runnable(IResourceLoader& loader, std::istream& is)
		: mLoader(loader), mIStream(is)
	{
	}

protected:
	sal_override void run(Thread& thread) throw()
	{
		while(thread.keepRun()) {
			if(mLoader.load(&mIStream) & IResourceLoader::Stopped)
				return;
			// Making some delay
			mSleep(100);
		}
	}

protected:
	IResourceLoader& mLoader;
	std::istream& mIStream;
};	// Runnable

class TestWindow : public BasicGlWindow
{
public:
	TestWindow()
		:
		BasicGlWindow(L"title=RotatingBox_TextureTest;width=800;height=600;fullscreen=0"),
		mAngle(0),
		mFullLoaded(false)
	{
		mTexture = new Texture(L"Media/InterlacedTrans256x256.png");
		Texture& texture = static_cast<Texture&>(*mTexture);

		RawFileSystem fs(L"./");
		mIStream.reset(fs.openRead(texture.fileId()).release());

		// Assure we can read the file
		if(!mIStream.get())
			return;

		mThread.start(*(new Runnable(mLoader, *mIStream)), true);

		// We disable the depth test to show our transparent cube
		glDisable(GL_DEPTH_TEST);
	}

	void drawUnitCube()
	{
		glPushMatrix();
		static const float colors[6][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {1, 0, 1},  {0, 1, 1} };
		// Face vertice are specified in counterclockwise direction
		static const float vertice[6][4][3] = {
			{ {-1, 1, 1}, {-1,-1, 1}, { 1,-1, 1}, { 1, 1, 1} },	// Front face	(fix z at 1)
			{ { 1, 1,-1}, { 1,-1,-1}, {-1,-1,-1}, {-1, 1,-1} },	// Back face	(fix z at -1)
			{ { 1, 1, 1}, { 1,-1, 1}, { 1,-1,-1}, { 1, 1,-1} },	// Right face	(fix x at 1)
			{ {-1, 1,-1}, {-1,-1,-1}, {-1,-1, 1}, {-1, 1, 1} },	// Left face	(fix x at -1)
			{ {-1, 1,-1}, {-1, 1, 1}, { 1, 1, 1}, { 1, 1,-1} },	// Top face		(fix y at 1)
			{ { 1,-1, 1}, {-1,-1, 1}, {-1,-1,-1}, { 1,-1,-1} },	// Bottom face	(fix y at -1)
		};

		static const float tex[4][2] = {
			{0, 0}, {0, 1}, {1, 1}, {1, 0}
		};

		glScalef(0.5, 0.5, 0.5);
		glBegin(GL_QUADS);
		for(size_t face=0; face<6; ++face) {
			const float* c = colors[face];
			MCD_ASSUME(c != nullptr);
			glColor3fv(c);
			for(size_t vertex=0; vertex<4; ++vertex) {
				const float* v = vertice[face][vertex];
				glTexCoord2fv(tex[vertex]);
				glVertex3fv(v);
			}
		}
		glEnd();
		glPopMatrix();
	}

	sal_override void update(float deltaTime)
	{
		Texture& texture = static_cast<Texture&>(*mTexture);

		if(!mFullLoaded && keepRun() && mLoader.getLoadingState() & IResourceLoader::CanCommit) {
			if(mLoader.getLoadingState() == IResourceLoader::Loaded)
				mFullLoaded = true;

			mLoader.commit(texture);
			std::cout << "Commited\n";
		}

		texture.bind();

		glTranslatef(0, 0, -2);

		mAngle += deltaTime;
		glRotatef(mAngle, 1, 0, 0);
		glRotatef(mAngle, 0, 1, 0);
		glRotatef(mAngle, 0, 0, 1);

		if(Texture::hasAlpha(texture.format())) {
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA,GL_ONE);
			glDisable(GL_CULL_FACE);
		} else
			glEnable(GL_CULL_FACE);

		drawUnitCube();
	}

private:
	float mAngle;
	PngLoader mLoader;
	std::auto_ptr<std::istream> mIStream;
	ResourcePtr mTexture;
	Thread mThread;
	bool mFullLoaded;
};

}	// namespace RotatingBox

TEST(RotatingBox_TextureTest)
{
	using namespace RotatingBox;

	TestWindow window;

	window.mainLoop();

	CHECK(true);
}
