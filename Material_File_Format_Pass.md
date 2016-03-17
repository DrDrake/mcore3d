# The `<pass />` element #
A pass element corresponds to a rendering pass.

# Attributes #
  * enable
    * Enabling option. A pass can be enable/disable in run time and this attribute sets the default behavior.
    * Syntax: `<pass enable = "true" />`
    * Values: "true" or "false"

  * drawLine
    * If true, the renderer will draw lines instead of triangles.
    * Syntax: `<pass drawLine="true"/>`
    * Values: "true" or "false"

  * lineWidth
    * Set the line width in # of pixels. This attribute is valid only if drawLine = "true".
    * Syntax: `<pass lineWidth="1.0"/>`
    * Values: "true" or "false"

  * cullMode
    * Set the triangle-facing to be culled
    * Syntax: `<pass cullMode="none"/>`
    * Values: "none", "front"

# Elements #
  * [&lt;standard /&gt;](Material_File_Format_Pass_Standard.md)
  * [&lt;texture /&gt;](Material_File_Format_Pass_Texture.md)
  * [&lt;shader /&gt;](Material_File_Format_Pass_Shader.md)
  * [&lt;blending /&gt;](Material_File_Format_Pass_Blending.md) (in-progress)
  * [&lt;depthStencil /&gt;](Material_File_Format_Pass_DepthStencil.md) (in-progress)