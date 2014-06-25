// SCardmgr.h: interface for the CSCardmgr class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCARDMGR_H__3604BED0_8806_4F02_9732_5F52F877A357__INCLUDED_)
#define AFX_SCARDMGR_H__3604BED0_8806_4F02_9732_5F52F877A357__INCLUDED_

///////////////////////////////////////////////////////////////////////////////
// PCSC Support.
#include "winscard.h"
#pragma comment (lib, "winscard.lib")

#include <afxtempl.h>

#define			MAX_RESPONSE		5120
#define			MAX_APDU_LEN		1024

typedef		struct STU_Reader{
	char	szReaderName[MAX_PATH];
}SCReader, *PSCReader;

///////////////////////////////////////////////////////////////////////////////
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CSCardmgr  
{
public:
	CString GetNowTime();
	BOOL SCardGetAtr(LPCTSTR strResp,int istation);
	BOOL EndApdu(int istation);
	BOOL StartAdpu(int istatiom);
	BOOL Inital();
	BOOL SCardgetId(int station,int type);       //ȡ��ĳһ��λ�����к� type=1:1���������к� type = 2:������к�
	BOOL SCardGetPcscList();            //��ȡ������Դ�����ҷŻض���������
	BOOL SCardTransmit(LPCTSTR strApdu, char * strResp, UINT *nSW,int station);  //����ADPָ��
	BOOL SCardReset(int station,int type);   //�Կ����и�λ
	BOOL SCardClose(int stationnum,int type);                //��ĳ����������
	BOOL SCardOpen(int nInx);                       //����ĳ����λ 
	CSCardmgr();
	virtual ~CSCardmgr();

	CArray<SCReader, SCReader>			m_arrReaders;
	char				m_szErrMsg[1024];
	long				lErr;
	int m_nInx;
    //	SCARD_IO_REQUEST	IO_Request;
	short			    m_actName;
	BYTE			    pResponseBuffer[MAX_RESPONSE];
	ULONG	            ResponseLength;
	DWORD				ActiveProtocol[33];
	SCARDCONTEXT		ContextHandle; // Context
    SCARDHANDLE			CardHandle[33];    // Handle of SCReader
    BOOL  m_firststart;    //��һ������
	BOOL  m_fristwrite[33];    //
	int                 m_readernum;  //����������
	CString   SerialID[33];    //���ܶ�������ID
	DWORD    cByte ;
    CFile     m_scardlog;
	//CStdioFile   m_scardlog;
};

typedef struct _READER_INFO_EXTENDED
{
	BYTE	byMajorVersion;
	BYTE	byMinorVersion;
	BYTE	bySupportedModes;
	WORD	wSupportedProtocols;
	WORD	wInputDevices;
	BYTE	byPersonality;
	BYTE	byMaxSlots;
	BYTE	bySerialNumberBufferSize;
	//	BYTE	pbySerialNumber[48];
	BYTE	pbySerialNumber[100];
}READER_INFO_EXTENDED, *PREADER_INFO_EXTENDED;
#endif // !defined(AFX_SCARDMGR_H__3604BED0_8806_4F02_9732_5F52F877A357__INCLUDED_)
