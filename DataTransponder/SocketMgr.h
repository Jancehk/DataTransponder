#pragma once

#include	<sys/types.h>
#include	<sys/stat.h>
#include	<winsock2.h>
//#include	"ComHandle.h"

#define		SEGNAME		"ESXM"
#define		RTN_SUCCESS		0x00
#define		RTN_FAILD01		0x01
#define		RTN_FAILD02		0x02


#define		ESXSVR_OPT_RT_MODE			0xFF000000
#define		ESXSVR_OPT_CHK_MODE			0x01000000
#define		ESXSVR_OPT_GET_MODE			0x02000000
#define		ESXSVR_OPT_ADD_MODE			0x03000000
#define		ESXSVR_OPT_DEL_MODE			0x04000000
#define		ESXSVR_OPT_CHA_MODE			0x05000000
#define		ESXSVR_OPT_RTN_MODE			0x06000000

#define		ESXSVR_OPT_SUB_MODE			0x00FF0000
#define		ESXSVR_OPT_SUB_USER			0x00010000
#define		ESXSVR_OPT_SUB_ESX			0x00020000
#define		ESXSVR_OPT_SUB_GUEST		0x00030000



typedef struct ESXMGR_HEADER_T
{
	char	strSegName[4];
	int		nDataLen;
	int		nOptMode;
	int		nResult;
}ESXMGR_HEADER,*pESXMGR_HEADER;

typedef struct _USERINFO_T{
	int		nID;
	char	nState;
	char	RFU1[2];
	int		RFU[2];
	char	nUserPermission;
	char	strIPAddress[32];
	char	strUserName[32];
	char	strPassword[32];
	char	strGroupName[32];
	char	strTel[16];
}USERINFO,*pUSERINFO;
typedef struct _ESXDATETIME_T
{
	short	nExpireTimeY;
	char	nExpireTimeM;
	char	nExpireTimeD;

	char	nExpireTimeH;
	char	nExpireTimeN;
	char	nExpireTimeS;
	char	nReserveStat;
}ESXDATETIME, *pESXDATETIME;
typedef struct _GUESTINFO_T{
	int		nID;
	int		nEsxID;
	int		nVimID;
	int		RFU;
	char	strGuestOSName[32];
	char	strIPAddress[16];
	char	strUserName[32];
	char	strPassword[32];
	char	strManagerName[32];

	ESXDATETIME stuDate;
	
	char	strPurpose[128];
}GUESTINFO, *pGUESTINFO;
class SocketMgr 
{
public:
	SocketMgr(void);
	SocketMgr(SocketMgr* pCParSocketMgr);
	~SocketMgr(void);
	int CreateSkt(void);
	int CreateSkt(char * pstrIP, int nPort);
	int CreateSkt(CString strIP, int nPort);
	int CreateSkt(int nSvrPort, CString strIP);
	int CreateSkt(SocketMgr* pCSocketSvr);
	
private:
	int		nSkt_fd;
	int		nOptMode;
	/* Esx Info Begin */
	int			nEsxID;
	char		nEsxState;
	char		strIPInfo[32];
	char		strCliIPInfo[32];
	/* Esx Info End */
	char *		pstrRecvDataBuffer;
	void *		pstuUserInfo;
	SocketMgr * pCPrevSocketMgr;
	SocketMgr * pCNextSocketMgr;
	int SetSktFD(fd_set *pread_fds);
	int SetSktFD(fd_set *pread_fds, int nMaxfd);
	SocketMgr* GetSktFD(fd_set *pread_fds);
	int SetSendData(int nResult, char *pstrSendBuff, int nSendLen);
	int RecvData(char * pstrDataBuffer, int nDataLen, int nType=0);
	char *	pstrRecvData;
	int		nTotalSize;
	int		nRecvSize;
	int		nTimes;
public:
	SocketMgr * pCDstSocketMgr;
	int		nSvrFlg;
	int		n_port;
	CString m_strIP;
	SocketMgr* AcceptSkt();
	SocketMgr * GetNewSkt();
	SocketMgr * GetNewDst();
	SocketMgr * GetNextSkt(){return pCNextSocketMgr; };
	int GetSpeed();
	int GetOptMode(int nType){return nOptMode&nType;}
	void SetOptMode(int nType){nOptMode = htonl(nType);}
	int SendData(char *pstrSendBuff, int nSendLen);
	int SendData(char *pstrSendBuff);
	int SendData(int nResult);
	int RecvHeadData();
	char * RecvData(int &nRecvLen);
	int TransData();
	SocketMgr* SelectSkt(int nTimeOut, int nType = 0);
	/* Get user info Begin */
	void * GetUserInf(){return pstuUserInfo;};
	/* Get user info End */
	/* Esx Info Begin */
	int GetEsxID(){ return nEsxID; };
	char GetEsxState(){ return nEsxState; };
	char* GetEsxIP(){ return strIPInfo; };
	char* GetCliIP(){ return strCliIPInfo; };
	void SetEsxID(int nSelectEsxID){ nEsxID = nSelectEsxID; }
	void SetEsxState(char nSelectEsxStat){ nEsxState = nSelectEsxStat; }
	void SetEsxIP(char *pstrEsxIPInfo){ strncpy_s(strIPInfo, pstrEsxIPInfo, 32); }
	/* Esx Info End */
private:
	int nOnlineState;
public:
	void SetOnLine(void * pstrInUserInfo){pstuUserInfo=pstrInUserInfo;nOnlineState = 1;}
	int  IsOnLine(void){return nOnlineState;}
};

