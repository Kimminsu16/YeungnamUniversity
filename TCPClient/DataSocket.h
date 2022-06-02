#pragma once

// CDataSocket ��� ����Դϴ�.

#include "TCPClientDlg.h"

class CTCPClientDlg;

class CDataSocket : public CSocket
{
public:
	CDataSocket(CTCPClientDlg* pDlg);
	virtual ~CDataSocket();
	CTCPClientDlg *m_pDlg;
	virtual void OnClose(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
};


