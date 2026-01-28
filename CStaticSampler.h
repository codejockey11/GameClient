#pragma once

#include "framework.h"

class CStaticSampler
{
public:

	enum
	{
		STATIC_SAMPLER_COUNT = 7
	};

	D3D12_STATIC_SAMPLER_DESC m_sampler[CStaticSampler::STATIC_SAMPLER_COUNT];

	int32_t m_count;

	D3D12_STATIC_SAMPLER_DESC m_samplerDesc;

	CStaticSampler();
	~CStaticSampler();

	void Add(D3D12_STATIC_SAMPLER_DESC desc);
	void Default();
};