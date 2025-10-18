#pragma once

#include "framework.h"

#include "../GameCommon/CErrorLog.h"

#include "CGraphicsAdapter.h"

class CShaderHeap
{
public:

	enum
	{
		E_MAX_CONSTANT_BUFFERS = 14,
		E_MAX_TEXTURE_BUFFERS = 128
	};

	ComPtr<ID3D12DescriptorHeap> m_heap;

	CGraphicsAdapter* m_graphicsAdapter;

	D3D12_DESCRIPTOR_HEAP_DESC m_desc;

	D3D12_CPU_DESCRIPTOR_HANDLE m_CPUDescriptorHandle;

	HRESULT m_hr;

	CShaderHeap();
	CShaderHeap(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, UINT descriptorCount);
	~CShaderHeap();
};

/*
https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-constants

Constant buffers reduce the bandwidth required to update shader constants by allowing shader constants to be grouped together and committed at the same time
rather than making individual calls to commit each constant separately.

A constant buffer is a specialized buffer resource that is accessed like a buffer.
Each constant buffer can hold up to 4096 vectors; each vector contains up to four 32-bit values.
You can bind up to 14 constant buffers per pipeline stage (2 additional slots are reserved for internal use).

32-bit values = 4 bytes
each vector must contain at least 4 values
this aligns at 16 bytes

max size

4096 * (4 * 4) = 65536 bytes max per constant buffer

or

4096 * (128 bits (four 32-bit values) / 8 bits per byte) = 65536 bytes

HOWEVER

if you have a float3 in your buffer the GPU will pad at the end of the float3
to make the boundary alignment.  There is no way around this.  So if your buffer looks like

cbuffer buffer : register(b0)
{
	float3 variable;
	float4 variable;
}

you will not be able to align from the cpu side correctly as the GPU will pad the float3
and the cpu will not.

SO

In order to accomodate float3 types with other types you will have to break them apart to get correct alignment

cbuffer buffer : register(b0)
{
	// float3
	float variable1_3_x;
	float variable1_3_y;
	float variable1_3_z;
	
	// float4
	float variable1_4_x;
	float variable1_4_y;
	float variable1_4_z;
	float variable1_4_w;
	
	// float3
	float variable2_3_x;
	float variable2_3_y;
	float variable2_3_z;
	
	// float3
	float variable3_3_x;
	float variable3_3_y;
	float variable3_3_z;
	
	// float4
	float variable4_4_x;
	float variable4_4_y;
	float variable4_4_z;
	float variable4_4_w;
}

AND THEN

reconstruct them in the shader

float3 variable1;

variable1.x = variable1_3_x;
variable1.y = variable1_3_y;
variable1.z = variable1_3_z;


etc ..


A texture buffer is a specialized buffer resource that is accessed like a texture.
Texture access (as compared with buffer access) can have better performance for arbitrarily indexed data. You can bind up to 128 texture buffers per pipeline stage.
*/