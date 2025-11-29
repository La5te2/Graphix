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

enum OutCode { INSIDE = 0, LEFT = 1, RIGHT = 2, BOTTOM = 4, TOP = 8 };

OutCode ComputeOutCode(CPoint p, CRect clip)
{
	OutCode code = INSIDE;
	if (p.x < clip.left) code = (OutCode)(code | LEFT);
	else if (p.x > clip.right) code = (OutCode)(code | RIGHT);
	if (p.y < clip.top) code = (OutCode)(code | TOP);
	else if (p.y > clip.bottom) code = (OutCode)(code | BOTTOM);
	return code;
}

bool CPaint3Dlg::ClipLineCohenSutherland(CPoint& p1, CPoint& p2, CRect clip)
{
	OutCode out1 = ComputeOutCode(p1, clip);
	OutCode out2 = ComputeOutCode(p2, clip);

	while (true)
	{
		if (!(out1 | out2)) return true; // 全可见
		if (out1 & out2) return false;   // 完全不可见

		CPoint p;
		OutCode outOut = out1 ? out1 : out2;

		if (outOut & TOP)
		{
			p.x = p1.x + (p2.x - p1.x) * (clip.top - p1.y) / (p2.y - p1.y);
			p.y = clip.top;
		}
		else if (outOut & BOTTOM)
		{
			p.x = p1.x + (p2.x - p1.x) * (clip.bottom - p1.y) / (p2.y - p1.y);
			p.y = clip.bottom;
		}
		else if (outOut & RIGHT)
		{
			p.y = p1.y + (p2.y - p1.y) * (clip.right - p1.x) / (p2.x - p1.x);
			p.x = clip.right;
		}
		else if (outOut & LEFT)
		{
			p.y = p1.y + (p2.y - p1.y) * (clip.left - p1.x) / (p2.x - p1.x);
			p.x = clip.left;
		}

		if (outOut == out1) { p1 = p; out1 = ComputeOutCode(p1, clip); }
		else { p2 = p; out2 = ComputeOutCode(p2, clip); }
	}
}


bool CPaint3Dlg::ClipLineMidpoint(CPoint& p1, CPoint& p2, CRect clip)
{
	auto isInside = [&](CPoint p) {
		return clip.PtInRect(p) != FALSE;
		};

	// 两端都在内部
	if (isInside(p1) && isInside(p2))
		return true;

	// 两端都在外部并在矩形同侧（快速拒绝）
	if ((p1.x < clip.left && p2.x < clip.left) ||
		(p1.x > clip.right && p2.x > clip.right) ||
		(p1.y < clip.top && p2.y < clip.top) ||
		(p1.y > clip.bottom && p2.y > clip.bottom))
		return false;

	// 递归停止条件
	if (abs(p1.x - p2.x) <= 1 && abs(p1.y - p2.y) <= 1)
	{
		// 若有一点在内部则保留
		if (isInside(p1) || isInside(p2))
		{
			// 调整到最近的内部点
			if (!isInside(p1)) {
				if (p1.x < clip.left) p1.x = clip.left;
				if (p1.x > clip.right) p1.x = clip.right;
				if (p1.y < clip.top) p1.y = clip.top;
				if (p1.y > clip.bottom) p1.y = clip.bottom;
			}
			if (!isInside(p2)) {
				if (p2.x < clip.left) p2.x = clip.left;
				if (p2.x > clip.right) p2.x = clip.right;
				if (p2.y < clip.top) p2.y = clip.top;
				if (p2.y > clip.bottom) p2.y = clip.bottom;
			}
			return true;
		}
		return false;
	}

	// 计算中点
	CPoint mid((p1.x + p2.x) / 2, (p1.y + p2.y) / 2);

	bool left = ClipLineMidpoint(p1, mid, clip);
	bool right = ClipLineMidpoint(mid, p2, clip);

	if (left && right)
	{
		// 找到两段都在可见区的交界中点
		p1 = p1;
		p2 = p2;
		return true;
	}
	else if (left)
	{
		p2 = mid;
		return true;
	}
	else if (right)
	{
		p1 = mid;
		return true;
	}
	return false;
}

// 辅助：向量运算
static inline double dotd(double ax, double ay, double bx, double by) { return ax * bx + ay * by; }
struct DPoint { double x, y; DPoint(double _x = 0, double _y = 0) :x(_x), y(_y) {} };
static inline DPoint toD(const CPoint& p) { return DPoint(p.x, p.y); }
static inline CPoint toC(const DPoint& p) { return CPoint((int)round(p.x), (int)round(p.y)); }

// 计算凸裁剪多边形是否为 CCW
static double polygonArea(const std::vector<CPoint>& poly) {
	double A = 0;
	for (size_t i = 0; i < poly.size(); ++i) {
		CPoint a = poly[i], b = poly[(i + 1) % poly.size()];
		A += (double)a.x * b.y - (double)b.x * a.y;
	}
	return A * 0.5;
}

// Cyrus–Beck：p1,p2 作为引用返回裁剪后线段（若可见返回 true）
bool CPaint3Dlg::CyrusBeckClipLine(CPoint& p1, CPoint& p2, const std::vector<CPoint>& clipPoly)
{
	if (clipPoly.size() < 3) return false;
	// ensure CCW order for normals outward calc; if area < 0 then poly is CW
	bool polyCCW = polygonArea(clipPoly) > 0;

	DPoint P0 = toD(p1), P1 = toD(p2);
	DPoint d = DPoint(P1.x - P0.x, P1.y - P0.y);

	double tE = 0.0; // max entering
	double tL = 1.0; // min leaving

	size_t n = clipPoly.size();
	for (size_t i = 0; i < n; ++i) {
		DPoint A = toD(clipPoly[i]);
		DPoint B = toD(clipPoly[(i + 1) % n]);

		// edge vector from A to B
		DPoint e = DPoint(B.x - A.x, B.y - A.y);
		// outward normal n = (e.y, -e.x) if poly is CCW? For CCW, inward normal is (-e.y, e.x)
		// We want normal pointing outward: for CCW polygon outward = ( -e.y, e.x )? let's derive:
		// For CCW, interior is left of each edge (A->B); outward is right => normal = ( e.y, -e.x )
		DPoint normal;
		if (polyCCW) normal = DPoint(e.y, -e.x);
		else normal = DPoint(-e.y, e.x);

		// compute numerator and denominator: n·(A - P0) and n·d
		double num = normal.x * (A.x - P0.x) + normal.y * (A.y - P0.y);
		double denom = normal.x * d.x + normal.y * d.y;

		if (fabs(denom) < 1e-12) {
			// line parallel to edge
			if (num < 0) {
				// line is outside (pointing outside), reject
				return false;
			}
			else {
				// parallel and inside wrt this edge, continue
				continue;
			}
		}
		else {
			double t = num / denom;
			if (denom < 0) {
				// potential entering
				if (t > tE) tE = t;
			}
			else {
				// denom > 0 potential leaving
				if (t < tL) tL = t;
			}
			if (tE > tL) return false; // trivially reject
		}
	}

	// if there is intersection
	if (tE <= tL) {
		DPoint Cp0 = DPoint(P0.x + d.x * tE, P0.y + d.y * tE);
		DPoint Cp1 = DPoint(P0.x + d.x * tL, P0.y + d.y * tL);
		p1 = toC(Cp0);
		p2 = toC(Cp1);
		return true;
	}
	return false;
}

// 判断点在针对一条裁剪边的内部（假设 clip edge from A->B）
static inline bool isInsideEdge(const DPoint& pt, const DPoint& A, const DPoint& B, bool clipIsCCW) {
	// for edge A->B, inside is left side if clip poly is CCW
	double cross = (B.x - A.x) * (pt.y - A.y) - (B.y - A.y) * (pt.x - A.x);
	return clipIsCCW ? (cross >= 0) : (cross <= 0);
}

// 计算交点（直线 AB 与直线 segment S->E 的交点）
// returns intersection in double coordinates
static DPoint intersectLineLine(const DPoint& A, const DPoint& B, const DPoint& S, const DPoint& E) {
	// solve A + u*(B-A) and S + t*(E-S)
	double a1 = B.x - A.x, b1 = S.x - E.x, c1 = S.x - A.x;
	double a2 = B.y - A.y, b2 = S.y - E.y, c2 = S.y - A.y;
	double denom = a1 * b2 - a2 * b1;
	if (fabs(denom) < 1e-12) return DPoint((S.x + E.x) / 2.0, (S.y + E.y) / 2.0); // parallel, fallback
	double u = (c1 * b2 - c2 * b1) / denom;
	return DPoint(A.x + u * (B.x - A.x), A.y + u * (B.y - A.y));
}

std::vector<CPoint> CPaint3Dlg::SutherlandHodgmanClipPolygon(const std::vector<CPoint>& subject, const std::vector<CPoint>& clipPoly)
{
	std::vector<DPoint> out;
	if (subject.empty()) return {};
	if (clipPoly.size() < 3) return {};

	bool clipCCW = polygonArea(clipPoly) > 0;

	// start with subject in double space
	std::vector<DPoint> input;
	input.reserve(subject.size());
	for (auto& pt : subject) input.emplace_back(pt.x, pt.y);

	// for each clip edge
	size_t m = clipPoly.size();
	for (size_t i = 0; i < m; ++i) {
		DPoint A = toD(clipPoly[i]);
		DPoint B = toD(clipPoly[(i + 1) % m]);
		out.clear();
		if (input.empty()) break;

		DPoint S = input.back();
		for (auto& E : input) {
			bool Ein = isInsideEdge(E, A, B, clipCCW);
			bool Sin = isInsideEdge(S, A, B, clipCCW);
			if (Sin && Ein) {
				// both inside -> keep E
				out.push_back(E);
			}
			else if (Sin && !Ein) {
				// leaving: add intersection
				DPoint ip = intersectLineLine(A, B, S, E);
				out.push_back(ip);
			}
			else if (!Sin && Ein) {
				// entering: add intersection then E
				DPoint ip = intersectLineLine(A, B, S, E);
				out.push_back(ip);
				out.push_back(E);
			} // else both outside -> nothing
			S = E;
		}
		input = out; // next phase
	}
	// convert back
	std::vector<CPoint> result;
	result.reserve(input.size());
	for (auto& dpt : input) result.emplace_back((int)round(dpt.x), (int)round(dpt.y));
	return result;
}