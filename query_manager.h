/**
 * query_manager.h — 需求5: 业务查询
 *
 * 功能: 多关键字查询(时间/客户类型/银行卡/金额/收入支出类型)
 * 数据结构: 顺序查找 + 条件过滤 + 汇总统计
 * 算法: 冒泡排序(时间) + 快速排序(金额) + 二分查找
 */

#ifndef QUERY_MANAGER_H
#define QUERY_MANAGER_H

#include "common.h"

// ==================== 筛选辅助 ====================

inline vector<Transaction> filter_by_time(const vector<Transaction>& all,
                                          const string& t_start, const string& t_end) {
    vector<Transaction> result;
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i].time >= t_start && all[i].time <= t_end + " 23:59:59")
            result.push_back(all[i]);
    }
    return result;
}

inline vector<Transaction> filter_by_customer_type(const vector<Transaction>& all,
                                                    const string& cust_type) {
    // 需要先收集所有该类型客户的卡号
    set<string> card_set;
    for (size_t i = 0; i < g_customers.size(); i++) {
        if (g_customers[i].type == cust_type) {
            for (size_t j = 0; j < g_customers[i].card_ids.size(); j++)
                card_set.insert(g_customers[i].card_ids[j]);
        }
    }
    vector<Transaction> result;
    for (size_t i = 0; i < all.size(); i++) {
        if (card_set.count(all[i].from_card) || card_set.count(all[i].to_card))
            result.push_back(all[i]);
    }
    return result;
}

inline vector<Transaction> filter_by_card(const vector<Transaction>& all,
                                          const string& card_id) {
    vector<Transaction> result;
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i].from_card == card_id || all[i].to_card == card_id)
            result.push_back(all[i]);
    }
    return result;
}

inline vector<Transaction> filter_by_amount(const vector<Transaction>& all,
                                            double min_amt, double max_amt) {
    vector<Transaction> result;
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i].amount >= min_amt && all[i].amount <= max_amt)
            result.push_back(all[i]);
    }
    return result;
}

inline vector<Transaction> filter_by_type(const vector<Transaction>& all,
                                          const string& txn_type) {
    vector<Transaction> result;
    for (size_t i = 0; i < all.size(); i++) {
        if (all[i].type == txn_type)
            result.push_back(all[i]);
    }
    return result;
}

// ==================== 查询菜单 ====================

inline void show_query_result(const vector<Transaction>& result) {
    hr();
    cout << "  " << left << setw(16) << "交易ID" << setw(10) << "类型"
         << setw(14) << "金额" << setw(12) << "转出卡" << setw(12) << "转入卡"
         << setw(22) << "时间" << setw(6) << "状态" << endl;
    hr();
    double total = 0;
    for (size_t i = 0; i < result.size(); i++) {
        const Transaction& t = result[i];
        cout << "  " << left << setw(16) << t.id << setw(10) << t.type
             << setw(14) << double_to_str(t.amount)
             << setw(12) << t.from_card << setw(12) << t.to_card
             << setw(22) << t.time << setw(6) << t.status << endl;
        total += t.amount;
    }
    hr();
    cout << "  共 " << result.size() << " 条记录, 总金额: "
         << double_to_str(total) << " 元\n";
}

/** 业务汇总统计 */
inline void show_summary(const vector<Transaction>& data) {
    double deposit_total = 0, withdraw_total = 0, transfer_total = 0;
    double loan_total = 0, repay_total = 0;
    int deposit_cnt = 0, withdraw_cnt = 0, transfer_cnt = 0;
    int loan_cnt = 0, repay_cnt = 0;

    for (size_t i = 0; i < data.size(); i++) {
        if (data[i].status != "成功") continue;
        if (data[i].type == "存款") { deposit_total += data[i].amount; deposit_cnt++; }
        else if (data[i].type == "取款") { withdraw_total += data[i].amount; withdraw_cnt++; }
        else if (data[i].type == "转账") { transfer_total += data[i].amount; transfer_cnt++; }
        else if (data[i].type == "贷款") { loan_total += data[i].amount; loan_cnt++; }
        else if (data[i].type == "还款") { repay_total += data[i].amount; repay_cnt++; }
    }

    hr();
    cout << "  ═══ 业务汇总统计 ═══\n";
    hr();
    cout << "  " << left << setw(12) << "业务类型" << setw(10) << "笔数"
         << setw(16) << "总金额(元)" << endl;
    hr();
    cout << "  " << left << setw(12) << "存款" << setw(10) << deposit_cnt
         << setw(16) << double_to_str(deposit_total) << endl;
    cout << "  " << left << setw(12) << "取款" << setw(10) << withdraw_cnt
         << setw(16) << double_to_str(withdraw_total) << endl;
    cout << "  " << left << setw(12) << "转账" << setw(10) << transfer_cnt
         << setw(16) << double_to_str(transfer_total) << endl;
    cout << "  " << left << setw(12) << "贷款" << setw(10) << loan_cnt
         << setw(16) << double_to_str(loan_total) << endl;
    cout << "  " << left << setw(12) << "还款" << setw(10) << repay_cnt
         << setw(16) << double_to_str(repay_total) << endl;
    hr();
}

/** 多条件组合查询 */
inline void query_menu() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║    需求5: 业务查询              ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 按时间段查询                ║\n";
        cout << "  ║  2. 按客户类型查询(VIP/普通)    ║\n";
        cout << "  ║  3. 按银行卡号查询              ║\n";
        cout << "  ║  4. 按金额区间查询              ║\n";
        cout << "  ║  5. 按交易类型查询              ║\n";
        cout << "  ║  6. 组合查询(多条件)            ║\n";
        cout << "  ║  7. 业务汇总统计                ║\n";
        cout << "  ║  8. 按金额排序(快速排序)        ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);

        if (ch == "1") {
            cout << "  开始时间 (YYYY-MM-DD): "; string ts; getline(cin, ts);
            cout << "  结束时间 (YYYY-MM-DD): "; string te; getline(cin, te);
            vector<Transaction> r = filter_by_time(g_transactions, ts, te);
            bubble_sort_txn(r, false);
            show_query_result(r);
            show_summary(r);
            pause();
        } else if (ch == "2") {
            cout << "  客户类型 (VIP/普通): "; string ct; getline(cin, ct);
            vector<Transaction> r = filter_by_customer_type(g_transactions, ct);
            show_query_result(r);
            show_summary(r);
            pause();
        } else if (ch == "3") {
            cout << "  银行卡号: "; string cid; getline(cin, cid);
            vector<Transaction> r = filter_by_card(g_transactions, cid);
            show_query_result(r);
            pause();
        } else if (ch == "4") {
            cout << "  最低金额: "; string mn; getline(cin, mn);
            cout << "  最高金额: "; string mx; getline(cin, mx);
            vector<Transaction> r = filter_by_amount(g_transactions,
                safe_double(mn), safe_double(mx));
            // 快速排序按金额
            if (r.size() > 1) quicksort_txn_amount(r, 0, r.size()-1);
            show_query_result(r);
            pause();
        } else if (ch == "5") {
            cout << "  交易类型 (存款/取款/转账/贷款/还款): ";
            string tt; getline(cin, tt);
            vector<Transaction> r = filter_by_type(g_transactions, tt);
            show_query_result(r);
            pause();
        } else if (ch == "6") {
            // 组合查询
            vector<Transaction> r = g_transactions;
            cout << "\n  组合查询 (回车跳过该条件):\n";
            cout << "  开始时间: "; string ts; getline(cin, ts);
            if (!ts.empty()) {
                cout << "  结束时间: "; string te; getline(cin, te);
                r = filter_by_time(r, ts, te);
            }
            cout << "  客户类型 (VIP/普通, 回车跳过): "; string ct; getline(cin, ct);
            if (!ct.empty()) r = filter_by_customer_type(r, ct);
            cout << "  银行卡号 (回车跳过): "; string cid; getline(cin, cid);
            if (!cid.empty()) r = filter_by_card(r, cid);
            cout << "  最低金额 (回车跳过): "; string mn; getline(cin, mn);
            if (!mn.empty()) {
                cout << "  最高金额: "; string mx; getline(cin, mx);
                r = filter_by_amount(r, safe_double(mn), safe_double(mx));
            }
            cout << "  交易类型 (回车跳过): "; string tt; getline(cin, tt);
            if (!tt.empty()) r = filter_by_type(r, tt);
            bubble_sort_txn(r, false);
            show_query_result(r);
            show_summary(r);
            pause();
        } else if (ch == "7") {
            show_summary(g_transactions);
            pause();
        } else if (ch == "8") {
            vector<Transaction> r = g_transactions;
            if (r.size() > 1) quicksort_txn_amount(r, 0, r.size()-1);
            show_query_result(r);
            pause();
        } else if (ch == "0") break;
    }
}

#endif
