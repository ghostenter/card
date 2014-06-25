// MyEdit.cpp : 实现文件
//

#include "stdafx.h"
#include "SCSample.h"
#include "MyEdit.h"


// CMyEdit

IMPLEMENT_DYNAMIC(CMyEdit, CEdit)

CMyEdit::CMyEdit()
{

}

CMyEdit::~CMyEdit()
{
}


BEGIN_MESSAGE_MAP(CMyEdit, CEdit)
	ON_WM_DROPFILES()
END_MESSAGE_MAP()



// CMyEdit 消息处理程序




void CMyEdit::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	char szFileName[MAX_PATH];		//文件名
	int iFileNumber;		//文件个数

	// 得到拖拽操作中的文件个数
	iFileNumber = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	if(iFileNumber > 1){
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd, IDC_OUT, "文件大于1个，请拖入1个文件");
	}else{
		// 得到文件名
		DragQueryFile(hDropInfo, 0, szFileName, MAX_PATH);
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd, IDC_OUT, szFileName);
		
		CStdioFile   file;
		if(!file.Open( szFileName,   CFile::modeRead))
		{
			AfxMessageBox( "can   not   open   file! ");
			return;
		}
		CString   strLine,   strEdit;
		char APDUBuf[1024000] = {'\0'};
		char* papdubuff;
		papdubuff = APDUBuf;
		strEdit = "";
		while(file.ReadString(strLine))
		{
			strEdit.Format("%s", APDUBuf);
			strEdit += strLine + "\r\n";
			strcat(APDUBuf, strLine.GetBuffer(strLine.GetLength()));
			strLine.ReleaseBuffer(-1);
			strcat(APDUBuf, "\r\n");
		}
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd, IDC_EDIT1, APDUBuf);
		file.Close();
	}

	CEdit::OnDropFiles(hDropInfo);
}
