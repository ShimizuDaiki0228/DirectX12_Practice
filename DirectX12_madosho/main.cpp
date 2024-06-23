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

ID3D12Device* _dev = nullptr;
IDXGIFactory6* _dxgiFactory = nullptr;
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
	auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));

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
	}
	
	//もうクラスは使わないので登録解除する
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}