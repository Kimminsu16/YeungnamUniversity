#pragma once

// CDataSocket 명령 대상입니다.

#include "TCPServerDlg.h"

class CTCPServerDlg;

class CDataSocket : public CSocket
{
public:
	CDataSocket(CTCPServerDlg *pDlg);
	virtual ~CDataSocket();
	CTCPServerDlg *m_pDlg;
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};


