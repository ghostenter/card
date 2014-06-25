#include "PCSC.h"
#include "Encode.h"

static BYTE inline TMEBYTE(BYTE a)
{
	BYTE hi = a & 0x0F;
	BYTE lw = a>>4;

	if(hi>=0 && hi<=9 && lw>=0 && hi<=9)
	{
		return (hi*10 + lw);
	}

	return 0;
}

static BYTE inline TMEBYTEN(BYTE a)
{
	BYTE hi = a>>4;
	BYTE lw = a & 0x0F;
	
	if(hi>=0 && hi<=9 && lw>=0 && hi<=9)
	{
		return (hi*10 + lw);
	}
	
	return 0;
}

static BYTE inline RTMBYTE(WORD a)
{
	BYTE hi = a%10;
	BYTE lw = a/10;

	if(hi>=0 && hi<=9 && lw>=0 && hi<=9)
	{
		return hi<<4 | lw;
	}

	return 0;
}

CPCSC::CPCSC()
{
	m_nPINStatus	=	STATUS_UNWAKE;

	m_bySW1		=	0;
	m_bySW2		=	0;
	m_hContext	=	0;
	m_hCard		=	0;
	m_szReader[0] = 0;
	m_rgscState.szReader		=	m_szReader;
	m_ioRequest.cbPciLength		=	sizeof(SCARD_IO_REQUEST);

	::InitializeCriticalSection(&m_csReader);
}

CPCSC::~CPCSC()
{
	DisConnectCard();
	Eject();

	::DeleteCriticalSection(&m_csReader);
}

bool CPCSC::ConnectCard(const char* sReader,int nShareMode)
{
	
	if(sReader == NULL)
	{
		sReader = m_szReader;
	}

	if(lstrlen(sReader)==0)
	{
		return false;
	}

	DisConnectCard();

	// Connect
	LOCK();
	
	m_ioRequest.dwProtocol = 0;
	LONG lReturn = SCardConnect( m_hContext, 
		sReader,
		nShareMode,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		&m_hCard,
		&m_ioRequest.dwProtocol );
	
	if ( SCARD_S_SUCCESS != lReturn )
	{
		m_hCard = 0;

		UNLOCK();
		return false;
	}

	if(lstrlen(m_rgscState.szReader) != 0)
	{
		//��ֹ���ӡ��Ͽ��������µ�״̬�仯
		if(SCardGetStatusChange(m_hContext,0,&m_rgscState,1 ) != SCARD_E_TIMEOUT)
		{
			m_rgscState.dwCurrentState	=	m_rgscState.dwEventState;
		}
	}

	UNLOCK();
	
	return true;
}



bool CPCSC::ResetCard(const char* sReader,int nShareMode)
{
	if(sReader == NULL)
	{
		sReader = m_szReader;
	}
	
	if(lstrlen(sReader)==0)
	{
		return false;
	}
	
	LOCK();	
	if(m_hCard)
	{
		::SCardDisconnect(m_hCard,SCARD_RESET_CARD);	
	}
	
	m_hCard		=	0;
	m_nPINStatus	=	STATUS_UNWAKE;
	
	//��ֹ���ӡ��Ͽ��������µ�״̬�仯
	if(lstrlen(m_rgscState.szReader) != 0)
	{		
		SCardGetStatusChange(m_hContext,0,&m_rgscState,1 );
		m_rgscState.dwCurrentState	=	m_rgscState.dwEventState;
	}
	
	m_ioRequest.dwProtocol = 0;
	LONG lReturn = SCardConnect( m_hContext, 
		sReader,
		nShareMode,
		SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1,
		&m_hCard,
		&m_ioRequest.dwProtocol );
	
	if ( SCARD_S_SUCCESS != lReturn )
	{
		m_hCard = 0;
		
		UNLOCK();
		return false;
	}
	
	if(lstrlen(m_rgscState.szReader) != 0)
	{
		//��ֹ���ӡ��Ͽ��������µ�״̬�仯
		if(SCardGetStatusChange(m_hContext,0,&m_rgscState,1 ) != SCARD_E_TIMEOUT)
		{
			m_rgscState.dwCurrentState	=	m_rgscState.dwEventState;
		}
	}
	
	UNLOCK();
	return true;
}


void CPCSC::DisConnectCard()
{
	LOCK();	
	if(m_hCard)
	{
		::SCardDisconnect(m_hCard,SCARD_EJECT_CARD);	
	}

	m_hCard		=	0;
	m_nPINStatus	=	STATUS_UNWAKE;

	//��ֹ���ӡ��Ͽ��������µ�״̬�仯
	if(lstrlen(m_rgscState.szReader) != 0)
	{		
		SCardGetStatusChange(m_hContext,0,&m_rgscState,1 );
		m_rgscState.dwCurrentState	=	m_rgscState.dwEventState;
	}
	UNLOCK();
}




bool CPCSC::SELECT(IN WORD wFileID)
{
	//��֯Select�����APDUָ��
	BYTE	ucCmd[]	=	{0xa0,0xa4,0x00,0x00,0x02,HIBYTE(wFileID),LOBYTE(wFileID)};
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;

	//����APDUָ��
	if(!RunAPDU(ucCmd,sizeof(ucCmd),ucRes,dwlen))
	{
		return false;
	}
	
	//У�鷵��ֵ
	//if the selected file is the MF or a DF: fileID,total memory space available,CHV status and other GSM specific data.
	//if the selected file is the EF: fileID,file size,access condition,invalidated/not invalidated indicator,structure
	//of EF and length of the records in case of linear fixed structure or cyclic structure.

	//return value:
	//90 00 normal ending
	//91 xx normal ending, with extra information
	//9f xx length xx of the response data
	//9f 00 sim application toolkit is busy,command cant be executed at present
	return SW_GSM();
}



bool CPCSC::STATUS()
{
	
	//��֯Status�����APDUָ��
	BYTE	ucCmd[]	=	{0xa0,0xf2,0x00,0x00,0x00};
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,sizeof(ucCmd),ucRes,dwlen))
	{
		return false;
	}
	
	return SW_GSM();
}





bool CPCSC::READ_BINARY(OUT BYTE* ucFile, IN BYTE byLen, IN WORD wOffset)
{
	//��֯READ BINARY�����APDUָ��
	BYTE	ucCmd[]	=	{0xa0,0xb0,HIBYTE(wOffset),LOBYTE(wOffset),byLen};
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,sizeof(ucCmd),ucRes,dwlen))
	{
		return false;
	}
	
	//�������ֵ
	memcpy(ucFile,ucRes,dwlen-2);

	return SW_GSM();
}





bool CPCSC::UPDATE_BINARY(IN BYTE* ucFile, IN BYTE byLen, IN WORD wOffset)
{
	//��֯UPDATE BINARY�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0xd6;
	ucCmd[2]	=	HIBYTE(wOffset);
	ucCmd[3]	=	LOBYTE(wOffset);
	ucCmd[4]	=	byLen;
	memcpy(&ucCmd[5],ucFile,byLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,byLen+5,ucRes,dwlen))
	{
		return false;
	}
	
	return SW_GSM();
}





bool CPCSC::READ_RECORD(IN BYTE byRecNo, OUT BYTE* ucData, IN BYTE byRecLen)
{
	//��֯READ RECORD�����APDUָ��
	BYTE	ucCmd[]	=	{0xa0,0xb2,byRecNo+1,0x04,byRecLen};
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,sizeof(ucCmd),ucRes,dwlen))
	{
		return false;
	}
	
	//�������ֵ
	memcpy(ucData,ucRes,dwlen-2);
	return SW_GSM();
}





bool CPCSC::UPDATE_RECORD(IN BYTE byRecNo,IN BYTE* ucData, IN BYTE byRecLen)
{
	//��֯UPDATE RECORD�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0xdc;
	ucCmd[2]	=	byRecNo+1;
	ucCmd[3]	=	0x04;
	ucCmd[4]	=	byRecLen;
	memcpy(&ucCmd[5],ucData,byRecLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,byRecLen+5,ucRes,dwlen))
	{
		return false;
	}
	
	return SW_GSM();
}





bool CPCSC::SEEK(IN BYTE* Patten, IN BYTE byPattenLen)
{
	//��֯SEEK�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0xa2;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	0x00;	//mode 1, from beging
	ucCmd[4]	=	byPattenLen;
	memcpy(&ucCmd[5],Patten,byPattenLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,byPattenLen+5,ucRes,dwlen))
	{
		return false;
	}
	
	//У�鷵��ֵ
	return SW_GSM();
}


bool CPCSC::GET_RESPONSE(OUT BYTE* ucResp, IN BYTE byResLen)
{
	//��֯GET RESPONSE�����APDUָ��
	BYTE	ucCmd[]	=	{0xa0,0xc0,0x00,0x00,byResLen};
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,sizeof(ucCmd),ucRes,dwlen))
	{
		return false;
	}
	
	//�������ֵ
	memcpy(ucResp,ucRes,dwlen-2);
	return SW_GSM();
}






bool CPCSC::VERIFY_CHV(IN BYTE byChvNo, IN const BYTE* ucChv, IN BYTE byChvLen)
{
	//��֯VERIFY CHV�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	memset(ucCmd,0xff,MAX_PATH);	//����Ĵ���0xff���
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0x20;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	byChvNo;
	ucCmd[4]	=	0x08;
	memcpy(&ucCmd[5],ucChv,byChvLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,13,ucRes,dwlen))
	{
		return false;
	}
	
	return SW_GSM();
}




bool CPCSC::CHANGE_CHV(IN BYTE byChvNo, IN const BYTE* ucOldChv, IN const BYTE* ucChv, IN BYTE byOldChvLen, IN BYTE byChvLen)
{
	//��֯VERIFY CHV�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	memset(ucCmd,0xff,MAX_PATH);	//����Ĵ���0xff���
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0x24;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	byChvNo;
	ucCmd[4]	=	0x10;
	memcpy(&ucCmd[5],ucOldChv,byOldChvLen);
	memcpy(&ucCmd[13],ucChv,byChvLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,21,ucRes,dwlen))
	{
		return false;
	}
	
	//У�鷵��ֵ
	return SW_GSM();
}





bool CPCSC::UNBLOCK_CHV(IN BYTE byChvNo, IN const BYTE* ucPuk, IN const BYTE* ucChv, IN BYTE byPukLen, IN BYTE byChvLen)
{
	//��֯VERIFY CHV�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	memset(ucCmd,0xff,MAX_PATH);	//����Ĵ���0xff���
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0x2c;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	byChvNo;
	ucCmd[4]	=	0x10;
	
	BYTE	puk[8];
	BYTE	chv[8];
	
	memset(puk,0xff,8);
	memset(chv,0xff,8);
	
	memcpy(puk,ucPuk,byPukLen);
	memcpy(chv,ucChv,byChvLen);

	memcpy(&ucCmd[5],puk,8);
	memcpy(&ucCmd[13],chv,8);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;

	if(!RunAPDU(ucCmd, 21, ucRes, dwlen))
	{
		return	false;
	}

	return SW_GSM();
}




bool CPCSC::DISABLE_CHV(IN const BYTE* ucChv,IN BYTE byChvLen)
{
	//��֯VERIFY CHV�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	memset(ucCmd,0xff,MAX_PATH);	//����Ĵ���0xff���
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0x26;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	0x01;
	ucCmd[4]	=	0x08;
	memcpy(&ucCmd[5],ucChv,byChvLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,13,ucRes,dwlen))
	{
		return false;
	}
	
	//У�鷵��ֵ
	return SW_GSM();
}


bool CPCSC::ENABLE_CHV(IN const BYTE* ucChv,IN BYTE byChvLen)
{
	//��֯VERIFY CHV�����APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	memset(ucCmd,0xff,MAX_PATH);	//����Ĵ���0xff���
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0x28;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	0x01;
	ucCmd[4]	=	0x08;
	memcpy(&ucCmd[5],ucChv,byChvLen);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,13,ucRes,dwlen))
	{
		return false;
	}
	
	//У�鷵��ֵ
	return SW_GSM();
}





bool CPCSC::RunAPDU(IN unsigned char* ucCmd, IN DWORD nCmdLen, OUT unsigned char* ucRes, IN OUT DWORD& nResLen)
{
	//�ж϶������Ƿ�����
	if(!m_hCard)
	{
		return false;
	}
	
	//������������
	LOCK();
	
	if(::SCardTransmit(m_hCard,&m_ioRequest,ucCmd,nCmdLen,NULL,ucRes,&nResLen)	!=	SCARD_S_SUCCESS)
	{
		UNLOCK();
		return false;
	}
	UNLOCK();

	if(nResLen < 2)
	{
		return false;
	}

	m_bySW1 = ucRes[nResLen-2];
	m_bySW2 = ucRes[nResLen-1];

	return true;
}

bool CPCSC::Init(const char* szReader)
{
	if(m_hContext == 0)
	{
		if(SCardEstablishContext(SCARD_SCOPE_USER,NULL,NULL,&m_hContext) != SCARD_S_SUCCESS)
		{
			return false;
		}
	}
	
	LOCK();
	m_rgscState.dwCurrentState	=	0;
	m_rgscState.szReader		=	m_szReader;
	m_szReader[0]				=	0;

	if(szReader != NULL)
	{
		lstrcpy(m_szReader,szReader);
		
		if (SCARD_S_SUCCESS != SCardGetStatusChange(m_hContext,0, &m_rgscState,1))
		{
			//ϵͳû�м�⵽������
			UNLOCK();
			return false;
		}
	}
	else
	{
		m_szReader[0]			=	0;
	}
	
	m_rgscState.dwCurrentState	=	m_rgscState.dwEventState;

	UNLOCK();
	return true;
}

void CPCSC::Eject()
{
	if(m_hContext)
	{
		::SCardReleaseContext(m_hContext);
		m_hContext	=	0;
	}
}


bool CPCSC::RUN_PIM_ALGORITHM(const BYTE* sRand)
{
	//��֯APDUָ��
	BYTE	ucCmd[MAX_PATH]	=	"";
	memset(ucCmd,0xff,MAX_PATH);	//����Ĵ���0xff���
	ucCmd[0]	=	0xa0;
	ucCmd[1]	=	0x80;
	ucCmd[2]	=	0x00;
	ucCmd[3]	=	0x00;
	ucCmd[4]	=	0x08;
	memcpy(&ucCmd[5],sRand,8);
	
	BYTE	ucRes[MAX_PATH];
	DWORD	dwlen	=	MAX_PATH;
	
	//����APDUָ��
	if(!RunAPDU(ucCmd,13,ucRes,dwlen))
	{
		return false;
	}
	
	//У�鷵��ֵ
	return SW_GSM();
}



bool CPCSC::IsConnc()
{
	if(m_hCard == NULL)
	{
		return false;
	}

	if(m_rgscState.cbAtr == 0)
	{
		m_hCard = NULL;
		return false;
	}

	return true;
}



LONG CPCSC::GetReaderStatusChange(int milseconds)
{
	LONG res = SCARD_E_TIMEOUT;

	LOCK();
	if(lstrlen(m_rgscState.szReader) != 0)
	{
		res = SCardGetStatusChange(m_hContext,milseconds,&m_rgscState,1 );
		m_rgscState.dwCurrentState	=	m_rgscState.dwEventState;
	}
	UNLOCK();

	return res;
}

/*
If the first octet in the alpha string is '80', then 
the remaining octets are 16 bit UCS2 characters, with 
the more significant octet (MSO) of the UCS2 character 
coded in the lower numbered octet of the alpha field, 
and the less significant octet (LSO) of the UCS2 character 
is coded in the higher numbered alpha field octet, i.e. 
octet 2 of the alpha field contains the more significant 
octet (MSO) of the first UCS2 character, and octet 3 of 
the alpha field contains the less significant octet (LSO) 
of the first UCS2 character (as shown below).  Unused 
octets shall be set to 'FF', and if the alpha field is 
an even number of octets in length, then the last (unusable) 
octet shall be set to 'FF'.

 If the first octet of the alpha string is set to '81', 
 then the second octet contains a value indicating the 
 number of characters in the string, and the third octet 
 contains an 8 bit number which defines bits 15 to 8 of 
 a 16 bit base pointer, where bit 16 is set to zero, and 
 bits 7 to 1 are also set to zero.  These sixteen bits 
 constitute a base pointer to a "half-page" in the UCS2 
 code space, to be used with some or all of the remaining 
 octets in the string. The fourth and subsequent octets in 
 the string contain codings as follows; if bit 8 of the 
 octet is set to zero, the remaining 7 bits of the octet 
 contain a GSM Default Alphabet character, whereas if bit 
 8 of the octet is set to one, then the remaining seven 
 bits are an offset value added to the 16 bit base pointer
 defined earlier, and the resultant 16 bit value is a UCS2 
 code point, and completely defines a UCS2 character.

  If the first octet of the alpha string is set to '82', 
  then the second octet contains a value indicating the 
  number of characters in the string, and the third and fourth
  octets contain a 16 bit number which defines the complete 16 
  bit base pointer to a "half-page" in the UCS2 code space, for 
  use with some or all of the remaining octets in the string. 
  The fifth and subsequent octets in the string contain codings 
  as follows; if bit 8 of the octet is set to zero, the remaining 
  7 bits of the octet contain a GSM Default Alphabet character, 
  whereas if bit 8 of the octet is set to one, the remaining seven 
  bits are an offset value added to the base pointer defined in 
  octets three and four, and the resultant 16 bit value is a UCS2 
  code point, and defines a UCS2 character.
 */
bool CPCSC::ReadADN(BYTE *ucADN, int nXLen,char* szMobile,char* szName)
{
	//�ж��ַ���
	//��
	szName[0]	= 0x00;

	//�ж�Name
	switch(ucADN[0]) {
	case 0xff:
		{
			
		}
		break;
	case 0x80:
		{
			//UCS2
			//��������
			int len = 0;

			for(int i=1;i<nXLen;i=i+2)
			{
				if(ucADN[i]==0xff)
				{
					CEnCode::DecodeUCS2(ucADN+1,szName,i-1);
					break;
				}
			}
		}
		break;
	case 0x81:
		{
			int len =	ucADN[1];
			BYTE		ucTmp[2];
			CHAR		szTmp[10];
			
			//UNICODE Pad
			ucTmp[0] = ucADN[2]>>1;

			for(int i=0;i<len;i++)
			{
				if(ucADN[i+3] & 0x80)
				{
					//UNICODE
					ucTmp[1] = (ucADN[2]<<7) + (ucADN[i+3]&0x7F);
					CEnCode::DecodeUCS2(ucTmp,szTmp,2);
					lstrcat(szName,szTmp);
				}
				else
				{
					//Alpha
					szTmp[0] = ucADN[i+3];
					szTmp[1] = 0x00;
					lstrcat(szName,szTmp);
				}
			}
		}
		break;
	case 0x82:
		{
			int len =	ucADN[1];
			BYTE		ucTmp[2];
			CHAR		szTmp[10];
			
			//Unicode Pad
			ucTmp[0] = ucADN[2];

			for(int i=0;i<len;i++)
			{
				if(ucADN[i+4] & 0x80)
				{
					//Unicode
					ucTmp[1] = ucADN[3] + (ucADN[i+4]&0x7F);
					CEnCode::DecodeUCS2(ucTmp,szTmp,2);
					lstrcat(szName,szTmp);
				}
				else
				{
					//Alpha
					szTmp[0] = ucADN[i+4];
					szTmp[1] = 0x00;
					lstrcat(szName,szTmp);
				}
			}
		}
		break;
	default:
		{
			for(int i=0;i<nXLen;i++)
			{
				if(ucADN[i]==0xff)
				{
					break;
				}
				szName[i] = ucADN[i];
			}
			szName[i] = 0x00;
		}
		break;
	}

	//�жϺ���
	szMobile[0] = 0x00;

	if(ucADN[nXLen]==0x00 || ucADN[nXLen]==0x01 || ucADN[nXLen] ==0xff)
	{
		return true;
	}
	
	//�Ƿ��й��ʱ�־
	if(ucADN[nXLen+1]==0x91)
	{
		szMobile[0]='+';
		CEnCode::Bytes2String(ucADN+(nXLen+2),szMobile+1,10);
		CEnCode::StrReverse(szMobile+1);
	}
	else
	{
		CEnCode::Bytes2String(ucADN+(nXLen+2),szMobile,10);
		CEnCode::StrReverse(szMobile);
	}
	
	//Ѱ��F���ص�
	for(int i=0;szMobile[i];i++)
	{
		if(szMobile[i]=='F')
		{
			szMobile[i] = 0x00;
			break;
		}
	}
	
	return true;
}


bool CPCSC::WriteADN(BYTE *ucADN, int nXLen,char* szMobile,char* szName)
{
	//
	BYTE ucTmp[255];
	int	 nTmp;
	bool	bUCS2 = false;

	//��0xff�����ڴ�
	memset(ucADN,0xff,nXLen+14);


	//Name
	//��ת����UCS2�����ж��Ƿ�������
	nTmp = CEnCode::EncodeUCS2(szName,ucTmp,lstrlen(szName));
	for(int i=0;i<nTmp/2;i++)
	{
		if(ucTmp[2*i] != 0x00)
		{
			//������
			bUCS2 = true;
			break;
		}
	}

	if(bUCS2)
	{
		//��80�ķ�ʽд��
		if(nTmp>nXLen-1)
		{
			nTmp	=	((nXLen-1)/2)*2;
		}
		ucADN[0]=0x80;
		memcpy(ucADN+1,ucTmp,nTmp);
	}
	else
	{
		nTmp = lstrlen(szName);
		if(nTmp>nXLen)
		{
			nTmp = nXLen;
		}
		memcpy(ucADN,szName,nTmp);
	}

	//����
	char szPhone[30];
	
	//TON NPI
	if(szMobile[0] == '+')
	{
		
		ucADN[nXLen+1] = 0x91;
		lstrcpy(szPhone,szMobile+1);
	}
	else
	{
		ucADN[nXLen+1] = 0x81;
		lstrcpy(szPhone,szMobile);
	}

	//�жϺ��볤��
	//���볤�Ȳ��ܳ���20
	int nPhone = lstrlen(szPhone);
	if(nPhone>20)
	{
		nPhone = 20;
	}

	//�������Ϊ������F
	if(nPhone%2)
	{
		lstrcat(szPhone,"F");
		nPhone ++;
	}

	//����
	ucADN[nXLen] = nPhone/2 + 1;
	
	//��ת
	CEnCode::StrReverse(szPhone);
	CEnCode::String2Bytes(szPhone,ucADN+(nXLen+2),nPhone);

	return true;
}


//0Ϊ�ռ�¼
int CPCSC::ReadSMS_SIM(BYTE *ucADN,char* szPhone,char* szMsg,SYSTEMTIME& time)
{
	int status  =	0;
	szPhone[0]	=	0;
	szMsg[0]	=	0;
	memset(&time,0x00,sizeof(SYSTEMTIME));
	
	//�жϵ�һ��byte��������������
	switch(ucADN[0]) 
	{
	case 0x01:		//Deliver-Msg read (�Ѷ�)
	case 0x03:		//Deliver-Msg to be read (δ��)
	case 0x07:		//Submit-Msg to be send.(����)
		{
			status = ucADN[0];
		}
		break;
	case 0x05:		//Submit-Msg message send to the network, status not request
	case 0x0D:		//status request, but not recieved
	case 0x15:		//status request, recived but not saved in EF-SMSR
	case 0x1D:		//status request, recived and store in EF-SMSR
		{
			status = 0x05;	//(�Է���)
		}
		break;
	default:
		{
			return 0;
		}
	}
	
	int noffset = 1;

	//Short Msg Center	
	noffset += ucADN[noffset];	//L
	//TON-NPI 1 byte
	//����Ϣ���ĺ���ռλ
	noffset ++;					//L��ռ�õ�λ��

	//TP-MTI:TP-MMS:TP-SRI:TP-UDHI:TP-RP
	//TP-MTI:TP-RD:TP-VPF:TP-SRR:TP-UDHI:TP-RP
	BYTE byVPF = ucADN[noffset];
	noffset ++;	

	if(status != 0x01 && status != 0x03)
	{
		//TP-MR
		noffset ++;
	}
	
	BYTE nDA = ucADN[noffset];	//TP-DA-->L
	noffset ++;					//L��ռ�õ�λ��
	BYTE nTag = ucADN[noffset];	//NPI
	noffset ++;					//NPIռ�õ�λ��
	
	if(nDA)
	{
		if(nTag == 0x91)
		{
			szPhone[0] = '+';
			CEnCode::Bytes2String(ucADN+noffset,szPhone+1,(nDA+1)/2);
			CEnCode::StrReverse(szPhone+1);
			//ȥ��F
			szPhone[nDA+1] = 0x00;
		}
		else
		{
			CEnCode::Bytes2String(ucADN+noffset,szPhone,(nDA+1)/2);
			CEnCode::StrReverse(szPhone);
			//ȥ��F
			szPhone[nDA] = 0x00;
		}
	}
	noffset += (nDA+1)/2;		//TP-DA
	
	noffset	++;					//TP-PID
	
	BYTE nDCS = ucADN[noffset];	//TP-DCS
	noffset ++;
	
	
	if(status != 0x01 && status != 0x03)
	{
		//VP
		switch(byVPF & 0x18)  
		{
		case 0x00:	//not provide vp
			break;
		case 0x10:	//relative mode 1 byte
			noffset ++;
			break;
		default:	//enhance mode & absolute mode - 7 byte
			noffset +=7;
		}

	}
	else
	{
		//SCTS
		time.wYear	=	TMEBYTE(ucADN[noffset]);	//TP-SCTS year
		noffset ++;
		time.wMonth =	TMEBYTE(ucADN[noffset]);	//TP-SCTS month
		noffset ++;
		time.wDay	=	TMEBYTE(ucADN[noffset]);	//TP-SCTS day
		noffset ++;
		time.wHour	=	TMEBYTE(ucADN[noffset]);	//TP-SCTS hour
		noffset ++;
		time.wMinute=	TMEBYTE(ucADN[noffset]);	//TP-SCTS minute
		noffset ++;
		time.wSecond=   TMEBYTE(ucADN[noffset]);	//TP-SCTS secon
		noffset ++;
		noffset	++;									//TP-SCTS time zone
	}
	
	
	BYTE nUDL	=	ucADN[noffset];
	noffset ++;
	
	//����������й�
	if(nUDL)
	{
		switch(nDCS) 
		{
		case 0x00:
			{
				//7 bit ѹ������
				CEnCode::Decode7bit(ucADN+noffset,szMsg,nUDL);
			}
			break;
		case 0x08:
			{
				//UCS2 ����
				CEnCode::DecodeUCS2(ucADN+noffset,szMsg,nUDL);
			}
			break;
		default:
			{
				//8 bit ����
				memcpy(ucADN+noffset,szMsg,nUDL);
				szMsg[nUDL]=0x00;
			}
		}
	}
	
	
	return status;
}

int CPCSC::ReadSMS_PIM(BYTE *ucADN,char* szPhone,char* szMsg,SYSTEMTIME& time)
{
	int status  =	0;
	szPhone[0]	=	0;
	szMsg[0]	=	0;
	memset(&time,0x00,sizeof(SYSTEMTIME));
	
	//�жϵ�һ��byte��������������
	switch(ucADN[0]) 
	{
	case 0x01:		//Deliver-Msg read (�Ѷ�)
	case 0x03:		//Deliver-Msg to be read (δ��)
	case 0x07:		//Submit-Msg to be send.(����)
		{
			status = ucADN[0];
		}
		break;
	case 0x05:		//Submit-Msg message send to the network, status not request
	case 0x0D:		//status request, but not recieved
	case 0x15:		//status request, recived but not saved in EF-SMSR
	case 0x1D:		//status request, recived and store in EF-SMSR
		{
			status = 0x05;	//(�Է���)
		}
		break;
	default:
		{
			return 0;
		}
	}
	
	int noffset = 1;

	//2-8��byte :��ʱ���
	time.wYear	=	TMEBYTE(ucADN[noffset]);	//TP-SCTS year
	noffset ++;
	time.wMonth =	TMEBYTE(ucADN[noffset]);	//TP-SCTS month
	noffset ++;
	time.wDay	=	TMEBYTE(ucADN[noffset]);	//TP-SCTS day
	noffset ++;
	time.wHour	=	TMEBYTE(ucADN[noffset]);	//TP-SCTS hour
	noffset ++;
	time.wMinute=	TMEBYTE(ucADN[noffset]);	//TP-SCTS minute
	noffset ++;
	time.wSecond=   TMEBYTE(ucADN[noffset]);	//TP-SCTS secon
	noffset ++;
	noffset	++;									//TP-SCTS time zone

	//���С�����
	noffset ++;
	
	//�绰���볤��
	BYTE nphone = ucADN[noffset]>33 ? 33:ucADN[noffset];
	nphone -= 1;
	//TON NPI
	noffset +=2;

	//Phone
	memcpy(szPhone,ucADN+noffset,nphone);
	szPhone[nphone] = 0x00;
	noffset += nphone;
	
	//�̶���7��byte
	noffset	+= 7;

	BYTE nlen = ucADN[noffset];		//����
	noffset ++;

	noffset ++;						//Ӧ��ʽ

	BYTE ndcs = ucADN[noffset];		//���뷽ʽ
	noffset ++;

	noffset ++;						//������

	if(ndcs)
	{
		noffset += 4;
		nlen -= 7;
	}
	else
	{
		nlen -= 3;
	}

	memcpy(szMsg,ucADN+noffset,nlen);
	szMsg[nlen] = 0x00;
	
	return status;

}


int CPCSC::ReadSMS_UIM(BYTE *ucADN,char* szPhone,char* szMsg,SYSTEMTIME& time)
{
	//��һ��byteΪstatus
	int status  =	0;
	szPhone[0]	=	0;
	szMsg[0]	=	0;
	memset(&time,0x00,sizeof(SYSTEMTIME));
	
	//�жϵ�һ��byte��������������
	switch(ucADN[0]) 
	{
	case 0x01:		//Deliver-Msg read (�Ѷ�)
	case 0x03:		//Deliver-Msg to be read (δ��)
	case 0x07:		//Submit-Msg to be send.(����)
		{
			status = ucADN[0];
		}
		break;
	case 0x05:		//Submit-Msg message send to the network, status not request
	case 0x0D:		//status request, but not recieved
	case 0x15:		//status request, recived but not saved in EF-SMSR
	case 0x1D:		//status request, recived and store in EF-SMSR
		{
			status = 0x05;	//(�Է���)
		}
		break;
	default:
		{
			return 0;
		}
	}
	
	/*
	 
	MSG_LEN(1 byte)
	SMS_MSG_TYPE(1 byte)
			0x00	SMS Point-to-Point
			0x01	SMS Broadcast
			0x02	SMS Acknowledge
	*/
	int nMsgLen = ucADN[1];
	if(nMsgLen==0xff)
	{
		//�������Ϣ
		return 0;
	}
	//ȥ��MSG_TYPEλռ�ĳ���
	nMsgLen +=2 ;
	
	for(int noffset =3;noffset<nMsgLen;)
	{
		BYTE byTag,byLen;

		//���TLV
		byTag = ucADN[noffset];
		noffset ++;
		byLen = ucADN[noffset];
		noffset ++;
		
		//�ж�T�ṹ
		if(byTag == 0x02 || byTag == 0x04)
		{
			//Originating Address
			if((ucADN[noffset] & 0xC0) == 0x00)
			{
				//4 bit ����
				//DTMS
				BYTE nPhone = ucADN[noffset]<<2 | ucADN[noffset+1]>>6;
				for(int i=0,j=0;j<nPhone;i++)
				{
					szPhone[j++] = (((ucADN[noffset + (i+1)]>>2)&0x0F)%10)+'0';
					szPhone[j++] = (((ucADN[noffset + (i+1)]<<2 | ucADN[noffset + (i+2)]>>6)&0x0F)%10)+'0';

					//szPhone[i] = (i%2) ? (((ucADN[noffset + i]<<2 | ucADN[noffset + (i+1)]>>6)&0x0F)%10)+'0' : (((ucADN[noffset+(i+1)]>>2)&0x0F)%10)+'0';
				}
				szPhone[nPhone] = 0x00;
			}
			else
			if((ucADN[noffset] & 0xC0) == 0x80)
			{
				//8 bit ����
				//ASCII
				BYTE nPhone = ucADN[noffset+1]<<1 | ucADN[noffset+2]>>7;
				for(int i=0;i<nPhone;i++)
				{
					szPhone[i] = ucADN[noffset+2+i]<<1 | ucADN[noffset+3+i]>>7;
				}
				szPhone[nPhone] = 0x00;
			}
		}
		else
		if(byTag == 0x08)
		{
			BYTE byI = 0;
			while(byI<byLen)
			{
				//Bearer Data
				BYTE bySubTag,bySubLen;
				
				//Sub TLV
				bySubTag = ucADN[noffset+byI];
				byI ++;
				bySubLen = ucADN[noffset+byI];
				byI ++;
				
				if(bySubTag == 0x01)
				{
					//5��byte�� MSG_ENCODING
					BYTE byDCS = ucADN[noffset+byI]>>3;
					
					if(byDCS==0x04)
					{
						//UCS2
						BYTE usMSG[255];
						BYTE byMsg;
						//UCS2 ���ȣ�
						byMsg = ucADN[noffset+byI]<<5 | ucADN[noffset+byI+1]>>3;
						byMsg *=2;
						for(int i=0;i<byMsg;i++)
						{
							usMSG[i] = ucADN[noffset+byI+i+1]<<5 | ucADN[noffset+byI+i+2]>>3;
						}
						//ת���ɺ���
						CEnCode::DecodeUCS2(usMSG,szMsg,byMsg);
					}
					else if (byDCS==0x02)
					{					    
					    // 7bit packing
//						BYTE usMSG[255];
						BYTE byMsg;
						//7bit ����
						byMsg = ucADN[noffset+byI]<<5 | ucADN[noffset+byI+1]>>3;

						for(int i=0;i<byMsg;i++)
						{
						    BYTE j = ((5+(7*i))/8);
						    BYTE k = ((5+(7*i))%8);
						    
							WORD wTmp = MAKEWORD(ucADN[noffset+byI+j+2],ucADN[noffset+byI+j+1]);
							szMsg[i] = (wTmp >> (16-7-k))&0x7F;
							
						}
						szMsg[byMsg] = 0;
					}
					else if (byDCS==0x00)
					{
					    // 8bit
						BYTE byMsg;
						byMsg = ucADN[noffset+byI]<<5 | ucADN[noffset+byI+1]>>3;

						for(int i=0;i<byMsg;i++)
						{
							szMsg[i] = ucADN[noffset+byI+i+1]<<5 | ucADN[noffset+byI+i+2]>>3;
						}
						szMsg[byMsg] = 0;
					}
				}
				else
				if(bySubTag == 0x03)
				{
					//ʱ��
					time.wYear = TMEBYTEN(ucADN[noffset+byI]);
					time.wMonth= TMEBYTEN(ucADN[noffset+byI+1]);
					time.wDay  = TMEBYTEN(ucADN[noffset+byI+2]);
					time.wHour = TMEBYTEN(ucADN[noffset+byI+3]);
					time.wMinute=TMEBYTEN(ucADN[noffset+byI+4]);
					time.wSecond=TMEBYTEN(ucADN[noffset+byI+5]);
				}

				byI += bySubLen;
			}
		}

		noffset += byLen;
	}
	
	return status;
	
}
bool CPCSC::WriteSMS_PIM(BYTE* ucADN,int reclen, int type, char* szPhone,char* szMsg,SYSTEMTIME& time)
{
	//memset
	memset(ucADN,0xFF,reclen);

	//���볤��
	BYTE nphonelen = lstrlen(szPhone);
	if(nphonelen>33)
	{
		nphonelen = 33;
	}

	//���ݳ���
	BYTE ndatalen = lstrlen(szMsg);
	if(ndatalen > (reclen-(27+nphonelen)))
	{
		//�����еļ�¼����Ϊ��׼
		ndatalen = reclen-(27+nphonelen);
	}

	ucADN[0] = type;

	//��ʱ���
	ucADN[1] = RTMBYTE(time.wYear);
	ucADN[2] = RTMBYTE(time.wMonth);
	ucADN[3] = RTMBYTE(time.wDay);
	ucADN[4] = RTMBYTE(time.wHour);
	ucADN[5] = RTMBYTE(time.wMinute);
	ucADN[6] = RTMBYTE(time.wSecond);
	ucADN[7] = 0x80;
	
	//�̶�Ϊ����
	ucADN[8] = 0x70;
	ucADN[9] = nphonelen+1;
	
	//TON���̶�Ϊ81
	ucADN[10] = 0x81;

	//���ƺ���
	memcpy(ucADN+11,szPhone,nphonelen);

	int noffset = 11+nphonelen;
	
	//�̶�
	ucADN[noffset++] = 0x7E;

	//UUI��Ԫ�ĳ���
	ucADN[noffset++] = ndatalen + 13;

	//�̶���Ԫ
	ucADN[noffset++] = 0x43;
	ucADN[noffset++] = 0x01;
	ucADN[noffset++] = 0x04;
	ucADN[noffset++] = 0x81;
	ucADN[noffset++] = 0x7E;

	//�������ݵĳ���
	ucADN[noffset++] = ndatalen + 7;
	ucADN[noffset++] = 0x01;
	ucADN[noffset++] = 0x01;
	
	//����λ
	ucADN[noffset++] = 0x00;
	
	//�̶���Ԫ
	ucADN[noffset++] = 0x80;
	ucADN[noffset++] = 0x91;
	ucADN[noffset++] = 0x00;
	ucADN[noffset++] = 0x0D;

	//����Ϣ��
	memcpy(ucADN+noffset,szMsg,ndatalen);

	return true;
}

int CPCSC::GetAtr(unsigned char* usATR)
{
	if(m_hCard == 0)
	{
		return -1;

	}

	LPBYTE   pbAttr = NULL;

	DWORD    cByte = SCARD_AUTOALLOCATE;
	LONG     lReturn;
				
	lReturn = SCardGetAttrib(m_hCard,
							SCARD_ATTR_ATR_STRING,
							(LPBYTE)&pbAttr,
							&cByte);
	if ( SCARD_S_SUCCESS != lReturn )
	{
		return 0;
	}
	
	memcpy(usATR,pbAttr,cByte);

	// Free the memory when done.
	// hContext was set earlier by SCardEstablishContext
	SCardFreeMemory(m_hCard, pbAttr);
	
	return cByte;
}

bool CPCSC::SW_GSM()
{
	if((m_bySW1==0x90) && (m_bySW2==0x00))
	{
		return true;
	}
	if(m_bySW1==0x91)
	{
		return true;
	}
	if(m_bySW1==0x9F)
	{
		return true;
	}

	return false;
}

WORD CPCSC::RS_GetFileSize(const BYTE *usResponse)
{
	return MAKEWORD(usResponse[3],usResponse[2]);
}

BYTE CPCSC::RS_GetRecordLen(const BYTE *usResponse) 
{
	return usResponse[14];
}

BYTE CPCSC::RS_GetRecordNum(const BYTE *usResponse)
{
	return RS_GetFileSize(usResponse)/RS_GetRecordLen(usResponse);
}

CPCSC::FILE_TYPE CPCSC::RS_GetFileType(const BYTE *usResponse)
{
	return (FILE_TYPE)usResponse[6];
}

CPCSC::FILE_STRUCT CPCSC::RS_GetFileStruct(const BYTE *usResponse)
{
	return (FILE_STRUCT)usResponse[13];
}

CPCSC::PIN_STATUS CPCSC::RS_GetPinStatus(const BYTE*usResponse)
{
	if((usResponse[19]&0x0f)==0)
	{
		return CPCSC::STATUS_DEADLOCK;
	}
	if((usResponse[18]&0x0f)==0)
	{
		return CPCSC::STATUS_BLOCK;
	}
	if((usResponse[13]&0x80) == 0x00)
	{
		return CPCSC::STATUS_ENABLE;
	}
	else
	{
		return CPCSC::STATUS_DISABLE;
	}
}

int CPCSC::ListReaders(char* readers)
{
	LPTSTR	pmszReaders;
	
	DWORD           cch = SCARD_AUTOALLOCATE;
	
	// Retrieve the list the readers.
	// hSC was set by a previous call to SCardEstablishContext (during object creation).
	LONG lReturn = SCardListReaders(m_hContext,
		NULL,
		(LPTSTR)&pmszReaders,
		&cch );


	if (lReturn == SCARD_S_SUCCESS) 
	{
		memcpy(readers,pmszReaders,cch);
		SCardFreeMemory(m_hContext,pmszReaders);
		return cch;
	}
	else
	{
		return 0;
	}
}
