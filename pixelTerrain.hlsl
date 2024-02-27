// I'm finding arrays are not working in shaders

struct SPixel
{
	float4 position	: SV_POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
	float3 vsPos	: POSITION;
};

// 256 / 16 = 16
cbuffer float4x4s : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4x4 fill[13];
};

// 256 / 4 = 64 floats
cbuffer floats : register(b1)
{
	float fv0;
	float fv1;
	float fv2;
	float fv3;
	float fv4;
	float fv5;
	float fv6;
	float fill1[57];
};

// 256 / 4 = 64 floats
cbuffer ints : register(b2)
{
	int iv0;
	int iv1;
	int iv2;
	int iv3;
	int iv4;
	int fill2[59];
};


SamplerState anisotropic	: register(s0);
SamplerState wrap			: register(s1);

SamplerComparisonState gsamShadow : register(s2);

// You can bind up to 128 texture buffers per pipeline stage.
Texture2D texture0 : register(t0);
Texture2D texture1 : register(t1);
Texture2D texture2 : register(t2);
Texture2D texture3 : register(t3);
Texture2D texture4 : register(t4);
Texture2D texture5 : register(t5);

Texture2D gShadowMap : register(t6);

//---------------------------------------------------------------------------------------
// PCF for shadow mapping.
//---------------------------------------------------------------------------------------

float CalcShadowFactor(float4 shadowPosH)
{
    // Complete projection by doing division by w.
    shadowPosH.xyz /= shadowPosH.w;

    // Depth in NDC space.
    float depth = shadowPosH.z;

    uint width, height, numMips;
    gShadowMap.GetDimensions(0, width, height, numMips);

    // Texel size.
    float dx = 1.0f / (float)width;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx,  -dx), float2(0.0f,  -dx), float2(dx,  -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx,  +dx), float2(0.0f,  +dx), float2(dx,  +dx)
    };

    [unroll]
    for(int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow,
            shadowPosH.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}

float4 PSMain(SPixel vs) : SV_Target0
{
	float4 p = texture0.Sample(wrap, vs.uv * 128);

	p.a = 1.0;

	//return p;

	float pa = p.a;

	p *= 0.75;

	float4 maskMap1 = texture3.Sample(wrap, vs.uv);

	if (maskMap1.r > 0.0)
	{
		float b1 = 1.0 - maskMap1.r;
		float b2 = 1.0 - b1;

		float4 ts = texture1.Sample(wrap, vs.uv * 128);

		p.r = (p.r * b1) + (ts.r * b2);
		p.g = (p.g * b1) + (ts.g * b2);
		p.b = (p.b * b1) + (ts.b * b2);
	}

	float4 maskMap2 = texture4.Sample(wrap, vs.uv);

	if (maskMap2.r > 0.0)
	{
		float b1 = 1.0 - maskMap2.r;
		float b2 = 1.0 - b1;

		float4 ts = texture2.Sample(wrap, vs.uv * 256);

		p.r = (p.r * b1) + (ts.r * b2);
		p.g = (p.g * b1) + (ts.g * b2);
		p.b = (p.b * b1) + (ts.b * b2);
	}

	// if using lightmap
	//p *= texture5.Sample(wrap, vs.uv);

	p.a = pa;

	return p;


	if (fv3 > 0.0)
	{
		float3 vsp = { (fv0 - vs.vsPos.x) , (fv1 - vs.vsPos.y), (fv2 - vs.vsPos.z) };

		float l = length(vsp);

		float a = fv6 / l;
			
		p.r *= fv3 * a;
		p.g *= fv4 * a;
		p.b *= fv5 * a;
	}
}