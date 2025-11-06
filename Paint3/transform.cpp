#include "pch.h"
#include "Paint3Dlg.h"
#include <cmath>
#include <algorithm> // 可选
#include <cstdint>   // 可选
#include <numeric> // For std::accumulate

// -------------------------------------------------------------------
// 内部辅助函数（匿名命名空间）
// -------------------------------------------------------------------
namespace {

    struct Mat3 {
        double m[3][3];
        static Mat3 I() {
            Mat3 A{};
            A.m[0][0] = A.m[1][1] = A.m[2][2] = 1.0;
            A.m[0][1] = A.m[0][2] = A.m[1][0] = A.m[1][2] = A.m[2][0] = A.m[2][1] = 0.0;
            return A;
        }
        static Mat3 Translate(double dx, double dy) {
            Mat3 A = I();
            A.m[0][2] = dx;
            A.m[1][2] = dy;
            return A;
        }
        static Mat3 Rotate(double rad) {
            Mat3 A = I();
            const double c = std::cos(rad);
            const double s = std::sin(rad);
            A.m[0][0] = c; A.m[0][1] = -s;
            A.m[1][0] = s; A.m[1][1] = c;
            return A;
        }
        static Mat3 Scale(double sx, double sy) {
            Mat3 A = I();
            A.m[0][0] = sx;
            A.m[1][1] = sy;
            return A;
        }
    };

    inline Mat3 operator*(const Mat3& A, const Mat3& B) {
        Mat3 C{};
        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                C.m[r][c] = A.m[r][0] * B.m[0][c] + A.m[r][1] * B.m[1][c] + A.m[r][2] * B.m[2][c];
            }
        }
        return C;
    }

    struct PointD { double x, y; };

    inline PointD apply(const Mat3& M, double x, double y) {
        PointD q;
        q.x = M.m[0][0] * x + M.m[0][1] * y + M.m[0][2];
        q.y = M.m[1][0] * x + M.m[1][1] * y + M.m[1][2];
        return q; // 仿射情况 w 恒为 1，无需归一化
    }

    inline CPoint roundPt(double x, double y) {
        return CPoint((LONG)std::lround(x), (LONG)std::lround(y));
    }
    // —— 用矩阵重写：围绕 ref 旋转 ——
    // 签名保持不变：CPoint RotatePoint(const CPoint& p, const CPoint& ref, double angleRad)
    CPoint RotatePoint(const CPoint& p, const CPoint& ref, double angleRad)
    {
        // M = T(ref) * R(angle) * T(-ref)
        Mat3 T1 = Mat3::Translate(-static_cast<double>(ref.x), -static_cast<double>(ref.y));
        Mat3 R = Mat3::Rotate(angleRad);
        Mat3 T2 = Mat3::Translate(static_cast<double>(ref.x), static_cast<double>(ref.y));
        Mat3 M = T2 * R * T1;

        PointD q = apply(M, static_cast<double>(p.x), static_cast<double>(p.y));
        return roundPt(q.x, q.y);
    }

    // —— 用矩阵重写：围绕 ref 缩放（非等比缩放也可） ——
    // 签名保持不变：CPoint ScalePoint(const CPoint& p, const CPoint& ref, double sx, double sy)
    CPoint ScalePoint(const CPoint& p, const CPoint& ref, double sx, double sy)
    {
        // M = T(ref) * S(sx, sy) * T(-ref)
        Mat3 T1 = Mat3::Translate(-static_cast<double>(ref.x), -static_cast<double>(ref.y));
        Mat3 S = Mat3::Scale(sx, sy);
        Mat3 T2 = Mat3::Translate(static_cast<double>(ref.x), static_cast<double>(ref.y));
        Mat3 M = T2 * S * T1;

        PointD q = apply(M, static_cast<double>(p.x), static_cast<double>(p.y));
        return roundPt(q.x, q.y);
    }
} // 匿名命名空间结束

// -------------------------------------------------------------------
// 重心计算 (CPaint3Dlg 成员函数实现)
// -------------------------------------------------------------------

/**
 * @brief 计算直线的重心（中点）
 */
CPoint CPaint3Dlg::GetCentroid(const LineObject& line) {
    return CPoint((line.start.x + line.end.x) / 2, (line.start.y + line.end.y) / 2);
}

/**
 * @brief 计算椭圆/圆的重心（矩形中心）
 */
CPoint CPaint3Dlg::GetCentroid(const EllipseObject& ell) {
    return ell.rect.CenterPoint();
}

/**
 * @brief 计算多边形的重心（所有顶点的平均值）
 * @note 这是一个简化的重心（顶点重心），对于实验目的足够。
 */
CPoint CPaint3Dlg::GetCentroid(const PolygonObject& poly) {
    if (poly.points.empty()) return CPoint(0, 0);
    double sumX = 0, sumY = 0;
    for (const CPoint& p : poly.points) {
        sumX += p.x;
        sumY += p.y;
    }
    return CPoint(
        static_cast<long>(sumX / poly.points.size()),
        static_cast<long>(sumY / poly.points.size())
    );
}

/**
 * @brief 计算圆弧的重心（近似为弦的中点）
 * @note 更精确的重心是圆心，但这需要从 start/end/angle 反算圆心，比较复杂。
 * 弦中点作为一个稳定的参考点是足够的。
 */
CPoint CPaint3Dlg::GetCentroid(const ArcObject& arc) {
    return CPoint((arc.start.x + arc.end.x) / 2, (arc.start.y + arc.end.y) / 2);
}
// 计算贝塞尔的中心
CPoint CPaint3Dlg::GetCentroid(const BezierObject& bz) {
    if (bz.ctrl.empty()) return CPoint(0, 0);
    double sx = 0, sy = 0;
    for (const auto& p : bz.ctrl) { sx += p.x; sy += p.y; }
    return CPoint((long)std::lround(sx / bz.ctrl.size()),
        (long)std::lround(sy / bz.ctrl.size()));
}

/**
 * @brief 计算所有当前选中图形的“平均重心”
 * @return CPoint 平均重心点
 */
CPoint CPaint3Dlg::GetAverageCentroidOfSelection() {
    double sumX = 0, sumY = 0;
    int count = 0;

    for (const auto& line : Lines) {
        if (line.selected && line.visible) {
            CPoint c = GetCentroid(line);
            sumX += c.x; sumY += c.y; count++;
        }
    }
    for (const auto& ell : Ellipses) {
        if (ell.selected && ell.visible) {
            CPoint c = GetCentroid(ell);
            sumX += c.x; sumY += c.y; count++;
        }
    }
    for (const auto& poly : Polygons) {
        if (poly.selected && poly.visible) {
            CPoint c = GetCentroid(poly);
            sumX += c.x; sumY += c.y; count++;
        }
    }
    for (const auto& arc : Arcs) {
        if (arc.selected && arc.visible) {
            CPoint c = GetCentroid(arc);
            sumX += c.x; sumY += c.y; count++;
        }
    }
    for (const auto& bz : Beziers) {
        if (bz.selected && bz.visible) {
            CPoint c = GetCentroid(bz);
            sumX += c.x; sumY += c.y; count++;
        }
    }

    if (count == 0) return CPoint(0, 0);
    return CPoint(static_cast<long>(sumX / count), static_cast<long>(sumY / count));
}

// -------------------------------------------------------------------
// 图形变换 (CPaint3Dlg 成员函数实现)
// -------------------------------------------------------------------

/**
 * @brief 对所有选中的图形应用平移变换
 * @param tx X方向位移
 * @param ty Y方向位移
 */
void CPaint3Dlg::ApplyTranslationToSelection(int tx, int ty) {
    // 1. 平移直线
    for (auto& line : Lines) {
        if (line.selected && line.visible) {
            line.start.x += tx; line.start.y += ty;
            line.end.x += tx;   line.end.y += ty;
        }
    }
    // 2. 平移椭圆 (通过平移矩形实现)
    for (auto& ell : Ellipses) {
        if (ell.selected && ell.visible) {
            ell.rect.OffsetRect(tx, ty);
        }
    }
    // 3. 平移多边形 (平移所有顶点)
    for (auto& poly : Polygons) {
        if (poly.selected && poly.visible) {
            for (CPoint& p : poly.points) {
                p.x += tx; p.y += ty;
            }
        }
    }
    // 4. 平移圆弧 (平移起点和终点)
    for (auto& arc : Arcs) {
        if (arc.selected && arc.visible) {
            arc.start.x += tx; arc.start.y += ty;
            arc.end.x += tx;   arc.end.y += ty;
        }
    }
    for (auto& bz : Beziers) {
        if (bz.selected && bz.visible) {
            for (auto& p : bz.ctrl) { p.x += tx; p.y += ty; }
            bz.cacheValid = false;
        }
    }

}

/**
 * @brief 对所有选中的图形应用缩放变换
 * @param ref 参考点
 * @param sx X轴缩放因子
 * @param sy Y轴缩放因子
 */
void CPaint3Dlg::ApplyScalingToSelection(const CPoint& ref, double sx, double sy) {
    // 1. 缩放直线
    for (auto& line : Lines) {
        if (line.selected && line.visible) {
            line.start = ScalePoint(line.start, ref, sx, sy);
            line.end = ScalePoint(line.end, ref, sx, sy);
        }
    }
    // 2. 缩放椭圆 (缩放矩形的左上角和右下角)
    for (auto& ell : Ellipses) {
        if (ell.selected && ell.visible) {
            CPoint newTopLeft = ScalePoint(ell.rect.TopLeft(), ref, sx, sy);
            CPoint newBottomRight = ScalePoint(ell.rect.BottomRight(), ref, sx, sy);
            ell.rect.SetRect(newTopLeft, newBottomRight);
            ell.rect.NormalizeRect(); // 确保 left < right, top < bottom
        }
    }
    // 3. 缩放多边形
    for (auto& poly : Polygons) {
        if (poly.selected && poly.visible) {
            for (CPoint& p : poly.points) {
                p = ScalePoint(p, ref, sx, sy);
            }
        }
    }
    // 4. 缩放圆弧
    for (auto& arc : Arcs) {
        if (arc.selected && arc.visible) {
            arc.start = ScalePoint(arc.start, ref, sx, sy);
            arc.end = ScalePoint(arc.end, ref, sx, sy);
            // 注意：非均匀缩放(sx!=sy)会使圆弧变为椭圆弧。
            // 我们的ArcObject无法表示椭圆弧，因此变换后的视觉可能不完美，
            // 但对于实验要求（变换起点终点）是足够的。
        }
    }
    for (auto& bz : Beziers) {
        if (bz.selected && bz.visible) {
            for (auto& p : bz.ctrl) p = ScalePoint(p, ref, sx, sy);
            bz.cacheValid = false;
        }
    }

}

/**
 * @brief 对所有选中的图形应用旋转变换
 * @param ref 参考点
 * @param angleRad 旋转角度（弧度）
 */
void CPaint3Dlg::ApplyRotationToSelection(const CPoint& ref, double angleRad) {
    // 1. 旋转直线
    for (auto& line : Lines) {
        if (line.selected && line.visible) {
            line.start = RotatePoint(line.start, ref, angleRad);
            line.end = RotatePoint(line.end, ref, angleRad);
        }
    }
    
    // 2. 旋转椭圆
    // !!! 重要：旋转一个（非圆）椭圆（CRect）会产生一个倾斜的椭圆。
    // CRect 无法存储倾斜的矩形。
    // 实验要求：将旋转后的椭圆转换为“多边形”来近似。
    // 我们需要一个临时列表来存储新产生的多边形。
    std::vector<PolygonObject> newPolygons;
    for (auto it = Ellipses.begin(); it != Ellipses.end(); /* no increment */) {
        auto& ell = *it;
        if (ell.selected && ell.visible) {
            // 如果是正圆 (Shift 绘制的)，旋转是无损的，只需旋转中心点
            if (ell.rect.Width() == ell.rect.Height()) {
                CPoint center = ell.rect.CenterPoint();
                CPoint newCenter = RotatePoint(center, ref, angleRad);
                ell.rect.MoveToXY(newCenter.x - ell.rect.Width() / 2, newCenter.y - ell.rect.Height() / 2);
                ++it;
            } else {
                // 不是正圆，必须转换为多边形
                PolygonObject poly;
                poly.id = ++idPolygon; // 需要访问 CPaint3Dlg::idPolygon
                poly.lineWidth = ell.lineWidth;
                poly.lineType = ell.lineType;
                poly.color = ell.color;
                poly.isfilled = ell.isfilled;
                poly.fillColor = ell.fillColor;
                poly.selected = true; // 保持选中状态
                poly.visible = true;
                poly.algorithm = 8; // Default Polygon

                // 取椭圆上的点（例如4个顶点 + 4个中点）
                CPoint p[8];
                p[0] = ell.rect.TopLeft();
                p[1] = CPoint(ell.rect.CenterPoint().x, ell.rect.top);
                p[2] = CPoint(ell.rect.right, ell.rect.top);
                p[3] = CPoint(ell.rect.right, ell.rect.CenterPoint().y);
                p[4] = ell.rect.BottomRight();
                p[5] = CPoint(ell.rect.CenterPoint().x, ell.rect.bottom);
                p[6] = CPoint(ell.rect.left, ell.rect.bottom);
                p[7] = CPoint(ell.rect.left, ell.rect.CenterPoint().y);
                
                for(int i=0; i<8; ++i) {
                    poly.points.push_back(RotatePoint(p[i], ref, angleRad));
                }

                newPolygons.push_back(poly);
                
                // 删除原椭圆
                it = Ellipses.erase(it);
            }
        } else {
            ++it;
        }
    }
    // 添加新转换的多边形
    Polygons.insert(Polygons.end(), newPolygons.begin(), newPolygons.end());


    // 3. 旋转多边形
    for (auto& poly : Polygons) {
        // 注意：这里我们只旋转 *原始* 选中的多边形
        // 刚刚从椭圆转换来的多边形不应被再次旋转（因为它们已经处于旋转后位置）
        if (poly.selected && poly.visible && std::find_if(newPolygons.begin(), newPolygons.end(), 
            [&](const PolygonObject& p) { return p.id == poly.id; }) == newPolygons.end()) 
        {
            for (CPoint& p : poly.points) {
                p = RotatePoint(p, ref, angleRad);
            }
        }
    }

    // 4. 旋转圆弧
    for (auto& arc : Arcs) {
        if (arc.selected && arc.visible) {
            arc.start = RotatePoint(arc.start, ref, angleRad);
            arc.end = RotatePoint(arc.end, ref, angleRad);
            // 旋转起点和终点，圆心角(angle)和方向(direction)保持不变
        }
    }
    for (auto& bz : Beziers) {
        if (bz.selected && bz.visible) {
            for (auto& p : bz.ctrl) p = RotatePoint(p, ref, angleRad);
            bz.cacheValid = false;
        }
    }

}


/**
 * @brief 取消所有图形的选中状态
 */
void CPaint3Dlg::DeselectAll()
{
    for (auto& line : Lines) line.selected = false;
    for (auto& ell : Ellipses) ell.selected = false;
    for (auto& poly : Polygons) poly.selected = false;
    for (auto& arc : Arcs) arc.selected = false;
    for (auto& bz : Beziers) bz.selected = false;

    selCnt = 0;
}