#include "Renderer.h"
#include "Game.h"

#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib")

Renderer::Renderer(Game* game, XMFLOAT3 backColor)
	: m_game(game)
	, m_featureLevel()
	, m_bufferIndex(0)
	, m_rtvIncSize(0)
	, m_csuIncSize(0)
	, m_renderTargetFormat(DXGI_FORMAT_R8G8B8A8_UNORM)
	, m_backColor{ backColor }
	, m_fenceValues{ 0 }
	, m_vertexBufferView{}
	, m_indexBufferView{}
{
	m_vertices[0] = { { -0.525f, -0.7f, 0.0f }, { 0.0f, 1.0f } };
	m_vertices[1] = { { -0.525f, 0.7f, 0.0f }, { 0.0f, 0.0f } };
	m_vertices[2] = { { 0.525f, -0.7f, 0.0f }, { 1.0f, 1.0f } };
	m_vertices[3] = { { 0.525f, 0.7f, 0.0f }, { 1.0f, 0.0f } };

	m_indices[0] = 0;	m_indices[1] = 1;	m_indices[2] = 2;
	m_indices[3] = 3;	m_indices[4] = 2;	m_indices[5] = 1;

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

	if (!createRootSignature(m_simpleRootSig.GetAddressOf())) return false;
	if (!createGPipelineState(m_simplePSO.GetAddressOf(), m_simpleRootSig.Get(),
		m_renderTargetFormat, L"shader\\simpleVS.cso", L"shader\\simplePS.cso",
		inputLayouts, 2)) return false;

	// 頂点バッファの生成
	if (!createResourceBuffer(m_vertexBuffer.GetAddressOf(),
		4 * sizeof(VertexUV))) return false;
	if (!uploadResourceBuffer(m_vertexBuffer.Get(),
		(void*)m_vertices, 4 * sizeof(VertexUV))) return false;
	setVertexBufferView(m_vertexBufferView, m_vertexBuffer.Get(),
		4 * sizeof(VertexUV), sizeof(VertexUV));

	// インデックスバッファの生成
	if (!createResourceBuffer(m_indexBuffer.GetAddressOf(),
		6 * sizeof(unsigned short))) return false;
	if (!uploadResourceBuffer(m_indexBuffer.Get(),
		(void*)m_indices, 6 * sizeof(unsigned short))) return false;
	setIndexBufferView(m_indexBufferView, m_indexBuffer.Get(),
		6 * sizeof(unsigned short));

	// シェーダーリソース用ディスクリプタヒープの生成
	if (!createDescHeap(m_scDescHeap.GetAddressOf(), 1)) return false;

	// 画像ファイルを読み込んでテクスチャを生成、ビューをセット
	TexMetadata meta;
	if (!readImageFile(m_textureBuffer.GetAddressOf(),
		meta, L"src\\oreka.dds", true)) return false;
	setShaderResourceView(m_textureBuffer.Get(), meta.format,
		m_scDescHeap.Get(), 0);

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

bool Renderer::readShaderObject(const wchar_t* shaderPath, ID3DBlob** shaderObj)
{
	//シェーダオブジェクトをデータの塊としてID3DBlobに読込む
	HRESULT hr = D3DReadFileToBlob(shaderPath, shaderObj);

	return SUCCEEDED(hr);
}

bool Renderer::createRootSignature(ID3D12RootSignature** rootSig)
{
	// D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBER_INPUT_LAYOUTをを設定し、入力レイアウトを有効
	// blob： ルートシグネチャの設定をデータ化したデータの塊
	// D3D12SerializeRootSignature関数で設定をシリアライズ化（データの塊を生成）
	// CreateRootSignatureメソッドでシリアライズ化したデータでルートシグネチャを生成
	D3D12_ROOT_SIGNATURE_DESC desc = {};
	desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE range = {};
	range.NumDescriptors = 1;
	range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	range.BaseShaderRegister = 0;
	range.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER param = {};
	param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	param.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
	param.DescriptorTable.pDescriptorRanges = &range;
	param.DescriptorTable.NumDescriptorRanges = 1;
	
	desc.pParameters = &param;
	desc.NumParameters = 1;

	D3D12_STATIC_SAMPLER_DESC sampler = {};
	sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
	sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
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

	hr = m_device->CreateRootSignature(
		0, blob->GetBufferPointer(), blob->GetBufferSize(),
		IID_PPV_ARGS(rootSig));

	return SUCCEEDED(hr);
}

bool Renderer::createGPipelineState(ID3D12PipelineState** pso,
	ID3D12RootSignature* rootSig, DXGI_FORMAT renderTargetFormat,
	const wchar_t* vertexShaderPath, const wchar_t* pixelShaderPath,
	D3D12_INPUT_ELEMENT_DESC* inputLayouts, UINT layoutNum)
{
	// D3D12_GRAPHICS_PIPELINE_STATE_DESC： PSOの設定構造体
	// vsBlob： 頂点シェーダのオブジェクト、psBlob： ピクセルシェーダのオブジェクト
	// pDesc.VS： 頂点シェーダのオブジェクトのポインタとサイズを設定
	// pDesc.PS： ピクセルシェーダのオブジェクトのポインタとサイズを設定

	D3D12_GRAPHICS_PIPELINE_STATE_DESC pDesc = {};

	ComPtr<ID3DBlob> vsBlob;
	ComPtr<ID3DBlob> psBlob;
	if (!readShaderObject(vertexShaderPath, vsBlob.GetAddressOf())) return false;
	if (!readShaderObject(pixelShaderPath, psBlob.GetAddressOf())) return false;
	
	pDesc.VS.pShaderBytecode = vsBlob->GetBufferPointer();
	pDesc.VS.BytecodeLength = vsBlob->GetBufferSize();
	pDesc.PS.pShaderBytecode = psBlob->GetBufferPointer();
	pDesc.PS.BytecodeLength = psBlob->GetBufferSize();

	// D3D12_RENDER_TARGET_BLEND_DESC： ブレンディング処理などの設定構造体
	// pDesc.BlendState： ブレンディング関連の設定

	pDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;

	D3D12_RENDER_TARGET_BLEND_DESC rtDesc = {};
	rtDesc.RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	rtDesc.LogicOpEnable = FALSE;
	rtDesc.BlendEnable = FALSE;

	pDesc.BlendState.AlphaToCoverageEnable = FALSE;
	pDesc.BlendState.IndependentBlendEnable = FALSE;
	pDesc.BlendState.RenderTarget[0] = rtDesc;


	// ラスタライザの設定

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

	// pDesc.DepthStencilState： 深度バッファ関連の設定
	// pDesc.InputLayout： 入力レイアウトの配列と要素数を設定
	// pDesc.IBStripCutValue： インデックスバッファに関する設定
	// pDesc.PrimitiveTopologyType： トポロジー設定（三角形ポリゴンを指定）

	pDesc.DepthStencilState.DepthEnable = FALSE;
	pDesc.DepthStencilState.StencilEnable = FALSE;

	pDesc.InputLayout.pInputElementDescs = inputLayouts;
	pDesc.InputLayout.NumElements = layoutNum;

	pDesc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
	pDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	// pDesc.NumRenderTargets： レンダーターゲットの個数
	// pDesc.RTVFormats： レンダーターゲットのフォーマット
	// pDesc.SampleDesc： サンプリングの設定
	// pDesc.pRootSignature： 使用するルートシグネチャのポインタ
	// CreateGraphicsPipelineStateメソッド： PSOを生成

	pDesc.NumRenderTargets = 1;
	pDesc.RTVFormats[0] = renderTargetFormat;

	pDesc.SampleDesc.Count = 1;
	pDesc.SampleDesc.Quality = 0;

	pDesc.pRootSignature = rootSig;
	
	HRESULT hr = m_device->CreateGraphicsPipelineState(&pDesc,
		IID_PPV_ARGS(pso));

	return SUCCEEDED(hr);
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

void Renderer::draw()
{
	// SetPipelineStateメソッド： PSOの設定コマンド
	// グラフィックスパイプライン周りの設定はほぼPSOだけで切り替えられる
	// SetGraphicsRootSignatureメソッド： ルートシグネチャの設定コマンド
	// IASetPrimitiveTopologyメソッド： ポリゴンの描画方法を設定コマンド
	// D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST： 独立した三角形を1個ずつ描画
	// IASetVertexBuffersメソッド： 描画に使う頂点バッファビューの設定コマンド
	// インデックスバッファによりポリゴンを描画するにはDrawIndexedInstancedメソッドを用いる
	
	m_cmdList->SetPipelineState(m_simplePSO.Get());
	m_cmdList->SetGraphicsRootSignature(m_simpleRootSig.Get());

	m_cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	m_cmdList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
	m_cmdList->IASetIndexBuffer(&m_indexBufferView);
	m_cmdList->SetDescriptorHeaps(1, m_scDescHeap.GetAddressOf());
	m_cmdList->SetGraphicsRootDescriptorTable(0,
		m_scDescHeap->GetGPUDescriptorHandleForHeapStart());

	m_cmdList->DrawIndexedInstanced(6, 1, 0, 0, 0);
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