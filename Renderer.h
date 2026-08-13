#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#include <vector>
#include "GMath.h"
#include "GameUtil.h"
#include "Camera.h"
#include "Shader.h"
#include <memory>

#include <wrl.h>
using namespace Microsoft::WRL;

#include <DirectXTex.h>
#pragma comment(lib, "DirectXTex.lib")

#include <unordered_map>

class Renderer
{
public:
	Renderer(class Game* game, XMFLOAT3 backColor = ZeroVec3d);
	~Renderer();

	bool initialize();

	void begin();
	void end();
	void update(float deltaTime);

	static const int ShaderNone = -1;
	static const int Shader2DLoopLinear = 0;
	static const int Shader2DAlphaLoopPoint = 1;
	static const int Shader2DAddLoopPoint = 2;

	void setBackColor(float r, float g, float b);
	void setBackColor(XMFLOAT3 backColor) { m_backColor = backColor; }
	XMFLOAT3 getBackColor() const { return m_backColor; }

	void setCameraInParam(CamInParam inParam);
	void setCameraExtParam(CamExtPram extParam);
	void setCameraParam(CamInParam inParam, CamExtPram extParam);
	XMMATRIX getCameraMatrix() const { return m_cameraMatrix; }

	int allocateConstBuffer(const void* pData, size_t dsize);
	void uploadConstBuffer(int index, const void* pData, size_t dsize);
	void releaseConstBuffer(int index);

	ImageData allocateShaderResource(const std::wstring& filePath,
		bool ddsFlag = false);
	void setMaterialSlot(int index, int slotNum, const ImageData& imgDate);
	void releaseShaderResource(const ImageData& imgData);

	bool drawSprite(int modelIndex, int shaderIndex, ImageData imgData,
		XMFLOAT2 pos, float theta = 0.0f,
		XMFLOAT2* scale = nullptr, XMFLOAT2 offset = ZeroVec2d,
		XMFLOAT2 imgPos = ZeroVec2d, XMFLOAT2* imgScale = nullptr,
		XMFLOAT3 color = Ones3d, float alpha = 1.0f);

	ImageData createUnicolorTexture(const wchar_t* imageName, ColorRGBA color);

	static const wchar_t* WhiteTexture;
	static const wchar_t* BlackTexture;
	static const wchar_t* GrayTexture;
	static const wchar_t* RedTexture;
	static const wchar_t* GreenTexture;
	static const wchar_t* BlueTexture;
	static const wchar_t* YellowTexture;
	static const wchar_t* CyanTexture;
	static const wchar_t* MagentaTexture;

private:
	class Game* m_game;

	// FrameNum： バッファの個数を設定する定数（ダブルバッファリングを指定）
	// MaxTextureNum： シェーダリソース（テクスチャ）の最大数
	// ConstViewNum： CBVの個数（ 𝑛𝑐 ）、TexViewNum： SRVの個数（ 𝑛𝑠 ）
	// ViewNum： 1回の描画（1個のメッシュ）で使用するビューの個数（ 𝑛𝑣 ）
	// MaxObjectNum： 描画できるオブジェクトの最大数
	// SCViewNum： m_scDescHeapのビューの個数（ 𝑛𝑎𝑙𝑙 ）

	static const UINT FrameNum = 2;
	static const UINT MaxTextureNum = 1000;
	static const UINT ConstViewNum = 1;
	static const UINT TexViewNum = 1;
	static const UINT ViewNum = ConstViewNum + TexViewNum;
	static const UINT MaxObjectNum = 10000;
	static const UINT SCViewNum = MaxObjectNum * ViewNum;

	static const UINT SystemShaderNum = 3;
	static const UINT UserShaderNum = 0;
	static const UINT ShaderNum = SystemShaderNum + UserShaderNum;

	static const int ReleaseCountStart = 5;
	std::vector<std::pair<int, int>> m_constBufferReleaseList;
	std::vector<std::pair<int, int>> m_texBufferReleaseList;
	
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
	ComPtr<ID3D12DescriptorHeap> m_scDescHeap[FrameNum];
	ComPtr<ID3D12Resource> m_textureBuffer[MaxTextureNum];
	int m_textureReferenceCount[MaxTextureNum];

	UINT m_textureNum;
	std::unordered_map<std::wstring, ImageData> m_textures;

	Camera m_camera;
	XMMATRIX m_cameraMatrix;
	XMMATRIX m_spriteMatrix;

	ComPtr<ID3D12Resource> m_spriteVertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_spriteVertexBufferView;

	ComPtr<ID3D12Resource> m_constBuffer[FrameNum][MaxObjectNum];
	void* m_constBufferMap[FrameNum][MaxObjectNum];

	std::unique_ptr<Shader> m_shaders[ShaderNum];
	int m_shaderIndex;

	struct SpriteDrawInfo
	{
		int modelIndex;
		int shaderIndex;
		SpriteDrawInfo() : modelIndex(0), shaderIndex(ShaderNone) {}
		SpriteDrawInfo(int model, int s) : modelIndex(model), shaderIndex(s) {}
	};
	std::vector<SpriteDrawInfo> m_spriteDrawList;
	int m_spriteNum;

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
	bool createConstBuffer(ID3D12Resource** buffer, const void* src,
		size_t dsize, void** pmap = nullptr);
	void setConstBufferView(ID3D12Resource* buffer,
		ID3D12DescriptorHeap* dHeap, UINT index);
	UINT64 calcAlignment256(size_t size);
	void setVertexBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
		ID3D12Resource* buffer, UINT bSize, UINT stride);
	void setIndexBufferView(D3D12_INDEX_BUFFER_VIEW& indexBufferView,
		ID3D12Resource* buffer, UINT bSize);

	void setCommandCSBufferView(int index);
	void drawBatchSprite();
	void setShader(int shaderIndex);
};

