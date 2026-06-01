/**
 * branch_manager.h — 需求7: 银行网点查询
 *
 * 功能: 网点地图, 网点信息查询, 最短路径导航(Dijkstra), 网点增删改
 * 数据结构核心:
 *   - 图(Graph): 邻接矩阵存储
 *   - Dijkstra算法: 单源最短路径 O(V²)
 *   - 路径重构: 前驱数组回溯
 */

#ifndef BRANCH_MANAGER_H
#define BRANCH_MANAGER_H

#include "common.h"

extern vector<Branch> g_branches;
extern vector<vector<double> > g_branch_graph; // 邻接矩阵 -1=无边
extern map<string, int> g_branch_index;

inline void load_branches() {
    g_branches.clear();
    g_branch_index.clear();

    ifstream f(BRANCH_FILE.c_str());
    if (!f.is_open()) {
        // 创建默认网点
        Branch b1; b1.id="BR01"; b1.name="总行营业部"; b1.address="北京市西城区金融街1号";
        b1.phone="010-88880001"; b1.hours="09:00-17:30";
        b1.services="全部业务"; b1.x=0; b1.y=0;
        g_branches.push_back(b1); g_branch_index["BR01"]=0;

        Branch b2; b2.id="BR02"; b2.name="朝阳支行"; b2.address="北京市朝阳区建国路100号";
        b2.phone="010-88880002"; b2.hours="09:00-17:00";
        b2.services="存取贷/理财"; b2.x=8.5; b2.y=3.2;
        g_branches.push_back(b2); g_branch_index["BR02"]=1;

        Branch b3; b3.id="BR03"; b3.name="海淀支行"; b3.address="北京市海淀区中关村大街50号";
        b3.phone="010-88880003"; b3.hours="09:00-17:00";
        b3.services="存取贷/外汇"; b3.x=12.0; b3.y=-2.5;
        g_branches.push_back(b3); g_branch_index["BR03"]=2;

        Branch b4; b4.id="BR04"; b4.name="西城支行"; b4.address="北京市西城区西单北大街80号";
        b4.phone="010-88880004"; b4.hours="09:00-17:00";
        b4.services="存取贷/保险"; b4.x=3.2; b4.y=-4.8;
        g_branches.push_back(b4); g_branch_index["BR04"]=3;

        Branch b5; b5.id="BR05"; b5.name="东城支行"; b5.address="北京市东城区王府井大街200号";
        b5.phone="010-88880005"; b5.hours="09:00-17:30";
        b5.services="存取贷/VIP理财"; b5.x=5.0; b5.y=6.0;
        g_branches.push_back(b5); g_branch_index["BR05"]=4;

        // 构建邻接矩阵
        int N = g_branches.size();
        g_branch_graph.assign(N, vector<double>(N, -1));
        for (int i = 0; i < N; i++) g_branch_graph[i][i] = 0;
        // 手动定义路径距离(km)
        g_branch_graph[0][1] = g_branch_graph[1][0] = 8.5;
        g_branch_graph[0][2] = g_branch_graph[2][0] = 12.0;
        g_branch_graph[0][3] = g_branch_graph[3][0] = 5.5;
        g_branch_graph[0][4] = g_branch_graph[4][0] = 7.0;
        g_branch_graph[1][2] = g_branch_graph[2][1] = 9.0;
        g_branch_graph[1][4] = g_branch_graph[4][1] = 6.0;
        g_branch_graph[2][3] = g_branch_graph[3][2] = 15.0;
        g_branch_graph[3][4] = g_branch_graph[4][3] = 11.0;
        return;
    }
    string line;
    while (getline(f, line)) {
        if (line.empty()) continue;
        vector<string> p = split(line, '|');
        if (p.size() < 8) continue;
        Branch b;
        b.id=p[0]; b.name=p[1]; b.address=p[2];
        b.phone=p[3]; b.hours=p[4]; b.services=p[5];
        b.x=safe_double(p[6]); b.y=safe_double(p[7]);
        g_branch_index[b.id] = g_branches.size();
        g_branches.push_back(b);
    }
    f.close();

    // 加载邻接矩阵
    ifstream gf(GRAPH_FILE.c_str());
    if (gf.is_open()) {
        int N = g_branches.size();
        g_branch_graph.assign(N, vector<double>(N, -1));
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                double d; gf >> d;
                g_branch_graph[i][j] = d;
            }
        }
        gf.close();
    }
}

inline void save_branches() {
    ofstream f(BRANCH_FILE.c_str());
    if (!f.is_open()) return;
    for (size_t i = 0; i < g_branches.size(); i++) {
        const Branch& b = g_branches[i];
        f << b.id << "|" << b.name << "|" << b.address << "|"
          << b.phone << "|" << b.hours << "|" << b.services << "|"
          << double_to_str(b.x) << "|" << double_to_str(b.y) << endl;
    }
    f.close();
    // 保存图
    ofstream gf(GRAPH_FILE.c_str());
    if (!gf.is_open()) return;
    int N = g_branches.size();
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            gf << g_branch_graph[i][j];
            if (j < N-1) gf << " ";
        }
        gf << endl;
    }
    gf.close();
}

// ==================== 网点CRUD ====================

inline void list_branches() {
    hr();
    cout << "  " << left << setw(8) << "编号" << setw(14) << "名称"
         << setw(26) << "地址" << setw(14) << "电话"
         << setw(14) << "营业时间" << setw(20) << "服务" << endl;
    hr();
    for (size_t i = 0; i < g_branches.size(); i++) {
        const Branch& b = g_branches[i];
        cout << "  " << left << setw(8) << b.id << setw(14) << b.name
             << setw(26) << b.address << setw(14) << b.phone
             << setw(14) << b.hours << setw(20) << b.services << endl;
    }
    hr();
}

inline void add_branch() {
    Branch b;
    cout << "\n  ═══ 添加网点 ═══\n";
    cout << "  网点编号: "; getline(cin, b.id);
    cout << "  名称: "; getline(cin, b.name);
    cout << "  地址: "; getline(cin, b.address);
    cout << "  电话: "; getline(cin, b.phone);
    cout << "  营业时间: "; getline(cin, b.hours);
    cout << "  服务项目: "; getline(cin, b.services);
    cout << "  坐标X: "; string sx; getline(cin, sx); b.x = safe_double(sx);
    cout << "  坐标Y: "; string sy; getline(cin, sy); b.y = safe_double(sy);

    int N = g_branches.size();
    g_branch_index[b.id] = N;
    g_branches.push_back(b);

    // 扩展邻接矩阵
    vector<vector<double> > new_graph(N+1, vector<double>(N+1, -1));
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            new_graph[i][j] = g_branch_graph[i][j];
    for (int i = 0; i <= N; i++) new_graph[i][i] = 0;
    g_branch_graph = new_graph;

    save_branches();
    cout << "  ✓ 网点 " << b.name << " 添加成功! (共" << g_branches.size() << "个网点)\n";
    cout << "  提示: 需要手动设置该网点与其他网点的路径距离。\n";
}

inline void delete_branch() {
    cout << "  网点编号: "; string id; getline(cin, id);
    map<string,int>::iterator it = g_branch_index.find(id);
    if (it == g_branch_index.end()) { cout << "  ✗ 网点不存在!\n"; return; }
    // 删除该行该列
    int idx = it->second;
    g_branches.erase(g_branches.begin() + idx);
    int N = g_branches.size();
    vector<vector<double> > new_graph(N, vector<double>(N, -1));
    for (int i = 0, ni = 0; i < (int)g_branch_graph.size(); i++) {
        if (i == idx) continue;
        for (int j = 0, nj = 0; j < (int)g_branch_graph.size(); j++) {
            if (j == idx) continue;
            new_graph[ni][nj] = g_branch_graph[i][j];
            nj++;
        }
        ni++;
    }
    g_branch_graph = new_graph;
    g_branch_index.clear();
    for (int i = 0; i < N; i++) g_branch_index[g_branches[i].id] = i;
    save_branches();
    cout << "  ✓ 网点已删除\n";
}

inline void modify_branch() {
    cout << "  网点编号: "; string id; getline(cin, id);
    map<string,int>::iterator it = g_branch_index.find(id);
    if (it == g_branch_index.end()) { cout << "  ✗ 网点不存在!\n"; return; }
    Branch& b = g_branches[it->second];
    cout << "  当前名称 [" << b.name << "]: "; string s; getline(cin, s);
    if (!s.empty()) b.name = s;
    cout << "  当前地址 [" << b.address << "]: "; getline(cin, s);
    if (!s.empty()) b.address = s;
    cout << "  当前电话 [" << b.phone << "]: "; getline(cin, s);
    if (!s.empty()) b.phone = s;
    cout << "  当前营业时间 [" << b.hours << "]: "; getline(cin, s);
    if (!s.empty()) b.hours = s;
    cout << "  当前服务 [" << b.services << "]: "; getline(cin, s);
    if (!s.empty()) b.services = s;
    save_branches();
    cout << "  ✓ 网点信息已更新!\n";
}

/** 设置/修改路径距离 */
inline void set_branch_path() {
    cout << "\n  ═══ 设置网点间路径 ═══\n";
    list_branches();
    cout << "  起始网点编号: "; string a; getline(cin, a);
    cout << "  目标网点编号: "; string b; getline(cin, b);
    map<string,int>::iterator ia = g_branch_index.find(a);
    map<string,int>::iterator ib = g_branch_index.find(b);
    if (ia == g_branch_index.end() || ib == g_branch_index.end()) {
        cout << "  ✗ 网点不存在!\n"; return;
    }
    cout << "  距离(km): "; string ds; getline(cin, ds);
    double dist = safe_double(ds);
    g_branch_graph[ia->second][ib->second] = dist;
    g_branch_graph[ib->second][ia->second] = dist;
    save_branches();
    cout << "  ✓ 路径距离已设置!\n";
}

/** 最短路径导航 (Dijkstra算法) */
inline void find_shortest_path() {
    cout << "\n  ═══ 最短路径导航 (Dijkstra算法) ═══\n";
    list_branches();
    cout << "  起始网点编号: "; string src_id; getline(cin, src_id);
    cout << "  目标网点编号: "; string dst_id; getline(cin, dst_id);
    map<string,int>::iterator isrc = g_branch_index.find(src_id);
    map<string,int>::iterator idst = g_branch_index.find(dst_id);
    if (isrc == g_branch_index.end() || idst == g_branch_index.end()) {
        cout << "  ✗ 网点不存在!\n"; return;
    }
    int src = isrc->second, dst = idst->second;
    int N = g_branches.size();

    vector<double> dist;
    vector<int> prev;
    dijkstra(g_branch_graph, N, src, dist, prev);

    if (dist[dst] >= 1e17) {
        cout << "  ✗ 两点之间无可达路径!\n"; return;
    }

    // 重构路径
    vector<int> path;
    for (int v = dst; v != -1; v = prev[v])
        path.push_back(v);

    cout << "\n  ═══ 路径结果 ═══\n";
    cout << "  总距离: " << double_to_str(dist[dst]) << " km\n";
    cout << "  路径: ";
    for (int i = path.size()-1; i >= 0; i--) {
        cout << g_branches[path[i]].name;
        if (i > 0) cout << " → ";
    }
    cout << "\n\n  详细路线:\n";
    hr();
    for (int i = path.size()-1; i >= 0; i--) {
        const Branch& br = g_branches[path[i]];
        cout << "  " << (path.size()-1-i+1) << ". " << br.name
             << " (" << br.address << ")";
        if (i > 0) {
            int from = path[i], to = path[i-1];
            cout << "\n     ↓ " << double_to_str(g_branch_graph[from][to]) << " km ↓";
        }
        cout << endl;
    }
    hr();
}

/** 查询所有可达网点 */
inline void query_reachable() {
    cout << "  起始网点编号: "; string src_id; getline(cin, src_id);
    map<string,int>::iterator isrc = g_branch_index.find(src_id);
    if (isrc == g_branch_index.end()) { cout << "  ✗ 网点不存在!\n"; return; }
    int src = isrc->second, N = g_branches.size();
    vector<double> dist;
    vector<int> prev;
    dijkstra(g_branch_graph, N, src, dist, prev);

    hr();
    cout << "  从 " << g_branches[src].name << " 出发:\n";
    hr();
    cout << "  " << left << setw(14) << "目的地" << setw(12) << "最短距离(km)" << endl;
    hr();
    for (int i = 0; i < N; i++) {
        if (i == src) continue;
        if (dist[i] < 1e17)
            cout << "  " << left << setw(14) << g_branches[i].name
                 << setw(12) << double_to_str(dist[i]) << endl;
        else
            cout << "  " << left << setw(14) << g_branches[i].name << "不可达" << endl;
    }
    hr();
}

inline void branch_menu() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║   需求7: 银行网点查询            ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  1. 查看所有网点                ║\n";
        cout << "  ║  2. 添加网点                    ║\n";
        cout << "  ║  3. 删除网点                    ║\n";
        cout << "  ║  4. 修改网点信息                ║\n";
        cout << "  ║  5. 设置网点间路径距离          ║\n";
        cout << "  ║  6. 最短路径导航 (Dijkstra)     ║\n";
        cout << "  ║  7. 查询所有可达网点            ║\n";
        cout << "  ║  0. 返回上级菜单                ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);
        if (ch == "1") { list_branches(); pause(); }
        else if (ch == "2") { add_branch(); pause(); }
        else if (ch == "3") { delete_branch(); pause(); }
        else if (ch == "4") { modify_branch(); pause(); }
        else if (ch == "5") { set_branch_path(); pause(); }
        else if (ch == "6") { find_shortest_path(); pause(); }
        else if (ch == "7") { query_reachable(); pause(); }
        else if (ch == "0") break;
    }
}

#endif
