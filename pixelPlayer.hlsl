struct SPixel
{
	float4 position	: SV_POSITION;
	float3 normal	: NORMAL;
	float2 uv		: TEXCOORD;
	float3 vsPos	: POSITION;
};

// 256 / 16 = 16 float4x4s per buffer
cbuffer float4x4s : register(b0)
{
	float4x4 worldMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float4x4 fill[13];
};

// 256 / 4 = 64 floats per buffer
cbuffer floats : register(b1)
{
	float fv0;
	float fv1;
	float fv2;
	float fv3;
	float fv4;
	float fv5;
	float fv6;
	float fv7;
	float fill1[56];
};

// 256 / 4 = 64 ints per buffer
cbuffer ints : register(b2)
{
	int iv0;
	int iv1;
	int iv2;
	int iv3;
	int iv4;
	float fill2[59];
};

SamplerState anisotropic : register(s0);
SamplerState wrap		 : register(s1);

Texture2D	textureKd : register(t0);

float4 PSMain(SPixel vs) : SV_Target0
{
	float4 p = textureKd.Sample(wrap, vs.uv);

	// team red
	if (fv0 == 0.0)
	{
		if (vs.normal.y > 0.0)
		{
			if (p.r == 1.0)
			{
				if (p.g == 1.0)
				{
					if (p.b == 1.0)
					{
						p.r = 1.0;
						p.g = 0.0;
						p.b = 0.0;
						p.a = 1.0;
					}
				}
			}
			else
			{
				p.r = 0.50;
				p.g = 0.0;
				p.b = 0.0;
				p.a = 1.0;
			}
		}
		else
		{
			p.r = 0.50;
			p.g = 0.0;
			p.b = 0.0;
			p.a = 1.0;
		}
	}
	
	// team blue
	if (fv0 == 1.0)
	{
		if (vs.normal.y > 0.0)
		{
			if (p.r == 1.0)
			{
				if (p.g == 1.0)
				{
					if (p.b == 1.0)
					{
						p.r = 0.0;
						p.g = 0.0;
						p.b = 1.0;
						p.a = 1.0;
					}
				}
			}
			else
			{
				p.r = 0.0;
				p.g = 0.0;
				p.b = 0.50;
				p.a = 1.0;
			}
		}
		else
		{
			p.r = 0.0;
			p.g = 0.0;
			p.b = 0.50;
			p.a = 1.0;
		}
	}
	
	// collectable
	if (fv0 == 2.0)
	{
		if (vs.normal.y > 0.0)
		{
			if (p.r == 1.0)
			{
				if (p.g == 1.0)
				{
					if (p.b == 1.0)
					{
						p.r = 0.0;
						p.g = 1.0;
						p.b = 0.0;
						p.a = 1.0;
					}
				}
			}
			else
			{
				p.r = 0.0;
				p.g = 0.50;
				p.b = 0.0;
				p.a = 1.0;
			}
		}
		else
		{
			p.r = 0.0;
			p.g = 0.50;
			p.b = 0.0;
		}
	}

	return p;
}