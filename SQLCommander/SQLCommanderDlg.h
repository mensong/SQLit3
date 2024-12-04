
// SQLCommanderDlg.h: 头文件
//

#pragma once
#include "..\SQLit3\SQLit3.h"
#include "CtrlScale.h"

// CSQLCommanderDlg 对话框
class CSQLCommanderDlg : public CDialogEx
{
// 构造
public:
	CSQLCommanderDlg(CWnd* pParent = nullptr);	// 标准构造函数

	enum { IDD = IDD_SQLCOMMANDER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	bool AdjustColumnWidth(CListCtrl* m_acclist);

// 实现
protected:
	HICON m_hIcon;
	Database* m_db;
	CCtrlScale m_scale;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedBtnOpen();
	afx_msg void OnBnClickedBtnConnect();
	CEdit m_editDbFile;
	CEdit m_editDbPwd;
	CButton m_btnConnect;
	CEdit m_editSql;
	CListCtrl m_listResult;
	afx_msg void OnBnClickedBtnExecute();
	CEdit m_editExeAffect;
};
