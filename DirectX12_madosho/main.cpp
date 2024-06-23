#include <Windows.h>
#include <tchar.h>
#ifdef _DEBUG
#include <iostream>
#endif

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
	}
	
	//�����N���X�͎g��Ȃ��̂œo�^��������
	UnregisterClass(w.lpszClassName, w.hInstance);
	return 0;
}