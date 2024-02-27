struct SPixel
{
	float4 position	: SV_POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
	float3 vsPos	: POSITION;
};

cbuffer float4x4s : register(b0)
{
	row_major float4x4 worldMatrix;
	row_major float4x4 viewMatrix;
	row_major float4x4 projectionMatrix;
	row_major float4x4 fill[13];
};

SPixel VSMain(float3 position : POSITION, float3 normal : NORMAL, float2 uv : TEXCOORD)
{
	SPixel result;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	float4 pos = float4(position.xyz, 1.0f);

	result.position = mul(pos, worldMatrix);
	result.position = mul(result.position, viewMatrix);
	result.position = mul(result.position, projectionMatrix);

	result.normal = normal;
	
	result.uv = uv;

	result.vsPos = position;

	return result;
}