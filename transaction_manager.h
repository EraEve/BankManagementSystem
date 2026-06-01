/**
 * transaction_manager.h — 需求4: 存取贷业务管理
 *
 * 功能: 存款/取款/转账/贷款/还款/利息计算, 文件存取交易记录
 * 数据结构: vector顺序表, 排序算法(冒泡/快排)
 */

#ifndef TRANSACTION_MANAGER_H
#define TRANSACTION_MANAGER_H

#include "common.h"

extern vector<Transaction> g_transactions;

inline void load_transactions() {
    g_transactions.clear();
    ifstream f(TRANSACTION_FILE.c_str());
    if (!f.is_open()) return;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = split(line, '|');
        if (p.size() < 8) continue;
        Transaction t;
        t.id = p[0]; t.from_card = p[1]; t.to_card = p[2];
        t.type = p[3]; t.amount = safe_double(p[4]);
        t.time = p[5]; t.status = p[6];
        t.employee_id = p[7]; t.remark = (p.size() > 8 ? p[8] : "");
        g_transactions.push_back(t);
    }
    f.close();
}

inline void save_transactions() {
    ofstream f(TRANSACTION_FILE.c_str());
    if (!f.is_open()) return;
    for (size_t i = 0; i < g_transactions.size(); i++) {
        const Transaction& t = g_transactions[i];
        f << t.id << "|" << t.from_card << "|" << t.to_card << "|"
          << t.type << "|" << double_to_str(t.amount) << "|"
          << t.time << "|" << t.status << "|"
          << t.employee_id << "|" << t.remark << endl;
    }
    f.close();
}

inline void record_txn(const string& from, const string& to,
                       const string& type, double amount,
                       const string& status, const string& emp_id,
                       const string& remark = "") {
    Transaction t;
    t.id = gen_txn_id();
    t.from_card = from; t.to_card = to;
    t.type = type; t.amount = amount;
    t.time = now_str(); t.status = status;
    t.employee_id = emp_id; t.remark = remark;
    g_transactions.push_back(t);
}

// ==================== 核心业务操作 ====================

/** 存款 (由职员操作) */
inline void do_deposit(const string& emp_id) {
    cout << "\n  ═══ 存款业务 ═══\n";
    cout << "  银行卡号: "; string card_id; getline(cin, card_id);
    int ci = find_card(card_id);
    if (ci == -1) { cout << "  ✗ 银行卡不存在!\n"; return; }
    if (g_cards[ci].status != "正常") { cout << "  ✗ 银行卡状态异常!\n"; return; }
    cout << "  存款金额: "; string amt; getline(cin, amt);
    if (!is_valid_amount(amt)) { cout << "  ✗ 金额无效!\n"; return; }
    double amount = safe_double(amt);
    g_cards[ci].balance += amount;
    record_txn("-", card_id, "存款", amount, "成功", emp_id);
    // 更新客户金融资产(简化: 累加到关联客户的资产)
    int cust_idx = find_customer(g_cards[ci].customer_id);
    if (cust_idx >= 0) {
        g_customers[cust_idx].financial_assets += amount;
        update_customer_type(g_customers[cust_idx]);
    }
    save_cards(); save_transactions(); save_customers();
    cout << "  ✓ 存款成功! 金额: " << double_to_str(amount)
         << " 元, 当前余额: " << double_to_str(g_cards[ci].balance) << " 元\n";
}

/** 取款 */
inline void do_withdraw(const string& emp_id) {
    cout << "\n  ═══ 取款业务 ═══\n";
    cout << "  银行卡号: "; string card_id; getline(cin, card_id);
    int ci = find_card(card_id);
    if (ci == -1) { cout << "  ✗ 银行卡不存在!\n"; return; }
    if (g_cards[ci].status != "正常") { cout << "  ✗ 银行卡状态异常!\n"; return; }
    cout << "  取款金额: "; string amt; getline(cin, amt);
    if (!is_valid_amount(amt)) { cout << "  ✗ 金额无效!\n"; return; }
    double amount = safe_double(amt);
    if (amount > g_cards[ci].balance) {
        record_txn(card_id, "-", "取款", amount, "失败", emp_id, "余额不足");
        cout << "  ✗ 余额不足! 当前余额: " << double_to_str(g_cards[ci].balance) << " 元\n";
        return;
    }
    if (amount > g_cards[ci].daily_limit) {
        record_txn(card_id, "-", "取款", amount, "失败", emp_id, "超日限额");
        cout << "  ✗ 超过日交易限额!\n"; return;
    }
    g_cards[ci].balance -= amount;
    record_txn(card_id, "-", "取款", amount, "成功", emp_id);
    int cust_idx = find_customer(g_cards[ci].customer_id);
    if (cust_idx >= 0) {
        g_customers[cust_idx].financial_assets -= amount;
        update_customer_type(g_customers[cust_idx]);
    }
    save_cards(); save_transactions(); save_customers();
    cout << "  ✓ 取款成功! 金额: " << double_to_str(amount)
         << " 元, 当前余额: " << double_to_str(g_cards[ci].balance) << " 元\n";
}

/** 转账 */
inline void do_transfer(const string& emp_id) {
    cout << "\n  ═══ 转账业务 ═══\n";
    cout << "  转出卡号: "; string from_id; getline(cin, from_id);
    int fi = find_card(from_id);
    if (fi == -1) { cout << "  ✗ 转出卡不存在!\n"; return; }
    if (g_cards[fi].status != "正常") { cout << "  ✗ 转出卡状态异常!\n"; return; }
    cout << "  转入卡号: "; string to_id; getline(cin, to_id);
    int ti = find_card(to_id);
    if (ti == -1) { cout << "  ✗ 转入卡不存在!\n"; return; }
    if (g_cards[ti].status != "正常") { cout << "  ✗ 转入卡状态异常!\n"; return; }
    if (from_id == to_id) { cout << "  ✗ 不能向同一张卡转账!\n"; return; }
    cout << "  转账金额: "; string amt; getline(cin, amt);
    if (!is_valid_amount(amt)) { cout << "  ✗ 金额无效!\n"; return; }
    double amount = safe_double(amt);
    if (amount > g_cards[fi].balance) {
        record_txn(from_id, to_id, "转账", amount, "失败", emp_id, "余额不足");
        cout << "  ✗ 余额不足!\n"; return;
    }
    g_cards[fi].balance -= amount;
    g_cards[ti].balance += amount;
    record_txn(from_id, to_id, "转账", amount, "成功", emp_id);
    save_cards(); save_transactions();
    cout << "  ✓ 转账成功! 金额: " << double_to_str(amount) << " 元\n";
}

/** 贷款 (信用卡) */
inline void do_loan(const string& emp_id) {
    cout << "\n  ═══ 贷款业务(信用卡) ═══\n";
    cout << "  信用卡号: "; string card_id; getline(cin, card_id);
    int ci = find_card(card_id);
    if (ci == -1) { cout << "  ✗ 银行卡不存在!\n"; return; }
    if (g_cards[ci].type != "信用卡") { cout << "  ✗ 该卡非信用卡,不支持贷款!\n"; return; }
    if (g_cards[ci].status != "正常") { cout << "  ✗ 卡状态异常!\n"; return; }
    double available = g_cards[ci].credit_limit - g_cards[ci].loan_balance;
    cout << "  可用额度: " << double_to_str(available) << " 元\n";
    cout << "  贷款金额: "; string amt; getline(cin, amt);
    if (!is_valid_amount(amt)) { cout << "  ✗ 金额无效!\n"; return; }
    double amount = safe_double(amt);
    if (amount > available) {
        record_txn(card_id, "-", "贷款", amount, "失败", emp_id, "超信用额度");
        cout << "  ✗ 超过可用额度!\n"; return;
    }
    g_cards[ci].loan_balance += amount;
    g_cards[ci].balance += amount;
    record_txn("-", card_id, "贷款", amount, "成功", emp_id);
    save_cards(); save_transactions();
    cout << "  ✓ 贷款成功! 金额: " << double_to_str(amount)
         << " 元, 当前借贷余额: " << double_to_str(g_cards[ci].loan_balance) << " 元\n";
}

/** 还款 (信用卡) */
inline void do_repay(const string& emp_id) {
    cout << "\n  ═══ 还款业务(信用卡) ═══\n";
    cout << "  信用卡号: "; string card_id; getline(cin, card_id);
    int ci = find_card(card_id);
    if (ci == -1) { cout << "  ✗ 银行卡不存在!\n"; return; }
    if (g_cards[ci].type != "信用卡") { cout << "  ✗ 该卡非信用卡!\n"; return; }
    cout << "  当前借贷余额: " << double_to_str(g_cards[ci].loan_balance) << " 元\n";
    cout << "  还款金额: "; string amt; getline(cin, amt);
    if (!is_valid_amount(amt)) { cout << "  ✗ 金额无效!\n"; return; }
    double amount = safe_double(amt);
    if (amount > g_cards[ci].loan_balance) {
        cout << "  ✗ 还款金额不能超过借贷余额!\n"; return;
    }
    if (amount > g_cards[ci].balance) {
        cout << "  ✗ 卡内余额不足!\n"; return;
    }
    g_cards[ci].loan_balance -= amount;
    g_cards[ci].balance -= amount;
    record_txn(card_id, "-", "还款", amount, "成功", emp_id);
    save_cards(); save_transactions();
    cout << "  ✓ 还款成功! 剩余借贷余额: " << double_to_str(g_cards[ci].loan_balance) << " 元\n";
}

/** 利息计算 - 单卡日利息 */
inline double calc_daily_interest(const BankCard& c) {
    double principal = c.balance;
    if (c.type == "信用卡") principal = c.loan_balance;
    return principal * (c.interest_rate / 100.0) / 365.0;
}

/** 利息计算 - 单卡月利息 */
inline double calc_monthly_interest(const BankCard& c) {
    double principal = c.balance;
    if (c.type == "信用卡") principal = c.loan_balance;
    return principal * (c.interest_rate / 100.0) / 12.0;
}

/** 显示利息信息 */
inline void show_interest() {
    cout << "\n  ═══ 利息信息 ═══\n";
    cout << "  储蓄卡/借记卡: 按余额计算利息\n";
    cout << "  信用卡: 按借贷余额计算利息\n\n";
    hr();
    cout << "  " << left << setw(12) << "卡号" << setw(8) << "类型"
         << setw(14) << "余额/借贷" << setw(10) << "年利率%"
         << setw(14) << "日利息" << setw(14) << "月利息" << endl;
    hr();
    for (size_t i = 0; i < g_cards.size(); i++) {
        const BankCard& c = g_cards[i];
        if (c.status != "正常") continue;
        double principal = (c.type == "信用卡") ? c.loan_balance : c.balance;
        cout << "  " << left << setw(12) << c.id << setw(8) << c.type
             << setw(14) << double_to_str(principal)
             << setw(10) << double_to_str(c.interest_rate)
             << setw(14) << double_to_str(calc_daily_interest(c))
             << setw(14) << double_to_str(calc_monthly_interest(c)) << endl;
    }
    hr();
}

/** 显示所有交易记录 */
inline void list_transactions() {
    hr();
    cout << "  " << left << setw(16) << "交易ID" << setw(10) << "类型"
         << setw(14) << "金额" << setw(12) << "转出卡" << setw(12) << "转入卡"
         << setw(22) << "时间" << setw(6) << "状态" << endl;
    hr();
    for (size_t i = 0; i < g_transactions.size(); i++) {
        const Transaction& t = g_transactions[i];
        cout << "  " << left << setw(16) << t.id << setw(10) << t.type
             << setw(14) << double_to_str(t.amount)
             << setw(12) << t.from_card << setw(12) << t.to_card
             << setw(22) << t.time << setw(6) << t.status << endl;
    }
    hr();
    cout << "  共 " << g_transactions.size() << " 条记录\n";
}

/** 查看某卡的交易记录 */
inline void show_card_transactions(const string& card_id) {
    hr();
    cout << "  卡号 " << card_id << " 的交易记录:\n";
    hr();
    cout << "  " << left << setw(16) << "交易ID" << setw(10) << "类型"
         << setw(14) << "金额" << setw(22) << "时间" << setw(6) << "状态" << endl;
    hr();
    bool found = false;
    for (size_t i = 0; i < g_transactions.size(); i++) {
        const Transaction& t = g_transactions[i];
        if (t.from_card == card_id || t.to_card == card_id) {
            cout << "  " << left << setw(16) << t.id << setw(10) << t.type
                 << setw(14) << double_to_str(t.amount)
                 << setw(22) << t.time << setw(6) << t.status << endl;
            found = true;
        }
    }
    if (!found) cout << "  暂无交易记录\n";
    hr();
}

/** 存储贷业务菜单(职员) */
inline void transaction_menu_staff(const string& emp_id) {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║   需求4: 存取贷业务管理         ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 存款                        ║\n";
        cout << "  ║  2. 取款                        ║\n";
        cout << "  ║  3. 转账                        ║\n";
        cout << "  ║  4. 贷款(信用卡)                ║\n";
        cout << "  ║  5. 还款(信用卡)                ║\n";
        cout << "  ║  6. 利息计算与查看              ║\n";
        cout << "  ║  7. 查看所有交易记录            ║\n";
        cout << "  ║  8. 查看指定卡交易记录          ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { do_deposit(emp_id); pause(); }
        else if (ch == "2") { do_withdraw(emp_id); pause(); }
        else if (ch == "3") { do_transfer(emp_id); pause(); }
        else if (ch == "4") { do_loan(emp_id); pause(); }
        else if (ch == "5") { do_repay(emp_id); pause(); }
        else if (ch == "6") { show_interest(); pause(); }
        else if (ch == "7") { list_transactions(); pause(); }
        else if (ch == "8") {
            cout << "  输入卡号: "; string cid; getline(cin, cid);
            show_card_transactions(cid); pause();
        }
        else if (ch == "0") break;
    }
}

#endif
