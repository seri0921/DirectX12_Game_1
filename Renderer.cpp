#include "Renderer.h"
#include "Game.h"

const wchar_t* Renderer::WhiteTexture(L"White");
const wchar_t* Renderer::BlackTexture(L"Black");
const wchar_t* Renderer::GrayTexture(L"Gray");
const wchar_t* Renderer::RedTexture(L"Red");
const wchar_t* Renderer::GreenTexture(L"Green");
const wchar_t* Renderer::BlueTexture(L"Blue");
const wchar_t* Renderer::YellowTexture(L"Yellow");
const wchar_t* Renderer::CyanTexture(L"Cyan");
const wchar_t* Renderer::MagentaTexture(L"Magenta");

const wchar_t* Renderer::BaseFont(L"BaseFont");

Renderer::Renderer(Game* game, XMFLOAT3 backColor)
	: m_game(game)
	, m_featureLevel()
	, m_bufferIndex(0)
	, m_rtvIncSize(0)
	, m_csuIncSize(0)
	, m_renderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
	, m_backColor{ backColor }
	, m_fenceValues{ 0 }
	, m_camera(CamInParam(90.0f, 0.01f, 1000.0f),
		CamExtPram(ZeroVec3d, UnitVecZ3d, UnitVecY3d))
	, m_cameraMatrix(XMMatrixIdentity())
	, m_spriteMatrix(XMMatrixIdentity())
	, m_spriteVertexBufferView{}
	, m_constBufferMap{}
	, m_textureNum(0)
	, m_spriteDrawList(MaxObjectNum)
	, m_spriteNum(0)
	, m_shaderIndex(ShaderNone)
{
	memset((void*)m_textureReferenceCount, 0, MaxTextureNum * sizeof(int));
}

Renderer::~Renderer()
{
	waitForGPU();
}

bool Renderer::initialize()
{
#ifdef _DEBUG
	enableDebugLayer();
#endif

	if (!createFactory()) return false;
	if (!createDevice(L"NVIDIA")) return false;
	if (!createCommandQueue()) return false;
	if (!createSwapchain()) return false;
	if (!createCommandAllocators()) return false;
	if (!createCommandList()) return false;
	if (!createRenderTargetView()) return false;
	if (!createFence()) return false;
	setViewport(m_viewport);
	setScissorRect(m_scissorRect);

	m_cmdList->Close();

	// 入力レイアウト、ルートシグネチャ、PSO
	D3D12_INPUT_ELEMENT_DESC inputLayouts[2];
	inputLayouts[0] = { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0,
		D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };
	inputLayouts[1] = { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,
		D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 };

	m_shaders[Shader2DLoopLinear] = std::make_unique<Shader>(
		m_device.Get(), L"shader\\simpleVS.cso", L"shader\\simplePS.cso",
		inputLayouts, 2, m_renderTargetFormat,
		true, true, Shader::BlendConfig::None);
	m_shaders[Shader2DAlphaLoopPoint] = std::make_unique<Shader>(
		m_device.Get(), L"shader\\simpleVS.cso", L"shader\\simplePS.cso",
		inputLayouts, 2, m_renderTargetFormat,
		true, false, Shader::BlendConfig::Alpha);

	m_shaders[Shader2DAddLoopPoint] = std::make_unique<Shader>(
		m_device.Get(), L"shader\\simpleVS.cso", L"shader\\AdditiveBlendPS.cso",
		inputLayouts, 2, m_renderTargetFormat,
		true, false, Shader::BlendConfig::Add);
	for (int i = 0; i < SystemShaderNum; ++i)
	{
		if (!m_shaders[i]->isEnabled()) return false;
	}

	// スプライト用頂点バッファの生成
	{
		VertexUV vertices[4];
		vertices[0] = { { -0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f } };
		vertices[1] = { { -0.5f, 0.5f, 0.0f }, { 0.0f, 0.0f } };
		vertices[2] = { { 0.5f, -0.5f, 0.0f }, { 1.0f, 1.0f } };
		vertices[3] = { { 0.5f, 0.5f, 0.0f}, { 1.0f, 0.0f } };
		if (!createResourceBuffer(m_spriteVertexBuffer.GetAddressOf(),
			4 * sizeof(VertexUV))) return false;
		if (!uploadResourceBuffer(m_spriteVertexBuffer.Get(),
			(void*)vertices, 4 * sizeof(VertexUV))) return false;
		setVertexBufferView(m_spriteVertexBufferView, m_spriteVertexBuffer.Get(),
			4 * sizeof(VertexUV), sizeof(VertexUV));
	}

	// シェーダーリソース、定数バッファ用ディスクリプタヒープの生成
	for (int i = 0; i < FrameNum; ++i)
	{
		if (!createDescHeap(m_scDescHeap[i].GetAddressOf(), SCViewNum)) return false;
	}
	// カメラ行列を計算
	m_cameraMatrix = m_camera.calcViewProjMatrix(
		(float)m_game->getWidth(), (float)m_game->getHeight());

	// スプライト行列を計算
	m_spriteMatrix =
		calcSpriteMatrix((float)m_game->getWidth(), (float)m_game->getHeight());

	// 単色テクスチャの生成
	{
		ImageData imgData;
		imgData = createUnicolorTexture(WhiteTexture, ColorRGBA(255, 255, 255, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(BlackTexture, ColorRGBA(0, 0, 0, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(GrayTexture, ColorRGBA(127, 127, 127, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(RedTexture, ColorRGBA(255, 0, 0, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(GreenTexture, ColorRGBA(0, 255, 0, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(BlueTexture, ColorRGBA(0, 0, 0, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(YellowTexture, ColorRGBA(255, 255, 0, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(CyanTexture, ColorRGBA(0, 255, 255, 255));
		if (imgData.imgIndex == -1) return false;
		imgData = createUnicolorTexture(MagentaTexture, ColorRGBA(255, 0, 255, 255));
		if (imgData.imgIndex == -1) return false;

	}

	// フォント生成
	if (!createFontData()) return false;

	return true;
}

void Renderer::begin()
{
	// m_bufferIndex： 描画対象のバッファのインデックス（番号）
	// 対象のコマンドアロケータをリセットし、コマンドリストに接続
	// m_rtvHeapから先頭要素のハンドルを取得し、使用するバッファに対応した要素となるように
	// ハンドルのptrメンバを計算
	// ※コマンドリスト（m_cmdList）のメソッドを呼び出すと、GPUに実行させる命令がコマンドアロケータに
	// 詰まれる（即実行するわけではない）
	// OMSetRenderTargetsメソッドでレンダーターゲットを指定（rtvHandleが指すバッファがセットされる）
	// clearColor： アルファ値（不透過度）を含めた4成分の背景色、アルファ値は1.0fに設定
	// ClearRenderTargetViewメソッドでバッファの映像をクリア（背景色で塗りつぶす）
	//バッファの状態をPRESENTからRENDER_TARGETに遷移させ、レンダーターゲットとして使用できる状態にする
	//m_cmdAllocators[m_bufferIndex]->Reset();
	m_cmdList->Reset(m_cmdAllocators[m_bufferIndex].Get(), nullptr);

	setResourceBarrier(D3D12_RESOURCE_STATE_PRESENT,
		D3D12_RESOURCE_STATE_RENDER_TARGET);

	auto rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += m_bufferIndex * m_rtvIncSize;
	m_cmdList->OMSetRenderTargets(1, &rtvHandle, true, nullptr);

	float clearColor[4] = { m_backColor.x, m_backColor.y, m_backColor.z, 1.0f };
	m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

	// RSSetViewportsメソッド： ビューポートの設定コマンド
	// RSSetScissorRectsメソッド： シザー矩形の設定コマンド
	m_cmdList->RSSetViewports(1, &m_viewport);
	m_cmdList->RSSetScissorRects(1, &m_scissorRect);
}

void Renderer::end()
{
	// ディスクリプタヒープm_scDescHeapのセット
	// 登録されたスプライトを一括描画
	//　Closeメソッドでコマンドリストの描画命令を区切る
	//　ExecuteCommandListsメソッドでコマンドキューにコマンドリストを送信
	//　（コマンドリストに紐づいたコマンドアロケータに貯められたコマンド（命令群）が実行される）
	//　コマンドリストは複数個送ることができるのでコマンドリストの配列の形で送ることに注意
	//　スワップチェインのPresentメソッドでフリップ処理が行われ、描画対象のバッファが切り替わる
	//　GetCurrentBackBufferIndexメソッドで次の描画対象のバッファのインデックスを取得

	m_cmdList->SetDescriptorHeaps(1, m_scDescHeap[m_bufferIndex].GetAddressOf());
	m_shaderIndex = ShaderNone;

	// スプライト描画
	drawBatchSprite();

	setResourceBarrier(D3D12_RESOURCE_STATE_RENDER_TARGET,
		D3D12_RESOURCE_STATE_PRESENT);
	m_cmdList->Close();
	ID3D12CommandList* cmdLists[] = { m_cmdList.Get() };
	m_comdQueue->ExecuteCommandLists(1, cmdLists);
	m_swapchain->Present(1, 0);
	moveToNextFrame();
}

bool Renderer::createFactory()
{
	UINT debugFlag = 0;
#ifdef _DEBUG
	debugFlag = DXGI_CREATE_FACTORY_DEBUG;
#endif

	HRESULT hr = CreateDXGIFactory2(debugFlag, IID_PPV_ARGS(m_dxgiFactory.GetAddressOf()));

	return SUCCEEDED(hr);
}

bool Renderer::createDevice(const wchar_t* adapterName)
{
	// 使用するグラフィックドライバー（アダプター）と機能レベルを選択し、デバイスを初期化
	std::vector<IDXGIAdapter*> adapters;
	IDXGIAdapter* sa = nullptr;
	int i = 0;
	while (m_dxgiFactory->EnumAdapters(i, &sa) != DXGI_ERROR_NOT_FOUND)
	{
		adapters.push_back(sa);
		++i;
	}

	// アダプターの情報（DXGI_ADAPTER_DESC構造体）を取得
	// createDeviceメソッドに渡した名前（adapterName）がアダプター名に含まれれるか探索
	// 発見した場合はsaに該当するアダプターを設定し、なければnullptrのままにする
	sa = nullptr;
	for (auto adp : adapters)
	{
		DXGI_ADAPTER_DESC adpDesc = {};
		adp->GetDesc(&adpDesc);
		std::wstring str = adpDesc.Description;
		if (str.find(adapterName) != std::wstring::npos)
		{
			sa = adp;
			break;
		}
	}

	// 機能レベル： ハードウェア（GPU）が対応している機能の水準を表す
	// 機能レベル（D3D_FEATURE_LEVEL）を12.1から11.0までの範囲で新しいものから順に試す
	// D3D12CreateDevice関数にアダプター（sa）と機能レベル（level）を指定してデバイスを作成し、
	// m_deviceに保存
	// アダプターがnullptrの場合はデフォルトで設定されているGPUが選択される
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	for (auto level : levels)
	{
		if (SUCCEEDED(D3D12CreateDevice(sa, level, IID_PPV_ARGS(m_device.GetAddressOf()))))
		{
			m_featureLevel = level;
			break;
		}
	}

	for (auto adp : adapters)
	{
		adp->Release();
	}

	if (m_device.Get() == nullptr) return false;

	m_rtvIncSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_csuIncSize =
		m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	return true;
}

void Renderer::enableDebugLayer()
{
	//ID3D12Debugクラス： デバッグ情報の管理設定
	//D3D12GetDebugInterface関数でID3D12Debugを取得
	//ID3D12Debug::EnableDebugLayerメソッド： デバッグ情報の出力を有効化
	//一度有効化すればID3D12Debugは不要なのでReleaseメソッドを呼び出して削除
	ComPtr<ID3D12Debug> debugLayer;
	D3D12GetDebugInterface(IID_PPV_ARGS(debugLayer.GetAddressOf()));
	debugLayer->EnableDebugLayer();
}

bool Renderer::createCommandQueue() 
{
	// D3D12_COMMAND_QUEUE_DESC： コマンドキューの設定用構造体
	// CreateCommandQueueメソッド： コマンドキューを生成
	D3D12_COMMAND_QUEUE_DESC desc = {};
	desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	desc.NodeMask = 0;
	desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	HRESULT hr = m_device->CreateCommandQueue(&desc, IID_PPV_ARGS(m_comdQueue.GetAddressOf()));
	return SUCCEEDED(hr);
}

bool Renderer::createSwapchain()
{
	// DXGI_SWAP_CHAIN_DESC1構造体： スワップチェインの設定用構造体
	// スクリーンの幅と高さ、バッファの数を設定
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = m_game->getWidth();
	scDesc.Height = m_game->getHeight();
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.Stereo = FALSE;
	scDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER;
	scDesc.BufferCount = FrameNum;
	scDesc.SampleDesc.Count = 1;
	scDesc.SampleDesc.Quality = 0;
	scDesc.Scaling = DXGI_SCALING_NONE;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	// DXGI_SWAP_CHAIN_FULLSCREEN_DESC： スクリーンモードの設定用構造体
	// WindowedメンバをTRUEにすることでウィンドウモードに設定
	// スクリーンモードを設定するため、一時的にIDXGISwapChain1の形で
	// CreateSwapChainForHwndメソッドを使って生成
	// 生成した一時的なスワップチェイン（swapchain）はComPtrにしておき自動で破棄
	// 一時的に用意したswapchainからAsメソッドでデータをコピーしてm_swapchainを生成
	// GetCurrentBackBufferIndexメソッドで現在の描画対象となるバッファのインデックスを取得
	// MakeWindowAssociationメソッドで第2引数をDXGI_MWA_NO_ALT_ENTERにすることで
	// フルスクリーンに切り替えるショートカット（Altキー＋Enterキー）を封印
	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsDesc = {};
	fsDesc.Windowed = TRUE;
	HWND hwnd = m_game->getHwnd();
	ComPtr<IDXGISwapChain1> swapchain;
	HRESULT hr = m_dxgiFactory->CreateSwapChainForHwnd(
		m_comdQueue.Get(), hwnd, &scDesc, &fsDesc,
		nullptr, swapchain.GetAddressOf());
	if (FAILED(hr)) return false;

	hr = swapchain.As(&m_swapchain);
	if (FAILED(hr)) return false;
	m_bufferIndex = m_swapchain->GetCurrentBackBufferIndex();

	hr = m_dxgiFactory->MakeWindowAssociation(hwnd, DXGI_MWA_NO_ALT_ENTER);

	return SUCCEEDED(hr);
}

// CreateCommandAllocatorメソッドでコマンドアロケータを生成
bool Renderer::createCommandAllocators()
{
	for (UINT i = 0; i < FrameNum; i++)
	{
		HRESULT hr = m_device->CreateCommandAllocator(
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			IID_PPV_ARGS(m_cmdAllocators[i].GetAddressOf()));
		if (FAILED(hr)) return false;
	}
	return true;
}

// CreateCommandListメソッドでコマンドリストを生成
// コマンドリストはコマンドアロケータを結び付けて使用する
bool Renderer::createCommandList()
{
	HRESULT hr = m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_cmdAllocators[m_bufferIndex].Get(), nullptr,
		IID_PPV_ARGS(m_cmdList.GetAddressOf()));

	return SUCCEEDED(hr);
}


// ディスクリプタヒープのGetCPUDescriptorHandleFroHeapStartメソッドで先頭要素への
// ハンドル（ポインタ的なもの）を取得
// ハンドルのptrメンバにm_rtvIncSizeを加算することで次の要素に移動
// スワップチェインのGetBufferメソッドにより、i番目のバッファへのリソースを取得
// D3D12_RENDER_TARGET_VIEW_DESC： レンダーターゲットビューの構造体
// CreateRenderTargetViewメソッドでディスクリプタヒープの指定要素にビューを設定
bool Renderer::createRenderTargetView()
{
	// レンダーターゲット用のディスクリプタヒープを生成
	// D3D12_DESCRIPTOR_HEAP_DESC： ディスクリプタヒープの設定
	// NumDescriptorsメンバに用意するビューの個数を設定
	// （スワップチェインのバッファリングの個数FrameNumを設定）
	// TypeメンバにD3D12_DESCRIPTOR_HEAP_TYPE_RTVをセット
	// CreateDescriptorHeapメソッドでディスクリプタヒープを生成
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.NumDescriptors = FrameNum;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	desc.NodeMask = 0;

	HRESULT hr = m_device->CreateDescriptorHeap(
		&desc, IID_PPV_ARGS(m_rtvHeap.GetAddressOf()));
	if (FAILED(hr)) return false;

	auto handle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < FrameNum; i++)
	{
		hr = m_swapchain->GetBuffer(i, IID_PPV_ARGS(m_backBuffers[i].GetAddressOf()));
		if (FAILED(hr)) return false;

		D3D12_RENDER_TARGET_VIEW_DESC rDesc = {};
		rDesc.Format = m_renderTargetFormat;
		rDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		rDesc.Texture2D.MipSlice = 0;
		rDesc.Texture2D.PlaneSlice = 0;

		m_device->CreateRenderTargetView(m_backBuffers[i].Get(), &rDesc, handle);
		handle.ptr += m_rtvIncSize;
	}

	return true;
}

void Renderer::setBackColor(float r, float g, float b)
{
	m_backColor = XMFLOAT3(r, g, b);
}

bool Renderer::createFence()
{
	// フェンスの初期値は0に設定
	// m_fanceValues[m_bufferIndex]の値を1に設定している（処理完了予定時の値）
	for (UINT i = 0; i < FrameNum; i++)
	{
		m_fenceValues[i] = 0;
	}

	HRESULT hr = m_device->CreateFence(
		m_fenceValues[m_bufferIndex],
		D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_fence.GetAddressOf()));
	if (FAILED(hr)) return false;

	m_fenceValues[m_bufferIndex]++;
	return true;
}

void Renderer::moveToNextFrame()
{
	// コマンドキューのSignalメソッドにフェンスと処理完了時のフェンスの値を設定
	// フェンス値で処理が完了したかを判断し、処理が完了していなければWaitForSingleObjectEX関数
	// で処理が完了するまで待機
	// 次のバッファの処理完了時のフェンス値を設定（現在のフェンス値に1加算したもの）
	auto currentValue = m_fenceValues[m_bufferIndex];
	m_comdQueue->Signal(m_fence.Get(), currentValue);
	m_bufferIndex = m_swapchain->GetCurrentBackBufferIndex();
	if (m_fence->GetCompletedValue() < m_fenceValues[m_bufferIndex])
	{
		HANDLE event = CreateEvent(nullptr, false, false, nullptr);
		if (event)
		{
			m_fence->SetEventOnCompletion(m_fenceValues[m_bufferIndex], event);
			WaitForSingleObjectEx(event, INFINITE, FALSE);
			CloseHandle(event);
		}
	}
	m_fenceValues[m_bufferIndex] = currentValue += 1;
}

void Renderer::waitForGPU()
{
	//GPUの処理が完了しない状態でRendererクラスの終了処理を行うとエラーが発生するので
	//処理を待機させるためのメソッド
	m_comdQueue->Signal(m_fence.Get(), m_fenceValues[m_bufferIndex]);
	HANDLE event = CreateEvent(nullptr, false, false, nullptr);
	if (event)
	{
		m_fence->SetEventOnCompletion(m_fenceValues[m_bufferIndex], event);
		WaitForSingleObjectEx(event, INFINITE, FALSE);
		CloseHandle(event);
	}
	m_fenceValues[m_bufferIndex]++;
	m_bufferIndex = m_swapchain->GetCurrentBackBufferIndex();
}

void Renderer::setResourceBarrier(D3D12_RESOURCE_STATES stateBefore,
	D3D12_RESOURCE_STATES stateAfter)
{
	// stateBefore： 前の状態、stateAfter： 後の状態
	// 処理対象のバッファm_backBuffers[m_bufferIndex]の状態遷移をResourceBarrierメソッドで設定
	D3D12_RESOURCE_BARRIER desc = {};
	desc.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	desc.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	desc.Transition.pResource = m_backBuffers[m_bufferIndex].Get();
	desc.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	desc.Transition.StateBefore = stateBefore;
	desc.Transition.StateAfter = stateAfter;

	m_cmdList->ResourceBarrier(1, &desc);
}

bool Renderer::createResourceBuffer(ID3D12Resource** buffer, UINT64 bSize)
{
	// ビデオメモリ上にバッファを生成（C言語のmalloc関数のようなもの）
	// buffer： ID3D12Resourceのポインタのポインタ
	// bSize： 生成するバッファのバイトサイズ
	// バッファを生成するにはアクセス方法など様々な設定が必要
	// D3D12_HEAP_PROPERTIES構造体でアクセス方法を設定
	// D3D12_RESOURCE_DESC構造体： バッファがどのようなデータか設定
	// 1次元データとしてバッファを設定（D3D12_RESOURCE_DESC構造体）
	D3D12_HEAP_PROPERTIES heapProp = {};
	heapProp.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProp.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProp.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Width = bSize;
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

	HRESULT hr = m_device->CreateCommittedResource(
		&heapProp, D3D12_HEAP_FLAG_NONE, &desc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
		IID_PPV_ARGS(buffer));

	return SUCCEEDED(hr);
}

bool Renderer::uploadResourceBuffer(ID3D12Resource* buffer, void* src, size_t bSize,
	void** map)
{
	// Mapを用いてバッファにデータを転送
	// ID3D12ResourceのMapメソッドを用い、アクセスに使うポインタのポインタを渡すことでバッファにアクセス可能となる
	// 転送が終わったあとはUnmapメソッドでMapの状態を解消する
	// buffer： 転送先のバッファ
	// src： 転送元データのポインタ（汎用ポインタvoid* とし、どのようなデータでも対応可にする）
	// bSize： 転送するデータのバイトサイズ
	// map： Mapした際のポインタを維持するためのポインタのポインタ
	// Mapした後でUnmapせずにポインタを介してデータ転送を行えるようにする
	// mapがnullptrの場合はUnmapする
	// Mapした際のポインタを介して、通常の方法でデータの書き込みが可能
	// ここではデータコピーのmemcpy関数を用いてsrcの場所にあるデータをbSizeバイト分コピーする
	// 形でバッファにデータを転送
	if (map != nullptr)
	{
		*map = nullptr;
		HRESULT hr = buffer->Map(0, nullptr, map);
		if (FAILED(hr) || *map == nullptr) return false;
		memcpy(*map, src, bSize);
	}
	else
	{
		void* pmap = nullptr;
		HRESULT hr = buffer->Map(0, nullptr, &pmap);
		if (FAILED(hr) || pmap == nullptr) return false;
		memcpy(pmap, src, bSize);
		buffer->Unmap(0, nullptr);
	}
	return true;
}

void Renderer::setVertexBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView,
	ID3D12Resource* buffer, UINT bSize, UINT stride)
{
	// D3D12_VERTEX_BUFFER_VIEW構造体： 頂点バッファの用途を記述するビューの構造体
	// vertexBufferView： 頂点バッファのビュー構造体
	// buffer： 関連付ける頂点バッファ
	// bSize： 頂点バッファ全体のバイトサイズ
	// stride： 1頂点のバイトサイズ
	vertexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
	vertexBufferView.SizeInBytes = bSize;
	vertexBufferView.StrideInBytes = stride;
}

void Renderer::setViewport(D3D12_VIEWPORT& viewport)
{
	// MinDepthメンバ、MaxDepthメンバで奥行き方向の範囲を0.0f～1.0fに設定

	viewport.Width = (float)m_game->getWidth();
	viewport.Height = (float)m_game->getHeight();
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
}

void Renderer::setScissorRect(D3D12_RECT& scissor)
{
	scissor.left = 0;
	scissor.top = 0;
	scissor.right = (LONG)m_game->getWidth();
	scissor.bottom = (LONG)m_game->getHeight();
}

void Renderer::setIndexBufferView(D3D12_INDEX_BUFFER_VIEW& indexBufferView,
	ID3D12Resource* buffer, UINT bSize)
{
	// indexBufferView： 設定するインデックスバッファビューの構造体
	// buffer： 関連付けるインデックスバッファ
	// bSize： インデックスバッファのバイトサイズ
	// Formatメンバにインデックスの型に対応したフォーマットを設定
	// DXGI_FORMAT_R16_UINT： R16は1成分のみの16bit、UINTは非負の整数型であることを示す

	indexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = bSize;
	indexBufferView.Format = DXGI_FORMAT_R16_UINT;

}

bool Renderer::createShaderResource(ID3D12Resource** buffer, UINT width, UINT height,
	DXGI_FORMAT format, UINT16 mipLevels, UINT16 depthOrArraySize,
	D3D12_RESOURCE_DIMENSION dimension)
{
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_CUSTOM;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_L0;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = dimension;
	desc.Width = width;
	desc.Height = height;
	desc.DepthOrArraySize = depthOrArraySize;
	desc.MipLevels = mipLevels;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;

	HRESULT hr = m_device->CreateCommittedResource(
		&prop, D3D12_HEAP_FLAG_NONE,
		&desc, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
		nullptr, IID_PPV_ARGS(buffer));

	return SUCCEEDED(hr);
}

bool Renderer::uploadShaderResource(ID3D12Resource* buffer, void* src,
	UINT lineSize, UINT allSize)
{
	HRESULT hr = buffer->WriteToSubresource(0, nullptr,
		src, lineSize, allSize);

	return SUCCEEDED(hr);
}

bool Renderer::createDescHeap(ID3D12DescriptorHeap** dHeap, UINT dNum)
{
	D3D12_DESCRIPTOR_HEAP_DESC desc = {};
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	desc.NodeMask = 0;
	desc.NumDescriptors = dNum;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

	HRESULT hr = m_device->CreateDescriptorHeap(
		&desc, IID_PPV_ARGS(dHeap));

	return SUCCEEDED(hr);
}

void Renderer::setShaderResourceView(ID3D12Resource* buffer, DXGI_FORMAT format,
	ID3D12DescriptorHeap* dHeap, UINT index)
{
	D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.Format = format;
	desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	desc.Texture2D.MipLevels = 1;

	auto handle = dHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += m_csuIncSize * index;
	m_device->CreateShaderResourceView(buffer, &desc, handle);
}

bool Renderer::readImageFile(ID3D12Resource** buffer, TexMetadata& metadata,
	const wchar_t* filePath, bool ddsFlag)
{
	ScratchImage img = {};

	HRESULT hr;
	if (ddsFlag)
	{
		hr = LoadFromDDSFile(filePath, DDS_FLAGS_NONE, &metadata, img);
	}
	else
	{
		hr = LoadFromWICFile(filePath, WIC_FLAGS_NONE, &metadata, img);
	}
	if (FAILED(hr)) return false;

	if (!createShaderResource(buffer, (UINT)metadata.width,
		(UINT)metadata.height, metadata.format,
		(UINT16)metadata.mipLevels, (UINT16)metadata.arraySize,
		(D3D12_RESOURCE_DIMENSION)metadata.dimension)) return false;

	auto image = img.GetImage(0, 0, 0);
	if (!uploadShaderResource(*buffer, image->pixels, (UINT)image->rowPitch,
		(UINT)image->slicePitch)) return false;

	return true;
}

bool Renderer::createConstBuffer(ID3D12Resource** buffer, const void* src,
	size_t dsize, void** pmap)
{
	// 定数バッファの生成とデータ転送をまとめたメソッド
	// buffer： 定数バッファ
	// src： 送信するデータのポインタ
	// dsize： データのバイトサイズ
	// pmap： マップへのポインタ（nullptrならUnmapする）
	// 定数バッファのバイトサイズは256の倍数という制約があるので、calcAlignment256メソッドでサイズを
	// 調整して定数バッファを生成

	if (!createResourceBuffer(buffer, calcAlignment256(dsize))) return false;

	if (pmap == nullptr)
	{
		void* map = nullptr;
		HRESULT hr = (*buffer)->Map(0, nullptr, &map);
		if (FAILED(hr) || map == nullptr) return false;
		memcpy(map, src, dsize);
		(*buffer)->Unmap(0, nullptr);
	}
	else
	{
		*pmap = nullptr;
		HRESULT hr = (*buffer)->Map(0, nullptr, pmap);
		if (FAILED(hr) || (*pmap) == nullptr) return false;
		memcpy(*pmap, src, dsize);
	}

	return true;
}

void Renderer::setConstBufferView(ID3D12Resource* buffer,
	ID3D12DescriptorHeap* dHeap, UINT index)
{
	// D3D12_CONST_BUFFER_VIEW_DESC： 定数バッファビューの設定構造体
	// CreateConstantBufferViewメソッドで定数バッファを生成
	// シェーダリソースと同じグループのディスクリプタヒープの中に配置
	// buffer： 定数バッファ
	// dHeap： ディスクリプタヒープ
	// index： ビューを配置する要素番号（インデックス）

	D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
	desc.BufferLocation = buffer->GetGPUVirtualAddress();
	desc.SizeInBytes = (UINT)buffer->GetDesc().Width;

	auto handle = dHeap->GetCPUDescriptorHandleForHeapStart();
	handle.ptr += index * m_csuIncSize;

	m_device->CreateConstantBufferView(&desc, handle);
}

UINT64 Renderer::calcAlignment256(size_t size)
{
	// size： 指定のバイトサイズ
	// sizeを下回らない最も近い256の倍数を計算
	// ビット演算を使って計算している
	// 0xffは255の16進数表記
	// ~はビットの反転の演算子、~0xffで256の倍数を表す
	// & 演算子はビットの論理積

	UINT64 s = (UINT64)size;

	return (size + 0xff) & ~0xff;
}

void Renderer::setCameraInParam(CamInParam inParam)
{
	// カメラの内部パラメータを変更
	// カメラ情報を変更した際に、ビュープロジェクション行列を再計算

	m_camera.setInParam(inParam);
	m_cameraMatrix = m_camera.calcViewProjMatrix(
		(float)m_game->getWidth(), (float)m_game->getHeight());
}

void Renderer::setCameraExtParam(CamExtPram extParam)
{
	// カメラの外部パラメータを変更
	// カメラ情報を変更した際に、ビュープロジェクション行列を再計算

	m_camera.setExtParam(extParam);
	m_cameraMatrix = m_camera.calcViewProjMatrix(
		(float)m_game->getWidth(), (float)m_game->getHeight());
}

void Renderer::setCameraParam(CamInParam inParam, CamExtPram extParam)
{
	// カメラの内部パラメータ、外部パラメータを変更
	// カメラ情報を変更した際に、ビュープロジェクション行列を再計算

	m_camera.setInParam(inParam);
	m_camera.setExtParam(extParam);
	m_cameraMatrix = m_camera.calcViewProjMatrix(
		(float)m_game->getWidth(), (float)m_game->getHeight());
}

int Renderer::allocateConstBuffer(const void* pData, size_t dsize)
{
	// 未使用の定数バッファのインデックスを探索、満杯なら-1を返して終了
	// 探索した管理インデックスの場所に定数バッファを生成
	// ディスクリプタヒープm_scDescHeapの対応する場所に定数バッファビューを配置
	// この段階ではシェーダリソースは割り当てないが、nullptrのシェーダリソースビューを配置しておく
	// 成功したら定数バッファの管理インデックスを返す

	int index = 0;
	for (; index < MaxObjectNum; ++index)
	{
		if (m_constBuffer[0][index].Get() == nullptr) break;
	}
	if (index >= MaxObjectNum) return -1;

	for (int i = 0; i < FrameNum; ++i)
	{
		if (!createConstBuffer(m_constBuffer[i][index].GetAddressOf(),
			pData, dsize, &m_constBufferMap[i][index]))
		{
			if (i > 0)
			{
				for (int j = i - 1; j >= 0; --j)
				{
					m_constBuffer[j][index].Reset();
				}
			}
			return -1;
		}

		setConstBufferView(m_constBuffer[i][index].Get(), m_scDescHeap[i].Get(),
			index * ViewNum);

		for (int k = 0; k < TexViewNum; ++k)
		{
			setShaderResourceView(nullptr, DXGI_FORMAT_R8G8B8A8_UNORM,
				m_scDescHeap[i].Get(), index * ViewNum + ConstViewNum + k);
		}
	}

	return index;
}

void Renderer::uploadConstBuffer(int index, const void* pData, size_t dsize)
{
	// 定数バッファの管理インデックスが有効な範囲なら定数バッファをデータ送信

	if (index < 0 || index >= MaxObjectNum) return;

	memcpy(m_constBufferMap[m_bufferIndex][index], pData, dsize);
}

void Renderer::releaseConstBuffer(int index)
{
	// 定数バッファの管理インデックスが有効な範囲かつ定数バッファのデータが存在する場合に実行
	// 削除予定のリストm_constBufferReleaseListに記録するだけで即時解放はしない
	// （注意）定数バッファをGPU側が描画処理に使用している可能性があるため
	if (index < 0 || index >= MaxObjectNum) return;
	if (m_constBuffer[0][index].Get() == nullptr) return;

	m_constBufferReleaseList.push_back(std::make_pair(index, ReleaseCountStart));
}

void Renderer::update(float deltaTime)
{
	{
		// 定数バッファの解放
		auto it = m_constBufferReleaseList.begin();
		while (it != m_constBufferReleaseList.end())
		{
			(*it).second -= 1;
			if ((*it).second <= 0)
			{
				int index = (*it).first;
				for (UINT i = 0; i < FrameNum; ++i)
				{
					m_constBuffer[i][index]->Unmap(0, nullptr);
					m_constBuffer[i][index].Reset();
				}
				it = m_constBufferReleaseList.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	{
		// シェーダーリソース（テクスチャ）の解放
		auto it = m_texBufferReleaseList.begin();
		while (it != m_texBufferReleaseList.end())
		{
			(*it).second -= 1;
			if ((*it).second <= 0)
			{
				int index = (*it).first;
				m_textureBuffer[index].Reset();
				it = m_texBufferReleaseList.erase(it);
			}
			else
			{
				++it;
			}
		}
	}
}

ImageData Renderer::allocateShaderResource(const std::wstring& filePath,
	bool ddsFlag)
{
	// 連想配列（m_textures）を探索し、発見したらそのImageData構造体を返して終了
	// 存在しない場合は未使用のシェーダリソースの管理インデックスを探索し、シェーダリソースを生成

	auto it = m_textures.find(filePath);
	if (it != m_textures.end())
	{
		ImageData id = (*it).second;
		m_textureReferenceCount[id.imgIndex] += 1;
		return id;
	}
	ImageData imgData;
	if (m_textureNum >= MaxTextureNum) return imgData;

	int index = 0;
	while (index < MaxTextureNum)
	{
		if (m_textureBuffer[index].Get() == nullptr) break;
		++index;
	}
	if (index >= MaxTextureNum) return imgData;

	TexMetadata metadata = {};
	if (!readImageFile(m_textureBuffer[index].GetAddressOf(),
		metadata, filePath.c_str(), ddsFlag)) return imgData;

	imgData.imgIndex = index;
	imgData.width = (int)metadata.width;
	imgData.height = (int)metadata.height;
	imgData.filePath = filePath;
	imgData.format = metadata.format;
	m_textures[filePath] = imgData;
	m_textureReferenceCount[index] += 1;

	return imgData;
}

void Renderer::setMaterialSlot(int index, int slotNum, const ImageData& imgData)
{
	// 指定の場所（slotNum）にシェーダリソースビューを配置
	// 1回の描画で使用できるシェーダリソース（テクスチャ）が0～TexViewNumまで対応可能

	if (index < 0 || index >= MaxObjectNum) return;
	if (slotNum < 0 || slotNum >= TexViewNum) return;

	for (int i = 0; i < FrameNum; i++)
	{
		setShaderResourceView(m_textureBuffer[imgData.imgIndex].Get(),
			imgData.format, m_scDescHeap[i].Get(), index * ViewNum + ConstViewNum + slotNum);
	}
}

bool Renderer::drawSprite(int modelIndex, int shaderIndex, ImageData imgData,
	XMFLOAT2 pos, float theta, XMFLOAT2* scale, XMFLOAT2 offset,
	XMFLOAT2 imgPos, XMFLOAT2* imgScale, XMFLOAT3 color, float alpha)
{
	// スプライト描画の最大数を超えた場合は処理しない
	// scale、imgScaleがnullptrならテクスチャの実サイズ（画像サイズ）を使用
	// SpriteTransDataを容易し、スプライトの変換行列を計算

	if (m_spriteNum >= MaxObjectNum) return false;

	XMFLOAT2 texSize = (scale != nullptr) ? *scale
		: XMFLOAT2((float)imgData.width, (float)imgData.height);
	XMFLOAT2 uvSize = (imgScale != nullptr) ? *imgScale
		: XMFLOAT2((float)imgData.width, (float)imgData.height);

	SpriteTransData mat;
	mat.posMat =
		calcSpriteModelMatrix(texSize, pos, offset, theta) * m_spriteMatrix;
	mat.uvMat = calcSpriteUVMatrix(imgPos, uvSize.x, uvSize.y,
		(float)imgData.width, (float)imgData.height);
	mat.color = color;
	mat.alpha = alpha;

	memcpy(m_constBufferMap[m_bufferIndex][modelIndex], (void*)&mat,
		sizeof(SpriteTransData));
	m_spriteDrawList[m_spriteNum] = SpriteDrawInfo(modelIndex, shaderIndex);

	m_spriteNum += 1;
	return true;
}

void Renderer::setCommandCSBufferView(int index)
{
	// 定数バッファの管理インデックスindexに基づいてビューの配置位置を計算
	// 定数バッファビューを1番のディスクリプタテーブルにバインド
	// シェーダリソースビューを0番のディスクリプタテーブルにバインド

	if (index < 0 || index >= MaxObjectNum) return;

	int idx = ViewNum * index;
	{
		// 定数バッファビュー
		auto handle = m_scDescHeap[m_bufferIndex]->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += idx * m_csuIncSize;
		m_cmdList->SetGraphicsRootDescriptorTable(1, handle);
	}
	{
		// シェーダーリソースビュー
		auto handle = m_scDescHeap[m_bufferIndex]->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += (idx + ConstViewNum) * m_csuIncSize;
		m_cmdList->SetGraphicsRootDescriptorTable(0, handle);
	}
}

void Renderer::drawBatchSprite()
{
	// STRIPモードに設定
	// ルートシグネチャ、PSO、スプライト用の頂点バッファビューを設定
	// m_spriteDrawListに登録された定数バッファの管理インデックスからビューをバインドし、順番に
	// スプライト（矩形ポリゴン）を描画
	
	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	m_cmdList->IASetVertexBuffers(0, 1, &m_spriteVertexBufferView);

	for (size_t i = 0; i < m_spriteNum; ++i)
	{
		setShader(m_spriteDrawList[i].shaderIndex);
		setCommandCSBufferView(m_spriteDrawList[i].modelIndex);
		m_cmdList->DrawInstanced(4, 1, 0, 0);
	}

	m_spriteNum = 0;
}

void Renderer::setShader(int shaderIndex)
{
	if (m_shaderIndex == shaderIndex) return;
	if (shaderIndex < 0 || shaderIndex >= ShaderNum) return;

	m_shaders[shaderIndex]->setShader(m_cmdList.Get());
	m_shaderIndex = shaderIndex;
}

ImageData Renderer::createUnicolorTexture(const wchar_t* imageName, ColorRGBA color)
{
	ImageData imgData;
	if (m_textureNum >= MaxTextureNum) return imgData;
	auto it = m_textures.find(imageName);
	if (it != m_textures.end())
	{
		ImageData id = (*it).second;
		m_textureReferenceCount[id.imgIndex] += 1;
		return id;
	}

	int index = 0;
	while (index < MaxTextureNum)
	{
		if (m_textureBuffer[index].Get() == nullptr) break;
		++index;
	}
	if (index >= MaxTextureNum) return imgData;

	std::vector<ColorRGBA> colors(4 * 4);
	for (ColorRGBA& c : colors)
	{
		c = color;
	}

	if (!createShaderResource(m_textureBuffer[index].GetAddressOf(),
		4, 4, m_renderTargetFormat)) return imgData;
	if (!uploadShaderResource(m_textureBuffer[index].Get(), colors.data(),
		(UINT)(sizeof(ColorRGBA) * 4), (UINT)(sizeof(ColorRGBA) * 16))) return imgData;

	imgData.imgIndex = index;
	imgData.width = 4;
	imgData.height = 4;
	imgData.filePath = imageName;
	m_textures[imageName] = imgData;
	m_textureReferenceCount[index] += 1;

	return imgData;
}

void Renderer::releaseShaderResource(const ImageData& imgData)
{
	int index = imgData.imgIndex;
	if (index < 0 || index >= MaxTextureNum) return;
	if (m_textureReferenceCount[index] <= 0) return;
	if (m_textureBuffer[index].Get() == nullptr) return;

	m_textureReferenceCount[index] -= 1;
	if (m_textureReferenceCount[index] == 0)
	{
		auto it = m_textures.find(imgData.filePath);
		if (it == m_textures.end()) return;

		m_texBufferReleaseList.push_back(std::make_pair(index, ReleaseCountStart));
		m_textures.erase(it);
	}
}

bool Renderer::createFontData()
{
	// ベースフォントの生成
	{
		std::wstring ip(L"src\\PixelMplus12.png");
		ImageData id = allocateShaderResource(ip);
		if (id.imgIndex == -1) return false;

		std::vector<FontIndex> findex;
		findex.push_back(FontIndex(L' ', L' ' + 96, 0));	// 空白文字を使用      
		findex.push_back(FontIndex(L'あ', L'あ' + 190, 96));
		findex.push_back(FontIndex(0x4E00, 0x4E00 + 20950, 96 + 190));

		m_fonts[BaseFont] = FontData(ip, findex, 256, 90, false);
	}

	return true;
}

bool Renderer::getFontData(const wchar_t* fontName, FontData& fd)
{
	auto it = m_fonts.find(fontName);
	if (it == m_fonts.end()) return false;

	fd = (*it).second;
	return true;
}