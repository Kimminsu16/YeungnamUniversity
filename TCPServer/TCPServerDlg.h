
// TCPServerDlg.h : 헤더 파일
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

// CTCPServerDlg 대화 상자
class CTCPServerDlg : public CDialogEx
{
// 생성입니다.
public:
	CTCPServerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TCPSERVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
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
