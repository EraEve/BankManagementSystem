/**
 * employee_manager.h — 需求1: 银行职员管理
 *
 * 功能: 管理员增删改查职员, 文件存取, 密码管理
 * 数据结构: vector顺序表 + unordered_map哈希索引 O(1)查找
 */

#ifndef EMPLOYEE_MANAGER_H
#define EMPLOYEE_MANAGER_H

#include "common.h"

// ==================== 全局职员数据 ====================
extern vector<Employee> g_employees;
extern map<string, int> g_emp_index;   // 哈希索引: ID → vector下标

// ==================== 文件操作 ====================

inline void load_employees() {
    g_employees.clear();
    g_emp_index.clear();
    ifstream f(EMPLOYEE_FILE.c_str());
    if (!f.is_open()) {
        // 创建默认管理员
        Employee admin;
        admin.id = "E001"; admin.name = "Admin";
        admin.password = "000000"; admin.role = "admin";
        admin.department = "总行管理部"; admin.phone = "13800000000";
        admin.email = "admin@bank.com"; admin.hire_date = "2024-01-01";
        g_employees.push_back(admin);
        g_emp_index["E001"] = 0;

        Employee staff;
        staff.id = "E002"; staff.name = "ZhangWei";
        staff.password = "111111"; staff.role = "staff";
        staff.department = "营业部"; staff.phone = "13800000001";
        staff.email = "zhangwei@bank.com"; staff.hire_date = "2024-06-01";
        g_employees.push_back(staff);
        g_emp_index["E002"] = 1;
        return;
    }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = split(line, '|');
        if (p.size() < 8) continue;
        Employee e;
        e.id = p[0]; e.name = p[1]; e.password = p[2];
        e.role = p[3]; e.department = p[4]; e.phone = p[5];
        e.email = p[6]; e.hire_date = p[7];
        e.is_active = (p.size() > 8 ? (p[8] == "1") : true);
        g_emp_index[e.id] = g_employees.size();
        g_employees.push_back(e);
    }
    f.close();
}

inline void save_employees() {
    ofstream f(EMPLOYEE_FILE.c_str());
    if (!f.is_open()) { cout << "  错误: 无法保存职员数据!" << endl; return; }
    for (size_t i = 0; i < g_employees.size(); i++) {
        const Employee& e = g_employees[i];
        f << e.id << "|" << e.name << "|" << e.password << "|"
          << e.role << "|" << e.department << "|" << e.phone << "|"
          << e.email << "|" << e.hire_date << "|"
          << (e.is_active ? "1" : "0") << endl;
    }
    f.close();
}

// ==================== 职员CRUD ====================

inline int find_employee(const string& id) {
    map<string,int>::iterator it = g_emp_index.find(id);
    if (it != g_emp_index.end() && it->second < (int)g_employees.size()
        && g_employees[it->second].id == id)
        return it->second;
    // 回退线性查找
    for (int i = 0; i < (int)g_employees.size(); i++)
        if (g_employees[i].id == id) return i;
    return -1;
}

inline void add_employee() {
    Employee e;
    cout << "\n  ═══ 添加职员 ═══\n";
    cout << "  职员编号 (E+3位数字, 如E003): "; getline(cin, e.id);
    if (find_employee(e.id) != -1) {
        cout << "  ✗ 职员编号已存在!\n"; return;
    }
    cout << "  姓名: "; getline(cin, e.name);
    cout << "  初始密码 (6位数字): "; getline(cin, e.password);
    if (!is_6digit(e.password)) { cout << "  ✗ 密码格式错误!\n"; return; }
    cout << "  角色 (admin/staff): "; getline(cin, e.role);
    cout << "  部门: "; getline(cin, e.department);
    cout << "  电话: "; getline(cin, e.phone);
    cout << "  邮箱: "; getline(cin, e.email);
    e.hire_date = today_str();
    e.is_active = true;
    g_emp_index[e.id] = g_employees.size();
    g_employees.push_back(e);
    save_employees();
    cout << "  ✓ 职员 " << e.name << " 添加成功!\n";
}

inline void delete_employee() {
    string id;
    cout << "\n  ═══ 删除职员 ═══\n";
    cout << "  输入职员编号: "; getline(cin, id);
    int idx = find_employee(id);
    if (idx == -1) { cout << "  ✗ 职员不存在!\n"; return; }
    if (g_employees[idx].role == "admin") {
        cout << "  ✗ 不能删除管理员账户!\n"; return;
    }
    cout << "  确认删除 " << g_employees[idx].name << "? (y/n): ";
    string confirm; getline(cin, confirm);
    if (confirm == "y" || confirm == "Y") {
        g_employees[idx].is_active = false;
        save_employees();
        cout << "  ✓ 职员已删除(停用)\n";
    }
}

inline void modify_employee() {
    string id;
    cout << "\n  ═══ 修改职员信息 ═══\n";
    cout << "  输入职员编号: "; getline(cin, id);
    int idx = find_employee(id);
    if (idx == -1) { cout << "  ✗ 职员不存在!\n"; return; }
    Employee& e = g_employees[idx];
    cout << "  当前姓名 [" << e.name << "]: "; string s; getline(cin, s);
    if (!s.empty()) e.name = s;
    cout << "  当前部门 [" << e.department << "]: "; getline(cin, s);
    if (!s.empty()) e.department = s;
    cout << "  当前电话 [" << e.phone << "]: "; getline(cin, s);
    if (!s.empty()) e.phone = s;
    cout << "  当前邮箱 [" << e.email << "]: "; getline(cin, s);
    if (!s.empty()) e.email = s;
    save_employees();
    cout << "  ✓ 职员信息已更新!\n";
}

inline void query_employee() {
    string keyword;
    cout << "\n  ═══ 查询职员 ═══\n";
    cout << "  输入姓名或编号关键字: "; getline(cin, keyword);
    hr(); cout << "  " << left << setw(8) << "编号" << setw(10) << "姓名"
         << setw(10) << "角色" << setw(14) << "部门" << setw(14) << "电话" << endl;
    hr();
    bool found = false;
    for (size_t i = 0; i < g_employees.size(); i++) {
        const Employee& e = g_employees[i];
        if (!e.is_active) continue;
        if (e.id.find(keyword) != string::npos ||
            e.name.find(keyword) != string::npos) {
            cout << "  " << left << setw(8) << e.id << setw(10) << e.name
                 << setw(10) << e.role << setw(14) << e.department
                 << setw(14) << e.phone << endl;
            found = true;
        }
    }
    if (!found) cout << "  未找到匹配的职员。\n";
    hr();
}

inline void list_employees() {
    hr();
    cout << "  " << left << setw(8) << "编号" << setw(10) << "姓名"
         << setw(10) << "角色" << setw(14) << "部门" << setw(14) << "电话"
         << setw(6) << "状态" << endl;
    hr();
    for (size_t i = 0; i < g_employees.size(); i++) {
        const Employee& e = g_employees[i];
        cout << "  " << left << setw(8) << e.id << setw(10) << e.name
             << setw(10) << e.role << setw(14) << e.department
             << setw(14) << e.phone << setw(6)
             << (e.is_active ? "在职" : "离职") << endl;
    }
    hr();
}

/** 职员重置密码 */
inline void reset_employee_password() {
    string id;
    cout << "\n  输入职员编号: "; getline(cin, id);
    int idx = find_employee(id);
    if (idx == -1) { cout << "  ✗ 职员不存在!\n"; return; }
    cout << "  新密码 (6位数字): "; string pw; getline(cin, pw);
    if (!is_6digit(pw)) { cout << "  ✗ 密码格式错误!\n"; return; }
    g_employees[idx].password = pw;
    save_employees();
    cout << "  ✓ 密码已重置!\n";
}

/** 职员自己修改密码 */
inline void change_emp_password(int emp_idx) {
    cout << "\n  ═══ 修改登录密码 ═══\n";
    cout << "  当前密码: "; string old_pw; getline(cin, old_pw);
    if (old_pw != g_employees[emp_idx].password) {
        cout << "  ✗ 当前密码错误!\n"; return;
    }
    cout << "  新密码 (6位数字): "; string new_pw; getline(cin, new_pw);
    if (!is_6digit(new_pw)) { cout << "  ✗ 密码格式错误!\n"; return; }
    g_employees[emp_idx].password = new_pw;
    save_employees();
    cout << "  ✓ 密码修改成功!\n";
}

/** 职员修改个人信息 */
inline void modify_emp_profile(int emp_idx) {
    Employee& e = g_employees[emp_idx];
    cout << "\n  ═══ 修改个人信息 ═══\n";
    cout << "  当前电话 [" << e.phone << "]: "; string s; getline(cin, s);
    if (!s.empty()) e.phone = s;
    cout << "  当前邮箱 [" << e.email << "]: "; getline(cin, s);
    if (!s.empty()) e.email = s;
    save_employees();
    cout << "  ✓ 个人信息已更新!\n";
}

/** 职员管理菜单(管理员使用) */
inline void employee_menu_admin() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║     需求1: 银行职员管理         ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 查看所有职员                ║\n";
        cout << "  ║  2. 添加职员                    ║\n";
        cout << "  ║  3. 删除职员(停用)              ║\n";
        cout << "  ║  4. 修改职员信息                ║\n";
        cout << "  ║  5. 查询职员                    ║\n";
        cout << "  ║  6. 重置职员密码                ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { list_employees(); pause(); }
        else if (ch == "2") { add_employee(); pause(); }
        else if (ch == "3") { delete_employee(); pause(); }
        else if (ch == "4") { modify_employee(); pause(); }
        else if (ch == "5") { query_employee(); pause(); }
        else if (ch == "6") { reset_employee_password(); pause(); }
        else if (ch == "0") break;
    }
}

#endif
