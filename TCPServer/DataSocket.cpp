// DataSocket.cpp : 구현 파일입니다.
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


// CDataSocket 멤버 함수


void CDataSocket::OnReceive(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pDlg->ProcessReceive(this, nErrorCode);
	CSocket::OnReceive(nErrorCode);
}


void CDataSocket::OnClose(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pDlg->ProcessClose(this, nErrorCode);
	CSocket::OnClose(nErrorCode);
}
