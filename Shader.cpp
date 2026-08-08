#include "shader.h"
#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

Shader::Shader(ID3D12Device* device,
	const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath,
	D3D12_INPUT_ELEMENT_DESC* inputLayouts, UINT layoutNum,
	DXGI_FORMAT renderFormat,
	bool uvWrapEnable, bool sampleLinearEnable, BlendConfig blend)
	: m_enabled(false)
{
	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;

	if (!readShaderObject(vertexShaderPath, vsBlob.GetAddressOf())) return;
	if (!readShaderObject(pixelShaderPath, psBlob.GetAddressOf())) return;
	if (!createRootSignature(device, uvWrapEnable, sampleLinearEnable)) return;
	if (!createGPipelineState(device, vsBlob.Get(), psBlob.Get(),
		renderFormat, inputLayouts, layoutNum, blend)) return;

	m_enabled = true;
}

Shader::~Shader()
{
}

bool Shader::readShaderObject(const wchar_t* shaderPath, ID3DBlob** shaderObj)
{
	HRESULT hr = D3DReadFileToBlob(shaderPath, shaderObj);
	
	return SUCCEEDED(hr);
}

bool Shader::createRootSignature(ID3D12Device* device, bool uvWrapEnable,
	bool sampleLinearEnable)
{
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE range[2] = {};
	range[0].NumDescriptors = 1;
	range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range[0].BaseShaderRegister = 0;
	range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	range[1].NumDescriptors = 1;
	range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
	range[1].BaseShaderRegister = 0;
	range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER param[2] = {};
	param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	param[0].DescriptorTable.pDescriptorRanges = &range[0];
	param[0].DescriptorTable.NumDescriptorRanges = 1;

	param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	param[1].DescriptorTable.pDescriptorRanges = &range[1];
	param[1].DescriptorTable.NumDescriptorRanges = 1;

	desc.pParameters = param;
	desc.NumParameters = 2;

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	D3D12_TEXTURE_ADDRESS_MODE addressMode = (uvWrapEnable) ?
		D3D12_TEXTURE_ADDRESS_MODE_WRAP : D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
	D3D12_FILTER filter = (sampleLinearEnable) ?
		D3D12_FILTER_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_POINT;
	sampler.AddressU = addressMode;
	sampler.AddressV = addressMode;
	sampler.AddressW = addressMode;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.Filter = filter;
	sampler.MinLOD = 0.0f;
	sampler.MaxLOD = D3D12_FLOAT32_MAX;
	sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;

	desc.pStaticSamplers = &sampler;
	desc.NumStaticSamplers = 1;

	ComPtr<ID3DBlob> blob;
	HRESULT hr = D3D12SerializeRootSignature(
		&desc, D3D_ROOT_SIGNATURE_VERSION_1_0,
		blob.GetAddressOf(), nullptr);
	if (FAILED(hr)) return false;

	hr = device->CreateRootSignature(
		0, blob->GetBufferPointer(), blob->GetBufferSize(),
		IID_PPV_ARGS(m_rootSignature.GetAddressOf()));

	return SUCCEEDED(hr);
}

bool Shader::createGPipelineState(ID3D12Device* device, ID3DBlob* vertexShader,
	ID3DBlob* pixelShader, DXGI_FORMAT renderTargetFormat,
	D3D12_INPUT_ELEMENT_DESC* inputLayouts, UINT layoutNum,
	BlendConfig blend)
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC pDesc = {};

	pDesc.VS.pShaderBytecode = vertexShader->GetBufferPointer();
	pDesc.VS.BytecodeLength = vertexShader->GetBufferSize();
	pDesc.PS.pShaderBytecode = pixelShader->GetBufferPointer();
	pDesc.PS.BytecodeLength = pixelShader->GetBufferSize();

	pDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	D3D12_RENDER_TARGET_BLEND_DESC rtDesc = {};
	rtDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	rtDesc.LogicOpEnable = FALSE;
	if (blend != BlendConfig::None)
	{
		rtDesc.BlendEnable = TRUE;
		rtDesc.BlendOp = D3D12_BLEND_OP_ADD;
		if (blend == BlendConfig::Alpha)
		{
			rtDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
			rtDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		}
		else if (blend == BlendConfig::Add)
		{
			rtDesc.SrcBlend = D3D12_BLEND_ONE;
			rtDesc.DestBlend = D3D12_BLEND_ONE;
		}
		rtDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		rtDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		rtDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
	}
	else
	{
		rtDesc.BlendEnable = FALSE;
	}

	pDesc.BlendState.AlphaToCoverageEnable = FALSE;
	pDesc.BlendState.IndependentBlendEnable = FALSE;
	pDesc.BlendState.RenderTarget[0] = rtDesc;

	pDesc.RasterizerState.MultisampleEnable = FALSE;
	pDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	pDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	pDesc.RasterizerState.DepthClipEnable = TRUE;
	pDesc.RasterizerState.FrontCounterClockwise = FALSE;
	pDesc.RasterizerState.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	pDesc.RasterizerState.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	pDesc.RasterizerState.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	pDesc.RasterizerState.AntialiasedLineEnable = FALSE;
	pDesc.RasterizerState.ForcedSampleCount = 0;
	pDesc.RasterizerState.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	pDesc.DepthStencilState.DepthEnable = FALSE;
	pDesc.DepthStencilState.StencilEnable = FALSE;

	pDesc.InputLayout.pInputElementDescs = inputLayouts;
	pDesc.InputLayout.NumElements = layoutNum;

	pDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	pDesc.NumRenderTargets = 1;
	pDesc.RTVFormats[0] = renderTargetFormat;

	pDesc.SampleDesc.Count = 1;
	pDesc.SampleDesc.Quality = 0;

	pDesc.pRootSignature = m_rootSignature.Get();

	HRESULT hr = device->CreateGraphicsPipelineState(&pDesc,
		IID_PPV_ARGS(m_pso.GetAddressOf()));

	return SUCCEEDED(hr);
}

void Shader::setShader(ID3D12GraphicsCommandList* cmdList)
{
	cmdList->SetPipelineState(m_pso.Get());
	cmdList->SetGraphicsRootSignature(m_rootSignature.Get());
}