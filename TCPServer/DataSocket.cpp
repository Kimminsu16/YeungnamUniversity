// DataSocket.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "TCPServer.h"
#include "DataSocket.h"


// CDataSocket

CDataSocket::CDataSocket(CTCPServerDlg *pDlg)
{
	m_pDlg = pDlg;
}

CDataSocket::~CDataSocket()
{
}


// CDataSocket ��� �Լ�


void CDataSocket::OnReceive(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	m_pDlg->ProcessReceive(this, nErrorCode);
	CSocket::OnReceive(nErrorCode);
}


void CDataSocket::OnClose(int nErrorCode)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	m_pDlg->ProcessClose(this, nErrorCode);
	CSocket::OnClose(nErrorCode);
}
