#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>

#include <wrl.h>
using namespace Microsoft::WRL;

class Renderer
{
public:
	Renderer(class Game* game, float r = 0.0f, float g = 0.0f, float b = 0.0f);
	~Renderer();

	bool initialize();

	void begin();
	void end();

	void setBackColor(float r, float g, float b);
	const float* getBackColor() const { return m_backColor; }

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
	ComPtr<ID3D12Resource> m_backBuffers[FrameNum];	//m_backBuffers： レンダーターゲットのバッファ用のリソース
	ComPtr<ID3D12Fence> m_fence; // m_fence： フェンス
	UINT64 m_fenceValues[FrameNum]; // m_fenceValues： 各バッファの処理完了時のフェンス値

	float m_backColor[3];

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
};

