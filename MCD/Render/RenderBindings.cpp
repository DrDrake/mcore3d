#include "Pch.h"
#include "RenderBindings.h"
#include "Camera.h"
#include "DisplayList.h"
#include "Font.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "SphereBuilder.h"
#include "Texture.h"
#include "../Core/Binding/CoreBindings.h"
#include "../Core/Binding/Declarator.h"
#include "../Core/Binding/VMCore.h"
#include "../Core/Entity/SystemComponent.h"
#include "../Core/System/ResourceManager.h"

namespace MCD {
namespace Binding {

// ColorRGB

SCRIPT_CLASS_DECLAR(ColorRGBf);
SCRIPT_CLASS_REGISTER(ColorRGBf)
	.declareClass<ColorRGBf>("ColorRGB")
	.constructor<float, float, float>("constructor")
	.var("r", &ColorRGBf::r)
	.var("g", &ColorRGBf::g)
	.var("b", &ColorRGBf::b)
;}

// ColorRGBA

SCRIPT_CLASS_DECLAR(ColorRGBAf);
SCRIPT_CLASS_REGISTER(ColorRGBAf)
	.declareClass<ColorRGBAf>("ColorRGBA")
	.constructor<float, float, float, float>("constructor")
	.var("r", &ColorRGBAf::r)
	.var("g", &ColorRGBAf::g)
	.var("b", &ColorRGBAf::b)
	.var("a", &ColorRGBAf::a)
;}

// Mesh

void push(HSQUIRRELVM v, Mesh* p, Mesh**) {
	push(v, static_cast<Resource*>(p), static_cast<Resource*>(p));
}

SCRIPT_CLASS_DECLAR(Mesh);
SCRIPT_CLASS_REGISTER(Mesh)
	.declareClass<Mesh, Resource>("MeshBuffer")
	.var("indexCount", &Mesh::indexCount)
	.var("vertexCount", &Mesh::vertexCount)
;}

// Texture

void push(HSQUIRRELVM v, Texture* p, Texture**) {
	push(v, static_cast<Resource*>(p), static_cast<Resource*>(p));
}

SCRIPT_CLASS_DECLAR(Texture);
SCRIPT_CLASS_REGISTER(Texture)
	.declareClass<Texture, Resource>("Texture")
	.var("width", &Texture::width)
	.var("height", &Texture::height)
	.varGet("hasAlpha", &Texture::hasAlpha)
;}

// CameraComponent

SCRIPT_CLASS_DECLAR(CameraComponent);
SCRIPT_CLASS_REGISTER(CameraComponent)
	.declareClass<CameraComponent, Component>("Camera")
;}

// DisplayListComponent

static void beginTraingles_DisplayListComponent(DisplayListComponent& c) { c.begin(DisplayListComponent::Triangles); }
static void beginLines_DisplayListComponent(DisplayListComponent& c) { c.begin(DisplayListComponent::Lines); }
static void beginLineStrip_DisplayListComponent(DisplayListComponent& c) { c.begin(DisplayListComponent::LineStrip); }

SCRIPT_CLASS_DECLAR(DisplayListComponent);
SCRIPT_CLASS_REGISTER(DisplayListComponent)
	.declareClass<DisplayListComponent, Component>("DisplayList")
	.constructor()
	.method("clear", &DisplayListComponent::clear)
	.method("beginTriangles", &beginTraingles_DisplayListComponent)
	.method("beginLines", &beginLines_DisplayListComponent)
	.method("beginLineStrip", &beginLineStrip_DisplayListComponent)
	.method("_color", (void (DisplayListComponent::*)(float,float,float,float))(&DisplayListComponent::color))
	.runScript("DisplayList.color<-function(r,g,b,a=1){this._color(r,g,b,a);}")
	.method("_texcoord", (void (DisplayListComponent::*)(float,float,float))(&DisplayListComponent::texcoord))
	.runScript("DisplayList.texcoord<-function(u,v,w=0){this._texcoord(u,v,w);}")
	.method("normal", (void (DisplayListComponent::*)(float,float,float))(&DisplayListComponent::normal))
	.method("vertex", (void (DisplayListComponent::*)(float,float,float))(&DisplayListComponent::vertex))
	.method("end", &DisplayListComponent::end)
;}

// LightComponent

SCRIPT_CLASS_DECLAR(LightComponent);
SCRIPT_CLASS_REGISTER(LightComponent)
	.declareClass<LightComponent, Component>("Light")
	.constructor()
	.var("type", &LightComponent::type)
	.var("color", &LightComponent::color)
;}

// MaterialComponent

static Resource* pathToResource(const char* path)
{
	ResourceManagerComponent* c = ResourceManagerComponent::fromCurrentEntityRoot();
	MCD_ASSUME(c);
	return c->resourceManager().load(path).get();
}

static Texture* alphaMap_MaterialComponent(MaterialComponent& c) { return c.alphaMap.get(); }
static void setAlphaMap_MaterialComponent(MaterialComponent& c, const char* path) { c.alphaMap = dynamic_cast<Texture*>(pathToResource(path)); }

static Texture* diffuseMap_MaterialComponent(MaterialComponent& c) { return c.diffuseMap.get(); }
static void setDiffuseMap_MaterialComponent(MaterialComponent& c, const char* path) { c.diffuseMap = dynamic_cast<Texture*>(pathToResource(path)); }

static Texture* emissionMap_MaterialComponent(MaterialComponent& c) { return c.emissionMap.get(); }
static void setEmissionMap_MaterialComponent(MaterialComponent& c, const char* path) { c.emissionMap = dynamic_cast<Texture*>(pathToResource(path)); }

static Texture* specularMap_MaterialComponent(MaterialComponent& c) { return c.specularMap.get(); }
static void setSpecularMap_MaterialComponent(MaterialComponent& c, const char* path) { c.specularMap = dynamic_cast<Texture*>(pathToResource(path)); }

static Texture* bumpMap_MaterialComponent(MaterialComponent& c) { return c.bumpMap.get(); }
static void setBumpMap_MaterialComponent(MaterialComponent& c, const char* path) { c.bumpMap = dynamic_cast<Texture*>(pathToResource(path)); }

SCRIPT_CLASS_DECLAR(MaterialComponent);
SCRIPT_CLASS_REGISTER(MaterialComponent)
	.declareClass<MaterialComponent, Component>("StandardMaterial")
	.constructor()
	.var("diffuseColor", &MaterialComponent::diffuseColor)
	.var("specularColor", &MaterialComponent::specularColor)
	.var("emissionColor", &MaterialComponent::emissionColor)
	.var("specularExponent", &MaterialComponent::specularExponent)
	.var("opacity", &MaterialComponent::opacity)
	.var("lighting", &MaterialComponent::lighting)
	.var("cullFace", &MaterialComponent::cullFace)
	.var("useVertexColor", &MaterialComponent::useVertexColor)
	.var("bumpFactor", &MaterialComponent::bumpFactor)
	.varGet("alphaMap", &alphaMap_MaterialComponent)
	.varSet("alphaMap", &setAlphaMap_MaterialComponent)
	.varGet("diffuseMap", &diffuseMap_MaterialComponent)
	.varSet("diffuseMap", &setDiffuseMap_MaterialComponent)
	.varGet("emissionMap", &emissionMap_MaterialComponent)
	.varSet("emissionMap", &setEmissionMap_MaterialComponent)
	.varGet("specularMap", &specularMap_MaterialComponent)
	.varSet("specularMap", &setSpecularMap_MaterialComponent)
	.varGet("bumpMap", &bumpMap_MaterialComponent)
	.varSet("bumpMap", &setBumpMap_MaterialComponent)
;}

// Mesh Component

static Mesh* buffer_MeshComponent(MeshComponent& c) { return c.mesh.get(); }
static void setBuffer_MeshComponent(MeshComponent& c, Mesh* m) { c.mesh = m; }

SCRIPT_CLASS_DECLAR(MeshComponent);
SCRIPT_CLASS_REGISTER(MeshComponent)
	.declareClass<MeshComponent, Component>("Mesh")
	.constructor()
	.varGet("buffer", &buffer_MeshComponent)
	.varSet("buffer", &setBuffer_MeshComponent)
;}

// TextLabelComponent

SCRIPT_CLASS_DECLAR(TextLabelComponent);
SCRIPT_CLASS_REGISTER(TextLabelComponent)
	.declareClass<TextLabelComponent, Component>("TextLabel")
	.constructor()
	.var("text", &TextLabelComponent::text)
	.var("lineWidth", &TextLabelComponent::lineWidth)
	.var("font", &TextLabelComponent::font)
	.var("anchor", &TextLabelComponent::anchor)
;}

Mesh* createSphere(float radius, uint16_t segments) {
	Mesh* mesh = new Mesh("");
	MCD_VERIFY(mesh->create(SphereBuilder(radius, segments), Mesh::Static));
	return mesh;
}

void registerRenderBinding(VMCore& vm)
{
	Binding::ClassTraits<ColorRGBf>::bind(&vm);
	Binding::ClassTraits<ColorRGBAf>::bind(&vm);
	Binding::ClassTraits<Mesh>::bind(&vm);
	Binding::ClassTraits<Texture>::bind(&vm);

	Binding::ClassTraits<CameraComponent>::bind(&vm);
	Binding::ClassTraits<DisplayListComponent>::bind(&vm);
	Binding::ClassTraits<MaterialComponent>::bind(&vm);
	Binding::ClassTraits<MeshComponent>::bind(&vm);
	Binding::ClassTraits<TextLabelComponent>::bind(&vm);

	RootDeclarator root(&vm);
	root.declareFunction("createSphere", &createSphere);
}

}	// namespace Binding
}	// namespace MCD
