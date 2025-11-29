// ==== BSpline.cpp ====
#include "pch.h"
#include "Paint3Dlg.h"
#include <cmath>

// 小工具：生成均匀夹持节点向量（clamped uniform）
static vector<double> MakeClampedUniformKnots(int nCtrl, int degree) {
    // nCtrl = n+1，节点数量 = nCtrl + degree + 1
    const int n = nCtrl - 1;
    const int m = n + degree + 1;
    vector<double> U(m + 1, 0.0);
    // 前后各重复 degree+1 次，中间均匀递增
    for (int j = 0; j <= m; ++j) {
        if (j <= degree) U[j] = 0.0;
        else if (j >= m - degree) U[j] = 1.0;
        else {
            // 中间线性分布
            double t = (double)(j - degree) / (double)(m - 2 * degree);
            U[j] = t;
        }
    }
    return U;
}

// de Boor 基函数：查找参数 t 所在的结点区间索引 k（U[k] <= t < U[k+1]）
static int FindSpan(double t, int nCtrl, int degree, const vector<double>& U) {
    const int n = nCtrl - 1;
    if (t >= U[n + 1]) return n; // t=1 的右端点情况
    int low = degree, high = n + 1, mid;
    while (low <= high) {
        mid = (low + high) / 2;
        if (t < U[mid]) high = mid - 1;
        else if (t >= U[mid + 1]) low = mid + 1;
        else return mid;
    }
    return n;
}

// de Boor：返回 p 次 B样条在参数 t ∈ [0,1] 的评估点
static CPoint DeBoorEval(const vector<CPoint>& P, int p, const vector<double>& U, double t)
{
    const int nCtrl = (int)P.size();
    if (nCtrl == 0) return CPoint(0, 0);
    if (nCtrl == 1) return P[0];
    p = max(1, min(p, nCtrl - 1));

    const int k = FindSpan(t, nCtrl, p, U);
    // 拿出 de Boor 控制点段：d[0..p]
    vector<double> dx(p + 1), dy(p + 1);
    for (int j = 0; j <= p; ++j) {
        int idx = k - p + j;
        idx = max(0, min(idx, nCtrl - 1));
        dx[j] = (double)P[idx].x;
        dy[j] = (double)P[idx].y;
    }
    for (int r = 1; r <= p; ++r) {
        for (int j = p; j >= r; --j) {
            int i = k - p + j;
            double denom = U[i + p + 1 - r] - U[i];
            double alpha = 0.0;
            if (abs(denom) > 1e-12) alpha = (t - U[i]) / denom;
            dx[j] = (1.0 - alpha) * dx[j - 1] + alpha * dx[j];
            dy[j] = (1.0 - alpha) * dy[j - 1] + alpha * dy[j];
        }
    }
    return CPoint((LONG)lround(dx[p]), (LONG)lround(dy[p]));
}

// —— 折线绘制（与 Bezier.cpp 的 DrawPolyline 同风格）
static void DrawPolylineBS(CDC& dc, const vector<CPoint>& pts,
    COLORREF color, int lineWidth, int lineType)
{
    if (pts.size() < 2) return;
    int penStyle = lineType ? PS_DASH : PS_SOLID;
    LOGBRUSH lb{ BS_SOLID, color, 0 };
    CPen pen(penStyle | PS_GEOMETRIC | PS_ENDCAP_ROUND, max(1, lineWidth), &lb);
    CPen* oldPen = dc.SelectObject(&pen);
    for (size_t i = 1; i < pts.size(); ++i) {
        dc.MoveTo(pts[i - 1]); dc.LineTo(pts[i]);
    }
    dc.SelectObject(oldPen);
}

// —— 采样缓存：按控制折线长度自适应步数（与 Bezier 一致）
void CPaint3Dlg::EnsureBSplineCache(BSplineObject& sp) {
    if (sp.cacheValid || sp.ctrl.size() < 2) return;
    if (sp.degree < 1) sp.degree = 1;
    sp.cachedPolyline.clear();

    // 若 knots 为空，则自动生成“均匀夹持”节点
    if (sp.knots.empty()) sp.knots = MakeClampedUniformKnots((int)sp.ctrl.size(), sp.degree);

    // 依据控制折线近似长度决定步数
    double L = 0.0;
    for (size_t i = 1; i < sp.ctrl.size(); ++i)
        L += hypot((double)sp.ctrl[i].x - sp.ctrl[i - 1].x,
            (double)sp.ctrl[i].y - sp.ctrl[i - 1].y);
    int steps = (int)max(24.0, min(256.0, ceil(L / 5.0)));

    for (int i = 0; i <= steps; ++i) {
        double t = (double)i / steps;                // 0..1
        sp.cachedPolyline.push_back(DeBoorEval(sp.ctrl, sp.degree, sp.knots, t));
    }
    sp.cacheValid = true;
}

void CPaint3Dlg::DrawBSplineFM(CDC& dc, BSplineObject& sp) {
    EnsureBSplineCache(sp);
    DrawPolylineBS(dc, sp.cachedPolyline, sp.color, sp.lineWidth, sp.lineType);
}

// —— 命中测试：对缓存折线做“点到线段距离”判定（与 Bezier 一致）
static inline double d2(const CPoint& a, const CPoint& b) {
    const double dx = (double)a.x - b.x, dy = (double)a.y - b.y;
    return dx * dx + dy * dy;
}
template <typename T> static T clampT(const T& v, const T& lo, const T& hi) {
    return v < lo ? lo : (v > hi ? hi : v);
}
bool CPaint3Dlg::IsPointNearBSpline(const CPoint& p, BSplineObject& sp) {
    EnsureBSplineCache(sp);
    const double tol = (sp.lineWidth / 2.0) + 3.0;
    const double tol2 = tol * tol;
    const auto& poly = sp.cachedPolyline;
    for (size_t i = 1; i < poly.size(); ++i) {
        CPoint A = poly[i - 1], B = poly[i];
        const double vx = B.x - A.x, vy = B.y - A.y;
        const double len2 = vx * vx + vy * vy;
        double t = 0.0;
        if (len2 > 1e-12) {
            t = ((p.x - A.x) * vx + (p.y - A.y) * vy) / len2;
            t = clampT(t, 0.0, 1.0);
        }
        CPoint H((LONG)lround(A.x + t * vx), (LONG)lround(A.y + t * vy));
        if (d2(p, H) <= tol2) return true;
    }
    return false;
}

