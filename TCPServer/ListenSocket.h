#pragma once

// CListenSocket ��� ����Դϴ�.

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


