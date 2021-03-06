
// HDDecFileDlg.h: 头文件
//
#include "HDEdit.h"


#pragma once


// CHDDecFileDlg 对话框
class CHDDecFileDlg : public CDialogEx
{
// 构造
public:
	CHDDecFileDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_HDDECFILE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

// 函数定义
public:
	afx_msg void OnBnClickedButtonDec();
	afx_msg void OnBnClickedButtonQuit();
	int Encrypt(char *filename,  char *tfilename);
	void EncryptBufer(unsigned char* buffer, int len);
	void ClearShowWindow();
	void AddPassword();
	int GetFileMD5(char* szfile, char* szMD5Value);
	int GetPassWord(char* szMd5, char* szPassword);
	BOOL CheckPassWord(char* szPw1, char* szPw2);
	BOOL IfFileOrDirExist(const char* fileOrDirName);
	int JudgeFileDir(void);


// 变量定义
public:
	CString m_csPassword;
	CString m_csPassword1;
	CString m_csPassword2;
	CString m_csPassword3;
	CString m_csPassword4;
	CString m_csPassword5;
	CString m_csPassword6;
	CHDEdit m_ctpw1;
	CHDEdit m_ctpw2;
	CHDEdit m_ctpw3;
	CHDEdit m_ctpw4;
	CHDEdit m_ctpw5;
	CHDEdit m_ctpw6;

	CFont m_HDEditFont;
	CFont m_LTextFont;

	//CStatic m_ltextoption;

	CBitmapButton m_butdec;
	CBitmapButton m_butquit;
};
