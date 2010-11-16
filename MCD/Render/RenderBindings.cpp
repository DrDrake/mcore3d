#include "Pch.h"
#include "RenderBindings.h"
#include "DisplayList.h"
#include "Font.h"
#include "Light.h"
#include "Material.h"
#include "Mesh.h"
#include "Texture.h"
#include "../Core/Binding/CoreBindings.h"
#include "../Core/Binding/Declarator.h"
#include "../Core/Binding/VMCore.h"

namespace MCD {
namespace Binding {

// ColorRGB

SCRIPT_CLASS_DECLAR(ColorRGBf);
SCRIPT_CLASS_REGISTER(ColorRGBf)
	.declareClass<ColorRGBf>("ColorRGB")
	.var("r", &ColorRGBf::r)
	.var("g", &ColorRGBf::g)
	.var("b", &ColorRGBf::b)
;}

// DisplayListComponent

static void beginTraingles_DisplayListComponent(DisplayListComponent& c) { c.begin(DisplayListComponent::Triangles); }
static void beginLines_DisplayListComponent(DisplayListComponent& c) { c.begin(DisplayListComponent::Lines); }
static void beginLineStrip_DisplayListComponent(DisplayListComponent& c) { c.begin(DisplayListComponent::LineStrip); }

SCRIPT_CLASS_DECLAR(DisplayListComponent);
SCRIPT_CLASS_REGISTER(DisplayListComponent)
	.declareClass<DisplayListComponent, Component>("DisplayListComponent")
	.constructor()
	.method("clear", &DisplayListComponent::clear)
	.method("beginTriangles", &beginTraingles_DisplayListComponent)
	.method("beginLines", &beginLines_DisplayListComponent)
	.method("beginLineStrip", &beginLineStrip_DisplayListComponent)
	.method("_color", (void (DisplayListComponent::*)(float,float,float,float))(&DisplayListComponent::color))
	.runScript("DisplayListComponent.color<-function(r,g,b,a=1){this._color(r,g,b,a);}")
	.method("_texcoord", (void (DisplayListComponent::*)(float,float,float))(&DisplayListComponent::texcoord))
	.runScript("DisplayListComponent.texcoord<-function(u,v,w=0){this._texcoord(u,v,w);}")
	.method("normal", (void (DisplayListComponent::*)(float,float,float))(&DisplayListComponent::normal))
	.method("vertex", (void (DisplayListComponent::*)(float,float,float))(&DisplayListComponent::vertex))
	.method("end", &DisplayListComponent::end)
;}

// LightComponent

SCRIPT_CLASS_DECLAR(LightComponent);
SCRIPT_CLASS_REGISTER(LightComponent)
	.declareClass<LightComponent, Component>("LightComponent")
	.constructor()
	.var("type", &LightComponent::type)
	.var("color", &LightComponent::color)
;}

// MaterialComponent

SCRIPT_CLASS_DECLAR(MaterialComponent);
SCRIPT_CLASS_REGISTER(MaterialComponent)
	.declareClass<MaterialComponent, Component>("MaterialComponent")
	.constructor()
	.var("specularExponent", &MaterialComponent::specularExponent)
	.var("opacity", &MaterialComponent::opacity)
	.var("lighting", &MaterialComponent::lighting)
	.var("cullFace", &MaterialComponent::cullFace)
	.var("useVertexColor", &MaterialComponent::useVertexColor)
;}

// Mesh

SCRIPT_CLASS_DECLAR(Mesh);
SCRIPT_CLASS_REGISTER(Mesh)
	.declareClass<Mesh, Resource>("Mesh")
	.var("indexCount", &Mesh::indexCount)
	.var("vertexCount", &Mesh::vertexCount)
;}

// TextLabelComponent

SCRIPT_CLASS_DECLAR(TextLabelComponent);
SCRIPT_CLASS_REGISTER(TextLabelComponent)
	.declareClass<TextLabelComponent, Component>("TextLabelComponent")
	.constructor()
	.var("text", &TextLabelComponent::text)
	.var("lineWidth", &TextLabelComponent::lineWidth)
	.var("font", &TextLabelComponent::font)
	.var("anchor", &TextLabelComponent::anchor)
;}

// Texture

SCRIPT_CLASS_DECLAR(Texture);
SCRIPT_CLASS_REGISTER(Texture)
	.declareClass<Texture, Resource>("Texture")
	.var("width", &Texture::width)
	.var("height", &Texture::height)
	.varGet("hasAlpha", &Texture::hasAlpha)
;}

void registerRenderBinding(VMCore& vm)
{
	Binding::ClassTraits<ColorRGBf>::bind(&vm);
	Binding::ClassTraits<DisplayListComponent>::bind(&vm);
	Binding::ClassTraits<MaterialComponent>::bind(&vm);
	Binding::ClassTraits<Mesh>::bind(&vm);
	Binding::ClassTraits<Texture>::bind(&vm);
	Binding::ClassTraits<TextLabelComponent>::bind(&vm);
}

}	// namespace Binding
}	// namespace MCD
