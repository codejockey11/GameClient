struct SPixel
{
	float4 position	: SV_POSITION;
	float2 uv		: TEXCOORD;
};

cbuffer float4x4s : register(b0)
{
	row_major float4x4 wvp;
	row_major float4x4 fill[15];
};

SPixel VSMain(float3 position : POSITION, float2 uv : TEXCOORD)
{
	SPixel result;

	float4 pos = float4(position.xyz, 1.0f);

	result.position = mul(pos, wvp);

	result.uv = uv;

	return result;
}