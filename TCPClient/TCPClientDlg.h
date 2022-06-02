
// TCPClientDlg.h : ��� ����
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "DataSocket.h"

class CDataSocket;
struct ThreadArg {
	CStringList *pList;
	CDialogEx *pDlg;
	int ThreadRun;
};


// CTCPClientDlg ��ȭ ����
class CTCPClientDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CTCPClientDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	CDataSocket *m_pDataSocket;

// ��ȭ ���� �������Դϴ�.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TCPCLIENT_DIALOG };
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
	CEdit tx_edit_short;
	CEdit tx_edit;
	CEdit rx_edit;
	afx_msg void OnBnClickedConnect();
	afx_msg void OnBnClickedDisconnect();
	afx_msg void OnBnClickedSend();
	CIPAddressCtrl m_ipaddr;
	CWinThread *pThread1, *pThread2;
	ThreadArg arg1, arg2;
	void ProcessReceive(CDataSocket* pSocket, int nErrorCode);
	void ProcessClose(CDataSocket* pSocket, int nErrorCode);
};
