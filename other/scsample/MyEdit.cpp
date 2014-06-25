// MyEdit.cpp : ʵ���ļ�
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



// CMyEdit ��Ϣ�������




void CMyEdit::OnDropFiles(HDROP hDropInfo)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	char szFileName[MAX_PATH];		//�ļ���
	int iFileNumber;		//�ļ�����

	// �õ���ק�����е��ļ�����
	iFileNumber = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);

	if(iFileNumber > 1){
		::SetDlgItemText(AfxGetApp()->m_pMainWnd->m_hWnd, IDC_OUT, "�ļ�����1����������1���ļ�");
	}else{
		// �õ��ļ���
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
