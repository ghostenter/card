
// SCSampleDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SCSample.h"
#include "SCSampleDlg.h"
#include "afxdialogex.h"
#include "winscard.h"
#include "MyEdit.h"
#include "BaseFunction.cpp"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

SCARDCONTEXT		hSC;		//A handle to the established resource manager context
SCARDHANDLE		hCardHandle;		//A handle that identifies the connection to the smart card in the designated reader.
SCARD_IO_REQUEST	IO_Request;


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSCSampleDlg �Ի���




CSCSampleDlg::CSCSampleDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSCSampleDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_driver_name_get = _T("");
	m_out = _T("");
	m_edit_value = _T("");
}

void CSCSampleDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBString(pDX, IDC_DRIVER_NAME, m_driver_name_get);
	DDX_Text(pDX, IDC_OUT, m_out);
	DDX_Control(pDX, IDC_EDIT1, m_Edit);
	DDX_Control(pDX, IDC_LIST1, m_listctrl);
	DDX_Text(pDX, IDC_EDIT1, m_edit_value);
}

BEGIN_MESSAGE_MAP(CSCSampleDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_CBN_SELCHANGE(IDC_DRIVER_NAME, &CSCSampleDlg::OnCbnSelchangeDriverName)
	ON_BN_CLICKED(IDC_CONNECT, &CSCSampleDlg::OnBnClickedConnect)
	ON_BN_CLICKED(IDC_BUTTON1, &CSCSampleDlg::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_EDIT1, &CSCSampleDlg::OnEnChangeEdit1)
END_MESSAGE_MAP()


// CSCSampleDlg ��Ϣ�������

BOOL CSCSampleDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//��ʼ��LIST
	m_listctrl.InsertColumn(0,		"����",				LVCFMT_LEFT,  100);
	m_listctrl.InsertColumn(1,		"APDUָ��",		LVCFMT_LEFT,	100);
	m_listctrl.InsertColumn(2,		"����״ֵ̬",	LVCFMT_LEFT,	100);
	m_listctrl.InsertColumn(3,		"ʵ��״ֵ̬",	LVCFMT_LEFT,	100);
	m_listctrl.InsertColumn(4,		"������Ӧֵ",	LVCFMT_LEFT,	100);
	m_listctrl.InsertColumn(5,		"ʵ��״ֵ̬",	LVCFMT_LEFT,	100);
	m_listctrl.InsertColumn(6,		"�ȶԽ��",		LVCFMT_LEFT,	100);

	//�豸LISTCTRL����չ��񣬴��и��ӵ�����
	m_listctrl.SetExtendedStyle(LVS_EX_GRIDLINES);

	//���������Ƴ�ʼ��Ϊ��
	for(int i = 0; i < NUMBER_OF_READERS; i++){
		memset(ReaderName[i], 0, NAME_LENGTH);
	}
	pmszReaders = NULL;					//��õĶ���������
	cch = SCARD_AUTOALLOCATE;	//����ֵ
	ReaderCount = 0;							//ʵ�ʶ���������
	ActiveProtocol = 0;						//֧�ֵ�Э��
	ProtocolType = SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1;		//Э������
	pbRecvBuffer = (PBYTE) malloc( MAX_RESPONSE );				//�����ַ�����ʼ��
	memset(pbRecvBuffer, 0x00, MAX_RESPONSE);
	ResponseLength = MAX_RESPONSE;									//���ճ��ȳ�ʼ��
	IO_Request.dwProtocol = ProtocolType;								//���͸���������Э��ͷ��ʼ��
	IO_Request.cbPciLength = (DWORD) sizeof(SCARD_IO_REQUEST);
	pInBuffer = (PBYTE) malloc( MAX_INPUT );							//���͸���������ָ���ʼ��

	//����CEDIT�ı�����󳤶�
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_EDIT1);
	EditLimitTextLen = cedit->GetLimitText();
	cedit->SetLimitText(3000000);

	// Open a context which communication to the Resource Manager
	// establishes the resource manager context (the scope) within which database operations are performed
	lReturn = SCardEstablishContext(SCARD_SCOPE_USER, NULL, NULL, &hSC);
	if (lReturn != SCARD_S_SUCCESS) {
		MessageBuffer  = "The error is " + lReturn;
		MessageBox(MessageBuffer, NULL, MB_OK| MB_ICONSTOP);
		return 0;
	}

	//�г����еĶ�����
	lReturn = SCardListReaders(hSC, NULL, (LPTSTR)&pmszReaders, &cch );
	if (lReturn != SCARD_S_SUCCESS) {
		MessageBuffer  = "The error is " + lReturn;
		MessageBox(MessageBuffer, NULL, MB_OK| MB_ICONSTOP);
		return 0;
	}else {
		//������Ӷ�����
		unsigned int		StringLen = 0;
		m_driver_name_get = pmszReaders;
		while ( cch > StringLen+1) {
			strcpy(ReaderName[ReaderCount], (LPCTSTR)pmszReaders+StringLen);
			//DWORD	ActiveProtocol = 0;
			lReturn = SCardConnect(hSC, (LPCTSTR)ReaderName[ReaderCount], SCARD_SHARE_SHARED, 
				ProtocolType, &hCardHandle, &ActiveProtocol);
			if (lReturn != SCARD_E_UNKNOWN_READER)
				ReaderCount++;
			if (lReturn == SCARD_S_SUCCESS)
				SCardDisconnect(hCardHandle, SCARD_EJECT_CARD);
			StringLen += strlen((char *) pmszReaders+StringLen+1);
			StringLen += 2;
		}
	}

	//û���ҵ�������
	if (ReaderCount == 0) {
		MessageBox("No driver is available for use with the resource manager!", "SCSample", MB_ICONSTOP);
		return 0;
	}

	CComboBox *pbox1 = (CComboBox*)GetDlgItem(IDC_DRIVER_NAME);
	pbox1->ResetContent();
	for (int i=0; i<ReaderCount; i++)
		pbox1->InsertString(i, ReaderName[i]);                             
	pbox1->EnableWindow(TRUE);
	pbox1->SetCurSel(0);	    

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSCSampleDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSCSampleDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSCSampleDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSCSampleDlg::GetErrorCode(long ret)
{
	switch (ret) {
	case SCARD_E_CANCELLED:
		SetDlgItemText(IDC_OUT,"The action was cancelled by an SCardCancel request.");
		break;
	case SCARD_E_CANT_DISPOSE:
		SetDlgItemText(IDC_OUT,"The system could not dispose of the media in the requested manner.");
		break;
	case SCARD_E_CARD_UNSUPPORTED:
		SetDlgItemText(IDC_OUT,"The smart card does not meet minimal requirements for support.");
		break;
	case SCARD_E_DUPLICATE_READER:
		SetDlgItemText(IDC_OUT,"The reader driver didn't produce a unique reader name.");
		break;
	case SCARD_E_INSUFFICIENT_BUFFER:
		SetDlgItemText(IDC_OUT,"The data buffer to receive returned data is too small for the returned data.");
		break;
	case SCARD_E_INVALID_ATR:
		SetDlgItemText(IDC_OUT,"An ATR obtained from the registry is not a valid ATR string.");
		break;
	case SCARD_E_INVALID_HANDLE:
		SetDlgItemText(IDC_OUT,"The supplied handle was invalid.");
		break;
	case SCARD_E_INVALID_PARAMETER:
		SetDlgItemText(IDC_OUT,"One or more of the supplied parameters could not be properly interpreted.");
		break;
	case SCARD_E_INVALID_TARGET:
		SetDlgItemText(IDC_OUT,"Registry startup information is missing or invalid.");
		break;
	case SCARD_E_INVALID_VALUE:
		SetDlgItemText(IDC_OUT,"One or more of the supplied parameters?values could not be properly interpreted.");
		break;
	case SCARD_E_NOT_READY:
		SetDlgItemText(IDC_OUT,"The reader or card is not ready to accept commands.");
		break;
	case SCARD_E_NOT_TRANSACTED:
		SetDlgItemText(IDC_OUT,"An attempt was made to end a non-existent transaction.");
		break;
	case SCARD_E_NO_MEMORY:
		SetDlgItemText(IDC_OUT,"Not enough memory available to complete this command.");
		break;
	case SCARD_E_NO_SERVICE:
		SetDlgItemText(IDC_OUT,"The Smart card resource manager is not running.");
		break;
	case SCARD_E_NO_SMARTCARD:
		SetDlgItemText(IDC_OUT,"The operation requires a smart card but no smart card is currently in the device.");
		break;
	case SCARD_E_PCI_TOO_SMALL:
		SetDlgItemText(IDC_OUT,"The PCI Receive buffer was too small.");
		break;
	case SCARD_E_PROTO_MISMATCH:
		SetDlgItemText(IDC_OUT,"The requested protocols are incompatible with the protocol currently in use with the card.");
		break;
	case SCARD_E_READER_UNAVAILABLE:
		SetDlgItemText(IDC_OUT,"The specified reader is not currently available for use.");
		break;
	case SCARD_E_READER_UNSUPPORTED:
		SetDlgItemText(IDC_OUT,"The reader driver does not meet minimal requirements for support.");
		break;
	case SCARD_E_SERVICE_STOPPED:
		SetDlgItemText(IDC_OUT,"The Smart card resource manager has shut down.");
		break;
	case SCARD_E_SHARING_VIOLATION:
		SetDlgItemText(IDC_OUT,"The card cannot be accessed because of other connections outstanding.");
		break;
	case SCARD_E_SYSTEM_CANCELLED:
		SetDlgItemText(IDC_OUT,"The action was cancelled by the system presumably to log off or shut down.");
		break;
	case SCARD_E_TIMEOUT:
		SetDlgItemText(IDC_OUT,"The user-specified timeout value has expired.");
		break;
	case SCARD_E_UNKNOWN_CARD:
		SetDlgItemText(IDC_OUT,"The specified card name is not recognized.");
		break;
	case SCARD_E_UNKNOWN_READER:
		SetDlgItemText(IDC_OUT,"The specified reader name is not recognized.");
		break;
	case SCARD_F_COMM_ERROR:
		SetDlgItemText(IDC_OUT,"An internal communications error has been detected.");
		break;
	case SCARD_F_INTERNAL_ERROR:
		SetDlgItemText(IDC_OUT,"An internal consistency check failed.");
		break;
	case SCARD_F_UNKNOWN_ERROR:
		SetDlgItemText(IDC_OUT,"An internal error has been detected but the source is unknown.");
		break;
	case SCARD_F_WAITED_TOO_LONG:
		SetDlgItemText(IDC_OUT,"An internal consistency timer has expired.");
		break;
	case SCARD_S_SUCCESS:
		SetDlgItemText(IDC_OUT,"OK");
		break;
	case SCARD_W_REMOVED_CARD:
		SetDlgItemText(IDC_OUT,"The card has been removed so that further communication is not possible.");
		break;
	case SCARD_W_RESET_CARD:
		SetDlgItemText(IDC_OUT,"The card has been reset so any shared state information is invalid.");
		break;
	case SCARD_W_UNPOWERED_CARD:
		SetDlgItemText(IDC_OUT,"Power has been removed from the card so that further communication is not possible.");
		break;
	case SCARD_W_UNRESPONSIVE_CARD:
		SetDlgItemText(IDC_OUT,"The card is not responding to a reset.");
		break;
	case SCARD_W_UNSUPPORTED_CARD:
		SetDlgItemText(IDC_OUT,"The reader cannot communicate with the card due to ATR configuration conflicts.");
		break;
	default:
		MessageBuffer.Format("Function returned 0x%X error code.", ret);
		SetDlgItemText(IDC_OUT, MessageBuffer);
		break;
	}
}


void CSCSampleDlg::OnCbnSelchangeDriverName()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CComboBox* ccombobox = (CComboBox*)GetDlgItem(IDC_DRIVER_NAME);
	ccombobox->GetLBText(ccombobox->GetCurSel(), m_driver_name_get);
	CEdit* cedit = (CEdit*)GetDlgItem(IDC_OUT);
	SetDlgItemText(IDC_OUT, m_driver_name_get);
}


void CSCSampleDlg::OnBnClickedConnect()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	DWORD           cch = 200;
	DWORD           dwState, dwProtocol;
	BYTE					bAttr[32];
	DWORD           cByte = 32;
	char					chatr[64];
	char*				pchatr;

//ר�е�
// 	lReturn = SCardConnect(hSC, m_driver_name_get, 
// 		SCARD_SHARE_EXCLUSIVE, ProtocolType, &hCardHandle, &ActiveProtocol);

	//�����
	lReturn = SCardConnect(hSC, m_driver_name_get, 
		SCARD_SHARE_SHARED, ProtocolType, &hCardHandle, &ActiveProtocol);

	if (lReturn != SCARD_S_SUCCESS){
		GetErrorCode(lReturn);
		return;
	}
	ProtocolType = ActiveProtocol;
	switch (ProtocolType) {
	case SCARD_PROTOCOL_T0: 
		SetDlgItemText(IDC_OUT, "Function SCardConnect ok\r\nProtocoltype = T0");
		break;
	case SCARD_PROTOCOL_T1: 
		SetDlgItemText(IDC_OUT, "Function SCardConnect ok\r\nProtocoltype = T1");
		break;
	default:
		SetDlgItemText(IDC_OUT, "Function SCardConnect ok");
		break;
	}

	lReturn = SCardStatus(hCardHandle, m_driver_name_get.GetBuffer(m_driver_name_get.GetLength()),
		&cch, &dwState, &dwProtocol, (LPBYTE)&bAttr, &cByte);
	m_driver_name_get.ReleaseBuffer(-1);

	IO_Request.dwProtocol = dwProtocol;
	
	pchatr = (char*)bAttr;
	memset(chatr, 0, 64);
	EBCDToStr(pchatr, chatr, cByte);

	SetDlgItemText(IDC_ATR, chatr);

	if ( SCARD_S_SUCCESS != lReturn )
	{
		GetErrorCode(lReturn);
		return;
	}

//�Ͽ�����
	if (lReturn == SCARD_S_SUCCESS)
		SCardDisconnect(hCardHandle, SCARD_EJECT_CARD);

}


void CSCSampleDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	m_listctrl.DeleteAllItems();

	UpdateData(TRUE);

	CEdit* cedit = (CEdit*)GetDlgItem(IDC_EDIT1);
	int nrow;										//EDIT���ж�����
	int posfirst, poslast;						//�ı��Ŀ�ʼλ�������λ��
	int ncount;									//ÿ�к��е��ı��ַ���
	CStringArray cstrarray;					//���ڱ���ÿ���ı����ַ�
	int cstrarrycount;							//�������ı�����
	int m_edit_value_len;						//EDIT�к��ж��ٸ��ַ������а���"\r\n"

	int DataLen;									//���͸������������ݳ���
	char chAPDU[MAX_INPUT];			//���͸���������ָ��
	char	chAPDUbuf[MAX_INPUT/2];//ѹ�����ָ��

	CString ExpectSW;						//����״ֵ̬
	CString ActualSW;						//ʵ��״ֵ̬
	CString ExpectResponse;				//������Ӧֵ
	CString ActualResponse;				//ʵ����Ӧֵ
	BYTE		chActualSW[5];
	BYTE     chActualResponse[256 * 2];


	//--------------------------------------������ʼ����ʼ-----------------------------
	posfirst = 0;
	poslast = 0;
	nrow =0;
	ncount = 0;
	cstrarrycount = 0;
	DataLen = 0;
	memset(chAPDU, 0, MAX_INPUT);
	memset(chAPDUbuf, 0, MAX_INPUT/2);
	memset(chActualSW, 0, 5);
	memset(chActualResponse, 0, 256 * 2);

	lReturn = SCardConnect(hSC, m_driver_name_get, 
		SCARD_SHARE_SHARED, ProtocolType, &hCardHandle, &ActiveProtocol);

	IO_Request.dwProtocol = ActiveProtocol;

	if (lReturn != SCARD_S_SUCCESS){
		GetErrorCode(lReturn);
		return;
	}
	//--------------------------------------������ʼ������-----------------------------

	nrow = cedit->GetLineCount();
	m_edit_value_len = m_edit_value.GetLength();

	if(0 == nrow){
		return;
	}else if(1 == nrow){
		cstrarray.Add(m_edit_value);
				cstrarrycount = cstrarray.GetSize();
	}else{
		posfirst = 0;
		poslast = m_edit_value.Find("\r\n");
		ncount = poslast - posfirst;

		for(int i = 0; i < nrow; i++){
			cstrarray.Add(m_edit_value.Mid(posfirst, ncount));
			posfirst = poslast + 2;
			poslast = m_edit_value.Find("\r\n", posfirst -1);
			if(-1 == poslast){
				poslast = m_edit_value_len;
			}
			ncount = poslast - posfirst;
		}
		cstrarrycount = cstrarray.GetSize();
	}

	for(int j = 0; j < cstrarrycount; j++){
		CString strAPDU;									//ÿ�е��ַ���������"\r\n"
		strAPDU = cstrarray.GetAt(j);				//���ÿ���ַ�
		CString templen;
		templen.Format("%d", j + 1);

		strAPDU.Remove(' ');								//ȥ�����пո�
		strAPDU.Remove('	');

		if(strAPDU.IsEmpty()){
			//��������Ϊ����ʱ
			m_listctrl.InsertItem(j, templen.GetBuffer(templen.GetLength()));
			m_listctrl.SetItemText(j, 1, "����");
		}else if(';' == strAPDU[0]){
			//��������Ϊע����ʱ
			m_listctrl.InsertItem(j, templen.GetBuffer(templen.GetLength()));
			m_listctrl.SetItemText(j, 1, "ע����");
		}else if(".R_R" == strAPDU.Mid(0, 4) || ".r_r" == strAPDU.Mid(0, 4)){
			//��������Ϊ��λ��ʱ,������Ҫ��Ӹ�λ����
			m_listctrl.InsertItem(j, templen.GetBuffer(templen.GetLength()));
			m_listctrl.SetItemText(j, 1, "��λ");
		}else{
			//��������Ϊʵ��ָ��ʱ
			m_listctrl.InsertItem(j, templen.GetBuffer(templen.GetLength()));
			m_listctrl.SetItemText(j, 1, strAPDU);

			strAPDU.MakeUpper();				//����Ϊ��д
			DataLen = 0;

			//ȡ�÷��͸����������ݵĳ��ȣ�����ָ��ĵ�5���ֽھ�������16����ת��Ϊ10����
			//��1λת��
			if((strAPDU[8] <= '9') && (strAPDU[8] >= '0')){
				DataLen += (strAPDU[8] - '0') * 16;
			}else if((strAPDU[8] <= 'F') && (strAPDU[8] >= 'A')){
				DataLen += (strAPDU[8] - 'A' + 10) * 16;
			}else if((strAPDU[8] <= 'f') && (strAPDU[8] >= 'a')){
				 DataLen += (strAPDU[8] - 'a' + 10) * 16;
			}
			//��2λת��
			if((strAPDU[9] <= '9') && (strAPDU[9] >= '0')){
				DataLen += (strAPDU[9] - '0');
			}else if((strAPDU[9] <= 'F') && (strAPDU[9] >= 'A')){
				DataLen += (strAPDU[9] - 'A' + 10);
			}else if((strAPDU[9] <= 'f') && (strAPDU[9] >= 'a')){
				DataLen += (strAPDU[9] - 'a' + 10);
			}

			//�������״ֵ̬
			posfirst = 0;
			posfirst = strAPDU.Find("(") + 1;
			if(0 != posfirst){
				ExpectSW = strAPDU.Mid(posfirst, 4);
			}

			//���������Ӧֵ
			posfirst = 0;
			posfirst = strAPDU.Find("[") + 1;
			if(0 != posfirst){
				ExpectResponse = strAPDU.Mid(posfirst, 4);
			}

			sprintf(chAPDU , "%s", LPCTSTR(strAPDU));
			StrToEBCD(chAPDU, chAPDUbuf, (5 + DataLen) * 2);

			//�ж�ָ����𣬷�Ϊ����Ӧֵ��û����Ӧֵ���֣������������Ӧֵ��ָ���������
			if("B0" == strAPDU.Mid(2 , 2) || "B2" == strAPDU.Mid(2 , 2) || "C0" == strAPDU.Mid(2 , 2)){
				pInBuffer = (PBYTE) malloc( MAX_INPUT );
				memcpy(pInBuffer, chAPDUbuf, 5);
				lReturn = SCardTransmit(hCardHandle, &IO_Request, pInBuffer, 5,
					0, pbRecvBuffer, &ResponseLength);
				if ( SCARD_S_SUCCESS != lReturn )
				{
					GetErrorCode(lReturn);
					return;
				}
				EBCDToStr(pbRecvBuffer, chActualResponse, DataLen);
				m_listctrl.SetItemText(j, 4, ExpectResponse);
				ActualResponse.Format("%s", chActualResponse);
				m_listctrl.SetItemText(j, 5, ActualResponse);
			}else{
				memcpy(pInBuffer, chAPDUbuf, 5 + DataLen);
				lReturn = SCardTransmit(hCardHandle, &IO_Request, pInBuffer, 5 + DataLen,
					0, pbRecvBuffer, &ResponseLength);
				 if ( SCARD_S_SUCCESS != lReturn )
				{
				 	GetErrorCode(lReturn);
				 	return;
				}
				EBCDToStr(pbRecvBuffer, chActualSW, 2);
				m_listctrl.SetItemText(j, 2, ExpectSW);
				ActualSW.Format("%s", chActualSW);
				m_listctrl.SetItemText(j, 3, ActualSW);
			}
		}
		templen.ReleaseBuffer(-1);
		cedit->SetScrollPos(SB_VERT, j);
		cedit->LineScroll(1, 0);
		m_listctrl.Update(j);
		m_listctrl.EnsureVisible(j, FALSE);
	}

	//�Ͽ�����
	if (lReturn == SCARD_S_SUCCESS)
		SCardDisconnect(hCardHandle, SCARD_EJECT_CARD);
}



void CSCSampleDlg::OnEnChangeEdit1()
{
	// TODO:  ����ÿؼ��� RICHEDIT �ؼ���������
	// ���ʹ�֪ͨ��������д CDialogEx::OnInitDialog()
	// ���������� CRichEditCtrl().SetEventMask()��
	// ͬʱ�� ENM_CHANGE ��־�������㵽�����С�

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}
