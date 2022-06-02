
// TCPClientDlg.cpp : ���� ����
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


// CTCPClientDlg ��ȭ ����



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


// CTCPClientDlg �޽��� ó����

UINT RXThread(LPVOID arg)
{
	ThreadArg *pArg = (ThreadArg *)arg; // ������ ����ü ������
	CStringList *pList = pArg->pList; // CStringList ������ ����
	CTCPClientDlg *pDlg = (CTCPClientDlg*)pArg->pDlg; // ��ü ��ġ

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
	CTCPClientDlg *pDlg = (CTCPClientDlg*)pArg->pDlg; // ��ü ��ġ

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
	m_ipaddr.SetWindowTextW(_T("127.0.0.1")); // IP ����

	// thread argument
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

	pThread1 = AfxBeginThread(TXThread, (LPVOID)&arg1); // �۽� ������
	pThread2 = AfxBeginThread(RXThread, (LPVOID)&arg2); // ���� ������
	return TRUE;  // ��Ŀ���� ��Ʈ�ѿ� �������� ������ TRUE�� ��ȯ�մϴ�.
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

// ��ȭ ���ڿ� �ּ�ȭ ���߸� �߰��� ��� �������� �׸�����
//  �Ʒ� �ڵ尡 �ʿ��մϴ�.  ����/�� ���� ����ϴ� MFC ���� ���α׷��� ��쿡��
//  �����ӿ�ũ���� �� �۾��� �ڵ����� �����մϴ�.

void CTCPClientDlg::OnPaint()
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
HCURSOR CTCPClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CTCPClientDlg::OnBnClickedConnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pDataSocket == NULL) // ������ ���������
	{
		m_pDataSocket = new CDataSocket(this);
		m_pDataSocket->Create();
		CString addr;
		m_ipaddr.GetWindowTextW(addr);

		if (m_pDataSocket->Connect(addr, 8000)) // ��Ʈ 8000�� ������ �Ǿ��ٸ�
		{
			MessageBox(_T("������ ���� ����!"), _T("�˸�"), MB_ICONINFORMATION);
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

			tx_edit.SetWindowTextW(_T("### ������ ���� ����! ###\r\n"));
			tx_edit_short.SetFocus();
		}
		else {
			MessageBox(_T("������ ���� ����!"), _T("�˸�"), MB_ICONERROR);
			delete m_pDataSocket;
			m_pDataSocket = NULL;
		}
	}
	else {
		(_T("������ �̹� ���ӵ�!"), _T("�˸�"), MB_ICONINFORMATION);
		tx_edit.SetFocus();
	}
}


void CTCPClientDlg::OnBnClickedDisconnect()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pDataSocket == NULL)
		MessageBox(_T("������ ���� �� ��!"), _T("�˸�"), MB_ICONERROR);

	else {
		m_pDataSocket->Close();
		delete m_pDataSocket;
		m_pDataSocket = NULL;
	}
}


void CTCPClientDlg::OnBnClickedSend()
{
	// TODO: ���⿡ ��Ʈ�� �˸� ó���� �ڵ带 �߰��մϴ�.
	if (m_pDataSocket == NULL)
		MessageBox(_T("������ ���� �� ��!"), _T("�˸�"), MB_ICONERROR);

	else
	{
		CString message;
		tx_edit_short.GetWindowTextW(message);
		message += _T("\r\n");

		tx_cs.Lock(); // �Ӱ豸�� ����
		arg1.pList->AddTail(message); // pList�� �޼��� ����
		tx_cs.Unlock(); // �Ӱ豸�� ����

		tx_edit_short.SetWindowTextW(_T("")); // �޼��� �ʱ�ȭ
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

	rx_cs.Lock(); // �Ӱ豸�� ����
	arg2.pList->AddTail((LPCTSTR)strData); // arg2 pList�� �޼��� ����
	rx_cs.Unlock(); // �Ӱ豸�� ����
}


void CTCPClientDlg::ProcessClose(CDataSocket* pSocket, int nErrorCode)
{
	pSocket->Close();
	delete m_pDataSocket;
	m_pDataSocket = NULL;

	int len = rx_edit.GetWindowTextLengthW();
	CString message = _T("���� ����.\n");
	rx_edit.SetSel(len, len);
	rx_edit.ReplaceSel(message);
}
