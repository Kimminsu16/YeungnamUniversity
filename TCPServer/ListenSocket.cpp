// ListenSocket.cpp : 구현 파일입니다.
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


// CListenSocket 멤버 함수


void CListenSocket::OnAccept(int nErrorCode)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	m_pDlg->ProcessAccept(nErrorCode);
	CSocket::OnAccept(nErrorCode);
}
