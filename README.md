# 🏦 银行智能管理系统 Bank Intelligent Management System

## 数据结构与算法课程设计 · 计科2402班 第十组 · v3.3

[![Deploy to Render](https://render.com/images/deploy-to-render-button.svg)](https://render.com/deploy?repo=https://github.com/EraEve/BankManagementSystem)

> 🌐 **在线体验**: [eraeve.github.io/DawnGuard](https://eraeve.github.io/DawnGuard) (前端) + [eraeve.pythonanywhere.com](https://eraeve.pythonanywhere.com) (API)

---

## 📋 项目概述

本项目是《数据结构与算法》课程设计的完整Web实现，将C++核心算法移植到Python Flask后端，结合Bootstrap 5 SPA前端，构建了一个功能完备的银行智能管理系统。

### 版本演进

| 版本 | 日期 | 内容 |
|------|------|------|
| **v1.0** | 2025-05 | C++ 控制台核心业务（登录/存取款/转账） |
| **v2.0** | 2025-06 | 完整10大模块，多文件C++架构 |
| **v2.1** | 2026-06 | Flask API + 前端, 50人花名册, 一键部署 |
| **v3.0** | 2026-06 | Web全功能版, 10模块全覆盖, Chart.js仪表盘 |
| **v3.1** | 2026-06 | CORS跨域修复, Session持久化, 安全性加固 |
| **v3.2** | 2026-06 | 动态图表, sdufe学生银行卡, 余额同步 |
| **v3.3** | 2026-06 | DeepSeek AI投资顾问, 个人信息/银行卡合并, 关于页伪代码+20参考文献 |

---

## 🏗️ 系统架构

```
┌──────────────────────────────────────────────────────────┐
│                   【表现层】 Presentation Layer            │
│  Bootstrap 5 + Vanilla JS SPA    Chart.js 可视化           │
│  AJAX (fetch) ←→ JSON RESTful API                        │
├──────────────────────────────────────────────────────────┤
│                  【业务逻辑层】 Business Logic Layer        │
│  Flask (Python)  │  10大模块  │  C++ 核心算法              │
│  ┌──────────┬──────────┬──────────┬──────────┐          │
│  │ 员工管理 │ 客户管理 │ 卡管理   │ 交易处理 │          │
│  ├──────────┼──────────┼──────────┼──────────┤          │
│  │ 业务查询 │ 排队管理 │ 网点导航 │ 智能管理 │          │
│  ├──────────┴──────────┴──────────┴──────────┤          │
│  │      身份核验        │  登录认证 + AI顾问  │          │
│  └──────────────────────┴─────────────────────┘          │
├──────────────────────────────────────────────────────────┤
│                  【数据访问层】 Data Access Layer           │
│  管道分隔符文件 (|)    │   线程安全锁    │   原子写入       │
│  account.txt  employee.txt  customer.txt  card.txt        │
│  transaction.txt  branch.txt  queue.txt                  │
└──────────────────────────────────────────────────────────┘
```

### 文件结构

```
BankManagementSystem_clone/
├── app.py                    # Flask 后端 (2200+ 行, 51个API路由)
├── index.html                # SPA 前端 (1200+ 行, 单文件)
├── requirements.txt          # Python 依赖
│
├── common.h                  # 通用数据结构 + 工具函数
├── employee_manager.h        # [模块1] 银行职员管理
├── customer_manager.h        # [模块2] 客户账户管理
├── card_manager.h            # [模块3] 银行卡管理
├── transaction_manager.h     # [模块4] 存取贷业务管理
├── query_manager.h           # [模块5] 业务查询
├── queue_manager.h           # [模块6] 银行排队管理
├── branch_manager.h          # [模块7] 网点查询与导航
├── smart_manager.h           # [模块8] 智能管理
├── identity_verifier.h       # [模块9] 身份核验与生物识别
├── main.cpp                  # C++ 主入口
│
├── data/                     # 数据文件 (管道分隔符)
├── Makefile                  # C++ 编译脚本
└── README.md                 # 本文件
```

---

## 🎯 10大功能模块

### 模块1: 银行职员管理 ✅
- 管理员增删改查职员信息
- 职员修改登录密码和个人信息
- 字段: 编号/姓名/密码/角色/部门/电话/邮箱/入职日期/状态
- 数据结构: `vector<Employee>` + 哈希索引

### 模块2: 客户账户管理 ✅
- 职员增删改查客户资料（VIP/普通分类）
- 广义表: 一个客户关联多张银行卡
- 字段: 编号/姓名/密码/类型/电话/开户日期/信用评分/金融资产/地址/身份证
- 数据结构: `vector<Customer>` + BST 信用分索引

### 模块3: 银行卡管理 ✅
- 职员增删改查银行卡，支持借记卡/储蓄卡/信用卡
- 自动为学生生成 sdufe+学号+1 格式的储蓄卡
- 字段: 卡号/客户ID/类型/余额/借贷余额/利率/信用额度/日限额/状态
- 数据结构: `vector<BankCard>` + 哈希映射

### 模块4: 存取贷业务管理 ✅
- 存款/取款/转账/贷款/还款 完整业务流程
- 余额校验/限额校验/状态校验
- 自动同步 account.txt ↔ card.txt 余额
- 数据结构: `vector<Transaction>`

### 模块5: 业务查询 ✅
- 8维度组合查询：类型/日期/金额/卡号/客户类型/排序
- 汇总统计：各类型交易笔数+金额
- 排序：快速排序(金额) + 冒泡排序(时间)

### 模块6: 银行排队管理 ✅
- VIP窗口 + 普通窗口 双队列
- 客户取号 → VIP优先叫号 → 业务评分(1-5)
- 排队历史 + 每日客流统计
- 持久化队列状态（重启不丢失）
- 数据结构核心: **LinkedQueue 链式队列**

### 模块7: 网点查询与导航 ✅
- 5个内置网点 + Canvas地图可视化
- 网点增删改查 + 路径距离设置
- **Dijkstra 最短路径导航**（含路径重构）
- 可达性分析
- 数据结构核心: **图(邻接矩阵) + Dijkstra**

### 模块8: 智能管理 ✅
1. **异常检测**: 大额交易(>5万) / 高频交易(滑动窗口, 1h>5笔)
2. **利息计算**: 全行日利息/月利息汇总
3. **风控审批**: 加权多因子模型 (债务率 + 信用评分 + 金融资产)
4. **投资顾问**: 规则引擎 (进取型/稳健型/保守型) + **DeepSeek AI 智能分析**
5. **客户统计**: 信用评级分布 / 资产分析

### 模块9: 身份核验与生物识别 ✅
- **ISO 7064:1983 MOD 11-2** 身份证校验
- 省份/出生日期/性别/年龄自动提取
- 生物特征模板生成与比对（模拟）
- 全行客户身份证批量核验

### 模块10: 统一登录与权限 ✅
- 三角色: 管理员/职员/客户 (差异化菜单)
- Session 鉴权 + @login_required 装饰器
- PBKDF2-SHA256 密码哈希 (明文自动迁移)
- 50人花名册学生账号 + 管理员账号

---

## 📊 数据结构与算法

| 数据结构/算法 | C++ 实现 | Python 等价 | 复杂度 | 应用 |
|-------------|----------|------------|--------|------|
| **动态数组** | `std::vector<T>` | `list` | O(1) 访问 | 全部模块数据存储 |
| **哈希映射** | `std::unordered_map<K,V>` | `dict` | O(1) 查找 | 客户-卡关联索引 |
| **链式队列** | `std::queue<T>` | `list` (FIFO) | O(1) 入/出队 | 排队叫号系统 |
| **图·邻接矩阵** | `graph[N][N]` | `list[list[int]]` | O(1) 查边 | 网点拓扑 |
| **BST** | `struct Node{K; *L,*R;}` | sortedcontainers | O(log n) | 信用分索引 |
| **Dijkstra** | 贪心+松弛 | 贪心+松弛 | O(n²) | 最短路径导航 |
| **ISO 7064** | MOD 11-2 | MOD 11-2 | O(1) | 身份证校验 |
| **滑动窗口** | 同小时窗口计数 | 同小时窗口计数 | O(n log n) | 高频交易检测 |
| **加权多因子** | 线性加权 | 线性加权 | O(1) | 风控审批 |
| **快速排序** | 分治+partition | Timsort | O(n log n) | 交易排序 |
| **二分查找** | 减治 | `bisect` | O(log n) | 有序检索 |

---

## 🌐 Web 技术栈

| 层级 | 技术 | 说明 |
|------|------|------|
| **后端** | Flask 3.1 + gunicorn | 51个 RESTful API 端点 |
| **前端** | Bootstrap 5.3 + Vanilla JS | SPA 单页应用，零框架依赖 |
| **图表** | Chart.js 4.4 | 柱状图/环形图/投资配置图 |
| **安全** | PBKDF2-SHA256 + Session + CORS | 6层安全防护 |
| **AI** | DeepSeek API (deepseek-chat) | 智能投资顾问 |
| **部署** | GitHub Pages + PythonAnywhere | 前后端分离部署 |
| **数据** | 管道分隔符 .txt 文件 | 与 C++ 格式完全兼容 |

---

## 🔧 本地运行

### 环境要求
- Python 3.9+
- (可选) C++11 编译器 (GCC/MinGW) — 仅C++核心模块需要

### 快速启动

```bash
# 1. 进入项目目录
cd BankManagementSystem_clone

# 2. 安装依赖
pip install -r requirements.txt

# 3. 启动服务
python app.py
```

浏览器自动打开 `http://127.0.0.1:5000`

### C++ 编译 (可选)

```bash
# 编译 C++ 核心模块
make
# 或
g++ main.cpp -o bank_system -std=c++11
```

---

## 🔑 测试账号

### Web 端

| 角色 | 账号 | 密码 | 说明 |
|------|------|------|------|
| 管理员 | E001 | 000000 | Admin, 全部权限 |
| 职员 | E002 | 111111 | ZhangWei, 营业部 |
| 普通客户 | C000001 | 123456 | ZhangSan, 金融资产5万 |
| VIP客户 | C000002 | 123456 | LiSi, 金融资产50万 |
| 普通客户 | C000003 | 123456 | WangWu, 金融资产3万 |
| VIP客户 | C000004 | 123456 | VIP_Zhao, 金融资产120万 |
| **学员** | **学号** | **学号后4位** | 50名计科2402班学生 |
| 管理员 | 202418440218 | 0218 | 谷元杰 |
| 管理员 | 202418440219 | 0219 | 李延豪 |
| 管理员 | 202418440221 | 0221 | 于名昊 |

### C++ 端 (控制台)

| 角色 | 编号 | 密码 |
|------|------|------|
| 管理员 | E001 | 000000 |
| 职员 | E002 | 111111 |
| 客户 | C000001~C000004 | 123456 |

---

## 🤖 DeepSeek AI 投资顾问

v3.3 集成了 DeepSeek API 提供智能投资分析：

1. 设置环境变量 `DEEPSEEK_API_KEY` 或在 PythonAnywhere WSGI 文件中配置
2. 登录后 → 投资顾问 → 点击 **"AI 智能分析"**
3. DeepSeek 根据客户财务状况生成个性化投资策略报告
4. 如未配置 API Key，自动降级为规则引擎（含详细的5段式建议）

---

## 📁 数据文件格式

| 文件 | 分隔符 | 字段 |
|------|--------|------|
| `account.txt` | \| | 学号\|姓名\|密码\|余额\|锁定\|管理员 |
| `employee.txt` | \| | 编号\|姓名\|密码\|角色\|部门\|电话\|邮箱\|入职\|在职 |
| `customer.txt` | \| | 编号\|姓名\|密码\|类型\|电话\|开户日\|信用分\|资产\|地址\|身份证\|激活 |
| `card.txt` | \| | 卡号\|客户ID\|类型\|余额\|借贷\|利率\|额度\|开户日\|状态\|日限额 |
| `transaction.txt` | \| | ID\|转出卡\|转入卡\|类型\|金额\|时间\|状态\|经办\|备注 |
| `branch.txt` | \| | 编号\|名称\|地址\|电话\|营业时间\|服务\|X\|Y |
| `queue.txt` | \| | 票号\|客户ID\|姓名\|类型\|窗口\|业务\|到达\|开始\|结束\|评分\|状态 |

---

## 🚀 部署

- **GitHub Pages**: 静态 SPA 前端 → [eraeve.github.io/DawnGuard](https://eraeve.github.io/DawnGuard)
- **PythonAnywhere**: Flask API 后端 → [eraeve.pythonanywhere.com](https://eraeve.pythonanywhere.com)
- **Render**: 一键部署 (见顶部按钮)
- **本地**: `python app.py` → http://127.0.0.1:5000

### 更新 PythonAnywhere

```bash
# SSH 到 PythonAnywhere 后:
cd ~/BankManagementSystem
git pull origin master
# 然后在 Web 页面点击 Reload
```

---

## 👥 第十组 · 计科2402班

- 数据结构与算法课程设计
- 银行智能管理系统 v3.3
- 10大模块完整Web实现
- C++ 核心算法 + Python Flask API + Bootstrap SPA 前端

---

## 📄 License

MIT License — 仅供学习交流使用
