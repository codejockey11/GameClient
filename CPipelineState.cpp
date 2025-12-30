#include "CPipelineState.h"

/*
*/
CPipelineState::CPipelineState()
{
	memset(this, 0x00, sizeof(CPipelineState));
}

/*
*/
CPipelineState::CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog)
{
	memset(this, 0x00, sizeof(CPipelineState));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	m_descriptorRange = new CDescriptorRange(2);

	m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1);
	m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1);

	m_rootParameter = new CRootParameter(3);

	m_rootParameter->InitDescriptorTable(0, 1, &m_descriptorRange->m_range[0]);
	m_rootParameter->InitDescriptorTable(1, 1, &m_descriptorRange->m_range[1]);

	m_rootParameter->Init32BitConstant(2, 2);

	m_rootSignature = new CRootSignature(m_graphicsAdapter, m_errorLog);

	m_rootSignature->Create(m_rootParameter->m_count, m_rootParameter->m_rootParameter);

	m_computeDesc.pRootSignature = m_rootSignature->m_signature.Get();
}

/*
*/
CPipelineState::CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t constantBufferCount, int32_t textureCount)
{
	memset(this, 0x00, sizeof(CPipelineState));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	CPipelineState::AllocateSignature(constantBufferCount, textureCount);
}

/*
*/
CPipelineState::CPipelineState(CGraphicsAdapter* graphicsAdapter, CErrorLog* errorLog, int32_t constantBufferCount, int32_t textureCount, BYTE type, D3D12_CULL_MODE cullMode)
{
	memset(this, 0x00, sizeof(CPipelineState));

	m_graphicsAdapter = graphicsAdapter;

	m_errorLog = errorLog;

	switch (type)
	{
	case CPipelineState::VertexType::E_VT_VERTEX:
	{
		m_vertexDescCount = 1;

		m_vertexDesc[0] = { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEX2D:
	{
		m_vertexDescCount = 2;

		m_vertexDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_vertexDesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXNT:
	{
		m_vertexDescCount = 3;

		m_vertexDesc[0] = { "POSITION",	0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_vertexDesc[1] = { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_vertexDesc[2] = { "TEXCOORD",	0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXRGBA:
	{
		m_vertexDescCount = 2;

		m_vertexDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_vertexDesc[1] = { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		break;
	}
	case CPipelineState::VertexType::E_VT_VERTEXT:
	{
		m_vertexDescCount = 2;

		m_vertexDesc[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,	0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
		m_vertexDesc[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

		break;
	}
	}

	m_desc.RasterizerState.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	m_desc.RasterizerState.CullMode = cullMode;
	m_desc.RasterizerState.FrontCounterClockwise = false;
	m_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	m_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	m_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	m_desc.RasterizerState.DepthClipEnable = true;
	m_desc.RasterizerState.MultisampleEnable = false;
	m_desc.RasterizerState.AntialiasedLineEnable = false;
	m_desc.RasterizerState.ForcedSampleCount = 0;
	m_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	m_desc.BlendState.IndependentBlendEnable = false;

	m_blendDesc.LogicOpEnable = false;
	m_blendDesc.SrcBlend = D3D12_BLEND::D3D12_BLEND_ONE;
	m_blendDesc.DestBlend = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
	m_blendDesc.BlendOp = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
	m_blendDesc.SrcBlendAlpha = D3D12_BLEND::D3D12_BLEND_ONE;
	m_blendDesc.DestBlendAlpha = D3D12_BLEND::D3D12_BLEND_INV_SRC_ALPHA;
	m_blendDesc.BlendOpAlpha = D3D12_BLEND_OP::D3D12_BLEND_OP_ADD;
	m_blendDesc.LogicOp = D3D12_LOGIC_OP::D3D12_LOGIC_OP_NOOP;
	m_blendDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE::D3D12_COLOR_WRITE_ENABLE_ALL;

	/*
	These are valid formats for a depth-stencil view:
	DXGI_FORMAT_D16_UNORM
	DXGI_FORMAT_D24_UNORM_S8_UINT
	DXGI_FORMAT_D32_FLOAT
	DXGI_FORMAT_D32_FLOAT_S8X24_UINT
	DXGI_FORMAT_UNKNOWN
	*/

	m_desc.DSVFormat = DXGI_FORMAT::DXGI_FORMAT_D32_FLOAT;

	m_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;
	m_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS_EQUAL;

	m_desc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	m_desc.DepthStencilState.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	m_desc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	m_desc.DepthStencilState.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;

	m_desc.DepthStencilState.BackFace.StencilFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	m_desc.DepthStencilState.BackFace.StencilDepthFailOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	m_desc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP::D3D12_STENCIL_OP_KEEP;
	m_desc.DepthStencilState.BackFace.StencilFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_ALWAYS;

	m_desc.DepthStencilState.StencilEnable = false;

	m_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	m_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	m_desc.SampleMask = UINT_MAX;
	m_desc.NumRenderTargets = 1;
	m_desc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_R8G8B8A8_UNORM;
	m_desc.SampleDesc.Count = 1;

	CPipelineState::AllocateSignature(constantBufferCount, textureCount);
}

/*
*/
CPipelineState::~CPipelineState()
{
	m_pipelineState.Reset();

	SAFE_DELETE(m_rootSignature);
	SAFE_DELETE(m_rootParameter);
	SAFE_DELETE(m_descriptorRange);
}

/*
*/
void CPipelineState::AllocateSignature(int32_t constantBufferCount, int32_t textureCount)
{
	m_descriptorRange = new CDescriptorRange(2);

	m_descriptorRange->SetRange(0, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_CBV, constantBufferCount);
	m_descriptorRange->SetRange(1, D3D12_DESCRIPTOR_RANGE_TYPE::D3D12_DESCRIPTOR_RANGE_TYPE_SRV, textureCount);

	m_rootParameter = new CRootParameter(1);

	m_rootParameter->InitDescriptorTable(0, m_descriptorRange->m_count, m_descriptorRange->m_range);

	m_rootSignature = new CRootSignature(m_graphicsAdapter, m_errorLog);

	m_rootSignature->Create(m_rootParameter->m_count, m_rootParameter->m_rootParameter);
}

/*
* 3 types of alpha blends as decided by the mesh/vertex buffer
*
* foliage: AlphaToCoverageEnable = true, BlendEnable = false
* alpha channels: AlphaToCoverageEnable = false, BlendEnable = true
* solids: AlphaToCoverageEnable = false, BlendEnable = false
*
* pixel shader blend for masked foliage - set true for masked blend and blendDesc.BlendEnable = false
* valid alpha values are 0 for no pixel and 1 for pixel
* 
* regular texture channel alpha blending - set true for alpha blend and desc.BlendState.AlphaToCoverageEnable = false
* can use alpha channel from texture or set in pixel shader;
*/
void CPipelineState::Create(bool alphaToCoverageEnable, bool blendEnable, bool depthEnable, D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType)
{
	m_desc.BlendState.AlphaToCoverageEnable = alphaToCoverageEnable;

	m_blendDesc.BlendEnable = blendEnable;

	for (int32_t i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
	{
		m_desc.BlendState.RenderTarget[i] = m_blendDesc;
	}

	m_desc.InputLayout.pInputElementDescs = m_vertexDesc;
	m_desc.InputLayout.NumElements = m_vertexDescCount;

	m_desc.pRootSignature = m_rootSignature->m_signature.Get();

	m_desc.DepthStencilState.DepthEnable = depthEnable;
	m_desc.PrimitiveTopologyType = primitiveType;

	m_hr = m_graphicsAdapter->m_device->CreateGraphicsPipelineState(&m_desc, __uuidof(ID3D12PipelineState), (void**)&m_pipelineState);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CPipelineState::Create::CreateGraphicsPipelineState:", m_hr);

		return;
	}

	m_pipelineState->SetName(L"CPipelineState::Create");
}

/*
*/
void CPipelineState::CreateCompute()
{
	m_graphicsAdapter->m_device->CreateComputePipelineState(&m_computeDesc, __uuidof(ID3D12PipelineState), (void**)&m_pipelineState);

	m_pipelineState->SetName(L"CPipelineState::CreateCompute");
}

/*
*/
void CPipelineState::CreateShadowMap(D3D12_PRIMITIVE_TOPOLOGY_TYPE primitiveType)
{
	m_desc.BlendState.AlphaToCoverageEnable = false;
	m_desc.BlendState.IndependentBlendEnable = false;

	m_defaultRenderTargetBlendDesc =
	{
		false,
		false,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE,
		D3D12_BLEND_ZERO,
		D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP,
		D3D12_COLOR_WRITE_ENABLE_ALL
	};

	for (int32_t i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; ++i)
	{
		m_desc.BlendState.RenderTarget[i] = m_defaultRenderTargetBlendDesc;
	}

	m_desc.DepthStencilState.DepthEnable = true;
	m_desc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK::D3D12_DEPTH_WRITE_MASK_ALL;
	m_desc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC::D3D12_COMPARISON_FUNC_LESS;

	m_desc.DepthStencilState.StencilEnable = false;

	m_desc.DepthStencilState.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	m_desc.DepthStencilState.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;

	m_defaultStencilOp =
	{
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_STENCIL_OP_KEEP,
		D3D12_COMPARISON_FUNC_ALWAYS
	};
	
	m_desc.DepthStencilState.FrontFace = m_defaultStencilOp;
	m_desc.DepthStencilState.BackFace = m_defaultStencilOp;
	
	m_desc.SampleMask = UINT_MAX;

	m_desc.PrimitiveTopologyType = primitiveType;

	m_desc.SampleDesc.Count = 1;
	m_desc.SampleDesc.Quality = 0;

	m_desc.DSVFormat = m_graphicsAdapter->m_depthStencilFormat;

	m_desc.RasterizerState.FillMode = D3D12_FILL_MODE::D3D12_FILL_MODE_SOLID;
	m_desc.RasterizerState.CullMode = D3D12_CULL_MODE::D3D12_CULL_MODE_NONE;
	m_desc.RasterizerState.FrontCounterClockwise = false;
	m_desc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	m_desc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	m_desc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	m_desc.RasterizerState.DepthClipEnable = true;
	m_desc.RasterizerState.MultisampleEnable = false;
	m_desc.RasterizerState.AntialiasedLineEnable = false;
	m_desc.RasterizerState.ForcedSampleCount = 0;
	m_desc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE::D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	m_desc.RTVFormats[0] = DXGI_FORMAT::DXGI_FORMAT_UNKNOWN;
	m_desc.NumRenderTargets = 0;

	m_desc.InputLayout.pInputElementDescs = m_vertexDesc;
	m_desc.InputLayout.NumElements = m_vertexDescCount;

	m_desc.pRootSignature = m_rootSignature->m_signature.Get();

	m_hr = m_graphicsAdapter->m_device->CreateGraphicsPipelineState(&m_desc, __uuidof(ID3D12PipelineState), (void**)&m_pipelineState);

	if (m_hr != S_OK)
	{
		m_errorLog->WriteDXGIErrorMessage(true, "CPipelineState::CreateShadowMap::CreateGraphicsPipelineState:", m_hr);

		return;
	}

	m_pipelineState->SetName(L"CPipelineState::CreateShadowMap");
}

/*
*/
void CPipelineState::SetComputeShader(CShaderBinary* shaderBinary)
{
	m_computeDesc.CS.pShaderBytecode = shaderBinary->m_shader->GetBufferPointer();
	m_computeDesc.CS.BytecodeLength = shaderBinary->m_shader->GetBufferSize();
}

/*
*/
void CPipelineState::SetPixelShader(CShaderBinary* shaderBinary)
{
	m_desc.PS.pShaderBytecode = shaderBinary->m_shader->GetBufferPointer();
	m_desc.PS.BytecodeLength = shaderBinary->m_shader->GetBufferSize();
}

/*
*/
void CPipelineState::SetVertexShader(CShaderBinary* shaderBinary)
{
	m_desc.VS.pShaderBytecode = shaderBinary->m_shader->GetBufferPointer();
	m_desc.VS.BytecodeLength = shaderBinary->m_shader->GetBufferSize();
}