// ListenSocket.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "TCPServer.h"
#include "ListenSocket.h"


// CListenSocket

CListenSocket::CListenSocket(CTCPServerDlg *pDlg)
{
	m_pDlg = pDlg;
}

CListenSocket::~CListenSocket()
{
}


// CListenSocket ��� �Լ�


void CListenSocket::OnAccept(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	m_pDlg->ProcessAccept(nErrorCode);
	CSocket::OnAccept(nErrorCode);
}
