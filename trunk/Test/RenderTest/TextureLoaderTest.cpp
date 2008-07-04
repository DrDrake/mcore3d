#include "Pch.h"
#include "../../SGE/Render/Texture.h"
#include "../../SGE/Render/PngLoader.h"
#include "../../SGE/Render/JpegLoader.h"

#include "../../SGE/Core/System/CondVar.h"
#include "../../SGE/Core/System/RawFileSystem.h"
#include "../../SGE/Core/System/Thread.h"

using namespace SGE;

//! Create the corresponding texture loader from the file extension
static std::auto_ptr<IResourceLoader> createLoader(const wchar_t* file)
{
	Path path(file);
	Path::string_type ext = path.getExtension();

	if(ext == L"png")
		return std::auto_ptr<IResourceLoader>(new PngLoader);
	else if(ext == L"jpg")
		return std::auto_ptr<IResourceLoader>(new JpegLoader);
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
	{ L"4x4.png",					4,   4   },
	{ L"4x4.jpg",					4,   4   },
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
		SGE::ResourcePtr resource = new Texture(gTestStruct[i].path);
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
		CHECK_EQUAL(IResourceLoader::NotLoaded, loader->load(nullptr));

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
		SGE::ResourcePtr resource = new Texture(gTestStruct[i].path);
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
	SGE::ResourcePtr resource = new Texture(L"error.png");
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



#include "../../SGE/Core/System/Window.h"
#include "../../SGE/Core/System/WindowEvent.h"
#include "../../SGE/Core/System/Timer.h"
#include "../../SGE/Render/OglContext.h"
#include "../../3Party/glew/glew.h"

#if defined(SGE_VC)
#	pragma comment(lib, "OpenGL32")
#	pragma comment(lib, "GLU32")
#	pragma comment(lib, "GlAux")
#endif

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
//			mSleep(100);
		}
	}

protected:
	IResourceLoader& mLoader;
	std::istream& mIStream;
};	// Runnable

class TWindow : public Window
{
public:
	TWindow() : mIsClosing(false), mFullLoaded(false)
	{
		create(L"title=RotatingBox_TextureTest");
		mOglContext.reset(new OglContext(handle()));

		mTexture = new Texture(L"Media/InterlacedTrans256x256.png");
		Texture& texture = static_cast<Texture&>(*mTexture);

		RawFileSystem fs(L"./");
		mIStream.reset(fs.openRead(texture.fileId()).release());

		// Assure we can read the file
		if(!mIStream.get())
			return;

		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		glEnable(GL_TEXTURE_2D);

		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
//		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		mThread.start(*(new Runnable(mLoader, *mIStream)), true);

		mAngle = 0;
	}

	void mainLoop()
	{
		while(true) {
			Event e;
			getEvent(e, false);
			if(e.Type == Event::Closed)
				break;

			update();
		}
	}

	void drawUnitCube()
	{
		glPushMatrix();
		static const float colors[6][3] = { {1, 0, 0}, {0, 1, 0}, {0, 0, 1}, {1, 1, 0}, {1, 0, 1},  {0, 1, 1} };
		// Face vertice are specified in clock-wise direction
		static const float vertice[6][4][3] = {
			{ {-1, 1, 1}, { 1, 1, 1}, { 1,-1, 1}, {-1,-1, 1} },	// Front face
			{ { 1, 1,-1}, {-1, 1,-1}, {-1,-1,-1}, { 1,-1,-1} },	// Back face
			{ { 1, 1, 1}, { 1, 1,-1}, { 1,-1,-1}, { 1,-1, 1} },	// Right face
			{ {-1, 1,-1}, {-1, 1, 1}, {-1,-1, 1}, {-1,-1,-1} },	// Left face
			{ {-1, 1,-1}, { 1, 1,-1}, { 1, 1, 1}, {-1, 1, 1} },	// Top face
			{ { 1,-1, 1}, { 1,-1,-1}, {-1,-1,-1}, {-1,-1, 1} },	// Bottom face
		};

		static const float tex[4][2] = {
			{0, 0}, {0, 1}, {1, 1}, {1, 0}
		};

		glScalef(0.5, 0.5, 0.5);
		glBegin(GL_QUADS);
		for(size_t face=0; face<6; ++face) {
			const float* c = colors[face];
			SGE_ASSUME(c != nullptr);
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

	void update() {
		glClear(GL_COLOR_BUFFER_BIT);

		// Clear matrix stack
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		Texture& texture = static_cast<Texture&>(*mTexture);

		if(!mFullLoaded && !mIsClosing && mLoader.getLoadingState() & IResourceLoader::CanCommit) {
			if(mLoader.getLoadingState() == IResourceLoader::Loaded)
				mFullLoaded = true;

			mLoader.commit(texture);
			std::cout << "Commited\n";
		}

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glBindTexture(GL_TEXTURE_2D, texture.handle());

		float deltaTime = float(timer.getDelta().asSecond());
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

		static size_t iteration = 0;

		if(++iteration % 1000  == 0)
			printf("FPS: %f\n", 1.0 / deltaTime);

		mOglContext->swapBuffers();
	}

	sal_override void onClose() {
		mIsClosing = true;
	}

private:
	bool mIsClosing;
	float mAngle;
	SGE::DeltaTimer timer;
	PngLoader mLoader;
	std::auto_ptr<std::istream> mIStream;
	ResourcePtr mTexture;
	Thread mThread;
	bool mFullLoaded;

	std::auto_ptr<OglContext> mOglContext;
};

}	// namespace RotatingBox

TEST(RotatingBox_TextureTest) {
	using namespace RotatingBox;

	TWindow window;

	window.mainLoop();
}
