
// TCPServerDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "TCPServer.h"
#include "TCPServerDlg.h"
#include "afxdialogex.h"
#include "ListenSocket.h"
#include "DataSocket.h"
#include "TCPServerDlg.h"

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


// CTCPServerDlg 대화 상자



CTCPServerDlg::CTCPServerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TCPSERVER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pListenSocket = NULL;
	m_pDataSocket = NULL;
}

void CTCPServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, tx_edit_short);
	DDX_Control(pDX, IDC_EDIT2, tx_edit);
	DDX_Control(pDX, IDC_EDIT3, rx_edit);
}

BEGIN_MESSAGE_MAP(CTCPServerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SEND, &CTCPServerDlg::OnBnClickedSend)
	ON_BN_CLICKED(IDC_CLOSE, &CTCPServerDlg::OnBnClickedClose)
END_MESSAGE_MAP()


// CTCPServerDlg 메시지 처리기

UINT RXThread(LPVOID arg)
{
	ThreadArg *pArg = (ThreadArg *)arg; // 스레드 구조체 포인터
	CStringList *pList = pArg->pList; // CStringList 포인터 변수
	CTCPServerDlg *pDlg = (CTCPServerDlg*) pArg->pDlg; // 객체 위치

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
	CTCPServerDlg *pDlg = (CTCPServerDlg*)pArg->pDlg; // 객체 위치

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

BOOL CTCPServerDlg::OnInitDialog()
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
	m_pListenSocket = NULL;

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

	ASSERT(m_pListenSocket == NULL);
	m_pListenSocket = new CListenSocket(this);
	if (m_pListenSocket->Create(8000))
	{
		if (m_pListenSocket->Listen())
		{
			AfxMessageBox(_T("서버를 시작합니다."), MB_ICONINFORMATION);
			pThread1 = AfxBeginThread(TXThread, (LPVOID)&arg1);
			pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2);
			return TRUE;
		}

		else
		{
			int err = m_pListenSocket->GetLastError();
			TCHAR buf[256];
			FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf, 256, NULL);
			AfxMessageBox(buf, MB_ICONERROR);
		}
		AfxMessageBox(_T("이미 실행 중인 서버가 있습니다.")
			_T("\n프로그램을 종료합니다."), MB_ICONERROR);

		return FALSE;
	}
}

void CTCPServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTCPServerDlg::OnPaint()
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
HCURSOR CTCPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTCPServerDlg::OnBnClickedSend()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString message;
	tx_edit_short.GetWindowTextW(message);
	message += _T("\r\n");

	tx_cs.Lock();
	arg1.pList->AddTail(message);
	tx_cs.Unlock();

	tx_edit_short.SetWindowTextW(_T(""));
	tx_edit_short.SetFocus();

	int len = tx_edit.GetWindowTextLengthW();
	tx_edit.SetSel(len, len);
	tx_edit.ReplaceSel(message);
}


void CTCPServerDlg::OnBnClickedClose()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pDataSocket == NULL)
	{
		AfxMessageBox(_T("이미 접속이 종료되었습니다."));
	}
	else {
		arg1.ThreadRun = 0;
		arg2.ThreadRun = 0;

		m_pDataSocket->Close();
		delete m_pDataSocket;
		m_pDataSocket = NULL;

		m_pListenSocket->Close();
		delete m_pListenSocket;
		m_pListenSocket = NULL;
	}
}

void CTCPServerDlg::ProcessAccept(int nErrorCode)
{
	CString PeerAddr;
	UINT PeerPort;
	CString str;

	ASSERT(nErrorCode == 0);
	if (m_pDataSocket == NULL)
	{
		m_pDataSocket = new CDataSocket(this);
		if (m_pListenSocket->Accept(*m_pDataSocket))
		{
			m_pDataSocket->GetPeerName(PeerAddr, PeerPort);
			str.Format(_T("### IP주소 : %s, 포트번호 : %d ###\r\n"), 
				PeerAddr, PeerPort);
			rx_edit.SetWindowTextW(str);
		}
		else
		{
			delete m_pDataSocket;
			m_pDataSocket = NULL;
		}
	}
}


void CTCPServerDlg::ProcessReceive(CDataSocket* pSocket, int nErrorCode)
{
	TCHAR buf[1024 + 1];
	CString strData;
	int nbytes;

	nbytes = pSocket->Receive(buf, 1024);
	buf[nbytes] = NULL;
	strData = (LPCTSTR)buf;

	rx_cs.Lock();
	arg2.pList->AddTail((LPCTSTR)strData);
	rx_cs.Unlock();
}


void CTCPServerDlg::ProcessClose(CDataSocket* pSocket, int nErrorCode)
{
	pSocket->Close();
	delete m_pDataSocket;
	m_pDataSocket = NULL;

	int len = rx_edit.GetWindowTextLengthW();
	CString message = _T("### 접속 종료 ###\r\n\n");
	rx_edit.SetSel(len, len);
	rx_edit.ReplaceSel(message);
}
