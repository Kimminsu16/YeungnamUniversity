
// TCPServerDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"
class CListenSocket;
class CDataSocket;
struct ThreadArg {
	CStringList *pList;
	CDialogEx *pDlg;
	int ThreadRun;
};

// CTCPServerDlg ��ȭ ����
class CTCPServerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CTCPServerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TCPSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CListenSocket *m_pListenSocket;
	CDataSocket *m_pDataSocket;
	CWinThread *pThread1, *pThread2;
	ThreadArg arg1, arg2;

	CEdit tx_edit;
	CEdit rx_edit;
	CEdit tx_edit_short;
	afx_msg void OnBnClickedSend();
	afx_msg void OnBnClickedClose();
	void ProcessAccept(int nErrorCode);
	void ProcessReceive(CDataSocket* pSocket, int nErrorCode);
	void ProcessClose(CDataSocket* pSocket, int nErrorCode);
};
