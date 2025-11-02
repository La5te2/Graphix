#include"Paint3Dlg.h"
#include "afxdialogex.h"
#include <corecrt_math_defines.h>
void CPaint3Dlg::DrawLineDefault(CPoint p1, CPoint p2, CDC& dc)
{
	dc.MoveTo(p1);
	dc.LineTo(p2);
}
void CPaint3Dlg::DrawLineDDAFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType)
{
	// 1. 计算局部绘制区域（添加一点 margin）
	int pad = max(2, lineWidth + 1);
	int minX = min(p1.x, p2.x) - pad;
	int minY = min(p1.y, p2.y) - pad;
	int maxX = max(p1.x, p2.x) + pad;
	int maxY = max(p1.y, p2.y) + pad;

	// 限制到客户区，避免创建超大位图
	CRect client;
	GetClientRect(&client);
	if (minX > client.right || maxX < client.left || minY > client.bottom || maxY < client.top)
		return; // 完全在视区外

	minX = max(minX, client.left);
	minY = max(minY, client.top);
	maxX = min(maxX, client.right);
	maxY = min(maxY, client.bottom);

	int w = maxX - minX + 1;
	int h = maxY - minY + 1;
	if (w <= 0 || h <= 0) return;

	// 2. 创建内存 DC 与 DIBSection（top-down）
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	BITMAPINFO bmi;
	ZeroMemory(&bmi, sizeof(bmi));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h; // top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr;
	HBITMAP hDib = (HBITMAP)CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
	if (!hDib || !pvBits) {
		if (hDib) DeleteObject(hDib);
		return;
	}

	// 将位图选入内存 DC，并保存旧位图句柄
	HBITMAP hOldBmp = (HBITMAP)memDC.SelectObject(hDib);

	// 3. 关键：先把目标区域背景从屏幕拷贝到内存位图
	//    这样我们在 DIB 上只修改需要改的像素，不会把剩下区域变成黑色
	memDC.BitBlt(0, 0, w, h, &dc, minX, minY, SRCCOPY);

	// 准备像素写入
	int strideBytes = w * 4; // 32bpp
	BYTE* base = (BYTE*)pvBits;

	auto putPixel = [&](int lx, int ly, BYTE R, BYTE G, BYTE B) {
		if (lx < 0 || lx >= w || ly < 0 || ly >= h) return;
		BYTE* p = base + ly * strideBytes + lx * 4;
		p[0] = B;
		p[1] = G;
		p[2] = R;
		p[3] = 0;
	};

	// 4. DDA 主循环（局部坐标）
	double dx = (double)p2.x - (double)p1.x;
	double dy = (double)p2.y - (double)p1.y;
	double steps = max(fabs(dx), fabs(dy));
	if (steps <= 0.0) {
		// 退化为点：直接画一个小方块
		int lx = p1.x - minX;
		int ly = p1.y - minY;
		int half = max(1, lineWidth / 2);
		BYTE R = GetRValue(color), G = GetGValue(color), B = GetBValue(color);
		for (int oy = -half; oy <= half; ++oy)
			for (int ox = -half; ox <= half; ++ox)
				putPixel(lx + ox, ly + oy, R, G, B);

		// blt 回去并清理
		dc.BitBlt(minX, minY, w, h, &memDC, 0, 0, SRCCOPY);
		memDC.SelectObject(hOldBmp);
		DeleteObject(hDib);
		return;
	}

	double xInc = dx / steps;
	double yInc = dy / steps;
	double x = (double)p1.x - minX;
	double y = (double)p1.y - minY;

	int dashLen = 12, gapLen = 6;
	int pattern = dashLen + gapLen;
	BYTE Rc = GetRValue(color), Gc = GetGValue(color), Bc = GetBValue(color);
	int halfW = max(1, lineWidth) / 2;

	for (int i = 0; i <= (int)ceil(steps); ++i)
	{
		bool draw = true;
		if (lineType == 1) {
			int pos = i % pattern;
			if (pos >= dashLen) draw = false;
		}
		if (draw) {
			int px = (int)floor(x + 0.5);
			int py = (int)floor(y + 0.5);
			for (int oy = -halfW; oy <= halfW; ++oy)
			{
				int ly = py + oy;
				if (ly < 0 || ly >= h) continue;
				for (int ox = -halfW; ox <= halfW; ++ox)
				{
					int lx = px + ox;
					if (lx < 0 || lx >= w) continue;
					putPixel(lx, ly, Rc, Gc, Bc);
				}
			}
		}
		x += xInc;
		y += yInc;
	}

	// 5. 一次性 blt 回屏幕
	dc.BitBlt(minX, minY, w, h, &memDC, 0, 0, SRCCOPY);

	// 6. 恢复旧对象并释放资源 —— 注意先恢复再 DeleteObject(hDib)
	memDC.SelectObject(hOldBmp);
	if (hDib) DeleteObject(hDib);
}

void CPaint3Dlg::DrawLineDDA(CPoint p1, CPoint p2, CDC& dc)
{
	double dx = p2.x - p1.x;
	double dy = p2.y - p1.y;
	double steps = fabs(dx) > fabs(dy) ? fabs(dx) : fabs(dy);
	double xInc = dx / steps;
	double yInc = dy / steps;
	double x = p1.x;
	double y = p1.y;
	int dashLength = 12; // 虚线段长度
	int gapLength = 6; // 虚线间隔长度
	for (int i = 0; i <= steps; ++i)
	{
		bool drawPixel = true;
		if (LineType == 1) // 虚线
		{
			int patternLength = dashLength + gapLength;
			int pos = i % patternLength;
			if (pos >= dashLength) drawPixel = false;
		}
		if (drawPixel)
		{
			int halfW = max(1, LineWidth - 1) / 2;
			for (int wx = -halfW; wx <= halfW; ++wx)
			{
				for (int wy = -halfW; wy <= halfW; ++wy)
				{
					int px = int(x + 0.5) + wx;
					int py = int(y + 0.5) + wy;
					dc.SetPixelV(px, py, LineColor);
				}
			}
		}
		x += xInc;
		y += yInc;
	}
}
void CPaint3Dlg::DrawLineMidpointFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType)
{
	// ====== 1. 局部区域 ======
	int pad = max(2, lineWidth + 1);
	int minX = min(p1.x, p2.x) - pad;
	int minY = min(p1.y, p2.y) - pad;
	int maxX = max(p1.x, p2.x) + pad;
	int maxY = max(p1.y, p2.y) + pad;

	CRect client;
	GetClientRect(&client);
	if (minX > client.right || maxX < client.left || minY > client.bottom || maxY < client.top)
		return;
	minX = max(minX, client.left);
	minY = max(minY, client.top);
	maxX = min(maxX, client.right);
	maxY = min(maxY, client.bottom);

	int w = maxX - minX + 1, h = maxY - minY + 1;
	if (w <= 0 || h <= 0) return;

	// ====== 2. 创建 DIB ======
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr;
	HBITMAP hDib = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
	if (!hDib || !pvBits) {
		if (hDib) DeleteObject(hDib);
		return;
	}
	HBITMAP hOld = (HBITMAP)memDC.SelectObject(hDib);
	memDC.BitBlt(0, 0, w, h, &dc, minX, minY, SRCCOPY);

	BYTE* base = (BYTE*)pvBits;
	int stride = w * 4;

	auto putPixel = [&](int lx, int ly, BYTE R, BYTE G, BYTE B) {
		if (lx < 0 || lx >= w || ly < 0 || ly >= h) return;
		BYTE* p = base + ly * stride + lx * 4;
		p[0] = B; p[1] = G; p[2] = R; p[3] = 0;
		};

	// ====== 3. Midpoint 主体 ======
	int x1 = p1.x, y1 = p1.y, x2 = p2.x, y2 = p2.y;
	int dx = abs(x2 - x1), dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	bool steep = dy > dx;
	if (steep) {
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(sx, sy);
	}

	int d = 2 * dy - dx;
	int y = y1;

	int dashLen = 12, gapLen = 6, pattern = dashLen + gapLen;
	BYTE R = GetRValue(color), G = GetGValue(color), B = GetBValue(color);
	int halfW = max(1, lineWidth) / 2;

	for (int i = 0, x = x1; i <= dx; ++i, x += sx)
	{
		bool draw = true;
		if (lineType == 1) {
			int pos = i % pattern;
			if (pos >= dashLen) draw = false;
		}

		if (draw) {
			int px = steep ? y : x;
			int py = steep ? x : y;
			px -= minX;
			py -= minY;
			for (int oy = -halfW; oy <= halfW; ++oy)
				for (int ox = -halfW; ox <= halfW; ++ox)
					putPixel(px + ox, py + oy, R, G, B);
		}

		if (d > 0) { y += sy; d -= 2 * dx; }
		d += 2 * dy;
	}

	// ====== 4. 输出到屏幕 ======
	dc.BitBlt(minX, minY, w, h, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(hOld);
	if (hDib) DeleteObject(hDib);
}

void CPaint3Dlg::DrawLineMidpoint(CPoint p1, CPoint p2, CDC& dc)
{
	int x1 = p1.x, y1 = p1.y;
	int x2 = p2.x, y2 = p2.y;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	bool steep = dy > dx;
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(sx, sy);
	}
	int d = 2 * dy + dx;
	int y = y1;
	int dashLength = 12;
	int gapLength = 6;
	for (int i = 0; i <= dx; ++i)
	{
		bool drawPixel = true;
		if (LineType == 1) // 虚线
		{
			int pattern = dashLength + gapLength;
			if ((i % pattern) >= dashLength)
				drawPixel = false;
		}
		if (drawPixel)
		{
			int halfW = max(1, LineWidth) / 2;
			for (int wx = -halfW; wx <= halfW; ++wx)
			{
				for (int wy = -halfW; wy <= halfW; ++wy)
				{
					if (steep)
						dc.SetPixelV(y + wy, x1 + wx, LineColor);
					else
						dc.SetPixelV(x1 + wx, y + wy, LineColor);
				}
			}
		}
		if (d > 0)
		{
			y += sy;
			d -= 2 * dx;
		}
		d += 2 * dy;
		x1 += sx;
	}
}
void CPaint3Dlg::DrawLineBresenhamFM(CPoint p1, CPoint p2, CDC& dc, COLORREF color, int lineWidth, int lineType)
{
	// ====== 1. 局部区域计算 ======
	int pad = max(2, lineWidth + 1);
	int minX = min(p1.x, p2.x) - pad;
	int minY = min(p1.y, p2.y) - pad;
	int maxX = max(p1.x, p2.x) + pad;
	int maxY = max(p1.y, p2.y) + pad;

	CRect client;
	GetClientRect(&client);
	if (minX > client.right || maxX < client.left || minY > client.bottom || maxY < client.top)
		return;
	minX = max(minX, client.left);
	minY = max(minY, client.top);
	maxX = min(maxX, client.right);
	maxY = min(maxY, client.bottom);

	int w = maxX - minX + 1, h = maxY - minY + 1;
	if (w <= 0 || h <= 0) return;

	// ====== 2. 创建内存 DIB Section ======
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	BITMAPINFO bmi = {};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr;
	HBITMAP hDib = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
	if (!hDib || !pvBits) {
		if (hDib) DeleteObject(hDib);
		return;
	}
	HBITMAP hOld = (HBITMAP)memDC.SelectObject(hDib);

	// 拷贝背景
	memDC.BitBlt(0, 0, w, h, &dc, minX, minY, SRCCOPY);

	BYTE* base = (BYTE*)pvBits;
	int stride = w * 4;

	auto putPixel = [&](int lx, int ly, BYTE R, BYTE G, BYTE B) {
		if (lx < 0 || lx >= w || ly < 0 || ly >= h) return;
		BYTE* p = base + ly * stride + lx * 4;
		p[0] = B; p[1] = G; p[2] = R; p[3] = 0;
		};

	// ====== 3. Bresenham 主逻辑 ======
	int x1 = p1.x, y1 = p1.y;
	int x2 = p2.x, y2 = p2.y;
	int dx = abs(x2 - x1), dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;
	int err = dx - dy;

	int dashLen = 12, gapLen = 6, pattern = dashLen + gapLen;
	BYTE R = GetRValue(color), G = GetGValue(color), B = GetBValue(color);
	int halfW = max(1, lineWidth) / 2;

	for (int i = 0;; ++i)
	{
		bool draw = true;
		if (lineType == 1) {
			int pos = i % pattern;
			if (pos >= dashLen) draw = false;
		}
		if (draw) {
			int px = x1 - minX, py = y1 - minY;
			for (int oy = -halfW; oy <= halfW; ++oy)
				for (int ox = -halfW; ox <= halfW; ++ox)
					putPixel(px + ox, py + oy, R, G, B);
		}

		if (x1 == x2 && y1 == y2) break;
		int e2 = 2 * err;
		if (e2 > -dy) { err -= dy; x1 += sx; }
		if (e2 < dx) { err += dx; y1 += sy; }
	}

	// ====== 4. 输出回屏幕 ======
	dc.BitBlt(minX, minY, w, h, &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(hOld);
	if (hDib) DeleteObject(hDib);
}

void CPaint3Dlg::DrawLineBresenham(CPoint p1, CPoint p2, CDC& dc)
{
	int x1 = p1.x, y1 = p1.y;
	int x2 = p2.x, y2 = p2.y;

	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = (x1 < x2) ? 1 : -1;
	int sy = (y1 < y2) ? 1 : -1;

	bool steep = dy > dx;
	if (steep)
	{
		std::swap(x1, y1);
		std::swap(x2, y2);
		std::swap(dx, dy);
		std::swap(sx, sy);
	}

	int err = -2 * abs(dy - dx);
	int y = y1;

	int dashLength = 6; // 实线段长度
	int gapLength = 3;  // 虚线间隔长度

	for (int i = 0; i <= dx; ++i)
	{
		bool drawPixel = true;
		// 线型控制（虚线）
		if (LineType == 1)
		{
			int pattern = dashLength + gapLength;
			if ((i % pattern) >= dashLength)
				drawPixel = false;
		}
		if (drawPixel)
		{
			int halfW = max(1, LineWidth) / 2;
			for (int wx = -halfW; wx <= halfW; ++wx)
			{
				for (int wy = -halfW; wy <= halfW; ++wy)
				{
					if (steep)
						dc.SetPixelV(y + wy, x1 + wx, LineColor);
					else
						dc.SetPixelV(x1 + wx, y + wy, LineColor);
				}
			}
		}
		if (err > 0)
		{
			y += sy;
			err -= 2 * dx;
		}
		err += 2 * dy;
		x1 += sx;
	}
}