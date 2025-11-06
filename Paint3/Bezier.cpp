#include "pch.h"
#include "Paint3Dlg.h"
#include <cmath>

// ----------------- 内部工具 -----------------
namespace {
    inline double dist2(const CPoint& a, const CPoint& b) {
        const double dx = a.x - b.x, dy = a.y - b.y;
        return dx * dx + dy * dy;
    }
    // de Casteljau：给一组控制点，在 t∈[0,1] 处评估
    static CPoint Casteljau(const std::vector<CPoint>& ctrl, double t) {
        if (ctrl.empty()) return CPoint(0, 0);
        std::vector<POINT> p(ctrl.begin(), ctrl.end());
        int n = (int)p.size();
        for (int r = 1; r < n; ++r) {
            for (int i = 0; i < n - r; ++i) {
                p[i].x = (LONG)std::lround((1.0 - t) * p[i].x + t * p[i + 1].x);
                p[i].y = (LONG)std::lround((1.0 - t) * p[i].y + t * p[i + 1].y);
            }
        }
        return CPoint(p[0].x, p[0].y);
    }
    // 自定义实现 clamp 函数
    template <typename T>
    T clamp(const T& value, const T& low, const T& high) {
        return (value < low) ? low : (value > high) ? high : value;
    }

}

// ----------------- 采样/缓存 -----------------
void CPaint3Dlg::EnsureBezierCache(BezierObject& bz) {
    if (bz.cacheValid || bz.ctrl.size() < 2) return;
    bz.cachedPolyline.clear();
    // 依据几何长度自适应步数（更平滑）
    double L = 0;
    for (size_t i = 1; i < bz.ctrl.size(); ++i)
        L += std::hypot(bz.ctrl[i].x - bz.ctrl[i - 1].x, bz.ctrl[i].y - bz.ctrl[i - 1].y);
    int steps = (int)clamp((int)std::ceil(L / 5.0), 24, 256); // 视距分辨率
    for (int i = 0; i <= steps; ++i) {
        double t = (double)i / steps;
        bz.cachedPolyline.push_back(Casteljau(bz.ctrl, t));
    }
    bz.cacheValid = true;
}

// 折线绘制工具（尊重线宽/线型）
static void DrawPolyline(CDC& dc, const std::vector<CPoint>& pts,
    COLORREF color, int lineWidth, int lineType)
{
    if (pts.size() < 2) return;
    int penStyle = lineType ? PS_DASH : PS_SOLID;
    LOGBRUSH lb{ BS_SOLID, color, 0 };
    CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, max(1, lineWidth), &lb);
    CPen* oldPen = dc.SelectObject(&pen);
    for (size_t i = 1; i < pts.size(); ++i) {
        dc.MoveTo(pts[i - 1]);
        dc.LineTo(pts[i]);
    }
    dc.SelectObject(oldPen);
}

// ----------------- 对外：画 Bezier -----------------
void CPaint3Dlg::DrawBezierFM(CDC& dc, BezierObject& bz) {
    EnsureBezierCache(bz);
    DrawPolyline(dc, bz.cachedPolyline, bz.color, bz.lineWidth, bz.lineType);
}

// 命中：对缓存折线做“到线段最短距离”判定
bool CPaint3Dlg::IsPointNearBezier(const CPoint& p, BezierObject& bz) {
    EnsureBezierCache(bz);
    const double tol = (bz.lineWidth / 2.0) + 3.0;
    const double tol2 = tol * tol;
    const auto& poly = bz.cachedPolyline;
    for (size_t i = 1; i < poly.size(); ++i) {
        // 点到线段距离平方
        const CPoint A = poly[i - 1], B = poly[i];
        const double vx = B.x - A.x, vy = B.y - A.y;
        const double len2 = vx * vx + vy * vy;
        double t = 0.0;
        if (len2 > 1e-12) {
            t = ((p.x - A.x) * vx + (p.y - A.y) * vy) / len2;
            t = clamp(t, 0.0, 1.0);
        }
        CPoint H((LONG)std::lround(A.x + t * vx),
            (LONG)std::lround(A.y + t * vy));
        if (dist2(p, H) <= tol2) return true;
    }
    return false;
}
