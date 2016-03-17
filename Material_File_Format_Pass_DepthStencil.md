# The `<depthStencil />` element #

The depthStencil element defines the parameters being used in the depth and stencil testing.

# Attributes #

  * depthTest (in progress)
    * Enable / disable the depth testing
    * Syntax: `<depthStencil depthTest="true"/>`
    * Values: "true" or "false"
    * Default: "true"

  * depthWrite (in progress)
    * Enable / disable the depth buffer writing
    * Syntax: `<depthStencil depthWrite="false"/>`
    * Values: "true" or "false"
    * Default: "true"

  * depthFunc (in progress)
    * Set the depth value comparison function
    * Syntax: `<depthStencil depthFunc="lessEqual"/>`
    * Values: "less", "lessEqual", "greater", "greaterEqual", "notEqual", "always", "never"
    * Default: "less"

  * stencilTest (in progress)
    * Enable / disable the stencil testing
    * Syntax: `<depthStencil stencilTest="true"/>`
    * Values: "true" or "false"
    * Default: "false"

  * stencilFunc (in progress)
    * Set the stencil value comparison function
    * Syntax: `<depthStencil stencilFunc="lessEqual"/>`
    * Values: "less", "lessEqual", "greater", "greaterEqual", "notEqual", "always", "never"
    * Default: "less"