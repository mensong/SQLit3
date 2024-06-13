
// SQLCommanderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "SQLCommander.h"
#include "SQLCommanderDlg.h"
#include "afxdialogex.h"
#include <sstream>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSQLCommanderDlg 对话框



CSQLCommanderDlg::CSQLCommanderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SQLCOMMANDER_DIALOG, pParent)
	, m_db(NULL)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSQLCommanderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_DB_FILE, m_editDbFile);
	DDX_Control(pDX, IDC_EDIT_DB_PWD, m_editDbPwd);
	DDX_Control(pDX, IDC_BTN_CONNECT, m_btnConnect);
	DDX_Control(pDX, IDC_EDIT_SQL, m_editSql);
	DDX_Control(pDX, IDC_LIST_RESULT, m_listResult);
	DDX_Control(pDX, IDC_EDIT_EXECUTE_AFFECT, m_editExeAffect);
}

BEGIN_MESSAGE_MAP(CSQLCommanderDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_OPEN, &CSQLCommanderDlg::OnBnClickedBtnOpen)
	ON_BN_CLICKED(IDC_BTN_CONNECT, &CSQLCommanderDlg::OnBnClickedBtnConnect)
	ON_BN_CLICKED(IDC_BTN_EXECUTE, &CSQLCommanderDlg::OnBnClickedBtnExecute)
END_MESSAGE_MAP()


// CSQLCommanderDlg 消息处理程序

BOOL CSQLCommanderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	m_listResult.SetExtendedStyle(m_listResult.GetExtendedStyle() | LVS_EX_FULLROWSELECT);

	m_scale.Init(GetSafeHwnd());

	m_editDbFile.SetWindowText(_T(":memory:"));

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CSQLCommanderDlg::OnPaint()
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
HCURSOR CSQLCommanderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSQLCommanderDlg::OnBnClickedBtnOpen()
{
	CString FilePathName;
	CFileDialog dlg(TRUE, //TRUE为OPEN对话框，FALSE为SAVE AS对话框
		NULL,
		NULL,
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,
		(LPCTSTR)_TEXT("Database Files (*.db)|*.db|All Files (*.*)|*.*||"),
		NULL);
	if (dlg.DoModal() == IDOK)
	{
		m_editDbFile.SetWindowText(dlg.GetPathName()); //文件名保存在了FilePathName里
	}
}


void CSQLCommanderDlg::OnBnClickedBtnConnect()
{
	CString dbFile;
	m_editDbFile.GetWindowText(dbFile);
	if (dbFile.IsEmpty())
		return;
	CString dbPwd;
	m_editDbPwd.GetWindowText(dbPwd);

	if (m_db)
	{
		SQLit3::Ins().DestoryDatabase(m_db);
		m_db = NULL;
	}

	m_db = SQLit3::Ins().CreateDatabase();
	bool b = m_db->Open(CW2A(dbFile), CW2A(dbPwd));
	if (!b)
	{
		SQLit3::Ins().DestoryDatabase(m_db);
		m_db = NULL;
		
		m_editExeAffect.SetWindowText(_T("连接数据库失败"));
	}
	else
	{
		m_editExeAffect.SetWindowText(_T("连接数据库成功"));
	}
}

void CSQLCommanderDlg::OnBnClickedBtnExecute()
{
	int headerCount = m_listResult.GetHeaderCtrl()->GetItemCount();
	for (int i = 0; i < headerCount; i++)
	{
		m_listResult.DeleteColumn(0);
	}	
	m_listResult.DeleteAllItems();

	if (!m_db)
		return;

	CString sql;
	m_editSql.GetWindowText(sql);
	if (sql.IsEmpty())
		return;

	//bool b = m_db->Execute(CW2A(sql));
	SqlStatement* stmt = m_db->StatementPrepare(CW2A(sql));
	if (!stmt)
		return;

	int colCount = stmt->GetColumnCount();
	for (int i = 0; i < colCount; i++)
	{
		CString name(stmt->GetColumnName(i));
		m_listResult.InsertColumn(i, name);
	}

	while (stmt->Next() == SqlStatement::SQLIT3_EXEC_HAS_ROW)
	{
		int row = m_listResult.InsertItem(m_listResult.GetItemCount(), _T(""));

		for (int i = 0; i < colCount; i++)
		{
			std::stringstream ss;
			auto type = stmt->GetType(i);
			switch (type)
			{
			case SqlStatement::SQLIT3_VALUE_INTEGER:
			{
				__int64 v = stmt->GetInt64(i);
				ss << v;
				break;
			}
			case SqlStatement::SQLIT3_VALUE_FLOAT:
			{
				double v = stmt->GetDouble(i);
				ss << v;
				break;
			}
			case SqlStatement::SQLIT3_VALUE_BLOB:
			{
				int dataLen = 0;
				const void* data = stmt->GetBlob(i, &dataLen);
				char* pBuf = new char[dataLen + 1];
				memcpy(pBuf, data, dataLen);
				pBuf[dataLen] = '\0';
				ss << pBuf;
				delete[] pBuf;
				break;
			}
			case SqlStatement::SQLIT3_VALUE_NULL:
			{
				ss << "<NULL>";
				break;
			}
			case SqlStatement::SQLIT3_VALUE_TEXT:
			default:
			{
				int dataLen = 0;
				auto data = stmt->GetText(i, &dataLen);
				ss << data;
				break;
			}
			}

			m_listResult.SetItemText(row, i, CString(ss.str().c_str()));
		}
	}

	AdjustColumnWidth(&m_listResult);

	int changeRowCount = m_db->GetChangeRowCount();
	CString affectText;
	affectText.Format(_T("影响行数:%d"), changeRowCount);
	m_editExeAffect.SetWindowText(affectText);

	stmt->Reset();
	m_db->StatementFinalize(stmt);
}

bool CSQLCommanderDlg::AdjustColumnWidth(CListCtrl* m_acclist)		// 将需要调整的list指针传递过来
{
	CHeaderCtrl* pHeaderCtrl = m_acclist->GetHeaderCtrl();		// 获得表头
	int nColumnCount = pHeaderCtrl->GetItemCount();				// 获得列数
	for (int i = 0; i < nColumnCount; i++)
	{
		m_acclist->SetColumnWidth(i, LVSCW_AUTOSIZE);
		int nColumnWidth = m_acclist->GetColumnWidth(i);		// 表头的宽度
		m_acclist->SetColumnWidth(i, LVSCW_AUTOSIZE_USEHEADER);	// 数据的宽度
		int nHeaderWidth = m_acclist->GetColumnWidth(i);
		m_acclist->SetColumnWidth(i, nColumnWidth > nHeaderWidth ? nColumnWidth : nHeaderWidth);  // 取较大值
	}
	return true;
}