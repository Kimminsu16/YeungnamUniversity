
// TCPClientDlg.h : 헤더 파일
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


// CTCPClientDlg 대화 상자
class CTCPClientDlg : public CDialogEx
{
// 생성입니다.
public:
	CTCPClientDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	CDataSocket *m_pDataSocket;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TCPCLIENT_DIALOG };
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
