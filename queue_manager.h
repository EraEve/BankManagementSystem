/**
 * queue_manager.h — 需求6: 银行排队管理
 *
 * 功能: VIP/普通窗口, 排队取号, VIP优先, 业务办理, 客户评分, 每日统计
 * 数据结构核心:
 *   - LinkedQueue<T>: 链式队列 (FIFO)
 *   - 优先队列逻辑: VIP客户优先分配窗口
 *   - 两个独立队列: VIP队列 + 普通队列
 */

#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include "common.h"

extern LinkedQueue<QueueTicket> g_vip_queue;
extern LinkedQueue<QueueTicket> g_normal_queue;
extern vector<QueueTicket> g_queue_history;
extern vector<DailyStats> g_daily_stats;
extern int g_ticket_counter;

inline void load_queue_data() {
    g_queue_history.clear();
    g_daily_stats.clear();
    g_ticket_counter = 1000;

    ifstream f(QUEUE_FILE.c_str());
    if (!f.is_open()) return;
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = split(line, '|');
        if (p.size() < 10) continue;
        QueueTicket t;
        t.id = p[0]; t.customer_id = p[1]; t.customer_name = p[2];
        t.customer_type = p[3]; t.window_id = atoi(p[4].c_str());
        t.service_type = p[5]; t.arrive_time = p[6]; t.start_time = p[7];
        t.end_time = p[8]; t.rating = atoi(p[9].c_str()); t.status = p[10];
        t.priority = (t.customer_type == "VIP") ? 10 : 0;
        g_queue_history.push_back(t);
    }
    f.close();

    // 加载每日统计
    ifstream sf(STATS_FILE.c_str());
    if (sf.is_open()) {
        while (getline(sf, line)) {
            if (line.empty()) continue;
            vector<string> p = split(line, '|');
            if (p.size() < 6) continue;
            DailyStats ds;
            ds.date = p[0]; ds.total_customers = atoi(p[1].c_str());
            ds.vip_customers = atoi(p[2].c_str()); ds.normal_customers = atoi(p[3].c_str());
            ds.avg_wait_time = safe_double(p[4]); ds.avg_rating = safe_double(p[5]);
            ds.completed = atoi(p[6].c_str());
            g_daily_stats.push_back(ds);
        }
        sf.close();
    }
}

inline void save_queue_history() {
    ofstream f(QUEUE_FILE.c_str());
    if (!f.is_open()) return;
    for (size_t i = 0; i < g_queue_history.size(); i++) {
        const QueueTicket& t = g_queue_history[i];
        f << t.id << "|" << t.customer_id << "|" << t.customer_name << "|"
          << t.customer_type << "|" << t.window_id << "|"
          << t.service_type << "|" << t.arrive_time << "|"
          << t.start_time << "|" << t.end_time << "|"
          << t.rating << "|" << t.status << endl;
    }
    f.close();
}

inline void save_daily_stats() {
    ofstream f(STATS_FILE.c_str());
    if (!f.is_open()) return;
    for (size_t i = 0; i < g_daily_stats.size(); i++) {
        const DailyStats& ds = g_daily_stats[i];
        f << ds.date << "|" << ds.total_customers << "|"
          << ds.vip_customers << "|" << ds.normal_customers << "|"
          << double_to_str(ds.avg_wait_time) << "|"
          << double_to_str(ds.avg_rating) << "|"
          << ds.completed << endl;
    }
    f.close();
}

// ==================== 排队操作 ====================

/** 客户取号 (由职员操作) */
inline void take_ticket() {
    cout << "\n  ═══ 排队取号 ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int cust_idx = find_customer(cid);
    if (cust_idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    cout << "  业务类型 (存款/取款/转账/贷款/开户/其他): ";
    string stype; getline(cin, stype);

    g_ticket_counter++;
    QueueTicket t;
    char buf[10]; sprintf(buf, "Q%04d", g_ticket_counter % 10000);
    t.id = string(buf);
    t.customer_id = cid;
    t.customer_name = g_customers[cust_idx].name;
    t.customer_type = g_customers[cust_idx].type;
    t.service_type = stype;
    t.arrive_time = now_str();
    t.status = "等待中";
    t.priority = (t.customer_type == "VIP") ? 10 : 0;

    if (t.customer_type == "VIP") {
        g_vip_queue.push(t);
        cout << "  ✓ 取号成功! 票号: " << t.id << " (VIP优先窗口)\n";
    } else {
        g_normal_queue.push(t);
        cout << "  ✓ 取号成功! 票号: " << t.id << " (普通窗口)\n";
    }
    cout << "  当前VIP队列: " << g_vip_queue.size()
         << " 人, 普通队列: " << g_normal_queue.size() << " 人\n";
}

/** 叫号 - VIP优先 (由职员操作) */
inline void call_ticket() {
    cout << "\n  ═══ 叫号办理 ═══\n";
    cout << "  窗口类型 (1.VIP窗口  2.普通窗口): ";
    string wtype; getline(cin, wtype);
    int window_id = (wtype == "1") ? 0 : 1;

    QueueTicket t;
    bool has_ticket = false;

    if (window_id == 0) {
        // VIP窗口: 先处理VIP队列, VIP队列空时处理普通队列
        if (!g_vip_queue.empty()) {
            t = g_vip_queue.peek(); g_vip_queue.pop();
            has_ticket = true;
        } else if (!g_normal_queue.empty()) {
            t = g_normal_queue.peek(); g_normal_queue.pop();
            has_ticket = true;
        }
    } else {
        // 普通窗口: 先看VIP队列有没有人(跨窗口服务), 再处理普通队列
        if (!g_vip_queue.empty()) {
            t = g_vip_queue.peek(); g_vip_queue.pop();
            has_ticket = true;
        } else if (!g_normal_queue.empty()) {
            t = g_normal_queue.peek(); g_normal_queue.pop();
            has_ticket = true;
        }
    }

    if (!has_ticket) {
        cout << "  当前无等待客户。\n"; return;
    }

    t.window_id = window_id;
    t.start_time = now_str();
    t.status = "办理中";
    cout << "  ▶ 请 " << t.customer_name << " (" << t.customer_type
         << ") 到窗口办理 [" << t.service_type << "] 票号: " << t.id << endl;

    // 模拟办理完成
    cout << "  业务是否完成? (y/n): "; string done; getline(cin, done);
    if (done == "y" || done == "Y") {
        t.end_time = now_str();
        t.status = "已完成";
        cout << "  请客户评分 (1-5): "; string rating; getline(cin, rating);
        t.rating = atoi(rating.c_str());
        if (t.rating < 1) t.rating = 5;
        if (t.rating > 5) t.rating = 5;
        cout << "  ✓ 业务完成! 评分: " << t.rating << " 分\n";
    } else {
        // 重新入队
        t.status = "等待中";
        if (t.customer_type == "VIP") g_vip_queue.push(t);
        else g_normal_queue.push(t);
        cout << "  已重新入队等待。\n";
        return;
    }

    g_queue_history.push_back(t);
    save_queue_history();

    // 更新每日统计
    string today = today_str();
    bool stats_updated = false;
    for (size_t i = 0; i < g_daily_stats.size(); i++) {
        if (g_daily_stats[i].date == today) {
            DailyStats& ds = g_daily_stats[i];
            ds.completed++;
            double total_rating = ds.avg_rating * (ds.completed - 1) + t.rating;
            ds.avg_rating = total_rating / ds.completed;
            stats_updated = true;
            break;
        }
    }
    if (!stats_updated) {
        DailyStats ds;
        ds.date = today;
        ds.total_customers = 1;
        if (t.customer_type == "VIP") ds.vip_customers = 1;
        else ds.normal_customers = 1;
        ds.avg_rating = t.rating;
        ds.completed = 1;
        g_daily_stats.push_back(ds);
    }
    save_daily_stats();
}

/** 显示排队状态 */
inline void show_queue_status() {
    cls();
    cout << "\n  ═══════════════ 排队状态 ═══════════════\n\n";

    cout << "  ┌─ VIP窗口 ─────────────────────────┐\n";
    cout << "  │  当前排队人数: " << setw(4) << g_vip_queue.size() << "                  │\n";
    cout << "  │  " << left << setw(6) << "票号" << setw(10) << "姓名"
         << setw(6) << "类型" << setw(12) << "业务" << setw(20) << "到达时间" << "│\n";
    cout << "  │────────────────────────────────────│\n";
    // 遍历队列显示
    LinkedQueue<QueueTicket> tmp_vip;
    while (!g_vip_queue.empty()) {
        QueueTicket t = g_vip_queue.peek(); g_vip_queue.pop();
        cout << "  │  " << left << setw(6) << t.id << setw(10) << t.customer_name
             << setw(6) << t.customer_type << setw(12) << t.service_type
             << setw(20) << t.arrive_time << "│\n";
        tmp_vip.push(t);
    }
    while (!tmp_vip.empty()) { g_vip_queue.push(tmp_vip.peek()); tmp_vip.pop(); }
    cout << "  └────────────────────────────────────┘\n\n";

    cout << "  ┌─ 普通窗口 ─────────────────────────┐\n";
    cout << "  │  当前排队人数: " << setw(4) << g_normal_queue.size() << "                  │\n";
    cout << "  │  " << left << setw(6) << "票号" << setw(10) << "姓名"
         << setw(6) << "类型" << setw(12) << "业务" << setw(20) << "到达时间" << "│\n";
    cout << "  │────────────────────────────────────│\n";
    LinkedQueue<QueueTicket> tmp_norm;
    while (!g_normal_queue.empty()) {
        QueueTicket t = g_normal_queue.peek(); g_normal_queue.pop();
        cout << "  │  " << left << setw(6) << t.id << setw(10) << t.customer_name
             << setw(6) << t.customer_type << setw(12) << t.service_type
             << setw(20) << t.arrive_time << "│\n";
        tmp_norm.push(t);
    }
    while (!tmp_norm.empty()) { g_normal_queue.push(tmp_norm.peek()); tmp_norm.pop(); }
    cout << "  └────────────────────────────────────┘\n";
}

/** 每日客流统计 */
inline void show_daily_stats() {
    hr();
    cout << "  ═══ 每日客流统计 ═══\n";
    hr();
    cout << "  " << left << setw(12) << "日期" << setw(10) << "总人数"
         << setw(10) << "VIP" << setw(10) << "普通"
         << setw(14) << "平均评分" << setw(10) << "完成数" << endl;
    hr();
    for (size_t i = 0; i < g_daily_stats.size(); i++) {
        const DailyStats& ds = g_daily_stats[i];
        cout << "  " << left << setw(12) << ds.date << setw(10) << ds.total_customers
             << setw(10) << ds.vip_customers << setw(10) << ds.normal_customers
             << setw(14) << double_to_str(ds.avg_rating)
             << setw(10) << ds.completed << endl;
    }
    if (g_daily_stats.empty()) cout << "  暂无统计数据\n";
    hr();
}

inline void queue_menu() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║   需求6: 银行排队管理            ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 客户取号 (VIP/普通)          ║\n";
        cout << "  ║  2. 叫号办理 (VIP优先)           ║\n";
        cout << "  ║  3. 查看排队状态                ║\n";
        cout << "  ║  4. 每日客流统计                ║\n";
        cout << "  ║  5. 历史排队记录                ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { take_ticket(); pause(); }
        else if (ch == "2") { call_ticket(); pause(); }
        else if (ch == "3") { show_queue_status(); pause(); }
        else if (ch == "4") { show_daily_stats(); pause(); }
        else if (ch == "5") {
            hr();
            for (size_t i = 0; i < g_queue_history.size(); i++) {
                const QueueTicket& t = g_queue_history[i];
                cout << "  " << t.id << " | " << t.customer_name
                     << " | " << t.service_type << " | " << t.status
                     << " | 评分:" << t.rating << endl;
            }
            hr(); pause();
        }
        else if (ch == "0") break;
    }
}

#endif
