
// SCSampleDlg.h : 头文件
//

#pragma once

#define NUMBER_OF_READERS	5
#define NAME_LENGTH			100
#define MAX_INPUT			1024
#define MAX_OUTPUT			4000
#define MAX_RESPONSE		2000


#include "MyEdit.h"

// CSCSampleDlg 对话框
class CSCSampleDlg : public CDialogEx
{
// 构造
public:
	CSCSampleDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_SCSAMPLE_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON		m_hIcon;
	char			ReaderName[NUMBER_OF_READERS][NAME_LENGTH];	//读卡器名称
	LONG      lReturn;					//返回值
	CString		MessageBuffer;		//弹出对话框的内容
	LPTSTR     pmszReaders;		//获得的读卡器名称
	DWORD   cch;						//长度值
	short		ReaderCount;		//读卡器数量
	DWORD	ActiveProtocol;		//支持的协议
	long			ProtocolType;		//协议类型
	LPBYTE		pbRecvBuffer;		//返回值
	unsigned long	ResponseLength;			//返回值长度
	PBYTE	pInBuffer;					//发送给读卡器的指令
	UINT	EditLimitTextLen;		//EDIT文本的最大长度

	// 生成的消息映射函数
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
