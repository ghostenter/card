// SCardmgr.cpp: 
//          SmartCard's encapsulation.
//
//			by Y.Volta Tang (y.volta@gmail.com)
//          2008-01-18 22:06
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SCardmgr.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// string/hex conversion by Xian-Qiang Yu.
int Hex2Asc(char *Dest,char *Src,int SrcLen);
int Asc2Hex(char *Dest,char *Src,int SrcLen);
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSCardmgr::CSCardmgr()
{
	sprintf(m_szErrMsg, "%s", "No error.");
	memset(pResponseBuffer, 0x0, sizeof(pResponseBuffer));
	ResponseLength = MAX_RESPONSE;
    cByte=SCARD_AUTOALLOCATE;
	m_arrReaders.RemoveAll();
	CString m_logpath = "D:\\scardlog.txt"; 
	m_firststart=TRUE;
	BOOL st = FALSE;
	for (int i=0;i<33;i++)
	{
		SerialID[i] = "";
		m_fristwrite[i]=TRUE;
	}
	st = m_scardlog.Open(m_logpath,CFile::modeCreate|CFile::modeNoTruncate|CFile::modeWrite);
	if (st == FALSE)
	{
		AfxMessageBox("打开文件D:\\scardlog.txt失败!");
		exit(1);
	}
    else
	{
		m_scardlog.SeekToEnd();
		SYSTEMTIME st;
		GetLocalTime(&st);
		CString templogtr;
		templogtr.Format("%4d年%02d月%02d日 %02d:%02d:%02d\r\n",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
	}
}

CSCardmgr::~CSCardmgr()
{
	m_arrReaders.RemoveAll();
	SCardReleaseContext(ContextHandle);
}

///////////////////////////////////////////////////////////////////////////////
// Open a SCard Reader by its index
BOOL CSCardmgr::SCardOpen(int nInx)
{  	
	BOOL st = FALSE;
	SCReader			stuReader;
     if (nInx>=m_arrReaders.GetSize() || nInx == -1)
     {
		 AfxMessageBox("打开读卡器时越界");
		 return FALSE;
     }
	if(nInx > m_arrReaders.GetSize())
	{
		sprintf(m_szErrMsg, "%s", "Index out of bound.");
		return FALSE;
	}
	m_nInx = nInx;
    DWORD ret = 0;
	stuReader = m_arrReaders.GetAt(nInx);	 	// Get reader name from ReaderArray
    if (CardHandle[nInx]!=NULL&&m_fristwrite[nInx] == TRUE)                 //如果处于连接中先进行下电断开连接并且是第一次写卡
    {		
		ret=SCardDisconnect(CardHandle[nInx],SCARD_UNPOWER_CARD);
		if (ret != SCARD_S_SUCCESS)
		{		
			CString templogtr;			
			templogtr.Format("第%d读卡器下电失败,工位句柄为:%d,返回错误(%08x),序列号:%s",nInx,CardHandle[nInx],ret,SerialID[nInx]);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			return FALSE;
		}
       m_fristwrite[nInx] = FALSE;
    }
	if (m_firststart == FALSE) //不是第一次启动机器
	{
		ret = SCardConnect(ContextHandle, stuReader.szReaderName, SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,	&CardHandle[nInx], &ActiveProtocol[nInx]);
		if (ret != SCARD_S_SUCCESS)
		{
			CString templogtr;
			templogtr.Format("第%d读卡器协议打开失败,工位句柄为:%d,返回错误(%08x),序列号:%s",nInx,CardHandle[nInx],ret,SerialID[nInx]);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			ret = SCardConnect(ContextHandle,stuReader.szReaderName,SCARD_SHARE_DIRECT, SCARD_PROTOCOL_UNDEFINED, &CardHandle[nInx],&ActiveProtocol[nInx]);
			if (ret != SCARD_S_SUCCESS)
			{		
				CString templogtr;
				templogtr.Format("第%d读卡器非协议打开失败(二次),工位句柄为:%d,返回错误(%08x),序列号:%s",nInx,CardHandle[nInx],ret,SerialID[nInx]);
				CString temstr;
				temstr = GetNowTime();
			    templogtr = templogtr+temstr;
				m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
				m_scardlog.Flush();				
			}
			return FALSE;
		}
			
	}    
	lErr = ret;	
	if (m_firststart == TRUE)    //第一次启动机器
    {
		ret = SCardConnect(ContextHandle,stuReader.szReaderName,SCARD_SHARE_DIRECT, SCARD_PROTOCOL_UNDEFINED, &CardHandle[nInx],&ActiveProtocol[nInx]);		
		if (ret != SCARD_S_SUCCESS)
		{
			CString templogtr;
			templogtr.Format("第%d读卡器非协议打开失败,工位句柄为:%d,返回错误(%08x),序列号:%s",nInx,CardHandle[nInx],ret,SerialID[nInx]);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			return FALSE;
		}
	
	}
	if (ret==SCARD_S_SUCCESS&&m_firststart == TRUE) 
	{
		SCardgetId(nInx,1);
		return TRUE;
	}
	else
	{
		if (ret == SCARD_S_SUCCESS)
		{
			st = SCardgetId(nInx,2);  //检验读卡器有没有挪位
			if (st == FALSE)
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Close a SCard Reader connection
BOOL CSCardmgr::SCardClose(int stationnum,int type)
{
//	DWORD ret = SCardDisconnect(CardHandle[stationnum], SCARD_EJECT_CARD);
    DWORD ret = -1;
	switch (type)
	{
	case 1 :
		ret = SCardDisconnect(CardHandle[stationnum], SCARD_UNPOWER_CARD); //给卡下电  
		break;
	case 2:
        ret = SCardDisconnect(CardHandle[stationnum], SCARD_EJECT_CARD);   //弹出智能卡
		break;
	case 3:
        ret = SCardDisconnect(CardHandle[stationnum], SCARD_LEAVE_CARD);   //不做任何操作
        break;
		
	}
	if (ret != SCARD_S_SUCCESS)
	{
        CString templogtr;
		templogtr.Format("第%d读卡器下电失败,工位句柄为:%d,返回错误(%08x),序列号:%s",stationnum,CardHandle[stationnum],ret,SerialID[stationnum]);
		CString temstr;
		temstr = GetNowTime();
		templogtr = templogtr+temstr;
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
		return FALSE;
	}
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Get ATR from a open SCard Reader
BOOL CSCardmgr::SCardReset(int station,int type)
{    
    DWORD     dwAP;
	LONG      ret;
	switch (type)
    {
    case 1 :    //cold
		ret = SCardReconnect(CardHandle[station],SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,	SCARD_UNPOWER_CARD,&dwAP );	
    	break;
	case 2:     //warm
		ret = SCardReconnect(CardHandle[station],SCARD_SHARE_SHARED,SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,	SCARD_RESET_CARD,&dwAP );	
		break;
	default:
		return FALSE;
    }
	if (ret != SCARD_S_SUCCESS)
	{
        CString templogtr;
		templogtr.Format("第%d读卡器SCardReconnect失败,工位句柄为:%d,返回错误(%08x),序列号:%s",station,CardHandle[station],ret,SerialID[station]);
		CString temstr;
		temstr = GetNowTime();
		templogtr = templogtr+temstr;
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
		return FALSE;
	}	
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// Transmit Apdu 
BOOL CSCardmgr::SCardTransmit(LPCTSTR strApdu, char *  strResp, UINT *nSW,int station)
{	
	char                SendData[MAX_APDU_LEN], ReceiveData[MAX_RESPONSE];
	ULONG               nCmdLen, nResp;
	SCARD_IO_REQUEST	IO_Request;

    ZeroMemory(ReceiveData, MAX_APDU_LEN);
    nCmdLen = strlen(strApdu);

    Asc2Hex(SendData, (char*)strApdu, nCmdLen);			// string convertion

    nResp = MAX_RESPONSE;
	IO_Request.dwProtocol = ActiveProtocol[station];
	IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);

	DWORD ret = ::SCardTransmit(CardHandle[station],				// SCard API
						&IO_Request, 
						(PUCHAR)SendData, 
						nCmdLen / 2, 
						0, 
						(PUCHAR)ReceiveData, 
						&nResp);
	
    	if (ret != SCARD_S_SUCCESS)
		{		
			CString templogtr;
			templogtr.Format("第%d读卡器SCardTransmit.Error:%s,工位句柄为:%d,返回错误(%x),序列号:%s",station, strApdu,CardHandle[station],ret,SerialID[station]);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			return FALSE;
		}
    if(nResp != 0)
    {
		Hex2Asc(strResp, ReceiveData, nResp);
		*nSW = ((unsigned char)(ReceiveData[nResp - 2]) * 256) + 
			(unsigned char)ReceiveData[nResp - 1];
		return TRUE;
    }
	return FALSE;
}


///////////////////////////////////////////////////////////////////////////////
// Get SCard Reader's List
BOOL CSCardmgr::SCardGetPcscList()
{
	
	DWORD ret = SCardEstablishContext(SCARD_SCOPE_USER, 
										NULL, 
										NULL, 
										&ContextHandle);
	lErr = ret;
	if (ret != SCARD_S_SUCCESS) 
	{
		CString templogtr;
		templogtr.Format("Function SCardEstablishContext returned 0x%08X error code.", ret);
		CString temstr;
		temstr = GetNowTime();
		templogtr = templogtr+temstr;
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
		m_scardlog.Close();
		exit(1);
		return FALSE;
	}
	ret = SCardListReaders(ContextHandle, 0, 
						(char *) pResponseBuffer, 
						&ResponseLength);

	lErr = ret;
	if (ret != SCARD_S_SUCCESS) 
	{
		sprintf(m_szErrMsg, "No reader available. (%x)", ret);
		AfxMessageBox(m_szErrMsg);
		CString templogtr;
		templogtr.Format("No reader available. (%x)", ret);
		CString temstr;
		temstr = GetNowTime();
		templogtr = templogtr+temstr;
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
		m_scardlog.Close();
		exit(1);
		return FALSE;
	}
	else 
	{
		// OK, Clear the Lists
		m_arrReaders.RemoveAll();
		UINT		StringLen = 0;
		SCReader	stuReader;		
		// Build the Reader List
		while ( ResponseLength > StringLen+1) 
		{                    
			ZeroMemory(stuReader.szReaderName, sizeof(stuReader.szReaderName));			
			strcpy(stuReader.szReaderName, (char*)pResponseBuffer+StringLen);
			m_arrReaders.Add(stuReader);
			StringLen += strlen((LPCTSTR) pResponseBuffer+StringLen+1);
			StringLen += 2;
		}
	}
    m_readernum=m_arrReaders.GetSize();
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// hex to asc: 0x22 -> "22"
int Hex2Asc(char *Dest,char *Src,int SrcLen)
{
	int i;
	for ( i = 0; i < SrcLen; i ++ )
	{
		sprintf(Dest + i * 2,"%02X",(unsigned char)Src[i]);
	}
	Dest[i * 2] = 0;
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// asc to hex: "22" -> 0x22
int Asc2Hex(char *Dest,char *Src,int SrcLen)
{
	int i;
	for ( i = 0; i < SrcLen / 2; i ++ )
	{
		sscanf(Src + i * 2,"%02X",(unsigned char *)&Dest[i]);
	}
	return TRUE;
}


// end-of-file
//////////////////////////////////////////////////////////////////////////
#define CMD_READER_INFO_EXTENDED 0x1E
#define IOCTL_CCID_ESCAPE SCARD_CTL_CODE(0xDAC)
BOOL CSCardmgr::SCardgetId(int station,int type)
{  
	if (type == 1)
	{
		LONG     lReturn;
		memset(SerialID[station].GetBuffer(100),0,100);
		lReturn = SCardGetAttrib(CardHandle[station],SCARD_ATTR_VENDOR_IFD_SERIAL_NO,(LPBYTE)SerialID[station].GetBuffer(100),&cByte);
		lReturn = SCardGetAttrib(CardHandle[station],SCARD_ATTR_VENDOR_IFD_SERIAL_NO,(LPBYTE)SerialID[station].GetBuffer(100),&cByte);
		if ( SCARD_S_SUCCESS != lReturn )
		{
			CString templogtr;
			templogtr.Format("第%d读卡器读取序列号失败,工位句柄为:%d,返回值为0x%08X,序列号:%s",station,CardHandle[station],lReturn,SerialID[station]);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			m_scardlog.Close();
			exit(1);
		}
	} 
	else
	{
		LONG     lReturn;		
		CString temserialno;
		memset(temserialno.GetBuffer(100),0,100);
		lReturn = SCardGetAttrib(CardHandle[station],SCARD_ATTR_VENDOR_IFD_SERIAL_NO,(LPBYTE)temserialno.GetBuffer(100),&cByte);
		lReturn = SCardGetAttrib(CardHandle[station],SCARD_ATTR_VENDOR_IFD_SERIAL_NO,(LPBYTE)temserialno.GetBuffer(100),&cByte);
		if ( SCARD_S_SUCCESS != lReturn )
		{
			CString templogtr;
			templogtr.Format("第%d读卡器读取序列号失败,工位句柄为:%d,返回值为0x%08X,序列号:%s",station,CardHandle[station],lReturn,SerialID[station]);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			m_scardlog.Close();
			return  FALSE;
		}
		if (temserialno != SerialID[station])
		{
			CString templogtr;
			templogtr.Format("第%d读卡器比较序列号失败,工位句柄为:%d,返回值为0x%08X,序列号:%s %s",station,CardHandle[station],lReturn,SerialID[station],temserialno);
			CString temstr;
			temstr = GetNowTime();
			templogtr = templogtr+temstr;
			m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
			m_scardlog.Flush();
			return  FALSE;
		}
	}
	return TRUE;
}

BOOL CSCardmgr::Inital()
{
   BOOL st;
   st=SCardGetPcscList();
   //对32个读卡器进行初始化
   for (int i=0;i<32;i++)
   {
	   CardHandle[i]=0;
   }
   if (st!=TRUE)
   {
	   return FALSE;
   } 
   for (i=0;i<m_readernum;i++)
   {
       st=SCardOpen(i);
	   if (st==FALSE)
	   {
		   AfxMessageBox("读卡器打开失败!");
	   }
   }
   m_firststart = FALSE;
   return TRUE;
}

BOOL CSCardmgr::StartAdpu(int istation)
{
   DWORD ret = 0;
   ret = SCardBeginTransaction(CardHandle[istation]);
   if (ret != SCARD_S_SUCCESS)
   {
	   CString templogtr;
	   templogtr.Format("第%d读卡器SCardBeginTransaction失败,工位句柄为:%d,返回错误(%x),序列号:%s",istation,CardHandle[istation],ret,SerialID[istation]);
	   CString temstr;
	   temstr = GetNowTime();
	   templogtr = templogtr+temstr;
	   m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
	   m_scardlog.Flush();
	   return FALSE;
   }
   return TRUE;
}

BOOL CSCardmgr::EndApdu(int istation)
{
	DWORD ret = 0;
	ret = SCardEndTransaction(CardHandle[istation],SCARD_LEAVE_CARD);
	if (ret != SCARD_S_SUCCESS)
	{
		CString templogtr;
		templogtr.Format("第%d读卡器SCardEndTransaction失败,工位句柄为:%d,返回错误(%x),序列号:%s",istation,CardHandle[istation],ret,SerialID[istation]);
		CString temstr;
		temstr = GetNowTime();
		templogtr = templogtr+temstr;
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
	   return FALSE;
	}
    return TRUE;
}

BOOL CSCardmgr::SCardGetAtr(LPCTSTR strResp,int station)
{
	DWORD               dwATRLength = 40; 
    BYTE                pbyATR[40]; 
	SCReader			stuReader;
	DWORD               dwLength    = 300;
    DWORD               dwCardState = 0;
	DWORD               dwActiveProtocol = 0;
    DWORD ret = 0;
	ret = SCardStatus(CardHandle[station],	 			// Trick
		stuReader.szReaderName,
		&dwLength,
		&dwCardState,
		&dwActiveProtocol,
		pbyATR,
		&dwATRLength);
	if ( SCARD_S_SUCCESS != ret ){
		strResp = "";			
		CString templogtr;
		templogtr.Format("第%d读卡器SCardStatust失败,工位句柄为:%d,返回错误(%x),序列号:%s",station,CardHandle[station],ret,SerialID[station]);
		CString temstr;
		temstr = GetNowTime();
		templogtr = templogtr+temstr;
		m_scardlog.WriteHuge(templogtr,templogtr.GetLength());
		m_scardlog.Flush();
		return FALSE;		
	}	
	// String convertion
	Hex2Asc((char*)strResp, (char*)pbyATR, dwATRLength );	
	return TRUE;
	return TRUE;
}

CString CSCardmgr::GetNowTime()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	CString templogtr;
	templogtr.Format(",%4d年%02d月%02d日 %02d:%02d:%02d\r\n",st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute,st.wSecond);
	return templogtr;
}
