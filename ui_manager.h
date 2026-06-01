/**
 * ui_manager.h — 需求9: 主界面 + 统一登录
 *
 * 功能: 字符图形界面, 主子菜单, 管理员/职员/客户统一登录
 * 登录流程: 选择角色 → 输入凭证 → 进入对应菜单
 */

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include "common.h"

/** 打印系统Logo */
inline void print_logo() {
    cout << "\n";
    cout << "  ╔══════════════════════════════════════════════════════╗\n";
    cout << "  ║                                                      ║\n";
    cout << "  ║      ██████╗  █████╗ ███╗   ██╗██╗  ██╗              ║\n";
    cout << "  ║      ██╔══██╗██╔══██╗████╗  ██║██║ ██╔╝              ║\n";
    cout << "  ║      ██████╔╝███████║██╔██╗ ██║█████╔╝               ║\n";
    cout << "  ║      ██╔══██╗██╔══██║██║╚██╗██║██╔═██╗               ║\n";
    cout << "  ║      ██████╔╝██║  ██║██║ ╚████║██║  ██╗              ║\n";
    cout << "  ║      ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝              ║\n";
    cout << "  ║                                                      ║\n";
    cout << "  ║       银行智能管理系统  v2.0 Final                   ║\n";
    cout << "  ║       数据结构与算法课程设计                          ║\n";
    cout << "  ║       第十组 · 中期→最终版                            ║\n";
    cout << "  ╚══════════════════════════════════════════════════════╝\n";
}

/** 主界面 */
inline void main_menu() {
    while (true) {
        cls();
        print_logo();
        cout << "\n";
        cout << "  ╔══════════════════════════════════╗\n";
        cout << "  ║       统 一 登 录 系 统          ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 管理员登录 (Admin)           ║\n";
        cout << "  ║  2. 职员登录   (Staff)           ║\n";
        cout << "  ║  3. 客户登录   (Customer)        ║\n";
        cout << "  ║  4. 新客户注册                   ║\n";
        cout << "  ║  0. 退出系统                     ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "\n  请选择登录身份: ";
        string ch; getline(cin, ch);

        if (ch == "1") {
            // 管理员登录
            cout << "\n  ═══ 管理员登录 ═══\n";
            cout << "  职员编号: "; string eid; getline(cin, eid);
            cout << "  密码: "; string pw; getline(cin, pw);
            int eidx = find_employee(eid);
            if (eidx == -1) { cout << "  ✗ 职员不存在!\n"; pause(); continue; }
            if (!g_employees[eidx].is_active) { cout << "  ✗ 该职员已离职!\n"; pause(); continue; }
            if (g_employees[eidx].role != "admin") { cout << "  ✗ 非管理员账户!\n"; pause(); continue; }
            if (g_employees[eidx].password != pw) { cout << "  ✗ 密码错误!\n"; pause(); continue; }
            admin_menu(eidx);
        } else if (ch == "2") {
            // 职员登录
            cout << "\n  ═══ 职员登录 ═══\n";
            cout << "  职员编号: "; string eid; getline(cin, eid);
            cout << "  密码: "; string pw; getline(cin, pw);
            int eidx = find_employee(eid);
            if (eidx == -1) { cout << "  ✗ 职员不存在!\n"; pause(); continue; }
            if (!g_employees[eidx].is_active) { cout << "  ✗ 该职员已离职!\n"; pause(); continue; }
            if (g_employees[eidx].password != pw) { cout << "  ✗ 密码错误!\n"; pause(); continue; }
            staff_menu(eidx);
        } else if (ch == "3") {
            // 客户登录
            cout << "\n  ═══ 客户登录 ═══\n";
            cout << "  账号 (C+6位数字): "; string cid; getline(cin, cid);
            cout << "  密码: "; string pw; getline(cin, pw);
            int cidx = find_customer(cid);
            if (cidx == -1) { cout << "  ✗ 客户不存在!\n"; pause(); continue; }
            if (!g_customers[cidx].is_active) { cout << "  ✗ 该账户已注销!\n"; pause(); continue; }
            if (g_customers[cidx].password != pw) { cout << "  ✗ 密码错误!\n"; pause(); continue; }
            customer_menu(cidx);
        } else if (ch == "4") {
            // 新客户注册
            add_customer();
            pause();
        } else if (ch == "0") {
            cout << "\n  正在保存数据...\n";
            save_employees();
            save_customers();
            save_cards();
            save_transactions();
            save_queue_history();
            save_daily_stats();
            save_branches();
            cout << "  数据已保存。感谢使用银行智能管理系统!\n";
            break;
        } else {
            cout << "  无效选择!\n"; pause();
        }
    }
}

// ==================== 管理员菜单 ====================
inline void admin_menu(int emp_idx) {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║   管理员主菜单 (Admin)           ║\n";
        cout << "  ║   当前用户: " << left << setw(20) << g_employees[emp_idx].name << "║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  [需求1] 1. 银行职员管理         ║\n";
        cout << "  ║  [需求2] 2. 客户账户管理         ║\n";
        cout << "  ║  [需求3] 3. 银行卡管理           ║\n";
        cout << "  ║  [需求4] 4. 存取贷业务管理       ║\n";
        cout << "  ║  [需求5] 5. 业务查询             ║\n";
        cout << "  ║  [需求6] 6. 银行排队管理         ║\n";
        cout << "  ║  [需求7] 7. 网点查询与导航       ║\n";
        cout << "  ║  [需求8] 8. 智能管理             ║\n";
        cout << "  ║         9. 修改个人密码           ║\n";
        cout << "  ║         0. 退出登录               ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") employee_menu_admin();
        else if (ch == "2") customer_menu_staff();
        else if (ch == "3") card_menu_staff();
        else if (ch == "4") transaction_menu_staff(g_employees[emp_idx].id);
        else if (ch == "5") query_menu();
        else if (ch == "6") queue_menu();
        else if (ch == "7") branch_menu();
        else if (ch == "8") smart_menu();
        else if (ch == "9") { change_emp_password(emp_idx); pause(); }
        else if (ch == "0") { cout << "  管理员已退出。\n"; break; }
    }
}

// ==================== 职员菜单 ====================
inline void staff_menu(int emp_idx) {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║   职员主菜单 (Staff)             ║\n";
        cout << "  ║   当前用户: " << left << setw(20) << g_employees[emp_idx].name << "║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  [需求2] 1. 客户账户管理         ║\n";
        cout << "  ║  [需求3] 2. 银行卡管理           ║\n";
        cout << "  ║  [需求4] 3. 存取贷业务管理       ║\n";
        cout << "  ║  [需求5] 4. 业务查询             ║\n";
        cout << "  ║  [需求6] 5. 银行排队管理         ║\n";
        cout << "  ║         6. 修改个人密码           ║\n";
        cout << "  ║         7. 修改个人信息           ║\n";
        cout << "  ║         0. 退出登录               ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") customer_menu_staff();
        else if (ch == "2") card_menu_staff();
        else if (ch == "3") transaction_menu_staff(g_employees[emp_idx].id);
        else if (ch == "4") query_menu();
        else if (ch == "5") queue_menu();
        else if (ch == "6") { change_emp_password(emp_idx); pause(); }
        else if (ch == "7") { modify_emp_profile(emp_idx); pause(); }
        else if (ch == "0") { cout << "  职员已退出。\n"; break; }
    }
}

// ==================== 客户菜单 ====================
inline void customer_menu(int cust_idx) {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║   客户主菜单                     ║\n";
        cout << "  ║   欢迎, " << left << setw(25) << (g_customers[cust_idx].name + " (" + g_customers[cust_idx].type + ")") << "║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 查看个人信息与账户          ║\n";
        cout << "  ║  2. 查看我的银行卡              ║\n";
        cout << "  ║  3. 查看我的交易记录            ║\n";
        cout << "  ║  4. 修改登录密码                ║\n";
        cout << "  ║  5. 查看利息信息                ║\n";
        cout << "  ║  6. 投资顾问建议                ║\n";
        cout << "  ║  0. 退出登录                    ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { view_customer_profile(cust_idx); pause(); }
        else if (ch == "2") {
            // 显示该客户的所有卡
            const Customer& c = g_customers[cust_idx];
            hr();
            cout << "  " << left << setw(12) << "卡号" << setw(8) << "类型"
                 << setw(14) << "余额" << setw(14) << "借贷余额"
                 << setw(8) << "利率%" << setw(8) << "状态" << endl;
            hr();
            for (size_t i = 0; i < c.card_ids.size(); i++) {
                int ci = find_card(c.card_ids[i]);
                if (ci >= 0) {
                    const BankCard& b = g_cards[ci];
                    cout << "  " << left << setw(12) << b.id << setw(8) << b.type
                         << setw(14) << double_to_str(b.balance)
                         << setw(14) << double_to_str(b.loan_balance)
                         << setw(8) << double_to_str(b.interest_rate)
                         << setw(8) << b.status << endl;
                }
            }
            hr();
            pause();
        }
        else if (ch == "3") {
            // 显示客户所有卡的交易记录
            const Customer& c = g_customers[cust_idx];
            for (size_t i = 0; i < c.card_ids.size(); i++)
                show_card_transactions(c.card_ids[i]);
            pause();
        }
        else if (ch == "4") { change_cust_password(cust_idx); pause(); }
        else if (ch == "5") { show_interest(); pause(); }
        else if (ch == "6") { investment_advisor(); pause(); }
        else if (ch == "0") { cout << "  客户已退出。\n"; break; }
    }
}

#endif
