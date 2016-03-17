# The `<blending />` element #
The blending element defines the parameters being used in the color / alpha blending process.

The color blending equation is as follow:<br />
```
Result = srcColor * srcFactor + dstColor * dstFactor
```

Optionally, so hardware supports the separated alpha channel blending such that:<br />
```
Result.rgb = srcColor.rgb * srcFactor + dstColor.rgb * dstFactor
Result.a = srcColor.a * srcFactorSep + dstColor.a * dstFactorSep
```

# Attributes #
  * colorBlend
    * Enable / disable the color blending
    * Syntax: `<blending colorBlend="true"/>`
    * Values: "true" or "false"
    * Default: "false"

  * srcFactor
    * Source blend factor in the blending equation
    * Syntax: `<blending srcFactor="one"/>`
    * Values: "blendFactors" (see below)
    * Default: "one"

  * dstFactor
    * Destination blend factor in the blending equation
    * Syntax: `<blending dstFactor="one"/>`
    * Values: "blendFactors" (see below)
    * Default: "zero"

  * colorBlendSep
    * Enable / disable the separated alpha channel blending
    * Syntax: `<blending colorBlendSep="true"/>`
    * Values: "true" or "false"
    * Default: "false"

  * srcFactorSep
    * Source blend factor in the separated alpha channel blending equation
    * Syntax: `<blending srcFactorSep="one"/>`
    * Values: "blendFactors" (see below)
    * Default: "one"

  * dstFactorSep
    * Destination blend factor in the separated alpha channel blending equation
    * Syntax: `<blending dstFactorSep="one"/>`
    * Values: "blendFactors" (see below)
    * Default: "zero"

  * blendConstant
    * Constant used as blend factor
    * Syntax: `<blending blendColor="1.0, 0.0, 0.0, 0.5" />`
    * Values: float, float, float, float
    * Default: "1.0, 1.0, 1.0, 1.0"

# blendFactors #
The blendFactor has the following items:
  * zero
  * one
  * srcColor
  * oneMinusSrcColor
  * dstColor
  * oneMinusDstColor
  * srcAlpha
  * oneMinusSrcAlpha
  * dstAlpha
  * oneMinusDstAlpha
  * constantColor
  * oneMinusConstantColor
  * constantAlpha
  * oneMinusConstantAlpha