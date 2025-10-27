
// Paint3Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Paint3.h"
#include "Paint3Dlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CPaint3Dlg 对话框



CPaint3Dlg::CPaint3Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PAINT3_DIALOG, pParent)
	, LineWidth(0)
	, LineType(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CPaint3Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, LineWidth);
	DDV_MinMaxInt(pDX, LineWidth, 0, 20);
	DDX_Radio(pDX, IDC_RADIO1, LineType);
	DDX_Control(pDX, IDC_COMBO1, m_fill);
}

BEGIN_MESSAGE_MAP(CPaint3Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CPaint3Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPaint3Dlg::OnBnClickedButton2)
	ON_EN_CHANGE(IDC_EDIT1, &CPaint3Dlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_RADIO1, &CPaint3Dlg::OnBnClickedRadio1)
	ON_BN_CLICKED(IDC_RADIO2, &CPaint3Dlg::OnBnClickedRadio2)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CPaint3Dlg::OnCbnSelchangeCombo1)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()


// CPaint3Dlg 消息处理程序

BOOL CPaint3Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	LineColor = RGB(0, 0, 255);
	ShapeColor = RGB(255, 2, 0);

	LineWidth = 5;
	m_fill.AddString(_T("Filled"));
	m_fill.AddString(_T("Not Filled"));
	m_fill.SetCurSel(0);

	UpdateData(FALSE);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CPaint3Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CPaint3Dlg::OnPaint()
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
HCURSOR CPaint3Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CPaint3Dlg::OnBnClickedButton1()
{
	CColorDialog colorDlg(LineColor, CC_FULLOPEN);
	// LineColor = color;
	if (IDOK == colorDlg.DoModal()) {
		LineColor = colorDlg.GetColor();
	}
}

void CPaint3Dlg::OnBnClickedButton2()
{
	CColorDialog colorDlg(ShapeColor, CC_FULLOPEN);
	// LineColor = color;
	if (IDOK == colorDlg.DoModal()) {
		ShapeColor = colorDlg.GetColor();
	}
}

void CPaint3Dlg::OnEnChangeEdit1()
{
	UpdateData(TRUE); // 将控件的值更新到变量
}

void CPaint3Dlg::OnBnClickedRadio1()
{
	LineType = 0;
}

void CPaint3Dlg::OnBnClickedRadio2()
{
	LineType = 1;
}

void CPaint3Dlg::OnCbnSelchangeCombo1()
{
	int sel = m_fill.GetCurSel();
	IsFill = (sel == 0) ? true : false;
}

void CPaint3Dlg::OnLButtonDown(UINT nFlags, CPoint point)
{
	CClientDC dc(this);
	dc.SetPixel(point.x, point.y, LineColor);
	startPoint = point;
	CDialogEx::OnLButtonDown(nFlags, point);
}

void CPaint3Dlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	CClientDC dc(this);
	dc.SetPixel(point.x, point.y, LineColor);
	endPoint = point;
	Lines.push_back(make_pair(startPoint, endPoint));
	int penStyle = LineType ? PS_DASH : PS_SOLID;
	LOGBRUSH logBrush;
	logBrush.lbStyle = BS_SOLID;
	logBrush.lbColor = LineColor;
	CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, LineWidth, &logBrush); // 选择画笔到设备上下文
	CPen* pOldPen = dc.SelectObject(&pen);
	dc.MoveTo(startPoint.x, startPoint.y);
	dc.LineTo(endPoint.x, endPoint.y);
	CDialogEx::OnLButtonUp(nFlags, point);
}
