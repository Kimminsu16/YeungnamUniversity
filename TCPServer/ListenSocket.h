#pragma once

// CListenSocket 명령 대상입니다.

#include "TCPServerDlg.h"

class CTCPServerDlg;

class CListenSocket : public CSocket
{
public:
	CListenSocket(CTCPServerDlg *pDlg);
	virtual ~CListenSocket();
	CTCPServerDlg *m_pDlg;
	virtual void OnAccept(int nErrorCode);
};


