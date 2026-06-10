/**
 * common.h — 银行智能管理系统 通用数据结构与工具函数
 *
 * 课程: 数据结构与算法课程设计
 * 版本: Final v2.0 (中期→最终版)
 *
 * 数据结构应用:
 *   - vector: 动态数组存储
 *   - unordered_map: 哈希表快速查找 O(1)
 *   - 字符串处理: 自定义实现(无STL高级特性)
 *   - 文件I/O: 管道分隔符持久化
 */

#ifndef COMMON_H
#define COMMON_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <map>
#include <queue>
#include <set>
#include <limits>

using namespace std;

// ==================== 常量定义 ====================
const string DATA_DIR = "data/";
const string EMPLOYEE_FILE = "data/employee.txt";
const string CUSTOMER_FILE = "data/customer.txt";
const string CARD_FILE = "data/card.txt";
const string TRANSACTION_FILE = "data/transaction.txt";
const string QUEUE_FILE = "data/queue.txt";
const string BRANCH_FILE = "data/branch.txt";
const string GRAPH_FILE = "data/branch_graph.txt";
const string STATS_FILE = "data/daily_stats.txt";

const double VIP_THRESHOLD = 100000.0;    // VIP 资产门槛
const double LARGE_AMOUNT = 50000.0;      // 大额交易报警阈值
const int MULTI_TRANSACTION = 5;          // 多笔交易报警阈值(每小时)
const double CREDIT_LIMIT_BASE = 50000.0; // 基础信用额度

// ==================== 数据结构定义 ====================

/**
 * 银行职员 (需求1: 银行职员管理)
 * 存储: 顺序表(vector) + 哈希索引(unordered_map)
 */
struct Employee {
    string id;           // 职员编号 (E + 4位数字)
    string name;         // 姓名
    string password;     // 登录密码(6位数字)
    string role;         // 角色: admin(管理员), staff(普通职员)
    string department;   // 部门
    string phone;        // 联系电话
    string email;        // 邮箱
    string hire_date;    // 入职日期
    bool is_active;      // 是否在职

    Employee() : is_active(true) {}
};

/**
 * 客户账户 (需求2: 客户账户管理)
 * 存储: 顺序表 + 哈希索引
 * 数据结构: 广义表思想(一个客户关联多张卡)
 */
struct Customer {
    string id;               // 主账户编号 (C + 6位数字)
    string name;             // 姓名
    string password;         // 登录密码(6位数字)
    string type;             // 类型: "普通" / "VIP"
    string phone;            // 联系电话
    string open_date;        // 开户日期
    double credit_score;     // 信用评分 (0-1000)
    double financial_assets; // 金融资产总额
    string address;          // 地址
    string id_card;          // 身份证号
    vector<string> card_ids; // 关联银行卡编号列表 (广义表)
    bool is_active;          // 是否激活
    // 需求8补充: 身份核验/生物识别/影像读取
    string id_photo_path;    // 证件照文件路径
    string biometric_data;   // 生物特征模板 (模拟指纹/人脸特征)
    string face_photo_path;  // 人脸照片文件路径

    Customer() : credit_score(600), financial_assets(0), is_active(true) {}
};

/**
 * 银行卡 (需求3: 银行卡管理)
 * 存储: 顺序表 + 哈希索引
 * 类型: 借记卡/储蓄卡/信用卡
 */
struct BankCard {
    string id;             // 卡编号 (B + 8位数字)
    string customer_id;    // 所属客户编号
    string type;           // 类型: "借记卡" / "储蓄卡" / "信用卡"
    double balance;        // 余额
    double loan_balance;   // 借贷余额(信用卡已用额度)
    double interest_rate;  // 年利率(%)
    double credit_limit;   // 信用额度(仅信用卡)
    string open_date;      // 开户日期
    string status;         // 状态: "正常" / "冻结" / "注销"
    double daily_limit;    // 日交易限额

    BankCard() : balance(0), loan_balance(0), interest_rate(0.35),
                 credit_limit(0), daily_limit(50000), status("正常") {}
};

/**
 * 交易记录 (需求4: 存取贷业务管理)
 * 存储: 顺序表(按时间追加)
 * 查询: 排序 + 筛选
 */
struct Transaction {
    string id;           // 交易编号 (T + 时间戳 + 随机数)
    string from_card;    // 转出卡号 ("-" 表示系统)
    string to_card;      // 转入卡号 ("-" 表示系统)
    string type;         // 类型: "存款"/"取款"/"转账"/"贷款"/"还款"
    double amount;       // 交易金额
    string time;         // 交易时间
    string status;       // 状态: "成功"/"失败"
    string employee_id;  // 经办职员编号
    string remark;       // 备注

    Transaction() : amount(0), status("成功") {}
};

/**
 * 排队票号 (需求6: 银行排队管理)
 * 数据结构核心: 链式队列 + 优先队列(VIP)
 */
struct QueueTicket {
    string id;            // 票号 (Q + 4位数字)
    string customer_id;   // 客户编号
    string customer_name; // 客户姓名
    string customer_type; // 客户类型: "VIP" / "普通"
    int window_id;        // 分配窗口 (-1: 未分配, 0: VIP窗口, 1+: 普通窗口)
    string service_type;  // 业务类型
    string arrive_time;   // 到达时间
    string start_time;    // 开始服务时间
    string end_time;      // 结束时间
    int rating;           // 客户评分 (1-5, 0=未评)
    string status;        // 状态: "等待中"/"办理中"/"已完成"
    int priority;         // 优先级 (VIP=10, 普通=0)

    QueueTicket() : window_id(-1), rating(0), status("等待中"), priority(0) {}
};

/**
 * 银行网点 (需求7: 银行网点查询)
 * 数据结构: 图的顶点
 * 最短路径: Dijkstra算法
 */
struct Branch {
    string id;       // 网点编号
    string name;     // 网点名称
    string address;  // 地址
    string phone;    // 电话
    string hours;    // 营业时间
    string services; // 提供服务
    double x, y;     // 坐标(用于距离计算)

    Branch() : x(0), y(0) {}
};

/**
 * 每日客流统计 (需求6补充)
 */
struct DailyStats {
    string date;           // 日期
    int total_customers;   // 总客户数
    int vip_customers;     // VIP客户数
    int normal_customers;  // 普通客户数
    double avg_wait_time;  // 平均等待时间(分钟)
    double avg_rating;     // 平均评分
    int completed;         // 完成业务数

    DailyStats() : total_customers(0), vip_customers(0),
                   normal_customers(0), avg_wait_time(0),
                   avg_rating(0), completed(0) {}
};

// ==================== 工具函数 ====================

/** 整数转字符串(不使用to_string) */
inline string int_to_str(long long n) {
    if (n == 0) return "0";
    bool neg = false;
    if (n < 0) { neg = true; n = -n; }
    string r;
    while (n > 0) { r = char('0' + n % 10) + r; n /= 10; }
    if (neg) r = "-" + r;
    return r;
}

/** 保留两位小数的字符串 */
inline string double_to_str(double v) {
    long long ip = (long long)v;
    double dec = v - ip;
    if (dec < 0) dec = -dec;
    long long dp = (long long)(dec * 100 + 0.5);
    if (dp >= 100) { ip++; dp = 0; }
    string r = int_to_str(ip) + ".";
    if (dp < 10) r += "0";
    r += int_to_str(dp);
    return r;
}

/** 获取当前时间字符串 YYYY-MM-DD HH:MM:SS */
inline string now_str() {
    time_t t = time(0);
    tm* l = localtime(&t);
    char buf[20];
    sprintf(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            l->tm_year + 1900, l->tm_mon + 1, l->tm_mday,
            l->tm_hour, l->tm_min, l->tm_sec);
    return string(buf);
}

/** 获取当前日期字符串 YYYY-MM-DD */
inline string today_str() {
    time_t t = time(0);
    tm* l = localtime(&t);
    char buf[11];
    sprintf(buf, "%04d-%02d-%02d", l->tm_year + 1900, l->tm_mon + 1, l->tm_mday);
    return string(buf);
}

/** 生成唯一交易ID */
inline string gen_txn_id() {
    static int seq = 0;
    seq++;
    time_t t = time(0);
    return "T" + int_to_str((long long)t) + int_to_str(seq % 10000);
}

/** 字符串分割 */
inline vector<string> split(const string& s, char delim) {
    vector<string> parts;
    string cur;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == delim) { parts.push_back(cur); cur = ""; }
        else cur += s[i];
    }
    parts.push_back(cur);
    return parts;
}

/** 判断是否为6位数字 */
inline bool is_6digit(const string& s) {
    if (s.length() != 6) return false;
    for (size_t i = 0; i < 6; i++)
        if (s[i] < '0' || s[i] > '9') return false;
    return true;
}

/** 判断是否为有效金额 */
inline bool is_valid_amount(const string& s) {
    if (s.empty()) return false;
    int dots = 0;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '.') { dots++; if (dots > 1) return false; }
        else if (s[i] < '0' || s[i] > '9') return false;
    }
    double v = atof(s.c_str());
    return v > 0;
}

/** 计算两个日期相差天数 */
inline int days_between(const string& d1, const string& d2) {
    // 简单实现: YYYY-MM-DD
    int y1, m1, d_1, y2, m2, d_2;
    sscanf(d1.c_str(), "%d-%d-%d", &y1, &m1, &d_1);
    sscanf(d2.c_str(), "%d-%d-%d", &y2, &m2, &d_2);
    int total1 = y1 * 365 + m1 * 30 + d_1;
    int total2 = y2 * 365 + m2 * 30 + d_2;
    return abs(total2 - total1);
}

/** 按回车继续 */
inline void pause() {
    cout << "\n  按回车键继续...";
    string tmp;
    getline(cin, tmp);
}

/** 打印分隔线 */
inline void hr(char c = '-', int n = 60) {
    cout << "  " << string(n, c) << endl;
}

/** 清屏 */
inline void cls() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

/** 安全读double */
inline double safe_double(const string& s) {
    return atof(s.c_str());
}

/** 时间比较 (t1 < t2 返回-1, t1 == t2 返回0, t1 > t2 返回1) */
inline int time_cmp(const string& t1, const string& t2) {
    if (t1 < t2) return -1;
    if (t1 > t2) return 1;
    return 0;
}

/**
 * Dijkstra 最短路径算法
 * @param graph 邻接矩阵 (NxN), graph[i][j] = 距离, -1表示无边
 * @param N 顶点数
 * @param src 源顶点
 * @param dist 输出: 最短距离数组
 * @param prev 输出: 前驱节点数组(用于路径重构)
 */
inline void dijkstra(const vector<vector<double> >& graph, int N, int src,
                     vector<double>& dist, vector<int>& prev) {
    dist.assign(N, 1e18);
    prev.assign(N, -1);
    vector<bool> visited(N, false);
    dist[src] = 0;

    for (int i = 0; i < N; i++) {
        // 找未访问的最小距离顶点
        int u = -1;
        double min_d = 1e18;
        for (int j = 0; j < N; j++) {
            if (!visited[j] && dist[j] < min_d) {
                min_d = dist[j];
                u = j;
            }
        }
        if (u == -1) break;
        visited[u] = true;

        // 松弛操作
        for (int v = 0; v < N; v++) {
            if (!visited[v] && graph[u][v] >= 0) {
                double alt = dist[u] + graph[u][v];
                if (alt < dist[v]) {
                    dist[v] = alt;
                    prev[v] = u;
                }
            }
        }
    }
}

/**
 * 冒泡排序 (用于交易记录排序 - 展示基础算法)
 * 按交易时间排序
 */
inline void bubble_sort_txn(vector<Transaction>& txns, bool ascending = true) {
    int n = txns.size();
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            bool swap_needed = ascending ?
                txns[j].time > txns[j+1].time :
                txns[j].time < txns[j+1].time;
            if (swap_needed) {
                Transaction tmp = txns[j];
                txns[j] = txns[j+1];
                txns[j+1] = tmp;
            }
        }
    }
}

/**
 * 快速排序分区函数 (按金额排序)
 */
inline int partition_by_amount(vector<Transaction>& txns, int low, int high) {
    double pivot = txns[high].amount;
    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (txns[j].amount <= pivot) {
            i++;
            swap(txns[i], txns[j]);
        }
    }
    swap(txns[i+1], txns[high]);
    return i + 1;
}

/** 快速排序 - 交易记录按金额 */
inline void quicksort_txn_amount(vector<Transaction>& txns, int low, int high) {
    if (low < high) {
        int pi = partition_by_amount(txns, low, high);
        quicksort_txn_amount(txns, low, pi - 1);
        quicksort_txn_amount(txns, pi + 1, high);
    }
}

/**
 * 二分查找 - 在已排序数组中查找客户
 * @return 索引, -1表示未找到
 */
inline int binary_search_customer(const vector<Customer>& customers, const string& id) {
    int low = 0, high = customers.size() - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (customers[mid].id == id) return mid;
        if (customers[mid].id < id) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

/**
 * 链式队列节点 (用于排队管理)
 */
template<typename T>
struct QueueNode {
    T data;
    QueueNode<T>* next;
    QueueNode(const T& d) : data(d), next(NULL) {}
};

/**
 * 链式队列 (FIFO - 用于银行排队)
 */
template<typename T>
class LinkedQueue {
private:
    QueueNode<T>* front;
    QueueNode<T>* rear;
    int sz;
public:
    LinkedQueue() : front(NULL), rear(NULL), sz(0) {}
    ~LinkedQueue() { while (!empty()) pop(); }

    void push(const T& val) {
        QueueNode<T>* node = new QueueNode<T>(val);
        if (rear) rear->next = node;
        else front = node;
        rear = node;
        sz++;
    }

    void pop() {
        if (!front) return;
        QueueNode<T>* tmp = front;
        front = front->next;
        if (!front) rear = NULL;
        delete tmp;
        sz--;
    }

    T peek() const { return front ? front->data : T(); }
    bool empty() const { return sz == 0; }
    int size() const { return sz; }

    // 遍历队列(不弹出)
    void for_each(void (*fn)(const T&)) const {
        QueueNode<T>* cur = front;
        while (cur) { fn(cur->data); cur = cur->next; }
    }

    // 查找并移除指定元素
    bool remove_if(bool (*pred)(const T&)) {
        QueueNode<T>*cur = front, *prev = NULL;
        while (cur) {
            if (pred(cur->data)) {
                if (prev) prev->next = cur->next;
                else front = cur->next;
                if (cur == rear) rear = prev;
                delete cur;
                sz--;
                return true;
            }
            prev = cur;
            cur = cur->next;
        }
        return false;
    }
};

/**
 * BST节点 (二叉搜索树 - 用于客户ID索引)
 */
template<typename K, typename V>
struct BSTNode {
    K key;
    V value;
    BSTNode<K,V>* left;
    BSTNode<K,V>* right;
    BSTNode(const K& k, const V& v) : key(k), value(v), left(NULL), right(NULL) {}
};

/**
 * 二叉搜索树 (用于动态索引)
 */
template<typename K, typename V>
class BSTree {
private:
    BSTNode<K,V>* root;
    int sz;

    BSTNode<K,V>* insert_node(BSTNode<K,V>* node, const K& key, const V& val) {
        if (!node) { sz++; return new BSTNode<K,V>(key, val); }
        if (key < node->key) node->left = insert_node(node->left, key, val);
        else if (key > node->key) node->right = insert_node(node->right, key, val);
        else node->value = val; // 更新
        return node;
    }

    BSTNode<K,V>* find_node(BSTNode<K,V>* node, const K& key) const {
        if (!node) return NULL;
        if (key == node->key) return node;
        if (key < node->key) return find_node(node->left, key);
        return find_node(node->right, key);
    }

    BSTNode<K,V>* remove_node(BSTNode<K,V>* node, const K& key) {
        if (!node) return NULL;
        if (key < node->key) node->left = remove_node(node->left, key);
        else if (key > node->key) node->right = remove_node(node->right, key);
        else {
            if (!node->left) { BSTNode<K,V>* r = node->right; delete node; sz--; return r; }
            if (!node->right) { BSTNode<K,V>* l = node->left; delete node; sz--; return l; }
            // 找后继
            BSTNode<K,V>* succ = node->right;
            while (succ->left) succ = succ->left;
            node->key = succ->key;
            node->value = succ->value;
            node->right = remove_node(node->right, succ->key);
        }
        return node;
    }

    void inorder_traverse(BSTNode<K,V>* node, void (*fn)(const K&, const V&)) const {
        if (!node) return;
        inorder_traverse(node->left, fn);
        fn(node->key, node->value);
        inorder_traverse(node->right, fn);
    }

public:
    BSTree() : root(NULL), sz(0) {}
    ~BSTree() { /* 简化:不递归删除,实际项目需要 */ }

    void insert(const K& key, const V& val) { root = insert_node(root, key, val); }
    bool find(const K& key, V& out) const {
        BSTNode<K,V>* n = find_node(root, key);
        if (n) { out = n->value; return true; }
        return false;
    }
    void remove(const K& key) { root = remove_node(root, key); }
    int size() const { return sz; }
    void inorder(void (*fn)(const K&, const V&)) const { inorder_traverse(root, fn); }
};

// ==================== 需求8补充: 身份证校验工具 ====================

/** 中国身份证号码行政区划代码(前6位)对照表(部分) */
inline string get_province_by_code(const string& code) {
    // 前2位代表省份
    if (code.length() < 2) return "未知";
    string p2 = code.substr(0, 2);
    if (p2 == "11") return "北京市";
    if (p2 == "12") return "天津市";
    if (p2 == "13") return "河北省";
    if (p2 == "14") return "山西省";
    if (p2 == "15") return "内蒙古自治区";
    if (p2 == "21") return "辽宁省";
    if (p2 == "22") return "吉林省";
    if (p2 == "23") return "黑龙江省";
    if (p2 == "31") return "上海市";
    if (p2 == "32") return "江苏省";
    if (p2 == "33") return "浙江省";
    if (p2 == "34") return "安徽省";
    if (p2 == "35") return "福建省";
    if (p2 == "36") return "江西省";
    if (p2 == "37") return "山东省";
    if (p2 == "41") return "河南省";
    if (p2 == "42") return "湖北省";
    if (p2 == "43") return "湖南省";
    if (p2 == "44") return "广东省";
    if (p2 == "45") return "广西壮族自治区";
    if (p2 == "46") return "海南省";
    if (p2 == "50") return "重庆市";
    if (p2 == "51") return "四川省";
    if (p2 == "52") return "贵州省";
    if (p2 == "53") return "云南省";
    if (p2 == "54") return "西藏自治区";
    if (p2 == "61") return "陕西省";
    if (p2 == "62") return "甘肃省";
    if (p2 == "63") return "青海省";
    if (p2 == "64") return "宁夏回族自治区";
    if (p2 == "65") return "新疆维吾尔自治区";
    if (p2 == "71") return "台湾省";
    if (p2 == "81") return "香港特别行政区";
    if (p2 == "82") return "澳门特别行政区";
    return "未知省份";
}

/**
 * 中国18位身份证号码校验 (ISO 7064:1983 MOD 11-2)
 * @param id 18位身份证号
 * @return true=格式合法, false=不合法
 */
inline bool validate_chinese_id_card(const string& id) {
    if (id.length() != 18) return false;
    // 前17位必须为数字
    for (int i = 0; i < 17; i++) {
        if (id[i] < '0' || id[i] > '9') return false;
    }
    // 第18位: 数字或X
    char last = id[17];
    if (!((last >= '0' && last <= '9') || last == 'X' || last == 'x')) return false;
    // 校验出生日期
    int year = (id[6]-'0')*1000 + (id[7]-'0')*100 + (id[8]-'0')*10 + (id[9]-'0');
    int month = (id[10]-'0')*10 + (id[11]-'0');
    int day = (id[12]-'0')*10 + (id[13]-'0');
    if (year < 1900 || year > 2100) return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    // 校验位计算 (ISO 7064 MOD 11-2)
    int weights[17] = {7,9,10,5,8,4,2,1,6,3,7,9,10,5,8,4,2};
    char check_map[11] = {'1','0','X','9','8','7','6','5','4','3','2'};
    int sum = 0;
    for (int i = 0; i < 17; i++) {
        sum += (id[i] - '0') * weights[i];
    }
    int remainder = sum % 11;
    char expected = check_map[remainder];
    return (last == expected || (last == 'x' && expected == 'X'));
}

/** 从身份证号提取出生日期 */
inline string extract_birth_from_id(const string& id) {
    if (id.length() < 14) return "未知";
    return id.substr(6, 4) + "-" + id.substr(10, 2) + "-" + id.substr(12, 2);
}

/** 从身份证号提取性别 (第17位: 奇=男, 偶=女) */
inline string extract_gender_from_id(const string& id) {
    if (id.length() < 17) return "未知";
    int digit = id[16] - '0';
    return (digit % 2 == 1) ? "男" : "女";
}

/** 从身份证号提取年龄 */
inline int extract_age_from_id(const string& id) {
    if (id.length() < 14) return -1;
    int by = (id[6]-'0')*1000 + (id[7]-'0')*100 + (id[8]-'0')*10 + (id[9]-'0');
    int bm = (id[10]-'0')*10 + (id[11]-'0');
    int bd = (id[12]-'0')*10 + (id[13]-'0');
    // 获取当前日期
    time_t t = time(0);
    tm* l = localtime(&t);
    int cy = l->tm_year + 1900;
    int cm = l->tm_mon + 1;
    int cd = l->tm_mday;
    int age = cy - by;
    if (cm < bm || (cm == bm && cd < bd)) age--;
    return age;
}

/** 生成模拟生物特征模板 (简化为身份证号哈希) */
inline string generate_biometric_template(const string& id_card) {
    // 模拟: 对身份证号做简单哈希
    string tmpl;
    for (size_t i = 0; i < id_card.length(); i++) {
        char c = id_card[i];
        int v = (c * 7 + i * 13) % 256;
        char hex[3];
        sprintf(hex, "%02X", v);
        tmpl += hex;
    }
    return tmpl;
}

/** 比对两个生物特征模板 (模拟: 字符串相似度) */
inline double compare_biometric_templates(const string& t1, const string& t2) {
    if (t1.empty() || t2.empty()) return 0.0;
    if (t1 == t2) return 100.0;
    int match = 0;
    size_t len = t1.length() < t2.length() ? t1.length() : t2.length();
    for (size_t i = 0; i < len; i++) {
        if (t1[i] == t2[i]) match++;
    }
    return (double)match / len * 100.0;
}

#endif // COMMON_H
