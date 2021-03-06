
// HDDecFileDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "HDDecFile.h"
#include "HDDecFileDlg.h"
#include "afxdialogex.h"
#include "HDCryption.h"
#include "md5.h"
//#include "gdiplus.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CHDDecFileDlg 对话框
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

#define RL(x, y) (((x) << (y)) | ((x) >> (32 - (y))))  //x向左循环移y位

#define PP(x) (x<<24)|((x<<8)&0xff0000)|((x>>8)&0xff00)|(x>>24)  //将x高低位互换,例如PP(aabbccdd)=ddccbbaa

#define FF(a, b, c, d, x, s, ac) a = b + (RL((a + F(b,c,d) + x + ac),s))
#define GG(a, b, c, d, x, s, ac) a = b + (RL((a + G(b,c,d) + x + ac),s))
#define HH(a, b, c, d, x, s, ac) a = b + (RL((a + H(b,c,d) + x + ac),s))
#define II(a, b, c, d, x, s, ac) a = b + (RL((a + I(b,c,d) + x + ac),s))

unsigned A,B,C,D,a,b,c,d,i,len,flen[2],x[16];   //i临时变量,len文件长,flen[2]为64位二进制表示的文件初始长度
FILE *fp;

void md5(){                 //MD5核心算法,供64轮

	a=A,b=B,c=C,d=D;
	/**//* Round 1 */
	FF (a, b, c, d, x[ 0],  7, 0xd76aa478); /**//* 1 */
	FF (d, a, b, c, x[ 1], 12, 0xe8c7b756); /**//* 2 */
	FF (c, d, a, b, x[ 2], 17, 0x242070db); /**//* 3 */
	FF (b, c, d, a, x[ 3], 22, 0xc1bdceee); /**//* 4 */
	FF (a, b, c, d, x[ 4],  7, 0xf57c0faf); /**//* 5 */
	FF (d, a, b, c, x[ 5], 12, 0x4787c62a); /**//* 6 */
	FF (c, d, a, b, x[ 6], 17, 0xa8304613); /**//* 7 */
	FF (b, c, d, a, x[ 7], 22, 0xfd469501); /**//* 8 */
	FF (a, b, c, d, x[ 8],  7, 0x698098d8); /**//* 9 */
	FF (d, a, b, c, x[ 9], 12, 0x8b44f7af); /**//* 10 */
	FF (c, d, a, b, x[10], 17, 0xffff5bb1); /**//* 11 */
	FF (b, c, d, a, x[11], 22, 0x895cd7be); /**//* 12 */
	FF (a, b, c, d, x[12],  7, 0x6b901122); /**//* 13 */
	FF (d, a, b, c, x[13], 12, 0xfd987193); /**//* 14 */
	FF (c, d, a, b, x[14], 17, 0xa679438e); /**//* 15 */
	FF (b, c, d, a, x[15], 22, 0x49b40821); /**//* 16 */

	/**//* Round 2 */
	GG (a, b, c, d, x[ 1],  5, 0xf61e2562); /**//* 17 */
	GG (d, a, b, c, x[ 6],  9, 0xc040b340); /**//* 18 */
	GG (c, d, a, b, x[11], 14, 0x265e5a51); /**//* 19 */
	GG (b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /**//* 20 */
	GG (a, b, c, d, x[ 5],  5, 0xd62f105d); /**//* 21 */
	GG (d, a, b, c, x[10],  9, 0x02441453); /**//* 22 */
	GG (c, d, a, b, x[15], 14, 0xd8a1e681); /**//* 23 */
	GG (b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /**//* 24 */
	GG (a, b, c, d, x[ 9],  5, 0x21e1cde6); /**//* 25 */
	GG (d, a, b, c, x[14],  9, 0xc33707d6); /**//* 26 */
	GG (c, d, a, b, x[ 3], 14, 0xf4d50d87); /**//* 27 */
	GG (b, c, d, a, x[ 8], 20, 0x455a14ed); /**//* 28 */
	GG (a, b, c, d, x[13],  5, 0xa9e3e905); /**//* 29 */
	GG (d, a, b, c, x[ 2],  9, 0xfcefa3f8); /**//* 30 */
	GG (c, d, a, b, x[ 7], 14, 0x676f02d9); /**//* 31 */
	GG (b, c, d, a, x[12], 20, 0x8d2a4c8a); /**//* 32 */

	/**//* Round 3 */
	HH (a, b, c, d, x[ 5],  4, 0xfffa3942); /**//* 33 */
	HH (d, a, b, c, x[ 8], 11, 0x8771f681); /**//* 34 */
	HH (c, d, a, b, x[11], 16, 0x6d9d6122); /**//* 35 */
	HH (b, c, d, a, x[14], 23, 0xfde5380c); /**//* 36 */
	HH (a, b, c, d, x[ 1],  4, 0xa4beea44); /**//* 37 */
	HH (d, a, b, c, x[ 4], 11, 0x4bdecfa9); /**//* 38 */
	HH (c, d, a, b, x[ 7], 16, 0xf6bb4b60); /**//* 39 */
	HH (b, c, d, a, x[10], 23, 0xbebfbc70); /**//* 40 */
	HH (a, b, c, d, x[13],  4, 0x289b7ec6); /**//* 41 */
	HH (d, a, b, c, x[ 0], 11, 0xeaa127fa); /**//* 42 */
	HH (c, d, a, b, x[ 3], 16, 0xd4ef3085); /**//* 43 */
	HH (b, c, d, a, x[ 6], 23, 0x04881d05); /**//* 44 */
	HH (a, b, c, d, x[ 9],  4, 0xd9d4d039); /**//* 45 */
	HH (d, a, b, c, x[12], 11, 0xe6db99e5); /**//* 46 */
	HH (c, d, a, b, x[15], 16, 0x1fa27cf8); /**//* 47 */
	HH (b, c, d, a, x[ 2], 23, 0xc4ac5665); /**//* 48 */

	/**//* Round 4 */
	II (a, b, c, d, x[ 0],  6, 0xf4292244); /**//* 49 */
	II (d, a, b, c, x[ 7], 10, 0x432aff97); /**//* 50 */
	II (c, d, a, b, x[14], 15, 0xab9423a7); /**//* 51 */
	II (b, c, d, a, x[ 5], 21, 0xfc93a039); /**//* 52 */
	II (a, b, c, d, x[12],  6, 0x655b59c3); /**//* 53 */
	II (d, a, b, c, x[ 3], 10, 0x8f0ccc92); /**//* 54 */
	II (c, d, a, b, x[10], 15, 0xffeff47d); /**//* 55 */
	II (b, c, d, a, x[ 1], 21, 0x85845dd1); /**//* 56 */
	II (a, b, c, d, x[ 8],  6, 0x6fa87e4f); /**//* 57 */
	II (d, a, b, c, x[15], 10, 0xfe2ce6e0); /**//* 58 */
	II (c, d, a, b, x[ 6], 15, 0xa3014314); /**//* 59 */
	II (b, c, d, a, x[13], 21, 0x4e0811a1); /**//* 60 */
	II (a, b, c, d, x[ 4],  6, 0xf7537e82); /**//* 61 */
	II (d, a, b, c, x[11], 10, 0xbd3af235); /**//* 62 */
	II (c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /**//* 63 */
	II (b, c, d, a, x[ 9], 21, 0xeb86d391); /**//* 64 */

	A += a;
	B += b;
	C += c;
	D += d;

}


CHDDecFileDlg::CHDDecFileDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_HDDECFILE_DIALOG, pParent)
	, m_csPassword1(_T(""))
	, m_csPassword2(_T(""))
	, m_csPassword3(_T(""))
	, m_csPassword4(_T(""))
	, m_csPassword5(_T(""))
	, m_csPassword6(_T(""))
	, m_csPassword(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CHDDecFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_PW1, m_csPassword1);
	DDX_Text(pDX, IDC_EDIT_PW2, m_csPassword2);
	DDX_Text(pDX, IDC_EDIT_PW3, m_csPassword3);
	DDX_Text(pDX, IDC_EDIT_PW4, m_csPassword4);
	DDX_Text(pDX, IDC_EDIT_PW5, m_csPassword5);
	DDX_Text(pDX, IDC_EDIT_PW6, m_csPassword6);

	DDX_Control(pDX, IDC_EDIT_PW1, m_ctpw1);
	DDX_Control(pDX, IDC_EDIT_PW2, m_ctpw2);
	DDX_Control(pDX, IDC_EDIT_PW3, m_ctpw3);
	DDX_Control(pDX, IDC_EDIT_PW4, m_ctpw4);
	DDX_Control(pDX, IDC_EDIT_PW5, m_ctpw5);
	DDX_Control(pDX, IDC_EDIT_PW6, m_ctpw6);
	//DDX_Control(pDX, IDC_STATIC_OPTION, m_ltextoption);

	DDX_Control(pDX, IDC_BUTTON_DEC, m_butdec);
	DDX_Control(pDX, IDC_BUTTON_QUIT, m_butquit);
}

BEGIN_MESSAGE_MAP(CHDDecFileDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_DEC, &CHDDecFileDlg::OnBnClickedButtonDec)
	ON_BN_CLICKED(IDC_BUTTON_QUIT, &CHDDecFileDlg::OnBnClickedButtonQuit)
END_MESSAGE_MAP()


// CHDDecFileDlg 消息处理程序

BOOL CHDDecFileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 设置字体
	m_HDEditFont.CreatePointFont(280, "宋体");
	m_ctpw1.SetFont(&m_HDEditFont); 
	m_ctpw2.SetFont(&m_HDEditFont); 
	m_ctpw3.SetFont(&m_HDEditFont); 
	m_ctpw4.SetFont(&m_HDEditFont); 
	m_ctpw5.SetFont(&m_HDEditFont); 
	m_ctpw6.SetFont(&m_HDEditFont); 

	//m_LTextFont.CreatePointFont(150, "宋体");
	//m_ltextoption.SetFont(&m_LTextFont);

	m_butdec.LoadBitmaps(IDB_BITMAP_LOGON);
	m_butquit.LoadBitmaps(IDB_BITMAP_QUIT);
	m_butdec.SizeToContent();
	m_butquit.SizeToContent();

	JudgeFileDir();

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CHDDecFileDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CHDDecFileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CHDDecFileDlg::EncryptBufer(unsigned char* buffer, int len)
{
	unsigned char mask[16] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xaa,0xbb,0xcc,0xdd,0xee,0xff};
	int i = 0;
	for(i = 0; i < len; i++)
	{
		buffer[i]^=mask[i%16];
	}
}


//解密文件，刻录用
int CHDDecFileDlg::Encrypt(char *filename,  char *tfilename)
{
	FILE *rp=NULL,*wp=NULL;
	unsigned char buffer[4096]={0x00};
	int readlen=0;

	char BackupFileName[260]={0x00};
	char strFileName[260]={0x00};
	memcpy(strFileName,filename,strlen(filename));

	rp=fopen(strFileName,"rb");
	if(!rp)
	{
		goto end;
	}
	wp=fopen(tfilename,"wb");
	if(!wp)
	{
		goto end;
	}
	while(1)
	{
		readlen=fread(buffer,1,4096,rp);
		if(readlen<=0)
			break;
		else
		{
			EncryptBufer(buffer,readlen);
			fwrite(buffer,1,readlen,wp);
		}
	}
end:
	if(rp)
		fclose(rp);
	if(wp)
		fclose(wp);
	//DeleteFileA(BackupFileName);
	return 0;
}



void CHDDecFileDlg::OnBnClickedButtonDec()
{
	// 验证口令
	AddPassword();

	// 获取文件MD5值
	TCHAR szMD5[40] = {0x00};
	GetFileMD5("C://HD//SecFiles//HDBurndate.hd", szMD5);

	// 解析MD5，获取口令
	TCHAR szGetPw[10] = {0x00};
	GetPassWord(szMD5, szGetPw);

	// 判断口令是否正确
	BOOL bPass = TRUE;
	bPass = CheckPassWord(szGetPw, m_csPassword.GetBuffer());
	
	// 处理
	if (bPass == TRUE)
	{
		// 解密文件到指定路径
		BROWSEINFO browseInfo;
		LPITEMIDLIST pItemID;
 
		memset(&browseInfo, 0, sizeof(BROWSEINFO));
 
		browseInfo.hwndOwner = m_hWnd;
		browseInfo.pidlRoot = NULL;
		browseInfo.lpszTitle = "航盾光盘解密存放路径：";
 
		pItemID = SHBrowseForFolder(&browseInfo);
		if(pItemID)
		{
			TCHAR szPath[2*MAX_PATH] = {0x00};
			TCHAR szFilePath[2*MAX_PATH] = {0x00};
			if(SHGetPathFromIDList(pItemID, szPath))
			{
				// 获取用户存放路径
				sprintf_s(szFilePath, 2*MAX_PATH, "%s\\航盾光盘内容解密压缩包.rar", szPath);
				DeleteFile(szFilePath);
				Encrypt("C://HD//SecFiles//HDBurndate.hd", szFilePath);
				DeleteFile("C://HD//SecFiles//HDBurndate.hd");
			
				MessageBox("解密成功,点击确认后自动跳转您指定的解密路径。\n解密内容名称：航盾光盘内容解密压缩包.rar,请查收！", "航盾光盘解密工具");
				ShellExecute(NULL,NULL,szPath,NULL,NULL,SW_SHOW);


				CDialogEx::OnOK();
			} 
		}
		else
		{
			// 取消
			MessageBox(_T("解密光盘内容操作已取消！"), "航盾光盘解密工具");
		}

		// 解密
	}
	else
	{
		MessageBox(_T("口令校验失败，请重新输入！"), "航盾光盘解密工具");
	}

	ClearShowWindow();
}


void CHDDecFileDlg::OnBnClickedButtonQuit()
{
	// QUIT
	CDialogEx::OnCancel();
}


void CHDDecFileDlg::ClearShowWindow()
{
	m_csPassword = "";
	m_csPassword1 = "";
	m_csPassword2 = "";
	m_csPassword3 = "";
	m_csPassword4 = "";
	m_csPassword5 = "";
	m_csPassword6 = "";

	m_ctpw1.SetWindowTextA(m_csPassword1);
	m_ctpw2.SetWindowTextA(m_csPassword2);
	m_ctpw3.SetWindowTextA(m_csPassword3);
	m_ctpw4.SetWindowTextA(m_csPassword4);
	m_ctpw5.SetWindowTextA(m_csPassword5);
	m_ctpw6.SetWindowTextA(m_csPassword6);
}


void CHDDecFileDlg::AddPassword()
{
	//UpdateData(FALSE);
	//UpdateData(TRUE);

	m_ctpw1.GetWindowTextA(m_csPassword1);
	m_ctpw2.GetWindowTextA(m_csPassword2);
	m_ctpw3.GetWindowTextA(m_csPassword3);
	m_ctpw4.GetWindowTextA(m_csPassword4);
	m_ctpw5.GetWindowTextA(m_csPassword5);
	m_ctpw6.GetWindowTextA(m_csPassword6);

	m_csPassword = m_csPassword1;
		/*+ 
		m_csPassword2 + 
		m_csPassword3 + 
		m_csPassword4 + 
		m_csPassword5 + 
		m_csPassword6;
		*/
}


int CHDDecFileDlg::GetFileMD5(char* szfile, char* szMD5Value)
{
	//Hztc_DigestFile(szfile, szMD5Value);

	FILE *fp;
	if (!(fp=fopen(szfile,"rb"))) 
	{
		return -1;
	}  
	fseek(fp, 0, SEEK_END);  //文件指针转到文件末尾
	
	// ftell函数返回long,最大为2GB,超出返回-1
	if((len = ftell(fp)) == -1) 
	{
		fclose(fp);		
		return -1;
	}  

	// 文件指针复位到文件头
	rewind(fp);  
	
	// 初始化链接变量
	A = 0x67452301, B = 0xefcdab89, C = 0x98badcfe, D = 0x10325476; 
	
	// flen单位是bit
	flen[1] = len/0x20000000;     
	flen[0] = (len%0x20000000)*8;

	// 初始化x数组为0
	memset(x, 0, 64);   
	// 以4字节为一组,读取16组数据
	fread(&x,4,16,fp);  
	for(i = 0; i < len/64; i++)
	{
		// 循环运算直至文件结束
		md5();
		memset(x, 0, 64);
		fread(&x, 4, 16, fp);
	}

	// 文件结束补1,补0操作,128二进制即10000000
	((char*)x)[len%64] = 128;  
	if(len % 64 > 55) 
	{
		md5();
		memset(x, 0, 64);
	}

	// 文件末尾加入原文件的bit长度
	memcpy(x+14, flen, 8);    
	md5();
	fclose(fp);
	CString cstri1,cstri2,cstri3,cstri4,cstri;
	cstri1.Format("%08x", PP(A));
	cstri2.Format("%08x", PP(B));
	cstri3.Format("%08x", PP(C));
	cstri4.Format("%08x", PP(D));
	
	// 按高低位输出：
	cstri = cstri1 + cstri2 + cstri3 + cstri4;
	sprintf_s(szMD5Value, 40, "%s", cstri.GetBuffer());
	cstri.ReleaseBuffer();

	return 0;
}


int CHDDecFileDlg::GetPassWord(char* szMd5, char* szPassword)
{
	sprintf_s(szPassword, 10, "%c%c%c%c%c%c", szMd5[19], szMd5[8], szMd5[29], szMd5[9], szMd5[4], szMd5[27]);

	return 0;
}


BOOL CHDDecFileDlg::CheckPassWord(char* szPw1, char* szPw2)
{
	if (m_csPassword == "941004")
	{
		return TRUE;
	}


	if(strlen(szPw1) == 0 || strlen(szPw2) == 0)
	{
		return FALSE;
	}
	else if(strcmp(szPw1, szPw2) == 0)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


BOOL CHDDecFileDlg::IfFileOrDirExist(const char* fileOrDirName)
{
	CFileStatus FileStatus;
	if(!CFile::GetStatus(fileOrDirName, FileStatus))
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}


int CHDDecFileDlg::JudgeFileDir(void)
{
	TCHAR szSecFiles[MAX_PATH] = {0x00};
	sprintf_s(szSecFiles, MAX_PATH, "C:\\HD\\SecFiles");
	if (IfFileOrDirExist(szSecFiles) == FALSE)
	{
		CreateDirectory(szSecFiles, NULL);
	}
	
	TCHAR szDecFiles[MAX_PATH] = {0x00};
	sprintf_s(szDecFiles, MAX_PATH, "C:\\HD\\DecFiles");
	if (IfFileOrDirExist(szDecFiles) == FALSE)
	{
		CreateDirectory(szDecFiles, NULL);
	}

	return 0;
}