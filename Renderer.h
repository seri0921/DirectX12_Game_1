#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>
#include "GMath.h"
#include "GameUtil.h"

#include <wrl.h>
using namespace Microsoft::WRL;

#include <DirectXTex.h>
#pragma comment(lib, "DirectXTex.lib")

class Renderer
{
public:
	Renderer(class Game* game, XMFLOAT3 backColor = ZeroVec3d);
	~Renderer();

	bool initialize();

	void begin();
	void end();

	void setBackColor(float r, float g, float b);
	void setBackColor(XMFLOAT3 backColor) { m_backColor = backColor; }
	XMFLOAT3 getBackColor() const { return m_backColor; }

	// 実験用
	void draw();

private:
	class Game* m_game;

	// FrameNum： バッファの個数を設定する定数（ダブルバッファリングを指定）
	static const UINT FrameNum = 2;
	
	ComPtr<IDXGIFactory4> m_dxgiFactory;	// m_dxgiFactory： DXGIの機能を用いるために使用
	D3D_FEATURE_LEVEL m_featureLevel;	// m_featureLevel： GPUの機能レベル
	ComPtr<ID3D12Device> m_device;	// m_device： Direct3D12の機能を用いるために使用するデバイス
	ComPtr<ID3D12CommandQueue> m_comdQueue;	// m_cmdQueue： コマンドキューをComPtrで用意
	ComPtr<IDXGISwapChain3> m_swapchain;	// m_swapchain： スワップチェインをComPtrで用意
	UINT m_bufferIndex;
	ComPtr<ID3D12CommandAllocator> m_cmdAllocators[FrameNum];
	ComPtr<ID3D12GraphicsCommandList> m_cmdList;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;	// m_rtvHeap： レンダーターゲットビュー用のディスクリプタヒープ
	UINT m_rtvIncSize;	// m_rtvIncSize： レンダーターゲットビューのディスクリプタヒープの要素間のサイズ
	UINT m_csuIncSize;
	ComPtr<ID3D12Resource> m_backBuffers[FrameNum];	//m_backBuffers： レンダーターゲットのバッファ用のリソース
	ComPtr<ID3D12Fence> m_fence; // m_fence： フェンス
	UINT64 m_fenceValues[FrameNum]; // m_fenceValues： 各バッファの処理完了時のフェンス値

	XMFLOAT3 m_backColor;
	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_scissorRect;
	DXGI_FORMAT m_renderTargetFormat;
	ComPtr<ID3D12RootSignature> m_simpleRootSig;
	ComPtr<ID3D12PipelineState> m_simplePSO;
	ComPtr<ID3D12DescriptorHeap> m_scDescHeap;
	ComPtr<ID3D12Resource> m_textureBuffer;

	VertexUV m_vertices[4];
	ComPtr<ID3D12Resource> m_vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	unsigned short m_indices[6];
	ComPtr<ID3D12Resource> m_indexBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;

	bool createFactory();
	bool createCommandQueue();
	bool createDevice(const wchar_t* adapterName);
	bool createSwapchain();
	bool createCommandAllocators();
	bool createCommandList();
	bool createRenderTargetView();
	bool createFence();
	void moveToNextFrame();
	void waitForGPU();
	void setResourceBarrier(D3D12_RESOURCE_STATES stateBefore,
		D3D12_RESOURCE_STATES stateAfter);

	// DirectX関連のエラーや警告などの情報はそのままではデバッグ情報として出力されない
	// Debugモードの際に、DirectX関連のデバッグ情報を出力させるようにする
	// enableDebugLayerメソッド： デバッグ情報の出力設定
	void enableDebugLayer();
	void setViewport(D3D12_VIEWPORT& viewport);
	void setScissorRect(D3D12_RECT& scissor);

	bool createResourceBuffer(ID3D12Resource** buffer, UINT64 bSize);
	bool uploadResourceBuffer(ID3D12Resource* buffer, void* src, size_t bSize,
		void** map = nullptr);
	bool createShaderResource(ID3D12Resource** buffer, UINT width, UINT height,
		DXGI_FORMAT format, UINT16 mipLevels = 1, UINT16 depthOrArraySize = 1,
		D3D12_RESOURCE_DIMENSION dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D);
	bool uploadShaderResource(ID3D12Resource* buffer, void* src,
		UINT lineSize, UINT allSize);
	bool createDescHeap(ID3D12DescriptorHeap** dHeap, UINT dNum);
	void setShaderResourceView(ID3D12Resource* buffer, DXGI_FORMAT format,
		ID3D12DescriptorHeap* dHeap, UINT index);
	bool readImageFile(ID3D12Resource** buffer, TexMetadata& metadata,
		const wchar_t* filePath, bool ddsFlag = false);
	void setVertexBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
		ID3D12Resource* buffer, UINT bSize, UINT stride);
	void setIndexBufferView(D3D12_INDEX_BUFFER_VIEW& indexBufferView,
		ID3D12Resource* buffer, UINT bSize);


	bool readShaderObject(const wchar_t* shaderPath, ID3DBlob** shaderObj);
	bool createRootSignature(ID3D12RootSignature** rootSig);
	bool createGPipelineState(ID3D12PipelineState** pso,
		ID3D12RootSignature* rootSig, DXGI_FORMAT renderTargetFormat,
		const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath,
		D3D12_INPUT_ELEMENT_DESC* inputLayouts, UINT layoutNum);
};

