#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
using namespace DirectX;
#include <wrl.h>
using namespace Microsoft::WRL;

class Shader
{
public:
	enum class BlendConfig
	{
		None, Alpha, Add
	};

	Shader(ID3D12Device* device,
		const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath,
		D3D12_INPUT_ELEMENT_DESC* inputLayouts, UINT layoutNum,
		DXGI_FORMAT rederFormat,
		bool uvWrapEnable, bool sampleLinearEnable,
		BlendConfig blend);
	~Shader();

	bool isEnabled() const { return m_enabled; }
	void setShader(ID3D12GraphicsCommandList* cmdList);

private:
	bool m_enabled;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pso;

	bool readShaderObject(const wchar_t* shaderPath, ID3DBlob** shaderObj);
	bool createRootSignature(ID3D12Device* device, bool uvWrapEnable,
		bool sampleLinearEnable);
	bool createGPipelineState(ID3D12Device* device, ID3DBlob* vertexShader,
		ID3DBlob* pixelShader, DXGI_FORMAT renderTargetFormat,
		D3D12_INPUT_ELEMENT_DESC* inputLayouts, UINT layoutNum,
		BlendConfig blend);
};