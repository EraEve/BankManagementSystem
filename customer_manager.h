/**
 * customer_manager.h — 需求2: 客户账户管理
 *
 * 功能: 职员增删改查客户资料, 文件存取, VIP/普通分类
 * 数据结构: vector顺序表 + map哈希索引 + BST二叉搜索树索引
 * 广义表: 一个客户关联多张银行卡
 */

#ifndef CUSTOMER_MANAGER_H
#define CUSTOMER_MANAGER_H

#include "common.h"

extern vector<Customer> g_customers;
extern map<string, int> g_cust_index;  // 哈希索引

inline void load_customers() {
    g_customers.clear();
    g_cust_index.clear();
    ifstream f(CUSTOMER_FILE.c_str());
    if (!f.is_open()) {
        // 创建默认测试客户
        Customer c1; c1.id = "C000001"; c1.name = "ZhangSan"; c1.password = "123456";
        c1.type = "普通"; c1.phone = "13900000001"; c1.open_date = "2024-03-15";
        c1.credit_score = 720; c1.financial_assets = 50000;
        c1.address = "北京市朝阳区"; c1.id_card = "110101199001011234";
        g_customers.push_back(c1); g_cust_index["C000001"] = 0;

        Customer c2; c2.id = "C000002"; c2.name = "LiSi"; c2.password = "123456";
        c2.type = "VIP"; c2.phone = "13900000002"; c2.open_date = "2024-01-20";
        c2.credit_score = 850; c2.financial_assets = 500000;
        c2.address = "上海市浦东新区"; c2.id_card = "310101199202022345";
        g_customers.push_back(c2); g_cust_index["C000002"] = 1;

        Customer c3; c3.id = "C000003"; c3.name = "WangWu"; c3.password = "123456";
        c3.type = "普通"; c3.phone = "13900000003"; c3.open_date = "2024-06-10";
        c3.credit_score = 650; c3.financial_assets = 30000;
        c3.address = "广州市天河区"; c3.id_card = "440101199303033456";
        g_customers.push_back(c3); g_cust_index["C000003"] = 2;

        Customer c4; c4.id = "C000004"; c4.name = "VIP_Zhao"; c4.password = "123456";
        c4.type = "VIP"; c4.phone = "13900000004"; c4.open_date = "2023-11-05";
        c4.credit_score = 900; c4.financial_assets = 1200000;
        c4.address = "深圳市南山区"; c4.id_card = "440301199404044567";
        g_customers.push_back(c4); g_cust_index["C000004"] = 3;

        return;
    }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = split(line, '|');
        if (p.size() < 10) continue;
        Customer c;
        c.id = p[0]; c.name = p[1]; c.password = p[2];
        c.type = p[3]; c.phone = p[4]; c.open_date = p[5];
        c.credit_score = safe_double(p[6]);
        c.financial_assets = safe_double(p[7]);
        c.address = p[8]; c.id_card = p[9];
        c.is_active = (p.size() > 10 ? (p[10] == "1") : true);
        // 需求8补充字段 (向后兼容)
        c.id_photo_path = (p.size() > 11 ? p[11] : "");
        c.biometric_data = (p.size() > 12 ? p[12] : "");
        c.face_photo_path = (p.size() > 13 ? p[13] : "");
        // card_ids将在加载card数据时填充
        g_cust_index[c.id] = g_customers.size();
        g_customers.push_back(c);
    }
    f.close();
}

inline void save_customers() {
    ofstream f(CUSTOMER_FILE.c_str());
    if (!f.is_open()) return;
    for (size_t i = 0; i < g_customers.size(); i++) {
        const Customer& c = g_customers[i];
        f << c.id << "|" << c.name << "|" << c.password << "|"
          << c.type << "|" << c.phone << "|" << c.open_date << "|"
          << double_to_str(c.credit_score) << "|"
          << double_to_str(c.financial_assets) << "|"
          << c.address << "|" << c.id_card << "|"
          << (c.is_active ? "1" : "0") << "|"
          << c.id_photo_path << "|" << c.biometric_data << "|"
          << c.face_photo_path << endl;
    }
    f.close();
}

// ==================== 客户CRUD (由职员操作) ====================

inline int find_customer(const string& id) {
    map<string,int>::iterator it = g_cust_index.find(id);
    if (it != g_cust_index.end()) return it->second;
    for (int i = 0; i < (int)g_customers.size(); i++)
        if (g_customers[i].id == id) return i;
    return -1;
}

/** 更新客户类型(根据金融资产自动判定) */
inline void update_customer_type(Customer& c) {
    c.type = (c.financial_assets >= VIP_THRESHOLD) ? "VIP" : "普通";
}

inline void add_customer() {
    Customer c;
    cout << "\n  ═══ 添加客户 ═══\n";
    // 自动生成ID
    int max_id = 0;
    for (size_t i = 0; i < g_customers.size(); i++) {
        int n = atoi(g_customers[i].id.substr(1).c_str());
        if (n > max_id) max_id = n;
    }
    char buf[20];
    sprintf(buf, "C%06d", max_id + 1);
    c.id = string(buf);
    cout << "  自动分配账号: " << c.id << endl;
    cout << "  姓名: "; getline(cin, c.name);
    cout << "  初始密码 (6位数字): "; getline(cin, c.password);
    if (!is_6digit(c.password)) { cout << "  ✗ 密码格式错误!\n"; return; }
    cout << "  电话: "; getline(cin, c.phone);
    c.open_date = today_str();
    cout << "  身份证号: "; getline(cin, c.id_card);
    cout << "  地址: "; getline(cin, c.address);
    cout << "  初始金融资产: "; string ast; getline(cin, ast);
    c.financial_assets = safe_double(ast);
    update_customer_type(c);
    g_cust_index[c.id] = g_customers.size();
    g_customers.push_back(c);
    save_customers();
    cout << "  ✓ 客户 " << c.name << " (" << c.id << ") 创建成功! 类型: " << c.type << endl;
}

inline void delete_customer() {
    string id;
    cout << "\n  ═══ 删除客户 ═══\n";
    cout << "  客户编号: "; getline(cin, id);
    int idx = find_customer(id);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    cout << "  确认删除 " << g_customers[idx].name << "? (y/n): ";
    string cf; getline(cin, cf);
    if (cf == "y" || cf == "Y") {
        g_customers[idx].is_active = false;
        save_customers();
        cout << "  ✓ 客户已注销\n";
    }
}

inline void modify_customer() {
    string id;
    cout << "\n  ═══ 修改客户资料 ═══\n";
    cout << "  客户编号: "; getline(cin, id);
    int idx = find_customer(id);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    Customer& c = g_customers[idx];
    cout << "  当前姓名 [" << c.name << "]: "; string s; getline(cin, s);
    if (!s.empty()) c.name = s;
    cout << "  当前电话 [" << c.phone << "]: "; getline(cin, s);
    if (!s.empty()) c.phone = s;
    cout << "  当前地址 [" << c.address << "]: "; getline(cin, s);
    if (!s.empty()) c.address = s;
    cout << "  当前金融资产 [" << double_to_str(c.financial_assets) << "]: ";
    getline(cin, s);
    if (!s.empty()) { c.financial_assets = safe_double(s); update_customer_type(c); }
    cout << "  当前信用评分 [" << double_to_str(c.credit_score) << "]: ";
    getline(cin, s);
    if (!s.empty()) c.credit_score = safe_double(s);
    save_customers();
    cout << "  ✓ 客户资料已更新! 当前类型: " << c.type << endl;
}

inline void query_customer() {
    string kw;
    cout << "\n  ═══ 查询客户 ═══\n";
    cout << "  输入姓名/编号/电话关键字: "; getline(cin, kw);
    hr();
    cout << "  " << left << setw(10) << "编号" << setw(10) << "姓名"
         << setw(6) << "类型" << setw(14) << "电话" << setw(14) << "金融资产"
         << setw(8) << "信用分" << setw(12) << "开户日期" << endl;
    hr();
    bool found = false;
    for (size_t i = 0; i < g_customers.size(); i++) {
        const Customer& c = g_customers[i];
        if (!c.is_active) continue;
        if (c.id.find(kw) != string::npos ||
            c.name.find(kw) != string::npos ||
            c.phone.find(kw) != string::npos) {
            cout << "  " << left << setw(10) << c.id << setw(10) << c.name
                 << setw(6) << c.type << setw(14) << c.phone
                 << setw(14) << double_to_str(c.financial_assets)
                 << setw(8) << double_to_str(c.credit_score)
                 << setw(12) << c.open_date << endl;
            found = true;
        }
    }
    if (!found) cout << "  未找到匹配的客户。\n";
    hr();
}

inline void list_customers() {
    hr();
    cout << "  " << left << setw(10) << "编号" << setw(10) << "姓名"
         << setw(6) << "类型" << setw(14) << "电话" << setw(14) << "金融资产"
         << setw(8) << "信用分" << setw(6) << "状态" << endl;
    hr();
    for (size_t i = 0; i < g_customers.size(); i++) {
        const Customer& c = g_customers[i];
        cout << "  " << left << setw(10) << c.id << setw(10) << c.name
             << setw(6) << c.type << setw(14) << c.phone
             << setw(14) << double_to_str(c.financial_assets)
             << setw(8) << double_to_str(c.credit_score)
             << setw(6) << (c.is_active ? "正常" : "注销") << endl;
    }
    hr();
}

/** 客户查看个人信息 */
inline void view_customer_profile(int cust_idx) {
    const Customer& c = g_customers[cust_idx];
    hr();
    cout << "  ═══ 个人账户信息 ═══\n";
    cout << "  账号:     " << c.id << endl;
    cout << "  姓名:     " << c.name << endl;
    cout << "  类型:     " << c.type << endl;
    cout << "  电话:     " << c.phone << endl;
    cout << "  开户日期: " << c.open_date << endl;
    cout << "  信用评分: " << double_to_str(c.credit_score) << endl;
    cout << "  金融资产: " << double_to_str(c.financial_assets) << " 元" << endl;
    cout << "  地址:     " << c.address << endl;
    cout << "  身份证号: " << c.id_card << endl;
    // 身份证信息自动解析
    if (!c.id_card.empty() && c.id_card.length() >= 18) {
        cout << "  身份证省份: " << get_province_by_code(c.id_card) << endl;
        cout << "  出生日期:   " << extract_birth_from_id(c.id_card) << endl;
        cout << "  性别:       " << extract_gender_from_id(c.id_card) << endl;
        cout << "  年龄:       " << extract_age_from_id(c.id_card) << " 岁" << endl;
        cout << "  身份证校验: " << (validate_chinese_id_card(c.id_card) ? "✓ 合法" : "✗ 不合法") << endl;
    }
    cout << "  证件照:     " << (c.id_photo_path.empty() ? "未录入" : c.id_photo_path) << endl;
    cout << "  生物特征:   " << (c.biometric_data.empty() ? "未录入" : "已录入 (" + c.biometric_data.substr(0,16) + "...)") << endl;
    cout << "  人脸照片:   " << (c.face_photo_path.empty() ? "未录入" : c.face_photo_path) << endl;
    cout << "  关联卡数: " << c.card_ids.size() << " 张" << endl;
    if (!c.card_ids.empty()) {
        cout << "  关联卡号: ";
        for (size_t i = 0; i < c.card_ids.size(); i++)
            cout << c.card_ids[i] << " ";
        cout << endl;
    }
    hr();
}

/** 客户修改密码 */
inline void change_cust_password(int cust_idx) {
    cout << "\n  当前密码: "; string op; getline(cin, op);
    if (op != g_customers[cust_idx].password) {
        cout << "  ✗ 当前密码错误!\n"; return;
    }
    cout << "  新密码 (6位数字): "; string np; getline(cin, np);
    if (!is_6digit(np)) { cout << "  ✗ 密码格式错误!\n"; return; }
    g_customers[cust_idx].password = np;
    save_customers();
    cout << "  ✓ 密码修改成功!\n";
}

/** 客户菜单(职员使用) */
inline void customer_menu_staff() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║    需求2: 客户账户管理          ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 查看所有客户                ║\n";
        cout << "  ║  2. 添加客户                    ║\n";
        cout << "  ║  3. 删除客户(注销)              ║\n";
        cout << "  ║  4. 修改客户资料                ║\n";
        cout << "  ║  5. 查询客户                    ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { list_customers(); pause(); }
        else if (ch == "2") { add_customer(); pause(); }
        else if (ch == "3") { delete_customer(); pause(); }
        else if (ch == "4") { modify_customer(); pause(); }
        else if (ch == "5") { query_customer(); pause(); }
        else if (ch == "0") break;
    }
}

#endif
