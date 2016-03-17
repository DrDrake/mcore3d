# The `<texture />` element #

The texture element defines the reference to a texture.<br />
Please notice that the first `<texture />` element occurs in the pass will be bind to texture-unit 0 and so on.

# Attributes #
  * file
    * file name of the texture
    * Syntax: `<texture file="../../wood.jpg"/>`
    * Values: texture file command (see below)

  * filter
    * filter options of the texture (min\_filter, mag\_filter, mip\_filter)
    * Syntax: `<texture filter="linear_linear_point"/>`
    * Values:
      1. "point\_point\_none"
      1. "point\_point\_point"
      1. "point\_point\_linear"
      1. "linear\_linear\_none"
      1. "linear\_linear\_point"
      1. "linear\_linear\_linear"
    * Default: "linear\_linear\_none"

  * shaderName
    * The name of the texure if it is being referenced in a shader.
    * Syntax: `<texture shaderName="diffuseMap"/>`
    * Values: "a string"

  * autoGenMipmap (in-progress)
    * enable auto mipmap generation for this texture.
    * Syntax: `<texture autoGenMipmap ="true"/>`
    * Values: "true" or "false"
    * Default: "true"

# Texture file command #
Filename(s) in the `<texture />` element is specified by a simple texture file command with the following syntax:<br /><br />
_{file\_path\_and\_basename}_ **.** _{texture\_type}_ **.** _{extension}_

  * Loading a 2d texture
    * file="../../wood.jpg"
      * The image "../../wood.jpg" will be loaded as a 2d texture

  * Loading a cubemap texture
    * file="skybox._**cubemap**_.jpg"
      * This assumes the image _skybox.cubemap.jpg_ containing the 6 faces of the cubemap (tiled vertically: +x -x +y -y +z -z).
      * Please notice the width of image defines the cubemap size (must be power of 2) and the height of the image must be equals to 6 times of the width. For example, the image width is 256 pixels, then the height should be 1536 pixels.

  * Loading a volume(3d) texture (in-progress)
    * file="noise._**volume**_.jpg"
      * This assumes the image _noise.volume.jpg_ containing the a list of slices of a volume texture (tiled vertically).
      * Please notice the width of image defines the size (i.e. width and height; must be power of 2) of each slice in volume texture and the height of the image must be multiple of the width. For example, the image width is 256 pixels and the height is 1024 pixels, then a volume texture with dimension 256 x 256 x 4 (w x h x d) will be created.