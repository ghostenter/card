#ifndef CPCSC_H_CAESAR__DEF
#define CPCSC_H_CAESAR__DEF
#include <winscard.h>
#pragma comment(lib,"winscard")
/********************************************************************
Copyright (C) 1999 - 2005, CaesarZou
This software is licensed as described in the file COPYING, which
you should have received as part of this distribution. 
You may opt to use, copy, modify, merge, publish, distribute and/or sell
copies of the Software, and permit persons to whom the Software is
furnished to do so, under the terms of the COPYING file.
This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
KIND, either express or implied.

  file base:	PCSC
  file ext:	h
  author:	�޵�ǿ
  email:	zoudeqiang1979@tsinghua.org.cn
			name_caesar@msn.com
  
  purpose:	ʵ���˲���GSM��PCSC�µ�Ӧ�á�
  modify his:	
	
*********************************************************************/

class CPCSC  
{
public:
	CPCSC();
	virtual ~CPCSC();

	//CHV1 status
	//UNWAKE				- Unkown status(init)
	//STATUS_DISABLE		- PIN Disable
	//STATUS_ENABLE			- PIN Enable
	//STATUS_VOK			- PIN Enable, but be verified, user should update this value after VERIFY_CHV success.
	//STATUS_BLOCK			- PIN Block, should use PUK unblock
	//STATUS_DEADLOCK		- PUK Block, never unblock.
	enum		PIN_STATUS {STATUS_UNWAKE,STATUS_DISABLE,STATUS_ENABLE,STATUS_VOK,STATUS_BLOCK,STATUS_DEADLOCK};
	//file type
	enum		FILE_TYPE	 {TYPE_RFU=0x00, TYPE_MF=0x01, TYPE_DF=0x02, TYPE_EF=0x04};
	//file struct
	enum		FILE_STRUCT{EF_TRANSFORT=0x00, EF_LINEAR=0x01, EF_CYCLIC=0x03};
	
	/*******************************************************************
	�� �� �� �ƣ�	Init(const char* szReader)
	�� �� �� ����	��ʹ��PC/SC����
	�� �� ˵ ����	szReader��ΪNULL��ָ����ʹ��������(���Լ�ؿ�Ƭ״̬)�����򲻾����������������ָ��
	����ֵ ˵����	bool �Ƿ��ʹ���ɹ�
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	bool Init(const char* szReader=NULL);
	
	
	/*******************************************************************
	�� �� �� �ƣ�	Eject()
	�� �� �� ����	�ͷ�PC/SC����
	�� �� ˵ ����	
	����ֵ ˵����	
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	void Eject();
	

	/*******************************************************************
	�� �� �� �ƣ�	ListReaders()
	�� �� �� ����	�г�ȫ��֧�ֵĶ�������
	�� �� ˵ ����	IN & OUT, ��Ҫ�����㹻���ȵ�buffer
	����ֵ ˵����	0--ʧ�ܣ�����Ϊreaders���ĳ��ȡ�readersΪ�മ��A\0B\0C\0\0
					����ʽΪ��
					char * pReader = pReaders;
					
					while('\0' != *pReader) 
					{
					  //do something to pReader
						pReader = pReader + strlen(pReader) + 1;
					}
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	int ListReaders(char* readers);


	/*******************************************************************
	�� �� �� �ƣ�	GetAtr(char* szATR);
	�� �� �� ����	���ATR
	�� �� ˵ ����	unsinged char* usATR--ATR Buffer
	����ֵ ˵����	0Ϊʧ�ܣ�����ΪATR����
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	int GetAtr(unsigned char* usATR);

	
	/*******************************************************************
	�� �� �� �ƣ�	ConnectCard(const char*	sReader)
	�� �� �� ����	���ӵ�����Ķ�������
	�� �� ˵ ����	sReader ����������(NULL��ʹ��init()�Ķ���������)
	����ֵ ˵����	bool �Ƿ����ӳɹ�
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	bool	ConnectCard(const char*	sReader=NULL, int nShareMode=SCARD_SHARE_SHARED);
	bool	ResetCard(const char* sReader=NULL,int nShareMode=SCARD_SHARE_SHARED);

	/*******************************************************************
	�� �� �� �ƣ�	DisConnectCard()
	�� �� �� ����	�Ͽ���ǰ�Ķ���������
	�� �� ˵ ����	
	����ֵ ˵����	
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	void	DisConnectCard();
	
	
	/*******************************************************************
	�� �� �� �ƣ�	GetReaderStatusChange(int milseconds)
	�� �� �� ����	��õ�ǰ��������״̬��(��Ҫ��Init��ʱ��ָ��������)
	�� �� ˵ ����	milseconds ��ʱ����
	����ֵ ˵����	long ״̬��ֵ
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	long GetReaderStatusChange(int milseconds);
	void SetReaderStatusChange() {m_rgscState.dwCurrentState ++;}


	/*******************************************************************
	�� �� �� �ƣ�	IsConnc()
	�� �� �� ����	�ж϶������Ƿ�����
	�� �� ˵ ����	
	����ֵ ˵����	bool ����״̬
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	bool IsConnc();


	/*******************************************************************
	�� �� �� �ƣ�	RunAPDU(IN unsigned char* ucCmd, IN DWORD nCmdLen, OUT unsigned char* ucRes, IN OUT DWORD& nResLen)
	�� �� �� ����	�·�APDU
	�� �� ˵ ����	ucCmd Apduָ��Buffer,nCmdLen Apduָ���,ucRes ����ָ���,nResLen ����ָ����С(IN)/����ָ��ĳ���(OUT)
	����ֵ ˵����	bool �Ƿ��·��ɹ�
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	bool	RunAPDU(IN unsigned char* ucCmd, IN DWORD nCmdLen, OUT unsigned char* ucRes, IN OUT DWORD& nResLen);



	//////////////////////////////////////////////////////////////////////////
	/*GSM 11.11����ʵ��*/
	bool	SELECT(IN WORD wFileID);
	bool	STATUS();
	bool	SEEK(IN BYTE* Patten, IN BYTE byPattenLen);
	bool	GET_RESPONSE(OUT BYTE* ucResp, IN BYTE byResLen);

	bool	READ_BINARY(OUT BYTE* ucFile, IN BYTE byLen, IN WORD wOffset);
	bool	UPDATE_BINARY(IN BYTE* ucFile, IN BYTE byLen, IN WORD wOffset);
	bool	READ_RECORD(IN BYTE byRecNo, OUT BYTE* ucData, IN BYTE byRecLen);	//����0x04����ģʽ��RecNo��0��ʼ
	bool	UPDATE_RECORD(IN BYTE byRecNo,IN BYTE* ucData, IN BYTE byRecLen);	//����0x04����ģʽ��RecNo��0��ʼ

	bool	VERIFY_CHV(IN BYTE byChvNo, IN const BYTE* ucChv, IN BYTE byChvLen);
	bool	CHANGE_CHV(IN BYTE byChvNo, IN const BYTE* ucOldChv, IN const BYTE* ucChv, IN BYTE byOldChvLen, IN BYTE byChvLen);
	bool	UNBLOCK_CHV(IN BYTE byChvNo, IN const BYTE* ucPuk, IN const BYTE* ucChv, IN BYTE byPukLen, IN BYTE byChvLen);
	
	//Only for CHV1
	bool	DISABLE_CHV(IN const BYTE* ucChv,IN BYTE byChvLen);	
	bool	ENABLE_CHV(IN const BYTE* ucChv,IN BYTE byChvLen);

	//////////////////////////////////////////////////////////////////////////
	/*PIM �淶ָ����չ*/
	bool	RUN_PIM_ALGORITHM(const BYTE* sRand);


	//////////////////////////////////////////////////////////////////////////
	/*SMSЭ���������*/
	/*******************************************************************
	�� �� �� �ƣ�	ReadSMS,WriteSMS
	�� �� �� ����	��SIM���ļ�Э���н�������Ϣ��Ϣ/�Ӷ���Ϣ��Ϣ��֯SIM���ļ�
	�� �� ˵ ����	ucADN SIM���ļ�������,szPhone �绰,szMsg��Ϣ, time ʱ��
					����������ʱ��szPhone,szMsg��bufferӦ���ɵ��÷�������
	����ֵ ˵����	״ֵ̬��0Ϊ�ռ�¼/1�Ѷ���3δ����5�ѷ���7����
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/
	static int  ReadSMS_SIM(BYTE *ucADN,char* szPhone,char* szMsg,SYSTEMTIME& time);
	static int  ReadSMS_PIM(BYTE *ucADN,char* szPhone,char* szMsg,SYSTEMTIME& time);	
	static int  ReadSMS_UIM(BYTE *ucADN,char* szPhone,char* szMsg,SYSTEMTIME& time);
	static bool WriteSMS_PIM(BYTE* ucADN,int reclen, int type, char* szPhone,char* szMsg,SYSTEMTIME& time);

	
	//////////////////////////////////////////////////////////////////////////
	/*ADNЭ���������*/
	/*******************************************************************
	�� �� �� �ƣ�	ReadADN,WriteADN
	�� �� �� ����	��SIM���ļ�Э���н����绰����Ϣ/�ӵ绰����Ϣ��֯SIM���ļ�
	�� �� ˵ ����	ucADN SIM���ļ�������,ADNFileLen = nXLen+14(��GSM 11.11),szMobile �绰,szName��Ϣ
			����������ʱ��szMobile,szName��bufferӦ���ɵ��÷�������
	����ֵ ˵����	�Ƿ�ɹ�
	��       ��:	�޵�ǿ
	�� �� �� �ڣ�	2005.11.29
	*******************************************************************/	
	static bool ReadADN(BYTE* ucADN,int nXLen,char* szMobile,char* szName);
	static bool WriteADN(BYTE* ucADN,int nXLen,char* szMobile,char* szName);


	/*Response���ݽ�������*/
	static FILE_TYPE RS_GetFileType(const BYTE *usResponse);
	//Only for response of EF
	static WORD RS_GetFileSize(const BYTE *usResponse);
	static BYTE RS_GetRecordLen(const BYTE *usResponse);
	static BYTE RS_GetRecordNum(const BYTE *usResponse);
	static FILE_STRUCT RS_GetFileStruct(const BYTE *usResponse);
	//Only for response of DF
	static PIN_STATUS RS_GetPinStatus(const BYTE*usResponse);

public:
	BYTE			m_bySW1;				//SW1
	BYTE			m_bySW2;				//SW2
	PIN_STATUS			m_nPINStatus;		//��ƬPIN1״̬���û��Լ�����״̬����ʹ��ΪUNWAKE

protected:
	SCARDCONTEXT		m_hContext;			//���豸������
	SCARDHANDLE			m_hCard;			//���������
	//
	SCARD_READERSTATE	m_rgscState;		//������״ֵ̬
	SCARD_IO_REQUEST	m_ioRequest;		//�Э��
	
	CHAR				m_szReader[1024];	//����������
	CRITICAL_SECTION	m_csReader;			//����������

	bool			SW_GSM();				
	
	void UNLOCK()
	{
		::LeaveCriticalSection(&m_csReader);
	}
	void LOCK()
	{
		::EnterCriticalSection(&m_csReader);
	}
	
};

#endif



















