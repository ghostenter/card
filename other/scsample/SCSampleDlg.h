
// SCSampleDlg.h : ͷ�ļ�
//

#pragma once

#define NUMBER_OF_READERS	5
#define NAME_LENGTH			100
#define MAX_INPUT			1024
#define MAX_OUTPUT			4000
#define MAX_RESPONSE		2000


#include "MyEdit.h"

// CSCSampleDlg �Ի���
class CSCSampleDlg : public CDialogEx
{
// ����
public:
	CSCSampleDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SCSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON		m_hIcon;
	char			ReaderName[NUMBER_OF_READERS][NAME_LENGTH];	//����������
	LONG      lReturn;					//����ֵ
	CString		MessageBuffer;		//�����Ի��������
	LPTSTR     pmszReaders;		//��õĶ���������
	DWORD   cch;						//����ֵ
	short		ReaderCount;		//����������
	DWORD	ActiveProtocol;		//֧�ֵ�Э��
	long			ProtocolType;		//Э������
	LPBYTE		pbRecvBuffer;		//����ֵ
	unsigned long	ResponseLength;			//����ֵ����
	PBYTE	pInBuffer;					//���͸���������ָ��
	UINT	EditLimitTextLen;		//EDIT�ı�����󳤶�

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	void GetErrorCode(long ret);
public:
	afx_msg void OnCbnSelchangeDriverName();
	CString m_driver_name_get;
	CString m_out;
	afx_msg void OnBnClickedConnect();
	CMyEdit m_Edit;
	CListCtrl m_listctrl;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnEnChangeEdit1();
	CString m_edit_value;
};
