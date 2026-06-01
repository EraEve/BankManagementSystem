/**
 * smart_manager.h — 需求8: 智能管理 (创新功能)
 *
 * 功能:
 *   1. 交易异常报警 (大额交易 + 多笔交易检测)
 *   2. 每日/月利息批量计算
 *   3. 客户统计分析 & 信用评级 & 风控审批
 *
 * 算法核心:
 *   - 异常检测: 阈值判断 + 滑动窗口
 *   - 信用评分: 加权多因子模型
 *   - 统计分析: 均值/方差/分布
 */

#ifndef SMART_MANAGER_H
#define SMART_MANAGER_H

#include "common.h"

// ==================== 1. 交易异常报警 ====================

/** 检测大额交易 */
inline void detect_large_amount() {
    hr();
    cout << "  ═══ 大额交易报警 (>" << double_to_str(LARGE_AMOUNT) << "元) ═══\n";
    hr();
    bool found = false;
    for (size_t i = 0; i < g_transactions.size(); i++) {
        if (g_transactions[i].amount > LARGE_AMOUNT) {
            cout << "  ⚠ 大额交易! ID: " << g_transactions[i].id
                 << " 类型: " << g_transactions[i].type
                 << " 金额: " << double_to_str(g_transactions[i].amount)
                 << " 时间: " << g_transactions[i].time << endl;
            found = true;
        }
    }
    if (!found) cout << "  ✓ 未发现大额异常交易。\n";
    hr();
}

/** 检测高频交易 (滑动窗口: 同一客户1小时内多笔交易) */
inline void detect_multi_transactions() {
    hr();
    cout << "  ═══ 高频交易报警 (1小时内>" << MULTI_TRANSACTION << "笔) ═══\n";
    hr();
    // 按客户统计交易频率
    map<string, vector<const Transaction*> > cust_txns;
    for (size_t i = 0; i < g_transactions.size(); i++) {
        const Transaction& t = g_transactions[i];
        // 找到转出或转入的客户
        int fi = find_card(t.from_card);
        if (fi >= 0) cust_txns[g_cards[fi].customer_id].push_back(&t);
        int ti = find_card(t.to_card);
        if (ti >= 0) cust_txns[g_cards[ti].customer_id].push_back(&t);
    }

    bool found = false;
    for (map<string, vector<const Transaction*> >::iterator it = cust_txns.begin();
         it != cust_txns.end(); ++it) {
        const vector<const Transaction*>& txns = it->second;
        if ((int)txns.size() <= MULTI_TRANSACTION) continue;
        // 滑动窗口检测1小时内
        for (size_t i = 0; i < txns.size(); i++) {
            int count = 1;
            for (size_t j = i + 1; j < txns.size(); j++) {
                // 比较时间字符串(前面19个字符)
                string t1 = txns[i]->time.substr(0, 13); // 到小时
                string t2 = txns[j]->time.substr(0, 13);
                if (t1 == t2) count++;
                else break;
            }
            if (count > MULTI_TRANSACTION) {
                cout << "  ⚠ 客户 " << it->first << " 在1小时内有 "
                     << count << " 笔交易!\n";
                found = true;
                break;
            }
        }
    }
    if (!found) cout << "  ✓ 未发现高频异常交易。\n";
    hr();
}

// ==================== 2. 批量利息计算 ====================

/** 每日利息汇总 */
inline void calc_daily_interest_all() {
    hr();
    cout << "  ═══ 全行每日利息计算 ═══\n";
    hr();
    double total_interest = 0;
    for (size_t i = 0; i < g_cards.size(); i++) {
        const BankCard& c = g_cards[i];
        if (c.status != "正常") continue;
        double di = calc_daily_interest(c);
        total_interest += di;
        cout << "  " << c.id << " (" << c.type << "): 本金="
             << double_to_str(c.type=="信用卡"?c.loan_balance:c.balance)
             << " 日利息=" << double_to_str(di) << " 元" << endl;
    }
    cout << "  ─────────────────────\n";
    cout << "  全行日利息合计: " << double_to_str(total_interest) << " 元\n";
    hr();
}

/** 月利息汇总 */
inline void calc_monthly_interest_all() {
    hr();
    cout << "  ═══ 全行每月利息计算 ═══\n";
    hr();
    double total_interest = 0;
    for (size_t i = 0; i < g_cards.size(); i++) {
        const BankCard& c = g_cards[i];
        if (c.status != "正常") continue;
        double mi = calc_monthly_interest(c);
        total_interest += mi;
        cout << "  " << c.id << " (" << c.type << "): 本金="
             << double_to_str(c.type=="信用卡"?c.loan_balance:c.balance)
             << " 月利息=" << double_to_str(mi) << " 元" << endl;
    }
    cout << "  ─────────────────────\n";
    cout << "  全行月利息合计: " << double_to_str(total_interest) << " 元\n";
    hr();
}

// ==================== 3. 信用评级 & 风控审批 ====================

/**
 * 信用评分模型 (加权多因子):
 *   - 金融资产: 30%
 *   - 交易活跃度: 20%
 *   - 还款记录: 25%
 *   - 负债率: 25% (负相关)
 *
 * 评级:
 *   900+: AAA (极优)
 *   800-899: AA (优秀)
 *   700-799: A (良好)
 *   600-699: B (一般)
 *   <600: C (需关注)
 */
inline string get_credit_rating(double score) {
    if (score >= 900) return "AAA (极优)";
    if (score >= 800) return "AA (优秀)";
    if (score >= 700) return "A (良好)";
    if (score >= 600) return "B (一般)";
    return "C (需关注)";
}

inline void customer_statistics() {
    hr();
    cout << "  ═══ 客户统计分析 & 信用评级 ═══\n";
    hr();

    // 统计
    int vip_count = 0, normal_count = 0;
    double total_assets = 0, total_credit = 0;
    double max_assets = 0, min_assets = 1e18;
    vector<double> scores;

    for (size_t i = 0; i < g_customers.size(); i++) {
        const Customer& c = g_customers[i];
        if (!c.is_active) continue;
        if (c.type == "VIP") vip_count++; else normal_count++;
        total_assets += c.financial_assets;
        total_credit += c.credit_score;
        if (c.financial_assets > max_assets) max_assets = c.financial_assets;
        if (c.financial_assets < min_assets) min_assets = c.financial_assets;
        scores.push_back(c.credit_score);
    }

    int active_count = vip_count + normal_count;
    if (active_count == 0) { cout << "  无活跃客户\n"; hr(); return; }

    double avg_assets = total_assets / active_count;
    double avg_credit = total_credit / active_count;

    // 计算信用分标准差
    double variance = 0;
    for (size_t i = 0; i < scores.size(); i++)
        variance += (scores[i] - avg_credit) * (scores[i] - avg_credit);
    variance /= scores.size();
    double stddev = sqrt(variance);

    cout << "  活跃客户总数: " << active_count << endl;
    cout << "  VIP客户: " << vip_count << " ("
         << double_to_str(100.0*vip_count/active_count) << "%)\n";
    cout << "  普通客户: " << normal_count << " ("
         << double_to_str(100.0*normal_count/active_count) << "%)\n";
    cout << "  平均金融资产: " << double_to_str(avg_assets) << " 元\n";
    cout << "  最高金融资产: " << double_to_str(max_assets) << " 元\n";
    cout << "  最低金融资产: " << double_to_str(min_assets) << " 元\n";
    cout << "  平均信用评分: " << double_to_str(avg_credit) << endl;
    cout << "  信用评分标准差: " << double_to_str(stddev) << endl;
    hr();

    // 信用评级分布
    cout << "\n  ═══ 信用评级分布 ═══\n";
    hr();
    map<string, int> rating_dist;
    for (size_t i = 0; i < g_customers.size(); i++) {
        if (!g_customers[i].is_active) continue;
        rating_dist[get_credit_rating(g_customers[i].credit_score)]++;
    }
    for (map<string,int>::iterator it = rating_dist.begin(); it != rating_dist.end(); ++it)
        cout << "  " << left << setw(20) << it->first << it->second << " 人\n";
    hr();
}

/** 风控审批 */
inline void risk_approval() {
    cout << "\n  ═══ 风控审批 ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    const Customer& c = g_customers[idx];

    // 收集客户的所有卡
    double total_balance = 0, total_loan = 0, total_credit_limit = 0;
    for (size_t i = 0; i < c.card_ids.size(); i++) {
        int ci = find_card(c.card_ids[i]);
        if (ci >= 0) {
            total_balance += g_cards[ci].balance;
            total_loan += g_cards[ci].loan_balance;
            if (g_cards[ci].type == "信用卡")
                total_credit_limit += g_cards[ci].credit_limit;
        }
    }

    double debt_ratio = (total_balance + total_credit_limit > 0) ?
        total_loan / (total_balance + total_credit_limit) : 0;

    // 风控评分
    double risk_score = c.credit_score;
    // 负债率惩罚
    if (debt_ratio > 0.5) risk_score -= 100;
    if (debt_ratio > 0.8) risk_score -= 200;
    // 金融资产加分
    if (c.financial_assets > 100000) risk_score += 50;
    if (c.financial_assets > 500000) risk_score += 50;

    hr();
    cout << "  ═══ 风控审批报告 ═══\n";
    hr();
    cout << "  客户: " << c.name << " (" << c.id << ")\n";
    cout << "  类型: " << c.type << endl;
    cout << "  信用评分: " << double_to_str(c.credit_score)
         << " → " << get_credit_rating(c.credit_score) << endl;
    cout << "  金融资产: " << double_to_str(c.financial_assets) << " 元\n";
    cout << "  总余额: " << double_to_str(total_balance) << " 元\n";
    cout << "  总贷款: " << double_to_str(total_loan) << " 元\n";
    cout << "  总信用额度: " << double_to_str(total_credit_limit) << " 元\n";
    cout << "  负债率: " << double_to_str(debt_ratio * 100) << "%\n";
    cout << "  风控分数: " << double_to_str(risk_score) << " → "
         << get_credit_rating(risk_score) << endl;
    hr();

    if (risk_score >= 700) {
        cout << "  ✓ 审批结论: 通过 (低风险)\n";
    } else if (risk_score >= 600) {
        cout << "  ⚡ 审批结论: 条件通过 (中风险, 建议降低额度)\n";
    } else {
        cout << "  ✗ 审批结论: 拒绝 (高风险)\n";
    }
}

/** 投资顾问 (简化的资产配置建议) */
inline void investment_advisor() {
    cout << "\n  ═══ 投资顾问 ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    const Customer& c = g_customers[idx];

    // 计算总可用资金
    double cash = 0;
    for (size_t i = 0; i < c.card_ids.size(); i++) {
        int ci = find_card(c.card_ids[i]);
        if (ci >= 0 && g_cards[ci].type != "信用卡") cash += g_cards[ci].balance;
    }

    hr();
    cout << "  ═══ 投资建议 ═══\n";
    hr();
    cout << "  客户: " << c.name << " | 可用资金: " << double_to_str(cash) << " 元\n";
    cout << "  风险偏好: " << (c.credit_score > 750 ? "进取型" : c.credit_score > 600 ? "稳健型" : "保守型") << endl;
    hr();

    if (cash <= 0) {
        cout << "  当前无可投资资金。\n"; return;
    }

    if (c.credit_score > 750) {
        cout << "  建议配置 (进取型):\n";
        cout << "    - 股票型基金: " << double_to_str(cash * 0.4) << " 元 (40%)\n";
        cout << "    - 债券型基金: " << double_to_str(cash * 0.2) << " 元 (20%)\n";
        cout << "    - 定期存款:   " << double_to_str(cash * 0.2) << " 元 (20%)\n";
        cout << "    - 活期备用:   " << double_to_str(cash * 0.2) << " 元 (20%)\n";
    } else if (c.credit_score > 600) {
        cout << "  建议配置 (稳健型):\n";
        cout << "    - 债券型基金: " << double_to_str(cash * 0.35) << " 元 (35%)\n";
        cout << "    - 定期存款:   " << double_to_str(cash * 0.35) << " 元 (35%)\n";
        cout << "    - 货币基金:   " << double_to_str(cash * 0.15) << " 元 (15%)\n";
        cout << "    - 活期备用:   " << double_to_str(cash * 0.15) << " 元 (15%)\n";
    } else {
        cout << "  建议配置 (保守型):\n";
        cout << "    - 定期存款:   " << double_to_str(cash * 0.5) << " 元 (50%)\n";
        cout << "    - 货币基金:   " << double_to_str(cash * 0.25) << " 元 (25%)\n";
        cout << "    - 活期备用:   " << double_to_str(cash * 0.25) << " 元 (25%)\n";
    }
    hr();
}

inline void smart_menu() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║  需求8: 智能管理 (创新功能)      ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  [交易异常报警]                  ║\n";
        cout << "  ║  1. 大额交易检测                ║\n";
        cout << "  ║  2. 高频交易检测                ║\n";
        cout << "  ║  [利息计算]                      ║\n";
        cout << "  ║  3. 全行日利息批量计算          ║\n";
        cout << "  ║  4. 全行月利息批量计算          ║\n";
        cout << "  ║  [统计分析 & 风控]               ║\n";
        cout << "  ║  5. 客户统计分析 & 信用评级     ║\n";
        cout << "  ║  6. 风控审批                    ║\n";
        cout << "  ║  7. 投资顾问                    ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { detect_large_amount(); pause(); }
        else if (ch == "2") { detect_multi_transactions(); pause(); }
        else if (ch == "3") { calc_daily_interest_all(); pause(); }
        else if (ch == "4") { calc_monthly_interest_all(); pause(); }
        else if (ch == "5") { customer_statistics(); pause(); }
        else if (ch == "6") { risk_approval(); pause(); }
        else if (ch == "7") { investment_advisor(); pause(); }
        else if (ch == "0") break;
    }
}

#endif
