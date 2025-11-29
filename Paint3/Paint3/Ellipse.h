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

void CPaint3Dlg::DrawEllipseA(CDC& dc, const CRect& rect, COLORREF color, int lineWidth, int lineType, int alg)
{
	if (alg == 4) // Default Circle
	{
		DrawEllipseDefault(dc, rect);
	}
	else if (alg == 5) // Midpoint Circle
	{
		DrawEllipseMidpointFM(dc, rect, color, lineWidth, lineType);
	}
	else if (alg == 6) // Bresenham Circle
	{
		DrawEllipseBresenhamFM(dc, rect, color, lineWidth, lineType);
	}
}
void CPaint3Dlg::DrawEllipseDefault(CDC& dc, const CRect& rect)
{
	dc.Ellipse(rect);
}
void CPaint3Dlg::DrawEllipseMidpointFM(CDC& dc, const CRect& rect, COLORREF color, int lineWidth, int lineType)
{
	int pad = max(2, lineWidth + 1);
	int minX = rect.left - pad;
	int minY = rect.top - pad;
	int maxX = rect.right + pad;
	int maxY = rect.bottom + pad;
	CRect client;
	GetClientRect(&client);
	minX = max(minX, client.left);
	minY = max(minY, client.top);
	maxX = min(maxX, client.right);
	maxY = min(maxY, client.bottom);

	int w = maxX - minX + 1;
	int h = maxY - minY + 1;
	if (w <= 0 || h <= 0) return;
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	BITMAPINFO bmi{};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h; // top-down
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr;
	HBITMAP hDib = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
	if (!hDib || !pvBits) {
		if (hDib) DeleteObject(hDib);
		return;
	}
	HBITMAP hOldBmp = (HBITMAP)memDC.SelectObject(hDib);
	memDC.BitBlt(0, 0, w, h, &dc, minX, minY, SRCCOPY);

	int xc = (rect.left + rect.right) / 2;
	int yc = (rect.top + rect.bottom) / 2;
	int a = abs(rect.right - rect.left) / 2;
	int b = abs(rect.bottom - rect.top) / 2;

	double a2 = (double)a * a;
	double b2 = (double)b * b;
	double d1 = b2 - a2 * b + 0.25 * a2;
	int x = 0;
	int y = b;

	int halfW = max(1, lineWidth) / 2;
	int dashLength = 6;
	int gapLength = 6;
	int patternLength = dashLength + gapLength;
	int step = 0;

	BYTE R = GetRValue(color);
	BYTE G = GetGValue(color);
	BYTE B = GetBValue(color);

	int stride = w * 4;
	BYTE* base = (BYTE*)pvBits;

	auto putPixel = [&](int px, int py) {
		if (px < minX || px > maxX || py < minY || py > maxY) return;
		int lx = px - minX;
		int ly = py - minY;
		if (lx < 0 || lx >= w || ly < 0 || ly >= h) return;
		BYTE* p = base + ly * stride + lx * 4;
		p[0] = B; p[1] = G; p[2] = R; p[3] = 0xFF;
		};

	auto drawPixel = [&](int px, int py) {
		if (lineType == 1) { // 虚线
			int pos = step % patternLength;
			if (pos >= dashLength) return;
		}
		for (int wx = -halfW; wx <= halfW; ++wx)
			for (int wy = -halfW; wy <= halfW; ++wy)
				putPixel(px + wx, py + wy);
		};

	long dx = 2 * b2 * x;
	long dy = 2 * a2 * y;
	while (dx < dy)
	{
		drawPixel(xc + x, yc + y);
		drawPixel(xc - x, yc + y);
		drawPixel(xc + x, yc - y);
		drawPixel(xc - x, yc - y);
		step++;

		if (d1 < 0)
		{
			x++;
			dx += 2 * b2;
			d1 += dx + b2;
		}
		else
		{
			x++;
			y--;
			dx += 2 * b2;
			dy -= 2 * a2;
			d1 += dx - dy + b2;
		}
	}
	long d2 = (long)(b2 * (x + 0.5) * (x + 0.5)
		+ a2 * (y - 1) * (y - 1)
		- a2 * b2);

	while (y >= 0)
	{
		drawPixel(xc + x, yc + y);
		drawPixel(xc - x, yc + y);
		drawPixel(xc + x, yc - y);
		drawPixel(xc - x, yc - y);
		step++;

		if (d2 > 0)
		{
			y--;
			dy -= 2 * a2;
			d2 += a2 - dy;
		}
		else
		{
			y--;
			x++;
			dx += 2 * b2;
			dy -= 2 * a2;
			d2 += dx - dy + a2;
		}
	}
	dc.BitBlt(minX, minY, w, h, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(hOldBmp);
	DeleteObject(hDib);
}

void CPaint3Dlg::DrawEllipseMidpoint(CDC& dc, const CRect& rect)
{
	int xc = (rect.left + rect.right) / 2;
	int yc = (rect.top + rect.bottom) / 2;
	int a = abs(rect.right - rect.left) / 2;
	int b = abs(rect.bottom - rect.top) / 2;

	double a2 = a * a;
	double b2 = b * b;
	double d1 = b2 - a2 * b + 0.25 * a2;
	int x = 0;
	int y = b;

	int halfW = max(1, LineWidth) / 2;
	int dashLength = 6;
	int gapLength = 6;
	int patternLength = dashLength + gapLength;
	int step = 0;

	auto drawPixel = [&](int px, int py) {
		if (LineType == 1) // 虚线
		{
			int pos = step % patternLength;
			if (pos >= dashLength) return;
		}

		for (int wx = -halfW; wx <= halfW; ++wx)
			for (int wy = -halfW; wy <= halfW; ++wy)
				dc.SetPixelV(px + wx, py + wy, LineColor);
		};

	long dx = 2 * b2 * x;
	long dy = 2 * a2 * y;

	while (dx < dy)
	{
		drawPixel(xc + x, yc + y);
		drawPixel(xc - x, yc + y);
		drawPixel(xc + x, yc - y);
		drawPixel(xc - x, yc - y);
		step++;

		if (d1 < 0)
		{
			x++;
			dx += 2 * b2;
			d1 += dx + b2;
		}
		else
		{
			x++;
			y--;
			dx += 2 * b2;
			dy -= 2 * a2;
			d1 += dx - dy + b2;
		}
	}
	long d2 = (long)(b2 * (x + 0.5) * (x + 0.5)
		+ a2 * (y - 1) * (y - 1)
		- a2 * b2);

	while (y >= 0)
	{
		drawPixel(xc + x, yc + y);
		drawPixel(xc - x, yc + y);
		drawPixel(xc + x, yc - y);
		drawPixel(xc - x, yc - y);
		step++;

		if (d2 > 0)
		{
			y--;
			dy -= 2 * a2;
			d2 += a2 - dy;
		}
		else
		{
			y--;
			x++;
			dx += 2 * b2;
			dy -= 2 * a2;
			d2 += dx - dy + a2;
		}
	}
}
void CPaint3Dlg::DrawEllipseBresenhamFM(CDC& dc, const CRect& rect, COLORREF color, int lineWidth, int lineType)
{
	int xc = (rect.left + rect.right) / 2;
	int yc = (rect.top + rect.bottom) / 2;
	int a = abs(rect.right - rect.left) / 2;
	int b = abs(rect.bottom - rect.top) / 2;
	if (a <= 0 || b <= 0) return;

	int pad = max(2, lineWidth + 1);
	int minX = rect.left - pad;
	int minY = rect.top - pad;
	int maxX = rect.right + pad;
	int maxY = rect.bottom + pad;

	CRect client;
	GetClientRect(&client);
	minX = max(minX, client.left);
	minY = max(minY, client.top);
	maxX = min(maxX, client.right);
	maxY = min(maxY, client.bottom);

	int w = maxX - minX + 1;
	int h = maxY - minY + 1;
	if (w <= 0 || h <= 0) return;
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	BITMAPINFO bmi{};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h; // top-down DIB
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pvBits = nullptr;
	HBITMAP hDib = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &pvBits, nullptr, 0);
	if (!hDib || !pvBits) {
		if (hDib) DeleteObject(hDib);
		return;
	}
	HBITMAP hOldBmp = (HBITMAP)memDC.SelectObject(hDib);
	memDC.BitBlt(0, 0, w, h, &dc, minX, minY, SRCCOPY);

	BYTE* base = (BYTE*)pvBits;
	int stride = w * 4;
	BYTE R = GetRValue(color);
	BYTE G = GetGValue(color);
	BYTE B = GetBValue(color);

	auto putPixel = [&](int px, int py) {
		if (px < minX || px > maxX || py < minY || py > maxY) return;
		int lx = px - minX;
		int ly = py - minY;
		if (lx < 0 || lx >= w || ly < 0 || ly >= h) return;
		BYTE* p = base + ly * stride + lx * 4;
		p[0] = B; p[1] = G; p[2] = R; p[3] = 0xFF;
	};
	int x = 0, y = b;
	long long a2 = 1LL * a * a;
	long long b2 = 1LL * b * b;
	long long d1 = b2 - a2 * b + a2 / 4;
	long long d2;

	int index = 0;
	int halfW = max(1, lineWidth) / 2;

	auto plot = [&](int px, int py, int idx) {
		// 虚线控制
		if (lineType == 1 && (idx / 5) % 2 == 0) return;

		for (int dx = -halfW; dx <= halfW; ++dx)
			for (int dy = -halfW; dy <= halfW; ++dy)
			{
				putPixel(xc + px + dx, yc + py + dy);
				putPixel(xc - px + dx, yc + py + dy);
				putPixel(xc + px + dx, yc - py + dy);
				putPixel(xc - px + dx, yc - py + dy);
			}
		};

	plot(x, y, index);
	while (b2 * (x + 1) < a2 * (y - 1))
	{
		if (d1 < 0)
		{
			d1 += b2 * (2 * x + 3);
			x++;
		}
		else
		{
			d1 += b2 * (2 * x + 3) + a2 * (-2 * y + 2);
			x++;
			y--;
		}
		++index;
		plot(x, y, index);
	}
	d2 = b2 * (x + 1) * (x + 1) + a2 * (y - 1) * (y - 1) - a2 * b2;
	while (y > 0)
	{
		if (d2 < 0)
		{
			d2 += b2 * (2 * x + 2) + a2 * (-2 * y + 3);
			x++;
			y--;
		}
		else
		{
			d2 += a2 * (-2 * y + 3);
			y--;
		}
		++index;
		plot(x, y, index);
	}
	dc.BitBlt(minX, minY, w, h, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(hOldBmp);
	DeleteObject(hDib);
}

void CPaint3Dlg::DrawEllipseBresenham(CDC& dc, const CRect& rect)
{
	// 计算椭圆的中心和半轴长度
	int xc = (rect.left + rect.right) / 2;
	int yc = (rect.top + rect.bottom) / 2;
	int a = abs(rect.right - rect.left) / 2;
	int b = abs(rect.bottom - rect.top) / 2;

	// 参数合法性检查
	if (a <= 0 || b <= 0) return;

	int x = 0, y = b;

	// 使用整数运算避免精度问题
	long long a2 = 1LL * a * a;
	long long b2 = 1LL * b * b;
	long long d1 = b2 - a2 * b + a2 / 4;
	long long d2;

	// 绘制四对称点 + 支持线宽
	auto plot = [&](int px, int py, int index) {
		// 虚线控制：每隔一定像素空一段
		if (LineType == 1 && (index / 5) % 2 == 0) return;

		// 用线宽加粗：在主方向上扩展像素
		for (int dx = -LineWidth / 2; dx <= LineWidth / 2; ++dx)
		{
			for (int dy = -LineWidth / 2; dy <= LineWidth / 2; ++dy)
			{
				dc.SetPixelV(xc + px + dx, yc + py + dy, LineColor);
				dc.SetPixelV(xc - px + dx, yc + py + dy, LineColor);
				dc.SetPixelV(xc + px + dx, yc - py + dy, LineColor);
				dc.SetPixelV(xc - px + dx, yc - py + dy, LineColor);
			}
		}
		};

	int index = 0;
	plot(x, y, index);

	// 区域1：斜率绝对值 > 1
	while (b2 * (x + 1) < a2 * (y - 1))
	{
		if (d1 < 0)
		{
			d1 += b2 * (2 * x + 3);
			x++;
		}
		else
		{
			d1 += b2 * (2 * x + 3) + a2 * (-2 * y + 2);
			x++;
			y--;
		}
		++index;
		plot(x, y, index);
	}

	// 区域2：斜率绝对值 <= 1
	d2 = b2 * (x + 1) * (x + 1) + a2 * (y - 1) * (y - 1) - a2 * b2;
	while (y > 0)
	{
		if (d2 < 0)
		{
			d2 += b2 * (2 * x + 2) + a2 * (-2 * y + 3);
			x++;
			y--;
		}
		else
		{
			d2 += a2 * (-2 * y + 3);
			y--;
		}
		++index;
		plot(x, y, index);
	}
}

void CPaint3Dlg::DrawArcFM(float angle, bool direction, CPoint p1, CPoint p2,
	CDC& dc, COLORREF color, int lineWidth, int lineType)
{
	if (fabs(angle) < 1e-6)
	{
		dc.SetPixel(p1.x, p1.y, color);
		return;
	}

	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double dx = x2 - x1, dy = y2 - y1;
	double d = sqrt(dx * dx + dy * dy);
	if (d < 1e-6) return;

	double halfAngle = fabs(angle) / 2.0;
	double r = (d / 2.0) / sin(halfAngle);
	double h = sqrt(max(0.0, r * r - (d / 2.0) * (d / 2.0)));

	// 中点与垂线方向
	double mx = (x1 + x2) / 2.0;
	double my = (y1 + y2) / 2.0;
	double ux = -dy / d, uy = dx / d;

	// 两个可能的圆心
	double cx1 = mx + ux * h;
	double cy1 = my + uy * h;
	double cx2 = mx - ux * h;
	double cy2 = my - uy * h;

	auto arcSpan = [&](double cx, double cy) {
		double a1 = atan2(y1 - cy, x1 - cx);
		double a2 = atan2(y2 - cy, x2 - cx);
		double da = a2 - a1;
		if (da < 0) da += 2 * M_PI;
		return da;
		};

	double span1 = arcSpan(cx1, cy1);
	double span2 = arcSpan(cx2, cy2);

	double target = fmod(fabs(angle), 2 * M_PI);
	if (target < 0) target += 2 * M_PI;
	double desiredSpan = direction ? target : fmod(2 * M_PI - target, 2 * M_PI);

	double diff1 = fabs(span1 - desiredSpan);
	double diff2 = fabs(span2 - desiredSpan);

	double cx, cy;
	if (diff1 < diff2)
		cx = cx1, cy = cy1;
	else
		cx = cx2, cy = cy2;

	// 创建内存 DC
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CRect clientRect;
	GetClientRect(&clientRect);

	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(&dc, clientRect.Width(), clientRect.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&memBitmap);

	// 先拷贝当前屏幕内容到内存位图（保留背景）
	memDC.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &dc, 0, 0, SRCCOPY);

	// 绘制弧
	double startA = atan2(y1 - cy, x1 - cx);
	double arcLen = fabs(angle * r);
	int steps = max(2, (int)ceil(arcLen / 0.8));

	for (int i = 0; i <= steps; ++i)
	{
		double t = (double)i / steps;
		double theta = direction ? (startA + t * angle) : (startA - t * angle);
		int sx = (int)round(cx + r * cos(theta));
		int sy = (int)round(cy + r * sin(theta));

		bool drawPixel = true;
		if (lineType == 1) // 虚线
		{
			const int dashLength = max(2, lineWidth * 4);
			const int gapLength = max(2, lineWidth * 2);
			int patternLength = dashLength + gapLength;
			int patternPos = i % patternLength;
			drawPixel = (patternPos < dashLength);
		}

		if (drawPixel)
		{
			int radius = max(1, lineWidth / 2);
			for (int dx = -radius; dx <= radius; ++dx)
			{
				for (int dy = -radius; dy <= radius; ++dy)
				{
					if (dx * dx + dy * dy <= radius * radius)
					{
						memDC.SetPixelV(sx + dx, sy + dy, color);
					}
				}
			}
		}
	}

	// 绘制端点
	int radius = max(1, lineWidth / 2);
	for (int dx = -radius; dx <= radius; ++dx)
	{
		for (int dy = -radius; dy <= radius; ++dy)
		{
			if (dx * dx + dy * dy <= radius * radius)
			{
				memDC.SetPixelV(p1.x + dx, p1.y + dy, color);
				memDC.SetPixelV(p2.x + dx, p2.y + dy, color);
			}
		}
	}

	// 将结果拷贝回主 DC
	dc.BitBlt(0, 0, clientRect.Width(), clientRect.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);
	memBitmap.DeleteObject();
	memDC.DeleteDC();
}


void CPaint3Dlg::DrawArc(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc)
{
	if (fabs(angle) < 1e-6) {
		dc.SetPixel(p1.x, p1.y, LineColor);
		return;
	}

	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double dx = x2 - x1, dy = y2 - y1;
	double d = sqrt(dx * dx + dy * dy);
	if (d < 1e-6) return;

	double halfAngle = fabs(angle) / 2.0;
	double r = (d / 2.0) / sin(halfAngle);
	double h = sqrt(max(0.0, r * r - (d / 2.0) * (d / 2.0)));

	// 中点与垂线方向
	double mx = (x1 + x2) / 2.0;
	double my = (y1 + y2) / 2.0;
	double ux = -dy / d, uy = dx / d;

	// 候选圆心
	double cx1 = mx + ux * h;
	double cy1 = my + uy * h;
	double cx2 = mx - ux * h;
	double cy2 = my - uy * h;

	auto arcSpan = [&](double cx, double cy) {
		double a1 = atan2(y1 - cy, x1 - cx);
		double a2 = atan2(y2 - cy, x2 - cx);
		double da = a2 - a1;
		if (da < 0) da += 2 * M_PI;
		return da; // CCW span from p1 to p2 in [0, 2PI)
		};

	double span1 = arcSpan(cx1, cy1);
	double span2 = arcSpan(cx2, cy2);

	// 目标弧长（按正方向），规范到 [0, 2PI)
	double target = fmod(fabs(angle), 2 * M_PI);
	if (target < 0) target += 2 * M_PI;
	double desiredSpan;
	if (direction) {
		// 逆时针 -> 直接和 target 比
		desiredSpan = target;
	}
	else {
		// 顺时针 -> 要比较的是 2PI - target（对应的 CCW 跨度应接近 2PI - target）
		desiredSpan = fmod(2 * M_PI - target, 2 * M_PI);
	}

	double diff1 = fabs(span1 - desiredSpan);
	double diff2 = fabs(span2 - desiredSpan);

	double cx, cy;
	if (diff1 < diff2)
		cx = cx1, cy = cy1;
	else
		cx = cx2, cy = cy2;

	// 根据 direction 决定绘制方向
	double startA = atan2(y1 - cy, x1 - cx);
	// 步长计算
	double arcLen = fabs(angle * r);
	int steps = max(2, (int)ceil(arcLen / 0.8));
	for (int i = 0; i <= steps; ++i)
	{
		double t = (double)i / steps;
		double theta = direction ? (startA + t * angle) : (startA - t * angle);
		int sx = (int)round(cx + r * cos(theta));
		int sy = (int)round(cy + r * sin(theta));
		bool drawPixel = true;
		if (LineType == 1) // 虚线
		{
			const int dashLength = max(2, LineWidth * 4);
			const int gapLength = max(2, LineWidth * 2);
			int patternLength = dashLength + gapLength;
			int patternPos = i % patternLength;
			drawPixel = (patternPos < dashLength);
		}
		if (drawPixel)
		{
			int radius = max(1, LineWidth / 2);
			for (int dx = -radius; dx <= radius; ++dx)
			{
				for (int dy = -radius; dy <= radius; ++dy)
				{
					if (dx * dx + dy * dy <= radius * radius)
					{
						dc.SetPixelV(sx + dx, sy + dy, LineColor);
					}
				}
			}
		}
	}

	// 确保端点也绘制
	int radius = max(1, LineWidth / 2);
	for (int dx = -radius; dx <= radius; ++dx)
	{
		for (int dy = -radius; dy <= radius; ++dy)
		{
			if (dx * dx + dy * dy <= radius * radius)
			{
				dc.SetPixelV(p1.x + dx, p1.y + dy, LineColor);
				dc.SetPixelV(p2.x + dx, p2.y + dy, LineColor);
			}
		}
	}
}
void CPaint3Dlg::DrawArcPreview(float angle, bool direction, CPoint p1, CPoint p2, CDC& dc)
{
	if (fabs(angle) < 1e-6) {
		dc.MoveTo(p1);
		dc.LineTo(p2);
		return;
	}

	double x1 = p1.x, y1 = p1.y;
	double x2 = p2.x, y2 = p2.y;
	double dx = x2 - x1, dy = y2 - y1;
	double d = sqrt(dx * dx + dy * dy);
	if (d < 1e-6) return;

	double halfAngle = fabs(angle) / 2.0;
	double r = (d / 2.0) / sin(halfAngle);
	double h = sqrt(max(0.0, r * r - (d / 2.0) * (d / 2.0)));

	// 中点与垂线方向
	double mx = (x1 + x2) / 2.0;
	double my = (y1 + y2) / 2.0;
	double ux = -dy / d, uy = dx / d;

	// 候选圆心
	double cx1 = mx + ux * h;
	double cy1 = my + uy * h;
	double cx2 = mx - ux * h;
	double cy2 = my - uy * h;

	auto arcSpan = [&](double cx, double cy) {
		double a1 = atan2(y1 - cy, x1 - cx);
		double a2 = atan2(y2 - cy, x2 - cx);
		double da = a2 - a1;
		if (da < 0) da += 2 * M_PI;
		return da; // CCW span
		};

	// CCW跨度
	double span1 = arcSpan(cx1, cy1);
	double span2 = arcSpan(cx2, cy2);

	// 目标弧度长度
	double target = fmod(fabs(angle), 2 * M_PI);
	if (target < 0) target += 2 * M_PI;

	// 根据方向调整比较的跨度
	double desiredSpan;
	if (direction) {
		desiredSpan = target; // 逆时针
	}
	else {
		desiredSpan = fmod(2 * M_PI - target, 2 * M_PI); // 顺时针
	}

	// 选出合适圆心
	double diff1 = fabs(span1 - desiredSpan);
	double diff2 = fabs(span2 - desiredSpan);
	double cx, cy;
	if (diff1 < diff2)
		cx = cx1, cy = cy1;
	else
		cx = cx2, cy = cy2;

	// 计算实际起始角度（数学坐标系下，逆时针为正）
	double startRad = atan2(y1 - cy, x1 - cx);
	double sweepRad = direction ? angle : -angle;

	double startDeg = -startRad * 180.0 / M_PI;
	double sweepDeg = -sweepRad * 180.0 / M_PI;

	double sx = cx + r * cos(startRad);
	double sy = cy + r * sin(startRad);
	dc.MoveTo((int)round(sx), (int)round(sy));
	dc.AngleArc((int)round(cx), (int)round(cy), (int)round(r), (float)startDeg, (float)sweepDeg);
}