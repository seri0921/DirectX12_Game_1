#include "Renderer.h"
#include "Game.h"

Renderer::Renderer(Game* game, float r, float g, float b)
	: m_game(game)
	, m_featureLevel()
	, m_bufferIndex(0)
	, m_rtvIncSize(0)
	, m_backColor{ r, g, b }
	, m_fenceValues{ 0 }
{
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

	m_cmdList->Close();

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

	float clearColor[4] = { m_backColor[0], m_backColor[1], m_backColor[2], 1.0f };
	m_cmdList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
}

void Renderer::end()
{
	//　Closeメソッドでコマンドリストの描画命令を区切る
	//　ExecuteCommandListsメソッドでコマンドキューにコマンドリストを送信
	//　（コマンドリストに紐づいたコマンドアロケータに貯められたコマンド（命令群）が実行される）
	//　コマンドリストは複数個送ることができるのでコマンドリストの配列の形で送ることに注意
	//　スワップチェインのPresentメソッドでフリップ処理が行われ、描画対象のバッファが切り替わる
	//　GetCurrentBackBufferIndexメソッドで次の描画対象のバッファのインデックスを取得
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
		rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
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
	m_backColor[0] = r;
	m_backColor[1] = g;
	m_backColor[2] = b;
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

