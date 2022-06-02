
// TCPServerDlg.cpp : ���� ����
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

// ���� ���α׷� ������ ���Ǵ� CAboutDlg ��ȭ �����Դϴ�.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

// �����Դϴ�.
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


// CTCPServerDlg ��ȭ ����



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


// CTCPServerDlg �޽��� ó����

UINT RXThread(LPVOID arg)
{
	ThreadArg *pArg = (ThreadArg *)arg; // ������ ����ü ������
	CStringList *pList = pArg->pList; // CStringList ������ ����
	CTCPServerDlg *pDlg = (CTCPServerDlg*) pArg->pDlg; // ��ü ��ġ

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
	ThreadArg *pArg = (ThreadArg *)arg; // ������ ����ü ������
	CStringList *pList = pArg->pList; // CStringList ������ ����
	CTCPServerDlg *pDlg = (CTCPServerDlg*)pArg->pDlg; // ��ü ��ġ

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

	// �ý��� �޴��� "����..." �޴� �׸��� �߰��մϴ�.

	// IDM_ABOUTBOX�� �ý��� ��� ������ �־�� �մϴ�.
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

	// �� ��ȭ ������ �������� �����մϴ�.  ���� ���α׷��� �� â�� ��ȭ ���ڰ� �ƴ� ��쿡��
	//  �����ӿ�ũ�� �� �۾��� �ڵ����� �����մϴ�.
	SetIcon(m_hIcon, TRUE);			// ū �������� �����մϴ�.
	SetIcon(m_hIcon, FALSE);		// ���� �������� �����մϴ�.

	// TODO: ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
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

	// ���� �ʱ�ȭ
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
			AfxMessageBox(_T("������ �����մϴ�."), MB_ICONINFORMATION);
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
		AfxMessageBox(_T("�̹� ���� ���� ������ �ֽ��ϴ�.")
			_T("\n���α׷��� �����մϴ�."), MB_ICONERROR);

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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CTCPServerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // �׸��⸦ ���� ����̽� ���ؽ�Ʈ�Դϴ�.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Ŭ���̾�Ʈ �簢������ �������� ����� ����ϴ�.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// �������� �׸��ϴ�.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// ����ڰ� �ּ�ȭ�� â�� ���� ���ȿ� Ŀ���� ǥ�õǵ��� �ý��ۿ���
//  �� �Լ��� ȣ���մϴ�.
HCURSOR CTCPServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTCPServerDlg::OnBnClickedSend()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
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
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pDataSocket == NULL)
	{
		AfxMessageBox(_T("�̹� ������ ����Ǿ����ϴ�."));
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
			str.Format(_T("### IP�ּ� : %s, ��Ʈ��ȣ : %d ###\r\n"), 
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
	CString message = _T("### ���� ���� ###\r\n\n");
	rx_edit.SetSel(len, len);
	rx_edit.ReplaceSel(message);
}
