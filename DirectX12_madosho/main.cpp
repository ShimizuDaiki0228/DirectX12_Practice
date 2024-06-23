#include <Windows.h>
#include <tchar.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <vector>
#ifdef _DEBUG
#include <iostream>
#endif

//�����N�̐ݒ�
//�w�b�_�[�ɍ��킹�����C�u�����������N����
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

using namespace std;

// @remarks ���̊֐��̓f�o�b�O�p�ł��B�f�o�b�O���ɂ������삵�܂���
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
	//�E�C���h�E���j�����ꂽ��Ă΂��
	if (msg == WM_DESTROY)
	{
		PostQuitMessage(0); //OS�ɑ΂��āu�������̃A�v���͏I���v�Ɠ`����
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
	

	// HINSTANCE : �C���X�^���X�n���h���^�̃f�[�^�^
	HINSTANCE hInst = GetModuleHandle(nullptr);
	//�E�B���h�E�N���X����&�o�^
	WNDCLASSEX w = {};
	w.cbSize = sizeof(WNDCLASSEX);
	w.lpfnWndProc = (WNDPROC)WindowProcedure; // �R�[���o�b�N�֐��̎w��
	w.lpszClassName = _T("DX12Sample"); //�A�v���P�[�V�����N���X��
	w.hInstance = GetModuleHandle(nullptr); // �n���h���̎擾

	RegisterClassEx(&w); // �A�v���P�[�V�����N���X (�E�C���h�E�N���X�̎w���OS�ɓ`����)

	RECT wrc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT }; // �E�C���h�E�T�C�Y�����߂�
	//�֐����g���ăE�C���h�E�̃T�C�Y��␳����
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	//�E�C���h�E�I�u�W�F�N�g�̐���
	HWND hwnd = CreateWindow(w.lpszClassName, //�N���X���w��
		_T("DX12�e�X�g"), // �^�C�g���o�[�̕���
		WS_OVERLAPPEDWINDOW, // �^�C�g���o�[�Ƌ��E��������E�C���h�E
		CW_USEDEFAULT, // �\��x���W��OS�ɂ��C��
		CW_USEDEFAULT, // �\��y���W��OS�ɂ��C��
		wrc.right - wrc.left, //�E�C���h�E��
		wrc.bottom - wrc.top, //�E�C���h�E��
		nullptr, // �e�E�C���h�E�n���h��
		nullptr, // ���j���[�n���h��
		w.hInstance, // �Ăяo���A�v���P�[�V�����n���h��
		nullptr); //�@�ǉ��p�����[�^�[









	//�������s����ꍇ��IDXGIFactory4*�ɂ��Ă݂�
	if (FAILED(CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&_dxgiFactory))))
	{
		if (FAILED(CreateDXGIFactory2(0, IID_PPV_ARGS(&_dxgiFactory))))
		{
			return -1;
		}
	}
	//auto result = CreateDXGIFactory1(IID_PPV_ARGS(&_dxgiFactory));

	vector<IDXGIAdapter*> adapters;

	//�����ɓ���̖��O�����A�_�v�^�[�I�u�W�F�N�g������
	IDXGIAdapter* tmpAdapter = nullptr;

	for (int i = 0; _dxgiFactory->EnumAdapters(i, &tmpAdapter) != DXGI_ERROR_NOT_FOUND; i++)
	{
		adapters.push_back(tmpAdapter);
	}
	for (auto adpt : adapters)
	{
		DXGI_ADAPTER_DESC adesc = {};
		adpt->GetDesc(&adesc); // �A�_�v�^�[�̐����I�u�W�F�N�g���擾

		wstring strDesc = adesc.Description;

		//�T�������A�_�v�^�[�̖��O���m�F
		// ����͖��O�� "NVIDIA" ���܂܂�Ă���A�_�v�^�[�I�u�W�F�N�g������
		// ������PC��NVIDIA���܂܂��A�_�v�^�[�I�u�W�F�N�g�����݂��邽�ߖ��Ȃ����A�����ƍl����K�v������
		if (strDesc.find(L"NVIDIA") != string::npos)
		{
			tmpAdapter = adpt;
			break;
		}
	}


	//HRESULT D3D12CreateDevice(
	//	IUnknown* pAdapter, //nullptr�ɂ��邱�ƂŎ����I�ɃA�_�v�^�[���I�������
	//	D3D_FEATURE_LEVEL MinimumFeatureLevel, //�Œ���K�v�ȃt�B�[�`���[���x��
	//	REFIID riid,
	//	void** ppDevice
	//);

	//DirectX12�܂�菉����
	//�t�B�[�`���[���x����
	//�O���t�B�b�N�X�{�[�h�ɂ���ăt�B�[�`���[���x���̌Ăяo���Ɏ��s����\�������邽��
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
		//�����\�ȃo�[�W���������������烋�[�v��ł��؂�
		if (D3D12CreateDevice(tmpAdapter, l, IID_PPV_ARGS(&_dev)) == S_OK)
		{
			featureLevel = l;
			break; 
		}
	}

	// �R�}���h���X�g�ƃR�}���h�A���P�[�^�[���쐬���Ă���
	result = _dev->CreateCommandAllocator(COMMAND_LIST_TYPE, IID_PPV_ARGS(&_cmdAllocator));

	result = _dev->CreateCommandList(
		0,
		COMMAND_LIST_TYPE,
		_cmdAllocator,
		nullptr,
		IID_PPV_ARGS(&_cmdList)
	);

	//�R�}���h�L���[�̎��Ԃ��쐬
	D3D12_COMMAND_QUEUE_DESC cmdQueueDesc = {};
	//�^�C���A�E�g����
	cmdQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	// �A�_�v�^�[��1�����g��Ȃ��Ƃ���0�ł悢
	cmdQueueDesc.NodeMask = 0;
	//�v���C�I���e�B�͓��Ɏw�肵�Ȃ�
	cmdQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	//�R�}���h���X�g�ƍ��킹��
	cmdQueueDesc.Type = COMMAND_LIST_TYPE;






	ShowWindow(hwnd, SW_SHOW); // �E�C���h�E�\��
	//getchar();
	MSG msg = {};
	while (true)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			//���z�L�[���b�Z�[�W�𕶎����b�Z�[�W�ɕϊ�
			TranslateMessage(&msg);
			//GetMessage�Ŏ擾�����E�C���h�E���b�Z�[�W���E�C���h�E�v���V�[�W���ɑ��o����API
			DispatchMessage(&msg);
		}

		//�A�v���P�[�V�������I���Ƃ���message��WM_QUIT�ɂȂ�
		if (msg.message == WM_QUIT)
		{
			break;
		}

		//�R�}���h���X�g�̎��s

	}
	
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}