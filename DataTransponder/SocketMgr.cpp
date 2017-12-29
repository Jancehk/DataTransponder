#include "stdafx.h"
#include <conio.h>
#include <WS2tcpip.h>
#include <Winsock2.h>
#include "SocketMgr.h"
#pragma comment(lib,"ws2_32.lib")


SocketMgr::SocketMgr(void)
{
	nSkt_fd = 0;
	n_port = 0;
	m_strIP.Empty();
	SetSktFlg(SKT_NUL_FLG);
	pstrRecvDataBuffer = NULL;
	pCPrevSocketMgr = NULL;
	pCNextSocketMgr = NULL;
	pCDstSocketMgr = NULL;
	pstrRecvData = NULL;
	nRecvSize = 0;
	nTotalSize = 0;
	hThread = 0;
	memset(strIPInfo, 0, sizeof(strIPInfo));
	memset(strCliIPInfo, 0, sizeof(strCliIPInfo));
}

SocketMgr::SocketMgr(SocketMgr* pCSocketMgr)
{
	nSkt_fd = 0;
	SetSktFlg(SKT_NUL_FLG);
	pstrRecvDataBuffer = NULL;
	pCPrevSocketMgr = NULL;
	pCNextSocketMgr = NULL;
	hThread = 0;
	if (NULL != pCSocketMgr->pCNextSocketMgr)
	{
		pCNextSocketMgr = pCSocketMgr->pCNextSocketMgr;
		pCNextSocketMgr->pCPrevSocketMgr = this;
	}
	pCSocketMgr->pCNextSocketMgr = this;
	pCPrevSocketMgr = pCSocketMgr;
}

SocketMgr::~SocketMgr(void)
{
	if(0 != nSkt_fd)
	{
		TRACE("client disconnection");
		closesocket(nSkt_fd);
		nSkt_fd =0;
	}
	/*if (0 != hThread)
	{
		TerminateThread(hThread, -1);
		CloseHandle(hThread);
	}*/
	if(NULL != pstrRecvDataBuffer)
	{
		free(pstrRecvDataBuffer);
		pstrRecvDataBuffer =NULL;
	}
	if(NULL != pCPrevSocketMgr)
	{
		pCPrevSocketMgr->pCNextSocketMgr = pCNextSocketMgr;
	}
	if(NULL != pCNextSocketMgr)
	{
		pCNextSocketMgr->pCPrevSocketMgr = pCPrevSocketMgr;
	}
	if (NULL != pCDstSocketMgr)
	{
		pCDstSocketMgr->pCDstSocketMgr = NULL;
		delete pCDstSocketMgr;
		pCDstSocketMgr = NULL;
	}
	if(NULL != pstrRecvData)
	{
		free(pstrRecvData);
		pstrRecvData = NULL;
	}
}


int SocketMgr::CreateSkt(void)
{
	int nRtn = -1;
	WORD sockVersion = MAKEWORD(2,2);  
    WSADATA wsaData; 
	static int binit_flg  = 0;
	if(0 == binit_flg) 
	{
		if(WSAStartup(sockVersion, &wsaData)!=0)  
		{
			TRACE("WSAStartup error !\r\n");  
			return nRtn;  
		}
		binit_flg = 1;
	}
    nSkt_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if(nSkt_fd == INVALID_SOCKET)  
    {  
		TRACE("socket error !\r\n");
        return nRtn;  
    }
	return 0;
}

int SocketMgr::CreateSkt(char * pstrIP,int nPort)
{
	int nRtn = -1;
	int timeout = 3000; //3s
	sockaddr_in serAddr;  
	nRtn = CreateSkt();
	if(0 != nRtn)
	{
		TRACE("create socket err!\r\n");
		return nRtn;
	}
    serAddr.sin_family = AF_INET;  
    serAddr.sin_port = htons(nPort);
	if (0 >= inet_pton(AF_INET, pstrIP, &serAddr.sin_addr.S_un.S_addr))
	{
		TRACE("Get IP err!\r\n");
		return nRtn;	
	}
	 //inet_addr(pstrIP);
	//inet_pton();
    if (connect(nSkt_fd, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)  
    {  
		TRACE("connect error !");
        closesocket(nSkt_fd);
		nSkt_fd = 0;
        return -1;  
	}
	setsockopt(nSkt_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));

	setsockopt(nSkt_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
	SetSktFlg(SKT_DST_FLG);
	return 0;
}

int SocketMgr::CreateSkt(CString strIP, int nPort)
{
	char strIPInfo[32] = { 0 };
	size_t   i;
	wcstombs_s(&i,strIPInfo, 32, strIP.GetBuffer(strIP.GetLength()), strIP.GetLength());
	return CreateSkt(strIPInfo, nPort);
}
int SocketMgr::CreateSkt(int nSvrPort, CString strIP)
{
	int nRtn = -1;
	sockaddr_in sin;
	nRtn = CreateSkt();
	if(0 != nRtn)
	{
		TRACE("create socket err!\r\n");
		return nRtn;
	}
    sin.sin_family = AF_INET;  
    sin.sin_port = htons(nSvrPort);
    sin.sin_addr.S_un.S_addr = INADDR_ANY;
	nRtn = bind(nSkt_fd, (LPSOCKADDR)&sin, sizeof(sin));
	if (nRtn == SOCKET_ERROR)
    {  
		TRACE("bind error !\r\n");
        return nRtn;  
    }
	nRtn = listen(nSkt_fd, 50);
	if (nRtn == SOCKET_ERROR)
    {
		TRACE("listen error !");
        return nRtn;  
	}
	SetSktFlg(SKT_SVR_FLG);
	n_port = nSvrPort;
	m_strIP = strIP;
	return 0;
}

int SocketMgr::DeleteSkt(CString strIP, int nPort,int nFlg)
{
	nSkt_fd = 0;
	SetSktFlg(SKT_NUL_FLG);
	pstrRecvDataBuffer = NULL;
	pCPrevSocketMgr = NULL;
	pCNextSocketMgr = NULL;
	hThread = 0;
	if( strIP == m_strIP &&
		nPort == n_port )
	{
		delete this;
		return 0;
	}
	if (NULL != pCNextSocketMgr)
	{
		pCNextSocketMgr->DeleteSkt(strIP,nPort);
	}
	return 0;
}
int SocketMgr::SetSktFlg(int nFlg)
{
	if(nFlg == SKT_NUL_FLG)
	{
		nSvrFlg = nFlg;
	}
	else
	{
		nSvrFlg |= nFlg;
	}
	return 0;
}
int SocketMgr::SetSktFlg(CString strIP, int nPort,int nFlg)
{
	if( strIP == m_strIP &&
		nPort == n_port )
	{
		return SetSktFlg(nFlg);
	}
	if (NULL != pCNextSocketMgr)
	{
		pCNextSocketMgr->SetSktFlg(strIP,nPort,nFlg);
	}
	return 0;
}
SocketMgr * SocketMgr::GetNewSkt()
{
	if (0 == nSkt_fd)
	{
		return this;
	}
	if (NULL != pCNextSocketMgr)
	{
		return pCNextSocketMgr->GetNewSkt();
	}
	return new SocketMgr(this);
}
SocketMgr * SocketMgr::GetNewDst()
{
	SocketMgr *pCNewDst = GetNewSkt();
	if (NULL == pCNewDst)
	{
		return NULL;
	}
	pCNewDst->pCDstSocketMgr = this;
	pCDstSocketMgr = pCNewDst;
	return pCNewDst;
}
int SocketMgr::CreateSkt(SocketMgr* pCSocketSvr)
{
	int timeout = 3000; //3s
	size_t converted = 0;
    sockaddr_in		remoteAddr;  
    int nAddrlen = sizeof(remoteAddr); 
	nSkt_fd = accept(pCSocketSvr->nSkt_fd, (SOCKADDR *)&remoteAddr, &nAddrlen);  
    if(nSkt_fd == INVALID_SOCKET)  
    {
		TRACE("accept error !");
		return -1;
	}
	setsockopt(nSkt_fd, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));

	setsockopt(nSkt_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));
	n_port = ntohs(remoteAddr.sin_port);
	inet_ntop(AF_INET, &remoteAddr.sin_addr.s_addr, strCliIPInfo, 16);
	mbstowcs_s(&converted, m_strIP.GetBuffer(16), 16, strCliIPInfo, _TRUNCATE);
	SetSktFlg(SKT_SRC_FLG);
	return 0;
}

SocketMgr* SocketMgr::GetSktFD(fd_set *pread_fds)
{
	if (FD_ISSET(nSkt_fd, pread_fds))
	{
		return this;
	}
	if(NULL != pCNextSocketMgr)
	{
		return pCNextSocketMgr->GetSktFD(pread_fds);
	}
	return NULL;
}
int SocketMgr::SetSktFD(fd_set *pread_fds)
{
	int nMaxfd = 0;
	if((nSvrFlg&SKT_DEL_FLG) != SKT_DEL_FLG)
	{
		FD_SET(nSkt_fd, pread_fds);
	}
	if(NULL != pCDstSocketMgr && SKT_SRC_FLG == nSvrFlg)
	{
		nMaxfd = pCDstSocketMgr->SetSktFD(pread_fds);
	}
	if(nMaxfd<nSkt_fd)
	{
		nMaxfd = nSkt_fd+1;
	}
	return nMaxfd;
}
int SocketMgr::SetSktFD(fd_set *pread_fds, int nMaxfd)
{
	//EsxMgr_logDebug("Add fd[%d] to Recd", nSkt_fd);
	if(((nSvrFlg&SKT_DEL_FLG) == SKT_DEL_FLG) &&
		((nSvrFlg&SKT_SVR_FLG) == SKT_SVR_FLG) )
	{
		if(0 != nSkt_fd)
		{
			TRACE("client disconnection");
			closesocket(nSkt_fd);
			nSkt_fd =0;
			n_port = 0;
			m_strIP = _T("");
		}
		SetSktFlg(SKT_NUL_FLG);
	}
	if( ((nSvrFlg&SKT_SVR_FLG) == SKT_SVR_FLG))
	{
		FD_SET(nSkt_fd, pread_fds);
	}
	if(NULL != pCNextSocketMgr)
	{
		if (nSkt_fd > nMaxfd)
		{
			nMaxfd = nSkt_fd+1;
		}
		return pCNextSocketMgr->SetSktFD(pread_fds, nMaxfd);
	}
	if (nSkt_fd > nMaxfd)
	{
		nMaxfd = nSkt_fd + 1;
	}
	return nMaxfd;
}
SocketMgr* SocketMgr::AcceptSkt()
{
	SocketMgr *		pCSocketNewClient = NULL;
    sockaddr_in		remoteAddr;  
	int				nSkt_fdTmp = 0;
    int				nAddrlen = sizeof(remoteAddr);

	pCSocketNewClient = new SocketMgr(this);
	if(NULL == pCSocketNewClient)
	{
		nSkt_fdTmp = accept(nSkt_fd, (SOCKADDR *)&remoteAddr, &nAddrlen);  
		if(nSkt_fd == INVALID_SOCKET)  
		{
			TRACE("accept error !");
			return NULL;
		}
		closesocket(nSkt_fdTmp);
		nSkt_fdTmp = NULL;
		delete pCSocketNewClient;
		pCSocketNewClient = NULL;
		return NULL;
	}
	if ( 0 != pCSocketNewClient->CreateSkt(this))
	{
		delete pCSocketNewClient;
		pCSocketNewClient = NULL;
		return NULL;
	}
	return pCSocketNewClient;
}
SocketMgr* SocketMgr::SelectSkt(int nTimeOut,int nType)
{
	fd_set				read_fds;
	struct timeval		select_timeout;
	int					select_num;
	int					nMaxFdCount = 0;

	FD_ZERO(&read_fds);
	switch(nType)
	{
	case 0:
		nMaxFdCount = SetSktFD(&read_fds, nMaxFdCount);
		break;
	case 1:
		if( ((nSvrFlg&SKT_DEL_FLG) == SKT_DEL_FLG) &&
			((nSvrFlg&SKT_SRC_FLG) == SKT_SRC_FLG))
		{
			return this;
		}
		nMaxFdCount = SetSktFD(&read_fds);
		break;
	}
	select_timeout.tv_sec  = nTimeOut;
	select_timeout.tv_usec = 0;

	//EsxMgr_logDebug("Select Time Out:%d Max fd:%d", nTimeOut, nMaxFdCount);
	if (0 == nMaxFdCount)
	{
		Sleep(100);
		return NULL;
	}
	select_num = select(nMaxFdCount, &read_fds, NULL, NULL, &select_timeout);
	if (select_num < 0)
	{
		Sleep(100);
		TRACE("accept error %d", GetLastError());
		return NULL;
	}
	if (select_num == 0)
	{
		Sleep(100);
		return NULL;
	}
	return GetSktFD(&read_fds);;
}

int SocketMgr::SetSendData(int nResult, char *pstrSendBuff, int nSendLen)
{
	int nMallocSize = sizeof(ESXMGR_HEADER);
	ESXMGR_HEADER *stuSendHead = NULL;
	char * pstrSend = NULL;
	int *pResult = 0;
	if(nSendLen != 0 )
	{
		nMallocSize +=nSendLen;
	}
	pstrSend = (char*)calloc(nMallocSize, sizeof(char));
	if(NULL == pstrSend)
	{
		TRACE("send head  error !");
		return 0;
	}
	stuSendHead = (ESXMGR_HEADER *)pstrSend;
	memcpy(stuSendHead->strSegName, SEGNAME, strlen(SEGNAME));
	stuSendHead->nDataLen   = htonl(nSendLen);
	stuSendHead->nResult = htonl(nResult);
	if(0 != nSendLen)
	{
		memcpy(pstrSend+sizeof(ESXMGR_HEADER), pstrSendBuff, nSendLen);
	}
	if(nMallocSize != send(nSkt_fd,pstrSend,nMallocSize,0))
	{
		return 0;
	}
	return nMallocSize;
}
int SocketMgr::SendData(char *pstrSendBuff, int nSendLen)
{
	if (nSendLen != send(nSkt_fd, pstrSendBuff, nSendLen, 0))
	{
		return -1;
	}
	return 0;
}
int SocketMgr::SendData(char *pstrSendBuff)
{
	return SetSendData(RTN_SUCCESS, pstrSendBuff, 0);
}
int SocketMgr::SendData(int nResult)
{
	return SetSendData(nResult,NULL,0);
}
int SocketMgr::RecvData(char * pstrDataBuffer, int nDataLen,int nType)
{
	int				nRecvLen = 0;
	for ( ; ; )
	{
		nRecvLen = recv(nSkt_fd, pstrDataBuffer, nDataLen, 0);

		if (nRecvLen <= 0 )
		{
			return -1;
		}

		if (nRecvLen == nDataLen || 1 == nType)
		{
			TRACE("Recv[%d] Data Len[%d]", nSkt_fd, nDataLen);
			break;
		}
		else if (nRecvLen < nDataLen)
		{
			nDataLen -= nRecvLen;
			pstrDataBuffer += nRecvLen;
		}
		else
		{
			TRACE("recv head size big !");
			return -1;
		}
	}
	return nRecvLen;
}
int SocketMgr::RecvHeadData()
{
	ESXMGR_HEADER	stuSendHead = {0};
	char *			pstrDataBuffer = (char*)&stuSendHead;
	int				nDataLen = sizeof(ESXMGR_HEADER);
	int				nRecvLen = 0;
	nRecvLen  = RecvData(pstrDataBuffer,nDataLen);
	if( nDataLen != nRecvLen)
	{
		if(0 > nRecvLen)
		{
			TRACE("recv head size error !");
		}
		return nRecvLen;
	}
	if(0 != memcmp(stuSendHead.strSegName, SEGNAME, strlen(SEGNAME)))
	{
		TRACE("seg name is incorrect !");
		return -1;
	}
	//SetEsxErrorCode(ntohl(stuSendHead.nResult));
	stuSendHead.nDataLen = ntohl(stuSendHead.nDataLen);
	return stuSendHead.nDataLen;
}
char * SocketMgr::RecvData(int &nRecvLen)
{
	int				nDataLen = 0;
	nDataLen = RecvHeadData();
	if(nDataLen <= 0)
	{
		nRecvLen = nDataLen;
		return NULL;
	}
	if(NULL != pstrRecvDataBuffer)
	{
		free(pstrRecvDataBuffer);
		pstrRecvDataBuffer = NULL;
	}
	pstrRecvDataBuffer = (char *) calloc(nDataLen,sizeof(char));
	if(NULL == pstrRecvDataBuffer)
	{
		TRACE("recv data malloc error !");
		nRecvLen = -1;
		return NULL;
	}
	if( nDataLen != RecvData(pstrRecvDataBuffer, nDataLen))
	{
		TRACE("recv data error !");
		nRecvLen = -1;
		return NULL;
	}
	nRecvLen = nDataLen;
	return pstrRecvDataBuffer;
}
int SocketMgr::GetSpeed()
{
	int			nRtnSize = nRecvSize - nTotalSize;
	if (nTotalSize != nRecvSize)
	{
		nTotalSize = nRecvSize;
		nTimes = 0;
	}
	else
	{
		nTimes++;
		if (nTimes > 5)
		{
			nTotalSize = 0;
			nRecvSize = 0;
			nTimes = 0;
		}
	}
	if (nRtnSize > 0)
	{
		return nRtnSize;
	}
	return 0;
}
int SocketMgr::TransData()
{
	int nRtn = -1;
	int nRectDataLen = 0;
	int nMallocSize = 5*1024*1024;
	if (NULL == pCDstSocketMgr)
	{
		return -1;
	}
	if(((nSvrFlg&SKT_DEL_FLG) == SKT_DEL_FLG) )
	{
		return -2;
	}
	if(NULL == pstrRecvData)
	{
		pstrRecvData = (char *)malloc(nMallocSize);
		if (NULL == pstrRecvData )
		{
			return -1;
		}
		memset(pstrRecvData, 0, nMallocSize);
	}
	nRectDataLen = RecvData(pstrRecvData, nMallocSize, 1);
	if (0>nRectDataLen)
	{
		return -3;
	}
	if (0 == pCDstSocketMgr->SendData(pstrRecvData, nRectDataLen))
	{
		nRtn= 0;
	}
	nRecvSize +=nRectDataLen;
	return nRtn;
}