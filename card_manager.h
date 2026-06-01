/**
 * card_manager.h — 需求3: 银行卡管理
 *
 * 功能: 职员增删改查银行卡, 借记卡/储蓄卡/信用卡分类, 文件存取
 * 数据结构: vector顺序表 + map哈希索引
 * 关联: 每张卡属于一个客户(广义表关联)
 */

#ifndef CARD_MANAGER_H
#define CARD_MANAGER_H

#include "common.h"

extern vector<BankCard> g_cards;
extern map<string, int> g_card_index;

inline void load_cards() {
    g_cards.clear();
    g_card_index.clear();
    ifstream f(CARD_FILE.c_str());
    if (!f.is_open()) {
        // 创建默认卡
        BankCard b1; b1.id = "B10000001"; b1.customer_id = "C000001";
        b1.type = "储蓄卡"; b1.balance = 50000; b1.interest_rate = 1.5;
        b1.open_date = "2024-03-15"; b1.status = "正常"; b1.daily_limit = 50000;
        g_cards.push_back(b1); g_card_index["B10000001"] = 0;

        BankCard b2; b2.id = "B10000002"; b2.customer_id = "C000002";
        b2.type = "信用卡"; b2.balance = 0; b2.loan_balance = 5000;
        b2.interest_rate = 18.0; b2.credit_limit = 200000;
        b2.open_date = "2024-01-20"; b2.status = "正常"; b2.daily_limit = 100000;
        g_cards.push_back(b2); g_card_index["B10000002"] = 1;

        BankCard b3; b3.id = "B10000003"; b3.customer_id = "C000003";
        b3.type = "借记卡"; b3.balance = 30000; b3.interest_rate = 0.35;
        b3.open_date = "2024-06-10"; b3.status = "正常"; b3.daily_limit = 20000;
        g_cards.push_back(b3); g_card_index["B10000003"] = 2;

        BankCard b4; b4.id = "B10000004"; b4.customer_id = "C000004";
        b4.type = "信用卡"; b4.balance = 0; b4.loan_balance = 0;
        b4.interest_rate = 15.0; b4.credit_limit = 500000;
        b4.open_date = "2023-11-05"; b4.status = "正常"; b4.daily_limit = 200000;
        g_cards.push_back(b4); g_card_index["B10000004"] = 3;

        BankCard b5; b5.id = "B10000005"; b5.customer_id = "C000004";
        b5.type = "储蓄卡"; b5.balance = 1200000; b5.interest_rate = 2.0;
        b5.open_date = "2023-11-05"; b5.status = "正常"; b5.daily_limit = 500000;
        g_cards.push_back(b5); g_card_index["B10000005"] = 4;

        // 更新客户的card_ids
        for (size_t i = 0; i < g_cards.size(); i++) {
            int ci = find_customer(g_cards[i].customer_id);
            if (ci >= 0) g_customers[ci].card_ids.push_back(g_cards[i].id);
        }
        return;
    }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = split(line, '|');
        if (p.size() < 10) continue;
        BankCard c;
        c.id = p[0]; c.customer_id = p[1]; c.type = p[2];
        c.balance = safe_double(p[3]); c.loan_balance = safe_double(p[4]);
        c.interest_rate = safe_double(p[5]); c.credit_limit = safe_double(p[6]);
        c.open_date = p[7]; c.status = p[8]; c.daily_limit = safe_double(p[9]);
        g_card_index[c.id] = g_cards.size();
        g_cards.push_back(c);
    }
    f.close();
    // 重建客户card_ids
    for (size_t i = 0; i < g_cards.size(); i++) {
        int ci = find_customer(g_cards[i].customer_id);
        if (ci >= 0) g_customers[ci].card_ids.push_back(g_cards[i].id);
    }
}

inline void save_cards() {
    ofstream f(CARD_FILE.c_str());
    if (!f.is_open()) return;
    for (size_t i = 0; i < g_cards.size(); i++) {
        const BankCard& c = g_cards[i];
        f << c.id << "|" << c.customer_id << "|" << c.type << "|"
          << double_to_str(c.balance) << "|" << double_to_str(c.loan_balance) << "|"
          << double_to_str(c.interest_rate) << "|" << double_to_str(c.credit_limit) << "|"
          << c.open_date << "|" << c.status << "|" << double_to_str(c.daily_limit) << endl;
    }
    f.close();
}

// ==================== 银行卡CRUD ====================

inline int find_card(const string& id) {
    map<string,int>::iterator it = g_card_index.find(id);
    if (it != g_card_index.end()) return it->second;
    for (int i = 0; i < (int)g_cards.size(); i++)
        if (g_cards[i].id == id) return i;
    return -1;
}

inline void add_card() {
    BankCard c;
    cout << "\n  ═══ 添加银行卡 ═══\n";
    // 生成卡号
    int max_id = 0;
    for (size_t i = 0; i < g_cards.size(); i++) {
        int n = atoi(g_cards[i].id.substr(1).c_str());
        if (n > max_id) max_id = n;
    }
    char buf[20]; sprintf(buf, "B1%07d", max_id + 1);
    c.id = string(buf);
    cout << "  自动分配卡号: " << c.id << endl;
    cout << "  客户编号: "; getline(cin, c.customer_id);
    int cust_idx = find_customer(c.customer_id);
    if (cust_idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    cout << "  卡类型 (借记卡/储蓄卡/信用卡): "; getline(cin, c.type);
    if (c.type == "信用卡") {
        cout << "  信用额度: "; string cl; getline(cin, cl);
        c.credit_limit = safe_double(cl);
        cout << "  年利率(%): "; string ir; getline(cin, ir);
        c.interest_rate = safe_double(ir);
    } else {
        cout << "  年利率(%): "; string ir; getline(cin, ir);
        c.interest_rate = safe_double(ir);
    }
    cout << "  日交易限额: "; string dl; getline(cin, dl);
    c.daily_limit = safe_double(dl);
    c.open_date = today_str();
    c.status = "正常";
    g_card_index[c.id] = g_cards.size();
    g_cards.push_back(c);
    g_customers[cust_idx].card_ids.push_back(c.id);
    save_cards();
    save_customers();
    cout << "  ✓ 银行卡 " << c.id << " 创建成功! 客户: " << g_customers[cust_idx].name << endl;
}

inline void delete_card() {
    string id;
    cout << "\n  ═══ 注销银行卡 ═══\n";
    cout << "  卡号: "; getline(cin, id);
    int idx = find_card(id);
    if (idx == -1) { cout << "  ✗ 银行卡不存在!\n"; return; }
    cout << "  确认注销 " << id << "? (y/n): ";
    string cf; getline(cin, cf);
    if (cf == "y" || cf == "Y") {
        g_cards[idx].status = "注销";
        save_cards();
        cout << "  ✓ 银行卡已注销\n";
    }
}

inline void modify_card() {
    string id;
    cout << "\n  ═══ 修改银行卡信息 ═══\n";
    cout << "  卡号: "; getline(cin, id);
    int idx = find_card(id);
    if (idx == -1) { cout << "  ✗ 银行卡不存在!\n"; return; }
    BankCard& c = g_cards[idx];
    cout << "  当前状态 [" << c.status << "] (正常/冻结/注销): "; string s; getline(cin, s);
    if (!s.empty()) c.status = s;
    cout << "  当前日限额 [" << double_to_str(c.daily_limit) << "]: "; getline(cin, s);
    if (!s.empty()) c.daily_limit = safe_double(s);
    if (c.type == "信用卡") {
        cout << "  当前信用额度 [" << double_to_str(c.credit_limit) << "]: "; getline(cin, s);
        if (!s.empty()) c.credit_limit = safe_double(s);
    }
    save_cards();
    cout << "  ✓ 银行卡信息已更新!\n";
}

inline void query_card() {
    string kw;
    cout << "\n  ═══ 查询银行卡 ═══\n";
    cout << "  输入卡号/客户编号关键字: "; getline(cin, kw);
    hr();
    cout << "  " << left << setw(12) << "卡号" << setw(10) << "客户ID"
         << setw(8) << "类型" << setw(14) << "余额" << setw(14) << "借贷余额"
         << setw(8) << "利率%" << setw(8) << "状态" << endl;
    hr();
    bool found = false;
    for (size_t i = 0; i < g_cards.size(); i++) {
        const BankCard& c = g_cards[i];
        if (c.id.find(kw) != string::npos ||
            c.customer_id.find(kw) != string::npos) {
            cout << "  " << left << setw(12) << c.id << setw(10) << c.customer_id
                 << setw(8) << c.type << setw(14) << double_to_str(c.balance)
                 << setw(14) << double_to_str(c.loan_balance)
                 << setw(8) << double_to_str(c.interest_rate)
                 << setw(8) << c.status << endl;
            found = true;
        }
    }
    if (!found) cout << "  未找到匹配的银行卡。\n";
    hr();
}

inline void list_cards() {
    hr();
    cout << "  " << left << setw(12) << "卡号" << setw(10) << "客户ID"
         << setw(8) << "类型" << setw(14) << "余额" << setw(12) << "信用额度"
         << setw(8) << "利率%" << setw(12) << "开户日期" << setw(8) << "状态" << endl;
    hr();
    for (size_t i = 0; i < g_cards.size(); i++) {
        const BankCard& c = g_cards[i];
        cout << "  " << left << setw(12) << c.id << setw(10) << c.customer_id
             << setw(8) << c.type << setw(14) << double_to_str(c.balance)
             << setw(12) << double_to_str(c.credit_limit)
             << setw(8) << double_to_str(c.interest_rate)
             << setw(12) << c.open_date << setw(8) << c.status << endl;
    }
    hr();
}

inline void card_menu_staff() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║     需求3: 银行卡管理           ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 查看所有银行卡              ║\n";
        cout << "  ║  2. 添加银行卡                  ║\n";
        cout << "  ║  3. 注销银行卡                  ║\n";
        cout << "  ║  4. 修改银行卡信息              ║\n";
        cout << "  ║  5. 查询银行卡                  ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { list_cards(); pause(); }
        else if (ch == "2") { add_card(); pause(); }
        else if (ch == "3") { delete_card(); pause(); }
        else if (ch == "4") { modify_card(); pause(); }
        else if (ch == "5") { query_card(); pause(); }
        else if (ch == "0") break;
    }
}

#endif
