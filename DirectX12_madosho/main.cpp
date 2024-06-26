#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

//リンクの設定
//ヘッダーに合わせたライブラリをリンクする
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;

// @remarks この関数はデバッグ用です。デバッグ時にしか動作しません
void DebugOutputFormatString(const char* format , ...)
{
#ifdef _DEBUG
	va_list valist;
	va_start(valist, format);
	printf(format, valist);
	va_end(valist);
#endif
}

LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	//ウインドウが破棄されたら呼ばれる
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OSに対して「もうこのアプリは終わる」と伝える
		return 0;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

const D3D12_COMMAND_LIST_TYPE COMMAND_LIST_TYPE = D3D12_COMMAND_LIST_TYPE_DIRECT;

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
ID3D12CommandAllocator* _cmdAllocator = nullptr;
ID3D12GraphicsCommandList* _cmdList = nullptr;
ID3D12CommandQueue* _cmdQueue = nullptr;
IDXGISwapChain4* _swapChain = nullptr;

#ifdef _DEBUG
int main()
{
#else
#include <Windows.h>
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
#endif
	DebugOutputFormatString("Show window test.");
	

	// HINSTANCE : インスタンスハンドル型のデータ型
	HINSTANCE hInst = GetModuleHandle(nullptr);
	//ウィンドウクラス生成&登録
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; // コールバック関数の指定
	w.lpszClassName = _T("DX12Sample"); //アプリケーションクラス名
	w.hInstance = GetModuleHandle(nullptr); // ハンドルの取得

	RegisterClassEx(&w); // アプリケーションクラス (ウインドウクラスの指定をOSに伝える)

	RECT wrc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }; // ウインドウサイズを決める
	//関数を使ってウインドウのサイズを補正する
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//ウインドウオブジェクトの生成
	HWND hwnd = CreateWindow(w.lpszClassName, //クラス名指定
		_T("DX12テスト"), // タイトルバーの文字
		WS_OVERLAPPEDWINDOW, // タイトルバーと境界線があるウインドウ
		CW_USEDEFAULT, // 表示x座標はOSにお任せ
		CW_USEDEFAULT, // 表示y座標はOSにお任せ
		wrc.right - wrc.left, //ウインドウ幅
		wrc.bottom - wrc.top, //ウインドウ高
		nullptr, // 親ウインドウハンドル
		nullptr, // メニューハンドル
		w.hInstance, // 呼び出しアプリケーションハンドル
		nullptr); //　追加パラメーター









	//もし失敗する場合はIDXGIFactory4*にしてみる
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory))))
	{
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory))))
		{
			return -1;
		}
	}
	//auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));

	vector<IDXGIAdapter*> adapters;

	//ここに特定の名前を持つアダプターオブジェクトが入る
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); // アダプターの説明オブジェクトを取得

		wstring strDesc = adesc.Description;

		//探したいアダプターの名前を確認
		// 今回は名前に "NVIDIA" が含まれているアダプターオブジェクトを検索
		// 自分のPCはNVIDIAが含まれるアダプターオブジェクトが存在するため問題ないが、ちゃんと考える必要がある
		if (strDesc.find(L"NVIDIA") != string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}


	//HRESULT D3D12CreateDevice(
	//	IUnknown* pAdapter, //nullptrにすることで自動的にアダプターが選択される
	//	D3D_FEATURE_LEVEL MinimumFeatureLevel, //最低限必要なフィーチャーレベル
	//	REFIID riid,
	//	void** ppDevice
	//);

	//DirectX12まわり初期化
	//フィーチャーレベル列挙
	//グラフィックスボードによってフィーチャーレベルの呼び出しに失敗する可能性があるため
	D3D_FEATURE_LEVEL levels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
	};

	HRESULT result = S_OK;

	D3D_FEATURE_LEVEL featureLevel;
	for (auto l : levels)
	{
		//生成可能なバージョンが見つかったらループを打ち切り
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = l;
			break; 
		}
	}

	// コマンドリストとコマンドアロケーターを作成していく
	result = _dev->CreateCommandAllocator(COMMAND_LIST_TYPE, IID_PPV_ARGS(&_cmdAllocator));

	result = _dev->CreateCommandList(
		0,
		COMMAND_LIST_TYPE,
		_cmdAllocator,
		nullptr,
		IID_PPV_ARGS(&_cmdList)
	);


	//コマンドキューの実態を作成
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	//タイムアウト無し
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// アダプターを1つしか使わないときは0でよい
	cmdQueueDesc.NodeMask = 0;
	//プライオリティは特に指定しない
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//コマンドリストと合わせる
	cmdQueueDesc.Type = COMMAND_LIST_TYPE;
	//キュー生成
	result = _dev->CreateCommandQueue(&cmdQueueDesc, IID_PPV_ARGS(&_cmdQueue));


#pragma region SwapChain
	DXGI_SWAP_CHAIN_DESC1 swapchainDesc = {};

	swapchainDesc.Width = WINDOW_WIDTH; // 画面解像度
	swapchainDesc.Height = WINDOW_HEIGHT; // 画面解像度
	swapchainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //ピクセルフォーマット
	swapchainDesc.Stereo = false; // ステレオ表示フラグ(3Dディスプレイのステレオモード)
	swapchainDesc.SampleDesc.Count = 1; //マルチサンプルの指定 Countは1でよい
	swapchainDesc.SampleDesc.Quality = 0; // Qualityは0でよい
	swapchainDesc.BufferUsage = DXGI_USAGE_BACK_BUFFER; // DXGI_USAGE_BACK_BUFFERでよい
	swapchainDesc.BufferCount = 2; // ダブルバッファなら2でよい

	// バックバッファは伸び縮み可能
	swapchainDesc.Scaling = DXGI_SCALING_STRETCH;

	//フリップ後は速やかに破壊
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	// 特に指定なし
	swapchainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

	//ウインドウとフルスクリーン切り替え可能
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	result = _dxgiFactory->CreateSwapChainForHwnd(
		_cmdQueue, // コマンドキューオブジェクト
		hwnd, // ウインドウハンドル
		&swapchainDesc, //スワップチェーン設定
		nullptr,
		nullptr,
		(IDXGISwapChain1**)&_swapChain // スワップチェーンオブジェクト取得用
	);
#pragma endregion
	
	

	ShowWindow(hwnd, SW_SHOW); // ウインドウ表示
	//getchar();
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//仮想キーメッセージを文字メッセージに変換
			TranslateMessage(&msg);
			//GetMessageで取得したウインドウメッセージをウインドウプロシージャに送出するAPI
			DispatchMessage(&msg);
		}

		//アプリケーションが終わるときにmessageがWM_QUITになる
		if (msg.message == WM_QUIT)
		{
			break;
		}

		//コマンドリストの実行


	}
	
	//もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}