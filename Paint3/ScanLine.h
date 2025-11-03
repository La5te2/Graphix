#include "pch.h"
#include "framework.h"
#include "Paint3.h"
#include "Paint3Dlg.h"
#include "afxdialogex.h"
#include "std.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
using namespace std;

void CPaint3Dlg::ScanConvertPolygonOutline(CDC& dc, const std::vector<CPoint>& poly, bool Clipper)
{
	if (poly.size() < 2) return;
	for (size_t i = 0; i < poly.size(); ++i) {
		CPoint a = poly[i];
		CPoint b = poly[(i + 1) % poly.size()];
		DrawLineDefault(a, b, dc);
	}
	if (!IsFill || Clipper) return;

	// --- 扫描线填充算法 ---
	// 获取多边形的 y 范围
	//int ymin = poly[0].y, ymax = poly[0].y;
	//for (const auto& p : poly) {
	//	ymin = min(ymin, p.y);
	//	ymax = max(ymax, p.y);
	//}

	//// --- 对每条扫描线求交点 ---
	//for (int y = ymin; y <= ymax; ++y)
	//{
	//	std::vector<int> xIntersections;

	//	for (size_t i = 0; i < poly.size(); ++i)
	//	{
	//		CPoint p1 = poly[i];
	//		CPoint p2 = poly[(i + 1) % poly.size()];

	//		// 保证 p1.y <= p2.y
	//		if (p1.y > p2.y) std::swap(p1, p2);

	//		// 跳过不相交的边
	//		if (y < p1.y || y >= p2.y) continue;

	//		// 计算交点 x 坐标（线性插值）
	//		if (p2.y != p1.y) {
	//			double x = p1.x + (double)(y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y);
	//			xIntersections.push_back((int)round(x));
	//		}
	//	}

	//	// --- 排序交点并两两连线 ---
	//	std::sort(xIntersections.begin(), xIntersections.end());
	//	for (size_t k = 0; k + 1 < xIntersections.size(); k += 2)
	//	{
	//		int xStart = xIntersections[k];
	//		int xEnd = xIntersections[k + 1];
	//		for (int x = xStart; x <= xEnd; ++x)
	//			dc.SetPixelV(x, y, ShapeColor);
	//	}
	//}
	COLORREF fillColor = ShapeColor;

	// 找出 y 范围
	int ymin = INT_MAX, ymax = INT_MIN;
	for (auto& p : poly)
	{
		ymin = min(ymin, p.y);
		ymax = max(ymax, p.y);
	}
	if (ymin >= ymax) return;

	// 限制在客户区范围
	CRect clientRect;
	GetClientRect(&clientRect);
	int width = clientRect.Width();
	int height = clientRect.Height();

	ymin = max(ymin, clientRect.top);
	ymax = min(ymax, clientRect.bottom - 1);
	int H = ymax - ymin + 1;
	if (H <= 0) return;

	// 建立边表 ET[y - ymin]
	struct Edge
	{
		int ymax;
		double x;
		double invSlope;
	};
	std::vector<std::vector<Edge>> ET(H);

	for (size_t i = 0; i < poly.size(); ++i)
	{
		CPoint p1 = poly[i];
		CPoint p2 = poly[(i + 1) % poly.size()];
		if (p1.y == p2.y) continue; // 忽略水平边
		if (p1.y > p2.y) std::swap(p1, p2);

		Edge e;
		e.ymax = p2.y;
		e.x = p1.x;
		e.invSlope = double(p2.x - p1.x) / double(p2.y - p1.y);

		int idx = p1.y - ymin;
		if (idx >= 0 && idx < H) ET[idx].push_back(e);
	}

	// 创建内存位图并复制当前画面
	CImage img;
	img.Create(width, height, 32);
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	HBITMAP hBmp = img;
	HGDIOBJ oldBmp = memDC.SelectObject(hBmp);

	memDC.BitBlt(0, 0, width, height, &dc, 0, 0, SRCCOPY);

	BYTE* bits = (BYTE*)img.GetBits();
	int pitch = img.GetPitch();

	auto setPixel = [&](int x, int y, COLORREF c) {
		if (x < 0 || x >= width || y < 0 || y >= height) return;
		BYTE* p = bits + (y - clientRect.top) * pitch + (x - clientRect.left) * 4;
		p[0] = GetBValue(c);
		p[1] = GetGValue(c);
		p[2] = GetRValue(c);
		};
	std::vector<Edge> AET;
	for (int y = ymin; y <= ymax; ++y)
	{
		int idx = y - ymin;

		// 加入新边
		for (auto& e : ET[idx]) AET.push_back(e);

		// 删除已到顶的边
		AET.erase(std::remove_if(AET.begin(), AET.end(),
			[&](const Edge& e) { return e.ymax <= y; }),
			AET.end());

		if (AET.empty()) continue;

		// 按 x 排序
		std::sort(AET.begin(), AET.end(), [](const Edge& a, const Edge& b)
			{ return a.x < b.x; });

		// 成对填充
		for (size_t i = 0; i + 1 < AET.size(); i += 2)
		{
			int xStart = int(ceil(AET[i].x));
			int xEnd = int(floor(AET[i + 1].x));
			for (int x = xStart; x <= xEnd; ++x)
				setPixel(x, y, fillColor);
		}

		// 更新交点
		for (auto& e : AET)
			e.x += e.invSlope;
	}

	// 一次性绘制到屏幕
	dc.BitBlt(clientRect.left, clientRect.top, width, height, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(oldBmp);
	img.Destroy();
}

void CPaint3Dlg::ScanlineFill(CDC& dc, CPoint seed, COLORREF fillColor, COLORREF borderColor)
{
	CClientDC clientDC(this);
	COLORREF targetColor = clientDC.GetPixel(seed); // 原始颜色

	if (targetColor == fillColor || targetColor == borderColor)
		return; // 已填充或边界

	std::stack<CPoint> stk;
	stk.push(seed);

	while (!stk.empty())
	{
		CPoint p = stk.top();
		stk.pop();

		int x = p.x;
		int y = p.y;

		// 向左扫描
		int xLeft = x;
		while (xLeft >= 0 && clientDC.GetPixel(xLeft, y) == targetColor)
			--xLeft;
		++xLeft; // 回到第一个可填充点

		// 向右扫描
		int xRight = x;
		while (clientDC.GetPixel(xRight, y) == targetColor)
			++xRight;
		--xRight; // 回到最后一个可填充点

		// 填充该行
		for (int xi = xLeft; xi <= xRight; ++xi)
			clientDC.SetPixelV(xi, y, fillColor);

		// 将上下行的未填充点压栈
		for (int xi = xLeft; xi <= xRight; ++xi)
		{
			// 上行
			if (y > 0 && clientDC.GetPixel(xi, y - 1) == targetColor)
				stk.push(CPoint(xi, y - 1));
			// 下行
			if (y < GetSystemMetrics(SM_CYSCREEN) - 1 && clientDC.GetPixel(xi, y + 1) == targetColor)
				stk.push(CPoint(xi, y + 1));
		}
	}
}
void CPaint3Dlg::ScanlineFillFM(CDC& dc, CPoint seed, COLORREF fillColor, COLORREF borderColor)
{
	CRect clientRect;
	GetClientRect(&clientRect);
	int width = clientRect.Width();
	int height = clientRect.Height();

	// 创建内存位图
	CImage img;
	img.Create(width, height, 32); // 32位 ARGB

	// 内存DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	// CImage 转为 HBITMAP
	HBITMAP hBmp = img;
	HGDIOBJ hOldBmp = memDC.SelectObject(hBmp);

	// 将窗口内容复制到内存
	memDC.BitBlt(0, 0, width, height, &dc, 0, 0, SRCCOPY);

	// 获取像素指针
	BYTE* pBits = (BYTE*)img.GetBits();
	int stride = img.GetPitch();

	auto getPixel = [&](int x, int y) -> COLORREF {
		BYTE* p = pBits + y * stride + x * 4;
		return RGB(p[2], p[1], p[0]);
		};

	COLORREF targetColor = getPixel(seed.x, seed.y);
	if (targetColor == fillColor || targetColor == borderColor)
	{
		memDC.SelectObject(hOldBmp);
		return;
	}

	// 扫描线填充
	auto setPixel = [&](int x, int y, COLORREF color) {
		BYTE* p = pBits + y * stride + x * 4;
		p[0] = GetBValue(color);
		p[1] = GetGValue(color);
		p[2] = GetRValue(color);
		};
	std::stack<CPoint> stk;
	stk.push(seed);
	while (!stk.empty())
	{
		CPoint p = stk.top();
		stk.pop();

		int y = p.y;
		int x = p.x;

		int xLeft = x;
		while (xLeft >= 0 && getPixel(xLeft, y) == targetColor) --xLeft;
		++xLeft;

		int xRight = x;
		while (xRight < width && getPixel(xRight, y) == targetColor) ++xRight;
		--xRight;

		for (int xi = xLeft; xi <= xRight; ++xi)
			setPixel(xi, y, fillColor);

		for (int xi = xLeft; xi <= xRight; ++xi)
		{
			if (y > 0 && getPixel(xi, y - 1) == targetColor)
				stk.push(CPoint(xi, y - 1));
			if (y < height - 1 && getPixel(xi, y + 1) == targetColor)
				stk.push(CPoint(xi, y + 1));
		}
	}

	// 将内存位图绘制回窗口
	dc.BitBlt(0, 0, width, height, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(hOldBmp);
	img.Destroy();
}