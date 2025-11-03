#pragma once

// 只前置声明，避免头文件相互包含过重
class CPaint3Dlg;

namespace AlgMenu {
    // 根据当前 Mode 重建“算法”下拉框，并写回 dlg.Algorithm
    void RefreshAlgorithmListForMode(CPaint3Dlg& dlg);
}
#pragma once
