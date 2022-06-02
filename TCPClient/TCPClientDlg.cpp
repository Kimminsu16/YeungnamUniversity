
// TCPClientDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "TCPClient.h"
#include "TCPClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Critical Section
CCriticalSection tx_cs;
CCriticalSection rx_cs;

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CTCPClientDlg 대화 상자



CTCPClientDlg::CTCPClientDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TCPCLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTCPClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, tx_edit_short);
	DDX_Control(pDX, IDC_EDIT2, tx_edit);
	DDX_Control(pDX, IDC_EDIT3, rx_edit);
	DDX_Control(pDX, IDC_IPADDRESS1, m_ipaddr);
}

BEGIN_MESSAGE_MAP(CTCPClientDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_CONNECT, &CTCPClientDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_DISCONNECT, &CTCPClientDlg::OnBnClickedDisconnect)
	ON_BN_CLICKED(IDC_SEND, &CTCPClientDlg::OnBnClickedSend)
END_MESSAGE_MAP()


// CTCPClientDlg 메시지 처리기

UINT RXThread(LPVOID arg)
{
	ThreadArg *pArg = (ThreadArg *)arg; // 스레드 구조체 포인터
	CStringList *pList = pArg->pList; // CStringList 포인터 변수
	CTCPClientDlg *pDlg = (CTCPClientDlg*)pArg->pDlg; // 객체 위치

	while (pArg->ThreadRun)
	{
		POSITION pos = pList->GetHeadPosition();
		POSITION current_pos;

		while (pos != NULL)
		{
			current_pos = pos;

			rx_cs.Lock();
			CString str = pList->GetNext(pos);
			rx_cs.Unlock();

			CString message;
			pDlg->rx_edit.GetWindowTextW(message);
			message += str;
			pDlg->rx_edit.SetWindowTextW(message);
			pDlg->rx_edit.LineScroll(pDlg->rx_edit.GetLineCount());

			pList->RemoveAt(current_pos);
		}
		Sleep(10);
	}
	return 0;
}

UINT TXThread(LPVOID arg)
{
	ThreadArg *pArg = (ThreadArg *)arg; // 스레드 구조체 포인터
	CStringList *pList = pArg->pList; // CStringList 포인터 변수
	CTCPClientDlg *pDlg = (CTCPClientDlg*)pArg->pDlg; // 객체 위치

	while (pArg->ThreadRun)
	{
		POSITION pos = pList->GetHeadPosition();
		POSITION current_pos;

		while (pos != NULL)
		{
			current_pos = pos;

			tx_cs.Lock();
			CString str = pList->GetNext(pos);
			tx_cs.Unlock();

			pDlg->m_pDataSocket->Send((LPCTSTR)str, (str.GetLength() + 1) * sizeof(TCHAR));

			pList->RemoveAt(current_pos);
		}
		Sleep(10);
	}
	return 0;
}


BOOL CTCPClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	m_pDataSocket = NULL;
	m_ipaddr.SetWindowTextW(_T("127.0.0.1")); // IP 설정

	// thread argument
	CStringList *newList1 = new CStringList;
	arg1.pList = newList1;
	arg1.ThreadRun = 1;
	arg1.pDlg = this;

	CStringList *newList2 = new CStringList;
	arg2.pList = newList2;
	arg2.ThreadRun = 1;
	arg2.pDlg = this;

	// 소켓 초기화
	WSADATA wsa;
	int error_code;
	if ((error_code = WSAStartup(MAKEWORD(2, 2), &wsa)) != 0) {
		TCHAR buf[256];
		FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, error_code,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
		AfxMessageBox(buf, MB_ICONERROR);
	}

	pThread1 = AfxBeginThread(TXThread, (LPVOID)&arg1); // 송신 스레드
	pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2); // 수신 스레드
	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CTCPClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CTCPClientDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTCPClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTCPClientDlg::OnBnClickedConnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pDataSocket == NULL) // 소켓이 비어있으면
	{
		m_pDataSocket = new CDataSocket(this);
		m_pDataSocket->Create();
		CString addr;
		m_ipaddr.GetWindowTextW(addr);

		if (m_pDataSocket->Connect(addr, 8000)) // 포트 8000에 연결이 되었다면
		{
			MessageBox(_T("서버에 접속 성공!"), _T("알림"), MB_ICONINFORMATION);
			// thread argument
			CStringList *newList1 = new CStringList;
			arg1.pList = newList1;
			arg1.ThreadRun = 1;
			arg1.pDlg = this;

			CStringList *newList2 = new CStringList;
			arg2.pList = newList2;
			arg2.ThreadRun = 1;
			arg2.pDlg = this;

			pThread1 = AfxBeginThread(TXThread, (LPVOID)&arg1); // tx Thread
			pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2); // rx Thread

			tx_edit.SetWindowTextW(_T("### 서버에 접속 성공! ###\r\n"));
			tx_edit_short.SetFocus();
		}
		else {
			MessageBox(_T("서버에 접속 실패!"), _T("알림"), MB_ICONERROR);
			delete m_pDataSocket;
			m_pDataSocket = NULL;
		}
	}
	else {
		(_T("서버에 이미 접속됨!"), _T("알림"), MB_ICONINFORMATION);
		tx_edit.SetFocus();
	}
}


void CTCPClientDlg::OnBnClickedDisconnect()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pDataSocket == NULL)
		MessageBox(_T("서버에 접속 안 함!"), _T("알림"), MB_ICONERROR);

	else {
		m_pDataSocket->Close();
		delete m_pDataSocket;
		m_pDataSocket = NULL;
	}
}


void CTCPClientDlg::OnBnClickedSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pDataSocket == NULL)
		MessageBox(_T("서버에 접속 안 함!"), _T("알림"), MB_ICONERROR);

	else
	{
		CString message;
		tx_edit_short.GetWindowTextW(message);
		message += _T("\r\n");

		tx_cs.Lock(); // 임계구역 실행
		arg1.pList->AddTail(message); // pList에 메세지 저장
		tx_cs.Unlock(); // 임계구역 종료

		tx_edit_short.SetWindowTextW(_T("")); // 메세지 초기화
		tx_edit_short.SetFocus();

		int len = tx_edit.GetWindowTextLengthW();
		tx_edit.SetSel(len, len);
		tx_edit.ReplaceSel(message);
	}
}


void CTCPClientDlg::ProcessReceive(CDataSocket* pSocket, int nErrorCode)
{
	TCHAR buf[1024 + 1];
	CString strData;
	int nbytes;
	
	nbytes = pSocket->Receive(buf, 1024);
	buf[nbytes] = NULL;
	strData = (LPCTSTR)buf;

	rx_cs.Lock(); // 임계구역 실행
	arg2.pList->AddTail((LPCTSTR)strData); // arg2 pList에 메세지 저장
	rx_cs.Unlock(); // 임계구역 종료
}


void CTCPClientDlg::ProcessClose(CDataSocket* pSocket, int nErrorCode)
{
	pSocket->Close();
	delete m_pDataSocket;
	m_pDataSocket = NULL;

	int len = rx_edit.GetWindowTextLengthW();
	CString message = _T("접속 종료.\n");
	rx_edit.SetSel(len, len);
	rx_edit.ReplaceSel(message);
}
