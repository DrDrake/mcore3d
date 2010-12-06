// http://www.paulinternet.nl/?page=bicubic
float cubicInterpolation(in float t, float v0, float v1, float v2, float v3) {
	// Cubic Hermite spline
	// http://en.wikipedia.org/wiki/Bicubic_interpolation
/*	float4x4 M = {
		 0,  2,  0,  0,
		-1,  0,  1,  0,
		 2, -5,  4, -1,
		-1,  3, -3,  1
	};
	return 0.5 * dot(float4(1, t, t*t, t*t*t), mul(M, float4(v0, v1, v2, v3)));*/

	// Uniform cubic B-splines
	// http://en.wikipedia.org/wiki/B-spline#Cubic_B-Spline
	float4x4 M = {
		-1,  3, -3,  1,
		 3, -6,  3,  0,
		-3,  0,  3,  0,
		 1,  4,  1,  0
	};
	return (1.0/6) * dot(float4(t*t*t, t*t, t, 1), mul(M, float4(v0, v1, v2, v3)));
}

// The straight forward implementation of bi-cubic interpolation
float bicubicInterpolation(in float2 uv, in sampler2D tex, in float2 texSize) {
	float2 w = 1.0 / texSize;
	float2 f = frac(uv * texSize);

	float v1 = cubicInterpolation(f.x,
		tex2D(tex, float2(uv.x - w.x,   uv.y - w.y)).x,
		tex2D(tex, float2(uv.x,         uv.y - w.y)).x,
		tex2D(tex, float2(uv.x + w.x,   uv.y - w.y)).x,
		tex2D(tex, float2(uv.x + 2*w.x, uv.y - w.y)).x);

	float v2 = cubicInterpolation(f.x,
		tex2D(tex, float2(uv.x - w.x,   uv.y)).x,
		tex2D(tex, float2(uv.x,         uv.y)).x,
		tex2D(tex, float2(uv.x + w.x,   uv.y)).x,
		tex2D(tex, float2(uv.x + 2*w.x, uv.y)).x);

	float v3 = cubicInterpolation(f.x,
		tex2D(tex, float2(uv.x - w.x,   uv.y + w.y)).x,
		tex2D(tex, float2(uv.x,         uv.y + w.y)).x,
		tex2D(tex, float2(uv.x + w.x,   uv.y + w.y)).x,
		tex2D(tex, float2(uv.x + 2*w.x, uv.y + w.y)).x);

	float v4 = cubicInterpolation(f.x,
		tex2D(tex, float2(uv.x - w.x,   uv.y + 2*w.y)).x,
		tex2D(tex, float2(uv.x,         uv.y + 2*w.y)).x,
		tex2D(tex, float2(uv.x + w.x,   uv.y + 2*w.y)).x,
		tex2D(tex, float2(uv.x + 2*w.x, uv.y + 2*w.y)).x);

	return cubicInterpolation(f.y, v1, v2, v3, v4);
}

// Improved bi-cubic interpolation that only need 4 texture fetch instead of 16
// From the paper: Accuracy of GPU-based B-Spline evaluation
// http://www.dannyruijters.nl/cubicinterpolation/
// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter20.html
float bicubicInterpolationFast(in float2 uv, in sampler2D tex, in float2 texSize) {
	float2 rec_nrCP = 1.0/texSize;
	float2 coord_hg = uv * texSize-0.5;
	float2 index = floor(coord_hg);

	float2 f = coord_hg - index;
	float4x4 M = {
		-1,  3, -3,  1,
		 3, -6,  3,  0,
		-3,  0,  3,  0,
		 1,  4,  1,  0
	};
	M /= 6;

	float4 wx = mul(float4(f.x*f.x*f.x, f.x*f.x, f.x, 1), M);
	float4 wy = mul(float4(f.y*f.y*f.y, f.y*f.y, f.y, 1), M);
	float2 w0 = float2(wx.x, wy.x);
	float2 w1 = float2(wx.y, wy.y);
	float2 w2 = float2(wx.z, wy.z);
	float2 w3 = float2(wx.w, wy.w);

	float2 g0 = w0 + w1;
	float2 g1 = w2 + w3;
	float2 h0 = w1 / g0 - 1;
	float2 h1 = w3 / g1 + 1;

	float2 coord00 = index + h0;
	float2 coord10 = index + float2(h1.x,h0.y);
	float2 coord01 = index + float2(h0.x,h1.y);
	float2 coord11 = index + h1;

	coord00 = (coord00 + 0.5) * rec_nrCP;
	coord10 = (coord10 + 0.5) * rec_nrCP;
	coord01 = (coord01 + 0.5) * rec_nrCP;
	coord11 = (coord11 + 0.5) * rec_nrCP;

	float tex00 = tex2Dlod(tex, float4(coord00, 0, 0)).x;
	float tex10 = tex2Dlod(tex, float4(coord10, 0, 0)).x;
	float tex01 = tex2Dlod(tex, float4(coord01, 0, 0)).x;
	float tex11 = tex2Dlod(tex, float4(coord11, 0, 0)).x;

	tex00 = lerp(tex01, tex00, g0.y);
	tex10 = lerp(tex11, tex10, g0.y);
	return lerp(tex10, tex00, g0.x);
}

// http://freespace.virgin.net/hugo.elias/models/m_perlin.htm
float cosInterpolate(in float2 uv, in sampler2D tex, in float2 texSize) {
	float2 pixelSize = 1.0 / texSize;
	float2 pixelCenterOffset = fmod(uv, pixelSize);
	pixelCenterOffset /= pixelSize;
	float2 uvtl = uv;
	float2 uvtr = float2(uv + float2(pixelSize.x, 0));
	float2 uvbl = float2(uv + float2(0, pixelSize.y));
	float2 uvbr = float2(uv + pixelSize);

	float vtl = tex2D(tex, uvtl).x;
	float vtr = tex2D(tex, uvtr).x;
	float vbl = tex2D(tex, uvbl).x;
	float vbr = tex2D(tex, uvbr).x;

	float2 f = frac(uv * texSize);
	f = f * 3.1415926536;
	f = (1.0 - cos(f)) * 0.5;

	float vx = lerp(vtl, vtr, f.x);
	float vy = lerp(vbl, vbr, f.x);
	return lerp(vx, vy, f.y);
}