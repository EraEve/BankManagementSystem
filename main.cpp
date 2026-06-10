/**
 * main.cpp — 银行智能管理系统 主入口
 *
 * 课程: 数据结构与算法课程设计 · 第十组
 * 版本: Final v2.0 (中期→最终版)
 *
 * 开发历程:
 *   中期版 (v1.0): 基础登录/存取款/转账 (bank_system.cpp, 743行)
 *   最终版 (v2.0): 完整10大需求模块 (多文件架构, ~5000行)
 *
 * 数据结构与算法应用:
 *   · vector 动态数组 - 数据存储
 *   · map/unordered_map 哈希表 - O(1)索引查找
 *   · LinkedQueue 链式队列 - FIFO排队管理
 *   · 优先队列逻辑 - VIP客户优先服务
 *   · 图(邻接矩阵) + Dijkstra算法 - 最短路径导航
 *   · BST二叉搜索树 - 客户索引
 *   · 冒泡排序 - 交易按时间排序
 *   · 快速排序 - 交易按金额排序
 *   · 二分查找 - 有序客户查询
 *   · 滑动窗口 - 高频交易检测
 *   · 加权多因子模型 - 信用评分 & 风控
 *
 * 编译方法:
 *   g++ main.cpp -o bank_system -std=c++11
 *
 * 文件结构:
 *   common.h            - 通用数据结构与工具函数
 *   employee_manager.h  - 需求1: 银行职员管理
 *   customer_manager.h  - 需求2: 客户账户管理
 *   card_manager.h      - 需求3: 银行卡管理
 *   transaction_manager.h - 需求4: 存取贷业务管理
 *   query_manager.h     - 需求5: 业务查询
 *   queue_manager.h     - 需求6: 银行排队管理
 *   branch_manager.h    - 需求7: 网点查询与导航
 *   smart_manager.h     - 需求8: 智能管理
 *   ui_manager.h        - 需求9: 主界面与统一登录
 */

#include "common.h"
#include "employee_manager.h"
#include "customer_manager.h"
#include "card_manager.h"
#include "transaction_manager.h"
#include "query_manager.h"
#include "queue_manager.h"
#include "branch_manager.h"
#include "identity_verifier.h"
#include "smart_manager.h"
#include "ui_manager.h"

// ==================== 全局变量定义 ====================

// 需求1: 职员
vector<Employee> g_employees;
map<string, int> g_emp_index;

// 需求2: 客户
vector<Customer> g_customers;
map<string, int> g_cust_index;

// 需求3: 银行卡
vector<BankCard> g_cards;
map<string, int> g_card_index;

// 需求4: 交易
vector<Transaction> g_transactions;

// 需求6: 排队
LinkedQueue<QueueTicket> g_vip_queue;
LinkedQueue<QueueTicket> g_normal_queue;
vector<QueueTicket> g_queue_history;
vector<DailyStats> g_daily_stats;
int g_ticket_counter = 1000;

// 需求7: 网点
vector<Branch> g_branches;
vector<vector<double> > g_branch_graph;
map<string, int> g_branch_index;

// ==================== 系统初始化 ====================

void init_system() {
    srand((unsigned int)time(0));

    cout << "\n  正在初始化系统...\n";

    // 确保data目录存在
    #ifdef _WIN32
        system("if not exist data mkdir data");
    #else
        system("mkdir -p data");
    #endif

    cout << "  [1/9] 加载职员数据...\n";
    load_employees();

    cout << "  [2/9] 加载客户数据...\n";
    load_customers();

    cout << "  [3/9] 加载银行卡数据...\n";
    load_cards();

    cout << "  [4/9] 加载交易数据...\n";
    load_transactions();

    cout << "  [5/9] 初始化查询模块...\n";

    cout << "  [6/9] 初始化排队管理...\n";
    load_queue_data();

    cout << "  [7/9] 加载网点数据...\n";
    load_branches();

    cout << "  [8/9] 初始化智能管理模块...\n";

    cout << "  [9/9] 初始化主界面...\n";

    // 保存初始数据
    save_employees();
    save_customers();
    save_cards();
    save_branches();
    save_queue_history();
    save_daily_stats();

    cout << "  ✓ 系统初始化完成!\n";
    pause();
}

// ==================== 主函数 ====================

int main() {
    init_system();
    main_menu();
    return 0;
}
