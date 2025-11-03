#include "pch.h"
#include <afxwin.h>       
#include "Paint3Dlg.h"    // 需要完整类型
#include "AlgMenu.h"

namespace {

    // 小工具：往算法下拉框加一项并写入稳定的算法ID
    inline void AddAlgoItem(CPaint3Dlg& dlg, LPCTSTR text, int algoId) {
        const int idx = dlg.m_algorithm.AddString(text);
        dlg.m_algorithm.SetItemData(idx, static_cast<DWORD_PTR>(algoId));
    }

} // anonymous namespace

void AlgMenu::RefreshAlgorithmListForMode(CPaint3Dlg& dlg)
{
    dlg.m_algorithm.ResetContent();
    dlg.m_algorithm.EnableWindow(TRUE);

    switch (dlg.Mode) {
    case 0: // PEN
        dlg.m_algorithm.EnableWindow(FALSE);
        dlg.Algorithm = -1;
        return;

    case 1: // LINE
        AddAlgoItem(dlg, _T("Default Line"), 0);
        AddAlgoItem(dlg, _T("DDA Line Algorithm"), 1);
        AddAlgoItem(dlg, _T("Midpoint Line Algorithm"), 2);
        AddAlgoItem(dlg, _T("Bresenham Line Algorithm"), 3);
        break;

    case 2: // ELLIPSE/CIRCLE
        AddAlgoItem(dlg, _T("Default Circle"), 4);
        AddAlgoItem(dlg, _T("Midpoint Circle"), 5);
        AddAlgoItem(dlg, _T("Bresenham Circle"), 6);
        break;

    case 3: // ARC
        AddAlgoItem(dlg, _T("Bresenham Arc"), 7);
        break;

    case 4: // POLYGON
        AddAlgoItem(dlg, _T("Default Polygon"), 8);
        break;

    case 5: // FILL
        AddAlgoItem(dlg, _T("Scanline Fill"), 9); // 先占位/待实现
        break;

    case 6: // CLIP 矩形/多边形裁剪窗口
        AddAlgoItem(dlg, _T("Rect Clip Window"), 9);
        AddAlgoItem(dlg, _T("Polygon Clip Window"), 10);
        break;

    case 7: // SELECT / TRANSFORM
        AddAlgoItem(dlg, _T("Translation"), 11); // 你现有选择/高亮逻辑判断的是 11
        AddAlgoItem(dlg, _T("Scaling"), 12);
        AddAlgoItem(dlg, _T("Rotation"), 13);
        break;
    }

    // 设默认选中第一项并同步写回 Algorithm
    if (dlg.m_algorithm.GetCount() > 0) {
        dlg.m_algorithm.SetCurSel(0);
        const int sel = dlg.m_algorithm.GetCurSel();
        dlg.Algorithm = static_cast<int>(dlg.m_algorithm.GetItemData(sel));
    }
    else {
        dlg.Algorithm = -1;
    }
}
