"""
Bank Management System v3.0 — Full-Stack Web Application
========================================================
Covers ALL 10 C++ modules:
  1. Employee Management (CRUD + auth)
  2. Customer Account Management (CRUD + profile)
  3. Bank Card Management (CRUD + multi-card per customer)
  4. Deposit/Withdrawal/Transfer/Loan/Repay
  5. Business Query (filters, stats, sorting, combined query)
  6. Queue Management (LinkedQueue FIFO, VIP priority)
  7. Branch Navigation (Dijkstra shortest path)
  8. Smart Management (anomaly detection, interest, credit, risk, investment)
  9. Identity Verification (ISO 7064, biometric simulation, OCR)
 10. Login/Auth with role-based menus

Data Structures: vector, map/hash, LinkedQueue, BST, graph adjacency matrix
Algorithms: Dijkstra, bubble sort, quicksort, binary search, sliding window, ISO 7064
"""

import os
import json
import time
import math
import uuid
import secrets
import threading
import webbrowser
import calendar
from datetime import datetime
from functools import wraps

from flask import Flask, jsonify, request, send_from_directory, session
from flask_cors import CORS
from werkzeug.security import generate_password_hash, check_password_hash

app = Flask(__name__)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
DATA_DIR = os.path.join(BASE_DIR, "data")

# Persistent secret key: env var → file → auto-generate-and-save
_secret_file = os.path.join(BASE_DIR, ".flask_secret")
if os.environ.get("SECRET_KEY"):
    app.secret_key = os.environ["SECRET_KEY"]
elif os.path.exists(_secret_file):
    with open(_secret_file, "rb") as _sf:
        app.secret_key = _sf.read()
else:
    app.secret_key = secrets.token_hex(32)
    try:
        with open(_secret_file, "wb") as _sf:
            raw = app.secret_key.encode() if isinstance(app.secret_key, str) else app.secret_key
            _sf.write(raw)
    except (OSError, IOError):
        pass  # read-only filesystem (some cloud envs) — key lasts until restart

# Detect deployment environment
_is_render = bool(os.environ.get("RENDER"))
_is_pythonanywhere = bool(os.environ.get("PYTHONANYWHERE_SITE") or os.environ.get("PYTHONANYWHERE_DOMAIN"))
_is_production = _is_render or _is_pythonanywhere or os.environ.get("FLASK_ENV") == "production"

# Session cookie config — SameSite=None for cross-origin (GitHub Pages → Render)
app.config["SESSION_COOKIE_HTTPONLY"] = True
app.config["SESSION_COOKIE_SECURE"] = _is_production       # HTTPS only in prod
app.config["SESSION_COOKIE_SAMESITE"] = "None" if _is_production else "Lax"

# CORS: regex origins for cross-origin credentials (GitHub Pages, local dev, PythonAnywhere)
CORS(app, origins=[
    r"https://.*\.github\.io",
    r"https://.*\.pythonanywhere\.com",
    r"http://localhost:\d+",
    r"http://127\.0\.0\.1:\d+",
], supports_credentials=True)

# ============================================================
# CONSTANTS (matching C++ common.h)
# ============================================================
VIP_THRESHOLD = 100000.0
LARGE_AMOUNT = 50000.0
MULTI_TRANSACTION = 5
CREDIT_LIMIT_BASE = 50000.0

# ============================================================
# FILE PATHS
# ============================================================
ACCOUNT_FILE = os.path.join(DATA_DIR, "account.txt")
EMPLOYEE_FILE = os.path.join(DATA_DIR, "employee.txt")
CUSTOMER_FILE = os.path.join(DATA_DIR, "customer.txt")
CARD_FILE = os.path.join(DATA_DIR, "card.txt")
TRANSACTION_FILE = os.path.join(DATA_DIR, "transaction.txt")
QUEUE_FILE = os.path.join(DATA_DIR, "queue.txt")
QUEUE_STATE_FILE = os.path.join(DATA_DIR, "queue_state.txt")  # in-memory queue snapshot
BRANCH_FILE = os.path.join(DATA_DIR, "branch.txt")
GRAPH_FILE = os.path.join(DATA_DIR, "branch_graph.txt")
STATS_FILE = os.path.join(DATA_DIR, "daily_stats.txt")

# ============================================================
# THREAD-SAFE FILE LOCKS (prevents data corruption under gunicorn)
# ============================================================
_file_locks = {}
def _flock(filepath):
    """Get-or-create a per-file threading.Lock."""
    if filepath not in _file_locks:
        _file_locks[filepath] = threading.Lock()
    return _file_locks[filepath]

# ============================================================
# LOGIN-REQUIRED DECORATOR
# ============================================================
def login_required(f):
    """Decorator: reject requests without a valid session."""
    @wraps(f)
    def decorated(*args, **kwargs):
        if "user" not in session:
            return jsonify({"success": False, "message": "请先登录"}), 401
        return f(*args, **kwargs)
    return decorated

# ============================================================
# PASSWORD HELPERS (auto-migrate plaintext → hashed)
# ============================================================
def verify_password(plain, stored):
    """Check plaintext password against stored (plaintext or hashed).
    Returns (is_valid, needs_rehash)."""
    if stored.startswith("pbkdf2:sha256:") or stored.startswith("scrypt:"):
        return check_password_hash(stored, plain), False
    # Legacy plaintext — valid, but needs migration
    return (plain == stored), (plain == stored)

# ============================================================
# UTILITY FUNCTIONS
# ============================================================

def ensure_data_dir():
    if not os.path.exists(DATA_DIR):
        os.makedirs(DATA_DIR)

def safe_float(value, default=0.0):
    try:
        return float(value)
    except (ValueError, TypeError):
        return default

def safe_int(value, default=0):
    try:
        return int(value)
    except (ValueError, TypeError):
        return default

def now_str():
    return datetime.now().strftime("%Y-%m-%d %H:%M:%S")

def today_str():
    return datetime.now().strftime("%Y-%m-%d")

def gen_txn_id():
    return "T{}{:04d}".format(int(time.time() * 1000), uuid.uuid4().int % 10000)

def resolve_card_id(customer_id):
    """Find the sdufe card ID for a customer, or return customer_id as fallback."""
    for c in read_cards():
        if c["customer_id"] == customer_id:
            return c["id"]
    return customer_id

def _sync_card_balance(customer_id, new_balance):
    """Sync account.txt balance to linked card."""
    cards = read_cards()
    changed = False
    for c in cards:
        if c["customer_id"] == customer_id and c["status"] == "正常":
            c["balance"] = new_balance
            changed = True
            break
    if changed:
        write_cards(cards)

def read_lines(filepath):
    """Read pipe-delimited lines, skip empty (thread-safe)."""
    lines = []
    if os.path.exists(filepath):
        with _flock(filepath):
            with open(filepath, "r", encoding="utf-8") as f:
                for line in f:
                    line = line.strip()
                    if line:
                        lines.append(line)
    return lines

def write_lines(filepath, lines):
    """Write lines with newline (thread-safe, atomic via temp+rename)."""
    import tempfile
    dirpath = os.path.dirname(filepath)
    with _flock(filepath):
        fd, tmppath = tempfile.mkstemp(dir=dirpath, suffix=".tmp")
        try:
            with os.fdopen(fd, "w", encoding="utf-8") as f:
                for line in lines:
                    f.write(line + "\n")
            os.replace(tmppath, filepath)  # atomic on same filesystem
        except Exception:
            os.unlink(tmppath)
            raise

def append_line(filepath, line):
    """Append one line (thread-safe)."""
    with _flock(filepath):
        with open(filepath, "a", encoding="utf-8") as f:
            f.write(line + "\n")

# ============================================================
# DATA FILE INITIALIZATION (create defaults if missing)
# ============================================================

def init_employee_file():
    if not os.path.exists(EMPLOYEE_FILE):
        lines = [
            "E001|Admin|000000|admin|总行管理部|13800000000|admin@bank.com|2024-01-01|1",
            "E002|ZhangWei|111111|staff|营业部|13800000001|zhangwei@bank.com|2024-06-01|1"
        ]
        write_lines(EMPLOYEE_FILE, lines)

def init_customer_file():
    if not os.path.exists(CUSTOMER_FILE):
        lines = [
            "C000001|ZhangSan|123456|普通|13900000001|2024-03-15|720|50000|北京市朝阳区|110101199001011234|1|||",
            "C000002|LiSi|123456|VIP|13900000002|2024-01-20|850|500000|上海市浦东新区|310101199202022345|1|||",
            "C000003|WangWu|123456|普通|13900000003|2024-06-10|650|30000|广州市天河区|440101199303033456|1|||",
            "C000004|VIP_Zhao|123456|VIP|13900000004|2023-11-05|900|1200000|深圳市南山区|440301199404044567|1|||",
        ]
        write_lines(CUSTOMER_FILE, lines)

def init_card_file():
    if not os.path.exists(CARD_FILE):
        lines = [
            "B10000001|C000001|储蓄卡|50000|0|1.5|0|2024-03-15|正常|50000",
            "B10000002|C000002|信用卡|0|5000|18|200000|2024-01-20|正常|100000",
            "B10000003|C000003|借记卡|30000|0|0.35|0|2024-06-10|正常|20000",
            "B10000004|C000004|信用卡|0|0|15|500000|2023-11-05|正常|200000",
            "B10000005|C000004|储蓄卡|1200000|0|2|0|2023-11-05|正常|500000",
        ]
        write_lines(CARD_FILE, lines)

def sync_student_cards():
    """Auto-generate sdufe+studentID+1 cards for all account.txt students.
    Idempotent — never overwrites existing cards."""
    accounts = read_accounts()
    cards = read_cards()
    existing_card_ids = {c["id"] for c in cards}
    today = datetime.now().strftime("%Y-%m-%d")
    new_cards = []

    for acc in accounts:
        student_id = str(acc["id"]).strip()
        card_id = f"sdufe{student_id}1"
        if card_id in existing_card_ids:
            continue  # Already has a card
        # Determine card type and limit based on balance
        balance = acc.get("balance", 0)
        card_type = "储蓄卡"
        daily_limit = max(balance, 50000)
        new_cards.append(f"{card_id}|{student_id}|{card_type}|{balance}|0|1.5|0|{today}|正常|{daily_limit}")
        existing_card_ids.add(card_id)

    if new_cards:
        # Append new cards to existing file
        with open(CARD_FILE, "a", encoding="utf-8") as f:
            for line in new_cards:
                f.write(line + "\n")
        print(f"[sync_student_cards] Added {len(new_cards)} student cards (sdufe+学号+1)")
    return len(new_cards)

def init_branch_file():
    if not os.path.exists(BRANCH_FILE):
        lines = [
            "BR01|总行营业部|北京市西城区金融街1号|010-88880001|09:00-17:30|全部业务|0|0",
            "BR02|朝阳支行|北京市朝阳区建国路100号|010-88880002|09:00-17:00|存取贷/理财|8.5|3.2",
            "BR03|海淀支行|北京市海淀区中关村大街50号|010-88880003|09:00-17:00|存取贷/外汇|12|-2.5",
            "BR04|西城支行|北京市西城区西单北大街80号|010-88880004|09:00-17:00|存取贷/保险|3.2|-4.8",
            "BR05|东城支行|北京市东城区王府井大街200号|010-88880005|09:00-17:30|存取贷/VIP理财|5|6",
        ]
        write_lines(BRANCH_FILE, lines)

def init_branch_graph_file():
    if not os.path.exists(GRAPH_FILE):
        N = 5
        graph = [[-1]*N for _ in range(N)]
        for i in range(N):
            graph[i][i] = 0
        graph[0][1] = graph[1][0] = 8.5
        graph[0][2] = graph[2][0] = 12.0
        graph[0][3] = graph[3][0] = 5.5
        graph[0][4] = graph[4][0] = 7.0
        graph[1][2] = graph[2][1] = 9.0
        graph[1][4] = graph[4][1] = 6.0
        graph[2][3] = graph[3][2] = 15.0
        graph[3][4] = graph[4][3] = 11.0
        lines = [" ".join(str(v) for v in row) for row in graph]
        write_lines(GRAPH_FILE, lines)

def init_all_data():
    ensure_data_dir()
    init_employee_file()
    init_customer_file()
    init_card_file()
    init_branch_file()
    init_branch_graph_file()

# Initialize data at module level so gunicorn (and any WSGI server)
# picks it up. All init_* functions check os.path.exists first,
# so existing data files are never overwritten.
ensure_data_dir()
init_all_data()

# ============================================================
# DATA READ/WRITE HELPERS
# ============================================================

# --- Employees ---
def read_employees():
    employees = []
    for line in read_lines(EMPLOYEE_FILE):
        p = line.split("|")
        if len(p) >= 9:
            employees.append({
                "id": p[0], "name": p[1], "password": p[2],
                "role": p[3], "department": p[4], "phone": p[5],
                "email": p[6], "hire_date": p[7],
                "is_active": p[8] == "1" if len(p) > 8 else True
            })
    return employees

def write_employees(data):
    lines = []
    for e in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
            e["id"], e["name"], e["password"], e["role"],
            e["department"], e["phone"], e["email"], e["hire_date"],
            "1" if e.get("is_active", True) else "0"
        ))
    write_lines(EMPLOYEE_FILE, lines)

# --- Customers ---
def read_customers():
    customers = []
    for line in read_lines(CUSTOMER_FILE):
        p = line.split("|")
        if len(p) >= 11:
            customers.append({
                "id": p[0], "name": p[1], "password": p[2],
                "type": p[3], "phone": p[4], "open_date": p[5],
                "credit_score": safe_float(p[6], 600),
                "financial_assets": safe_float(p[7], 0),
                "address": p[8], "id_card": p[9],
                "is_active": p[10] == "1" if len(p) > 10 else True,
                "id_photo_path": p[11] if len(p) > 11 else "",
                "biometric_data": p[12] if len(p) > 12 else "",
                "face_photo_path": p[13] if len(p) > 13 else "",
            })
    return customers

def write_customers(data):
    lines = []
    for c in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
            c["id"], c["name"], c["password"], c["type"],
            c["phone"], c["open_date"],
            c.get("credit_score", 600), c.get("financial_assets", 0),
            c["address"], c.get("id_card", ""),
            "1" if c.get("is_active", True) else "0",
            c.get("id_photo_path", ""), c.get("biometric_data", ""),
            c.get("face_photo_path", "")
        ))
    write_lines(CUSTOMER_FILE, lines)

# --- Cards ---
def read_cards():
    cards = []
    for line in read_lines(CARD_FILE):
        p = line.split("|")
        if len(p) >= 10:
            cards.append({
                "id": p[0], "customer_id": p[1], "type": p[2],
                "balance": safe_float(p[3], 0),
                "loan_balance": safe_float(p[4], 0),
                "interest_rate": safe_float(p[5], 0.35),
                "credit_limit": safe_float(p[6], 0),
                "open_date": p[7], "status": p[8],
                "daily_limit": safe_float(p[9], 50000)
            })
    return cards

def write_cards(data):
    lines = []
    for c in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
            c["id"], c["customer_id"], c["type"],
            c.get("balance", 0), c.get("loan_balance", 0),
            c.get("interest_rate", 0.35), c.get("credit_limit", 0),
            c["open_date"], c.get("status", "正常"),
            c.get("daily_limit", 50000)
        ))
    write_lines(CARD_FILE, lines)

# --- Transactions ---
def read_transactions():
    transactions = []
    for line in read_lines(TRANSACTION_FILE):
        p = line.split("|")
        if len(p) >= 7:
            transactions.append({
                "id": p[0], "from_card": p[1], "to_card": p[2],
                "type": p[3], "amount": safe_float(p[4], 0),
                "time": p[5], "status": p[6],
                "employee_id": p[7] if len(p) > 7 else "",
                "remark": p[8] if len(p) > 8 else ""
            })
    return transactions

def write_transactions(data):
    lines = []
    for t in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
            t["id"], t.get("from_card", t.get("from","-")),
            t.get("to_card", t.get("to","-")),
            t["type"], t["amount"], t["time"], t.get("status", "成功"),
            t.get("employee_id", ""), t.get("remark", "")
        ))
    write_lines(TRANSACTION_FILE, lines)

def append_transaction(t):
    append_line(TRANSACTION_FILE, "{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
        t["id"], t.get("from_card", t.get("from","-")),
        t.get("to_card", t.get("to","-")),
        t["type"], t["amount"], t["time"], t.get("status", "成功"),
        t.get("employee_id", ""), t.get("remark", "")
    ))

# --- Branches ---
def read_branches():
    branches = []
    for line in read_lines(BRANCH_FILE):
        p = line.split("|")
        if len(p) >= 8:
            branches.append({
                "id": p[0], "name": p[1], "address": p[2],
                "phone": p[3], "hours": p[4], "services": p[5],
                "x": safe_float(p[6], 0), "y": safe_float(p[7], 0)
            })
    return branches

def write_branches(data):
    lines = []
    for b in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}".format(
            b["id"], b["name"], b["address"], b["phone"],
            b["hours"], b["services"], b.get("x", 0), b.get("y", 0)
        ))
    write_lines(BRANCH_FILE, lines)

def read_branch_graph():
    graph = []
    for line in read_lines(GRAPH_FILE):
        row = [safe_float(x, -1) for x in line.split()]
        graph.append(row)
    return graph

def write_branch_graph(graph):
    lines = [" ".join(str(v) for v in row) for row in graph]
    write_lines(GRAPH_FILE, lines)

# --- Queue ---
def read_queue_history():
    history = []
    for line in read_lines(QUEUE_FILE):
        p = line.split("|")
        if len(p) >= 11:
            history.append({
                "id": p[0], "customer_id": p[1], "customer_name": p[2],
                "customer_type": p[3], "window_id": safe_int(p[4], -1),
                "service_type": p[5], "arrive_time": p[6],
                "start_time": p[7], "end_time": p[8],
                "rating": safe_int(p[9], 0), "status": p[10],
                "priority": 10 if p[3] == "VIP" else 0
            })
    return history

def write_queue_history(data):
    lines = []
    for t in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
            t["id"], t["customer_id"], t["customer_name"],
            t["customer_type"], t.get("window_id", -1),
            t["service_type"], t["arrive_time"],
            t.get("start_time", ""), t.get("end_time", ""),
            t.get("rating", 0), t.get("status", "等待中")
        ))
    write_lines(QUEUE_FILE, lines)

def read_daily_stats():
    stats = []
    for line in read_lines(STATS_FILE):
        p = line.split("|")
        if len(p) >= 7:
            stats.append({
                "date": p[0], "total_customers": safe_int(p[1]),
                "vip_customers": safe_int(p[2]),
                "normal_customers": safe_int(p[3]),
                "avg_wait_time": safe_float(p[4]),
                "avg_rating": safe_float(p[5]),
                "completed": safe_int(p[6])
            })
    return stats

def write_daily_stats(data):
    lines = []
    for s in data:
        lines.append("{}|{}|{}|{}|{}|{}|{}".format(
            s["date"], s["total_customers"], s["vip_customers"],
            s["normal_customers"], s.get("avg_wait_time", 0),
            s.get("avg_rating", 0), s.get("completed", 0)
        ))
    write_lines(STATS_FILE, lines)

# ============================================================
# IN-MEMORY QUEUE STATE (with persistence for restart survival)
# ============================================================
vip_queue = []       # list simulating LinkedQueue FIFO
normal_queue = []
ticket_counter = 1000

def save_queue_state():
    """Save in-memory queue to disk so waiting tickets survive restart."""
    lines = []
    for t in vip_queue + normal_queue:
        lines.append("{}|{}|{}|{}|{}|{}|{}|{}|{}|{}|{}".format(
            t["id"], t["customer_id"], t["customer_name"],
            t["customer_type"], t.get("window_id", -1),
            t["service_type"], t["arrive_time"],
            t.get("start_time", ""), t.get("end_time", ""),
            t.get("rating", 0), t.get("status", "等待中")
        ))
    write_lines(QUEUE_STATE_FILE, lines)

def restore_queue_state():
    """Restore in-memory queue from disk on startup."""
    global vip_queue, normal_queue, ticket_counter
    for line in read_lines(QUEUE_STATE_FILE):
        p = line.split("|")
        if len(p) < 11:
            continue
        ticket = {
            "id": p[0], "customer_id": p[1], "customer_name": p[2],
            "customer_type": p[3], "window_id": safe_int(p[4], -1),
            "service_type": p[5], "arrive_time": p[6],
            "start_time": p[7], "end_time": p[8],
            "rating": safe_int(p[9], 0), "status": p[10],
            "priority": 10 if p[3] == "VIP" else 0
        }
        if ticket["customer_type"] == "VIP":
            vip_queue.append(ticket)
        else:
            normal_queue.append(ticket)
        # Keep ticket_counter above the max restored ticket ID
        try:
            num = int(p[0][1:])  # e.g. "Q1234" → 1234
            if num > ticket_counter:
                ticket_counter = num
        except ValueError:
            pass

# Restore queue on module load
restore_queue_state()

# ============================================================
# ALGORITHM IMPLEMENTATIONS
# ============================================================

def dijkstra(graph, N, src):
    """Dijkstra shortest path. graph[i][j] = distance, -1 = no edge."""
    INF = 1e18
    dist = [INF] * N
    prev = [-1] * N
    visited = [False] * N
    dist[src] = 0

    for _ in range(N):
        u = -1
        min_d = INF
        for j in range(N):
            if not visited[j] and dist[j] < min_d:
                min_d = dist[j]
                u = j
        if u == -1:
            break
        visited[u] = True
        for v in range(N):
            if not visited[v] and graph[u][v] >= 0:
                alt = dist[u] + graph[u][v]
                if alt < dist[v]:
                    dist[v] = alt
                    prev[v] = u
    return dist, prev

def validate_chinese_id_card(id_str):
    """ISO 7064:1983 MOD 11-2 validation for Chinese 18-digit ID card."""
    if len(id_str) != 18:
        return False, "身份证必须为18位"
    for i in range(17):
        if not id_str[i].isdigit():
            return False, f"第{i+1}位 '{id_str[i]}' 不是数字"
    last = id_str[17]
    if not (last.isdigit() or last.upper() == 'X'):
        return False, "第18位校验码无效"

    year = int(id_str[6:10])
    month = int(id_str[10:12])
    day = int(id_str[12:14])
    if year < 1900 or year > 2100:
        return False, "出生年份超出范围"
    if month < 1 or month > 12:
        return False, "出生月份无效"
    # Validate actual calendar days (e.g. Feb 30 is invalid)
    max_day = calendar.monthrange(year, month)[1]
    if day < 1 or day > max_day:
        return False, f"出生日期无效 ({year}年{month}月最多{max_day}天)"

    weights = [7, 9, 10, 5, 8, 4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2]
    check_map = ['1', '0', 'X', '9', '8', '7', '6', '5', '4', '3', '2']
    total = sum((ord(id_str[i]) - 48) * weights[i] for i in range(17))
    expected = check_map[total % 11]
    if last.upper() != expected:
        return False, f"校验位不匹配 (期望{expected})"
    return True, "身份证校验通过"

def extract_from_id_card(id_str):
    """Extract province, birth, gender, age from ID card."""
    province_codes = {
        "11": "北京市", "12": "天津市", "13": "河北省", "14": "山西省",
        "15": "内蒙古自治区", "21": "辽宁省", "22": "吉林省", "23": "黑龙江省",
        "31": "上海市", "32": "江苏省", "33": "浙江省", "34": "安徽省",
        "35": "福建省", "36": "江西省", "37": "山东省", "41": "河南省",
        "42": "湖北省", "43": "湖南省", "44": "广东省", "45": "广西壮族自治区",
        "46": "海南省", "50": "重庆市", "51": "四川省", "52": "贵州省",
        "53": "云南省", "54": "西藏自治区", "61": "陕西省", "62": "甘肃省",
        "63": "青海省", "64": "宁夏回族自治区", "65": "新疆维吾尔自治区",
        "71": "台湾省", "81": "香港特别行政区", "82": "澳门特别行政区"
    }
    province = province_codes.get(id_str[:2], "未知省份")
    birth = f"{id_str[6:10]}-{id_str[10:12]}-{id_str[12:14]}"
    gender = "男" if int(id_str[16]) % 2 == 1 else "女"

    now = datetime.now()
    age = now.year - int(id_str[6:10])
    if now.month < int(id_str[10:12]) or (now.month == int(id_str[10:12]) and now.day < int(id_str[12:14])):
        age -= 1

    return {"province": province, "birth": birth, "gender": gender, "age": age}

def generate_biometric_template(id_card):
    """Simulate biometric template generation from ID card hash."""
    tmpl = ""
    for i, c in enumerate(id_card):
        v = (ord(c) * 7 + i * 13) % 256
        tmpl += f"{v:02X}"
    return tmpl

def compare_biometric_templates(t1, t2):
    """Compare two biometric templates (string similarity)."""
    if not t1 or not t2:
        return 0.0
    if t1 == t2:
        return 100.0
    min_len = min(len(t1), len(t2))
    match = sum(1 for i in range(min_len) if t1[i] == t2[i])
    return (match / min_len) * 100.0

def get_credit_rating(score):
    if score >= 900:
        return "AAA (极优)"
    if score >= 800:
        return "AA (优秀)"
    if score >= 700:
        return "A (良好)"
    if score >= 600:
        return "B (一般)"
    return "C (需关注)"

def calc_daily_interest(card):
    """Calculate daily interest for a card."""
    if card["type"] == "信用卡":
        principal = card.get("loan_balance", 0)
    else:
        principal = card.get("balance", 0)
    rate = card.get("interest_rate", 0.35)
    return principal * (rate / 100) / 365

def calc_monthly_interest(card):
    """Calculate monthly interest for a card."""
    if card["type"] == "信用卡":
        principal = card.get("loan_balance", 0)
    else:
        principal = card.get("balance", 0)
    rate = card.get("interest_rate", 0.35)
    return principal * (rate / 100) / 12

# ============================================================
# HELPER: Account-based operations (backward compat with 50 students)
# ============================================================

def read_accounts():
    accounts = []
    for line in read_lines(ACCOUNT_FILE):
        p = line.split("|")
        if len(p) == 6:
            accounts.append({
                "id": p[0], "name": p[1], "password": p[2],
                "balance": safe_float(p[3]), "is_locked": p[4] == "1",
                "is_admin": p[5] == "1"
            })
    return accounts

def write_accounts(data):
    lines = []
    for a in data:
        lines.append("{}|{}|{}|{}|{}|{}".format(
            a["id"], a["name"], a["password"],
            a.get("balance", 0),
            "1" if a.get("is_locked") else "0",
            "1" if a.get("is_admin") else "0"
        ))
    write_lines(ACCOUNT_FILE, lines)

# Auto-create sdufe cards for account.txt students (runs after all helpers defined)
sync_student_cards()

# ============================================================
# AUTH ROUTES
# ============================================================

@app.route('/')
def home():
    return send_from_directory(BASE_DIR, "index.html")

@app.route("/api/login", methods=["POST"])
def api_login():
    data = request.get_json(silent=True) or {}
    role = data.get("role", "customer")  # admin, employee, customer
    account_id = str(data.get("account_id", "")).strip()
    password = str(data.get("password", "")).strip()

    if role == "admin":
        employees = read_employees()
        for emp in employees:
            if emp["id"] == account_id and emp["is_active"]:
                valid, needs_rehash = verify_password(password, emp["password"])
                if valid:
                    if emp["role"] != "admin":
                        return jsonify({"success": False, "message": "非管理员账户!"}), 403
                    if needs_rehash:
                        emp["password"] = generate_password_hash(password)
                        write_employees(employees)
                    session["user"] = {"id": emp["id"], "name": emp["name"],
                                       "role": "admin", "department": emp["department"]}
                    return jsonify({"success": True, "message": "管理员登录成功",
                                    "data": {"id": emp["id"], "name": emp["name"], "role": "admin",
                                             "department": emp["department"]}})
        return jsonify({"success": False, "message": "账号或密码错误"}), 401

    elif role == "employee":
        employees = read_employees()
        for emp in employees:
            if emp["id"] == account_id and emp["is_active"]:
                valid, needs_rehash = verify_password(password, emp["password"])
                if valid:
                    if needs_rehash:
                        emp["password"] = generate_password_hash(password)
                        write_employees(employees)
                    session["user"] = {"id": emp["id"], "name": emp["name"],
                                       "role": emp["role"], "department": emp["department"]}
                    return jsonify({"success": True, "message": "职员登录成功",
                                    "data": {"id": emp["id"], "name": emp["name"], "role": emp["role"],
                                             "department": emp["department"]}})
        return jsonify({"success": False, "message": "账号或密码错误"}), 401

    else:  # customer
        # Try C++ customers first
        customers = read_customers()
        for c in customers:
            if c["id"] == account_id and c.get("is_active", True):
                valid, needs_rehash = verify_password(password, c["password"])
                if valid:
                    if needs_rehash:
                        c["password"] = generate_password_hash(password)
                        write_customers(customers)
                    cards = read_cards()
                    my_cards = [cd for cd in cards if cd["customer_id"] == c["id"]]
                    session["user"] = {"id": c["id"], "name": c["name"], "role": "customer"}
                    return jsonify({"success": True, "message": "客户登录成功",
                                    "data": {"id": c["id"], "name": c["name"], "role": "customer",
                                             "type": c["type"], "credit_score": c["credit_score"],
                                             "financial_assets": c["financial_assets"],
                                             "phone": c["phone"], "address": c["address"],
                                             "id_card": c.get("id_card", ""),
                                             "cards": my_cards}})

        # Fallback: try account.txt (50 students) — legacy accounts
        accounts = read_accounts()
        for acc in accounts:
            if acc["id"] == account_id:
                if acc["is_locked"]:
                    return jsonify({"success": False, "message": "账户已锁定"}), 403
                valid, needs_rehash = verify_password(password, acc["password"])
                if valid:
                    if needs_rehash:
                        acc["password"] = generate_password_hash(password)
                        write_accounts(accounts)
                    session["user"] = {"id": acc["id"], "name": acc["name"],
                                       "role": "admin" if acc["is_admin"] else "customer"}
                    return jsonify({"success": True, "message": "登录成功",
                                    "data": {"id": acc["id"], "name": acc["name"],
                                             "role": "admin" if acc["is_admin"] else "customer",
                                             "balance": acc["balance"],
                                             "type": "普通"}})

        return jsonify({"success": False, "message": "账号或密码错误"}), 401

@app.route("/api/logout", methods=["POST"])
def api_logout():
    session.pop("user", None)
    return jsonify({"success": True, "message": "已登出"})

# ============================================================
# MODULE 1: EMPLOYEE MANAGEMENT
# ============================================================

@app.route("/api/employees", methods=["GET"])
@login_required
def api_list_employees():
    employees = read_employees()
    return jsonify({"success": True, "count": len(employees), "data": employees})

@app.route("/api/employees", methods=["POST"])
@login_required
def api_add_employee():
    data = request.get_json(silent=True) or {}
    eid = str(data.get("id", "")).strip()
    if not eid:
        return jsonify({"success": False, "message": "职员编号不能为空"}), 400

    employees = read_employees()
    for emp in employees:
        if emp["id"] == eid:
            return jsonify({"success": False, "message": "职员编号已存在"}), 400

    pw = str(data.get("password", "")).strip()
    if len(pw) != 6 or not pw.isdigit():
        return jsonify({"success": False, "message": "密码必须为6位数字"}), 400

    emp = {
        "id": eid, "name": data.get("name", ""),
        "password": generate_password_hash(pw),  # hashed
        "role": data.get("role", "staff"), "department": data.get("department", ""),
        "phone": data.get("phone", ""), "email": data.get("email", ""),
        "hire_date": today_str(), "is_active": True
    }
    employees.append(emp)
    write_employees(employees)
    return jsonify({"success": True, "message": f"职员 {emp['name']} 添加成功!", "data": emp})

@app.route("/api/employees/<eid>", methods=["PUT"])
@login_required
def api_modify_employee(eid):
    data = request.get_json(silent=True) or {}
    employees = read_employees()
    for emp in employees:
        if emp["id"] == eid:
            for key in ["name", "department", "phone", "email"]:
                if key in data and data[key]:
                    emp[key] = data[key]
            if "is_active" in data:
                emp["is_active"] = bool(data["is_active"])
            write_employees(employees)
            return jsonify({"success": True, "message": "职员信息已更新!", "data": emp})
    return jsonify({"success": False, "message": "职员不存在"}), 404

@app.route("/api/employees/<eid>", methods=["DELETE"])
@login_required
def api_delete_employee(eid):
    employees = read_employees()
    for emp in employees:
        if emp["id"] == eid:
            if emp["role"] == "admin":
                return jsonify({"success": False, "message": "不能删除管理员账户!"}), 400
            emp["is_active"] = False
            write_employees(employees)
            return jsonify({"success": True, "message": f"职员 {emp['name']} 已停用"})
    return jsonify({"success": False, "message": "职员不存在"}), 404

@app.route("/api/employees/<eid>/password", methods=["PUT"])
@login_required
def api_reset_employee_password(eid):
    data = request.get_json(silent=True) or {}
    new_pw = str(data.get("password", "")).strip()
    if len(new_pw) != 6 or not new_pw.isdigit():
        return jsonify({"success": False, "message": "密码必须为6位数字"}), 400

    employees = read_employees()
    for emp in employees:
        if emp["id"] == eid:
            emp["password"] = generate_password_hash(new_pw)  # hashed
            write_employees(employees)
            return jsonify({"success": True, "message": "密码已重置!"})
    return jsonify({"success": False, "message": "职员不存在"}), 404

# ============================================================
# MODULE 2: CUSTOMER ACCOUNT MANAGEMENT
# ============================================================

@app.route("/api/customers", methods=["GET"])
@login_required
def api_list_customers():
    customers = read_customers()
    # Also include accounts from account.txt
    accounts = read_accounts()
    for acc in accounts:
        # Check if already in customers
        found = any(c["id"] == acc["id"] for c in customers)
        if not found:
            customers.append({
                "id": acc["id"], "name": acc["name"],
                "type": "普通", "phone": "", "open_date": "",
                "credit_score": 600, "financial_assets": acc.get("balance", 0),
                "address": "", "id_card": "",
                "is_active": not acc.get("is_locked", False)
            })
    return jsonify({"success": True, "count": len(customers), "data": customers})

@app.route("/api/customers", methods=["POST"])
@login_required
def api_add_customer():
    data = request.get_json(silent=True) or {}
    cid = str(data.get("id", "")).strip()
    if not cid:
        return jsonify({"success": False, "message": "客户编号不能为空"}), 400

    customers = read_customers()
    if any(c["id"] == cid for c in customers):
        return jsonify({"success": False, "message": "客户编号已存在"}), 400

    customer = {
        "id": cid, "name": data.get("name", ""),
        "password": generate_password_hash(data.get("password", "123456")),  # hashed
        "type": data.get("type", "普通"),
        "phone": data.get("phone", ""),
        "open_date": today_str(),
        "credit_score": safe_float(data.get("credit_score"), 600),
        "financial_assets": safe_float(data.get("financial_assets"), 0),
        "address": data.get("address", ""),
        "id_card": data.get("id_card", ""),
        "is_active": True,
        "id_photo_path": "", "biometric_data": "", "face_photo_path": ""
    }
    customers.append(customer)
    write_customers(customers)
    return jsonify({"success": True, "message": f"客户 {customer['name']} 添加成功!", "data": customer})

@app.route("/api/customers/<cid>", methods=["GET"])
@login_required
def api_get_customer(cid):
    customers = read_customers()
    for c in customers:
        if c["id"] == cid:
            cards = read_cards()
            my_cards = [cd for cd in cards if cd["customer_id"] == cid]
            c["cards"] = my_cards
            return jsonify({"success": True, "data": c})
    return jsonify({"success": False, "message": "客户不存在"}), 404

@app.route("/api/customers/<cid>", methods=["PUT"])
@login_required
def api_modify_customer(cid):
    data = request.get_json(silent=True) or {}
    customers = read_customers()
    for c in customers:
        if c["id"] == cid:
            for key in ["name", "phone", "address", "id_card", "type"]:
                if key in data:
                    c[key] = data[key]
            for key in ["credit_score", "financial_assets"]:
                if key in data:
                    c[key] = safe_float(data[key])
            write_customers(customers)
            return jsonify({"success": True, "message": "客户信息已更新!", "data": c})
    return jsonify({"success": False, "message": "客户不存在"}), 404

@app.route("/api/customers/<cid>", methods=["DELETE"])
@login_required
def api_delete_customer(cid):
    customers = read_customers()
    for c in customers:
        if c["id"] == cid:
            c["is_active"] = False
            write_customers(customers)
            return jsonify({"success": True, "message": f"客户 {c['name']} 已注销"})
    return jsonify({"success": False, "message": "客户不存在"}), 404

# ============================================================
# MODULE 3: BANK CARD MANAGEMENT
# ============================================================

@app.route("/api/cards", methods=["GET"])
@login_required
def api_list_cards():
    cards = read_cards()
    # Enrich with customer names
    customers = {c["id"]: c["name"] for c in read_customers()}
    for card in cards:
        card["customer_name"] = customers.get(card["customer_id"], "")
    return jsonify({"success": True, "count": len(cards), "data": cards})

@app.route("/api/cards", methods=["POST"])
@login_required
def api_add_card():
    data = request.get_json(silent=True) or {}
    cid = str(data.get("id", "")).strip()
    if not cid:
        return jsonify({"success": False, "message": "卡号不能为空"}), 400
    cards = read_cards()
    if any(c["id"] == cid for c in cards):
        return jsonify({"success": False, "message": "卡号已存在"}), 400

    card = {
        "id": cid, "customer_id": data.get("customer_id", ""),
        "type": data.get("type", "借记卡"),
        "balance": safe_float(data.get("balance"), 0),
        "loan_balance": safe_float(data.get("loan_balance"), 0),
        "interest_rate": safe_float(data.get("interest_rate"), 0.35),
        "credit_limit": safe_float(data.get("credit_limit"), 0),
        "open_date": today_str(),
        "status": data.get("status", "正常"),
        "daily_limit": safe_float(data.get("daily_limit"), 50000)
    }
    cards.append(card)
    write_cards(cards)
    return jsonify({"success": True, "message": f"银行卡 {card['id']} 添加成功!", "data": card})

@app.route("/api/cards/<cid>", methods=["PUT"])
@login_required
def api_modify_card(cid):
    data = request.get_json(silent=True) or {}
    cards = read_cards()
    for card in cards:
        if card["id"] == cid:
            for key in ["type", "status"]:
                if key in data:
                    card[key] = data[key]
            for key in ["balance", "loan_balance", "interest_rate", "credit_limit", "daily_limit"]:
                if key in data:
                    card[key] = safe_float(data[key])
            write_cards(cards)
            return jsonify({"success": True, "message": "银行卡信息已更新!", "data": card})
    return jsonify({"success": False, "message": "银行卡不存在"}), 404

@app.route("/api/cards/<cid>", methods=["DELETE"])
@login_required
def api_delete_card(cid):
    cards = read_cards()
    for card in cards:
        if card["id"] == cid:
            card["status"] = "注销"
            write_cards(cards)
            return jsonify({"success": True, "message": f"银行卡 {cid} 已注销"})
    return jsonify({"success": False, "message": "银行卡不存在"}), 404

# ============================================================
# MODULE 4: TRANSACTION MANAGEMENT (Deposit/Withdraw/Transfer/Loan/Repay)
# ============================================================

def _do_transaction(from_card, to_card, ttype, amount, emp_id="", remark=""):
    """Core transaction logic. Updates card balances."""
    cards = read_cards()
    from_idx = None
    to_idx = None

    for i, c in enumerate(cards):
        if c["id"] == from_card and from_card != "系统":
            from_idx = i
        if c["id"] == to_card and to_card != "系统":
            to_idx = i

    if ttype in ("取款", "转账", "还款"):
        if from_idx is None and from_card != "系统":
            return False, "转出卡不存在"
        if from_idx is not None:
            if cards[from_idx]["status"] != "正常":
                return False, "转出卡已冻结/注销"
            if cards[from_idx]["balance"] < amount:
                return False, f"余额不足! 当前余额: {cards[from_idx]['balance']:.2f}"

    if ttype in ("存款", "转账", "贷款"):
        if to_idx is None and to_card != "系统":
            return False, "转入卡不存在"
        if to_idx is not None and cards[to_idx]["status"] != "正常":
            return False, "转入卡已冻结/注销"

    # Execute
    if from_idx is not None:
        if ttype in ("取款", "转账", "还款"):
            cards[from_idx]["balance"] -= amount
        elif ttype == "存款":
            cards[from_idx]["balance"] += amount
        elif ttype == "贷款":
            cards[from_idx]["loan_balance"] = cards[from_idx].get("loan_balance", 0) + amount
            cards[from_idx]["balance"] += amount

    if to_idx is not None and to_idx != from_idx:
        if ttype == "转账":
            cards[to_idx]["balance"] += amount

    write_cards(cards)

    # Record transaction
    txn = {
        "id": gen_txn_id(), "from_card": from_card, "to_card": to_card,
        "type": ttype, "amount": amount, "time": now_str(),
        "status": "成功", "employee_id": emp_id, "remark": remark
    }
    append_transaction(txn)
    return True, txn

@app.route("/api/deposit", methods=["POST"])
@login_required
def api_deposit():
    data = request.get_json(silent=True) or {}
    account_id = str(data.get("account_id", "")).strip()
    amount = safe_float(data.get("amount"), 0)
    if amount <= 0:
        return jsonify({"success": False, "message": "金额必须大于0"}), 400
    if amount > 10000000:
        return jsonify({"success": False, "message": "单笔存款不得超过1000万元"}), 400

    # Handle account.txt style
    accounts = read_accounts()
    for acc in accounts:
        if acc["id"] == account_id:
            if acc["is_locked"]:
                return jsonify({"success": False, "message": "账户已锁定"}), 403
            acc["balance"] = acc.get("balance", 0) + amount
            write_accounts(accounts)
            # Also sync linked card balance
            _sync_card_balance(account_id, acc["balance"])
            txn = {"id": gen_txn_id(), "from_card": resolve_card_id(account_id), "to_card": "系统",
                   "type": "存款", "amount": amount, "time": now_str(), "status": "成功",
                   "employee_id": "", "remark": ""}
            append_transaction(txn)
            return jsonify({"success": True, "message": "存款成功",
                            "data": {"transaction_id": txn["id"], "amount": amount,
                                     "new_balance": acc["balance"]}})

    # Handle card-based
    ok, result = _do_transaction(account_id, "系统", "存款", amount)
    if ok:
        return jsonify({"success": True, "message": "存款成功", "data": result})
    return jsonify({"success": False, "message": result}), 400

@app.route("/api/withdraw", methods=["POST"])
@login_required
def api_withdraw():
    data = request.get_json(silent=True) or {}
    account_id = str(data.get("account_id", "")).strip()
    amount = safe_float(data.get("amount"), 0)
    if amount <= 0:
        return jsonify({"success": False, "message": "金额必须大于0"}), 400

    accounts = read_accounts()
    for acc in accounts:
        if acc["id"] == account_id:
            if acc["is_locked"]:
                return jsonify({"success": False, "message": "账户已锁定"}), 403
            if acc.get("balance", 0) < amount:
                return jsonify({"success": False, "message": f"余额不足! 当前余额: {acc['balance']:.2f}"}), 400
            acc["balance"] -= amount
            write_accounts(accounts)
            _sync_card_balance(account_id, acc["balance"])
            txn = {"id": gen_txn_id(), "from_card": resolve_card_id(account_id), "to_card": "系统",
                   "type": "取款", "amount": amount, "time": now_str(), "status": "成功",
                   "employee_id": "", "remark": ""}
            append_transaction(txn)
            return jsonify({"success": True, "message": "取款成功",
                            "data": {"transaction_id": txn["id"], "amount": amount,
                                     "new_balance": acc["balance"]}})

    ok, result = _do_transaction(account_id, "系统", "取款", amount)
    if ok:
        return jsonify({"success": True, "message": "取款成功", "data": result})
    return jsonify({"success": False, "message": result}), 400

@app.route("/api/transfer", methods=["POST"])
@login_required
def api_transfer():
    data = request.get_json(silent=True) or {}
    from_id = str(data.get("from_account", "")).strip()
    to_id = str(data.get("to_account", "")).strip()
    amount = safe_float(data.get("amount"), 0)
    if amount <= 0:
        return jsonify({"success": False, "message": "金额必须大于0"}), 400
    if from_id == to_id:
        return jsonify({"success": False, "message": "不能给自己转账"}), 400

    # Handle account.txt style
    accounts = read_accounts()
    from_acc = None
    to_acc = None
    for acc in accounts:
        if acc["id"] == from_id:
            from_acc = acc
        if acc["id"] == to_id:
            to_acc = acc

    if from_acc and to_acc:
        if from_acc["is_locked"]:
            return jsonify({"success": False, "message": "转出账户已锁定"}), 403
        if from_acc.get("balance", 0) < amount:
            return jsonify({"success": False, "message": f"余额不足! 当前余额: {from_acc['balance']:.2f}"}), 400
        from_acc["balance"] -= amount
        to_acc["balance"] = to_acc.get("balance", 0) + amount
        write_accounts(accounts)
        _sync_card_balance(from_id, from_acc["balance"])
        _sync_card_balance(to_id, to_acc["balance"])
        txn = {"id": gen_txn_id(), "from_card": resolve_card_id(from_id), "to_card": resolve_card_id(to_id),
               "type": "转账", "amount": amount, "time": now_str(), "status": "成功",
               "employee_id": "", "remark": ""}
        append_transaction(txn)
        return jsonify({"success": True, "message": "转账成功", "data": txn})

    # Handle card-based
    ok, result = _do_transaction(from_id, to_id, "转账", amount)
    if ok:
        return jsonify({"success": True, "message": "转账成功", "data": result})
    return jsonify({"success": False, "message": result}), 400

@app.route("/api/loan", methods=["POST"])
@login_required
def api_loan():
    data = request.get_json(silent=True) or {}
    account_id = str(data.get("account_id", "")).strip()
    amount = safe_float(data.get("amount"), 0)
    if amount <= 0:
        return jsonify({"success": False, "message": "金额必须大于0"}), 400
    if amount > 500000:
        return jsonify({"success": False, "message": "单笔贷款不得超过50万元"}), 400

    ok, result = _do_transaction(account_id, "系统", "贷款", amount)
    if ok:
        return jsonify({"success": True, "message": "贷款成功", "data": result})
    return jsonify({"success": False, "message": result}), 400

@app.route("/api/repay", methods=["POST"])
@login_required
def api_repay():
    data = request.get_json(silent=True) or {}
    account_id = str(data.get("account_id", "")).strip()
    amount = safe_float(data.get("amount"), 0)
    if amount <= 0:
        return jsonify({"success": False, "message": "金额必须大于0"}), 400

    cards = read_cards()
    for card in cards:
        if card["id"] == account_id:
            if card["status"] != "正常":
                return jsonify({"success": False, "message": "卡已冻结/注销"}), 400
            loan = card.get("loan_balance", 0)
            if loan <= 0:
                return jsonify({"success": False, "message": "无贷款需要偿还"}), 400
            if amount > loan:
                amount = loan
            # BUGFIX: reject if card balance insufficient (match C++ logic)
            balance = card.get("balance", 0)
            if amount > balance:
                return jsonify({"success": False, "message":
                    f"卡内余额不足! 当前余额: {balance:.2f}, 需还款: {amount:.2f}"}), 400
            card["loan_balance"] = loan - amount
            card["balance"] = balance - amount
            write_cards(cards)
            txn = {"id": gen_txn_id(), "from_card": resolve_card_id(account_id), "to_card": "系统",
                   "type": "还款", "amount": amount, "time": now_str(), "status": "成功",
                   "employee_id": "", "remark": ""}
            append_transaction(txn)
            return jsonify({"success": True, "message": f"还款成功! 剩余贷款: {card['loan_balance']:.2f}",
                            "data": txn})
    return jsonify({"success": False, "message": "银行卡不存在"}), 404

# ============================================================
# MODULE 5: BUSINESS QUERY
# ============================================================

@app.route("/api/transactions", methods=["GET"])
@login_required
def api_get_transactions():
    transactions = read_transactions()
    # Support query params
    ttype = request.args.get("type", "")
    start_time = request.args.get("start", "")
    end_time = request.args.get("end", "")
    min_amt = request.args.get("min_amt", "")
    max_amt = request.args.get("max_amt", "")
    card_id = request.args.get("card_id", "")
    customer_type = request.args.get("customer_type", "")
    sort = request.args.get("sort", "")  # "time_desc", "amount_asc", "amount_desc"

    result = transactions

    if ttype:
        result = [t for t in result if t["type"] == ttype]
    if start_time:
        result = [t for t in result if t["time"] >= start_time]
    if end_time:
        result = [t for t in result if t["time"] <= end_time + " 23:59:59"]
    if min_amt:
        result = [t for t in result if t["amount"] >= safe_float(min_amt)]
    if max_amt:
        result = [t for t in result if t["amount"] <= safe_float(max_amt)]
    if card_id:
        result = [t for t in result if t.get("from_card","") == card_id
                  or t.get("to_card","") == card_id or t.get("from","") == card_id
                  or t.get("to","") == card_id]
    if customer_type:
        # Filter by customer type: need to cross-reference cards
        customers = read_customers()
        cards = read_cards()
        type_cards = set()
        for c in customers:
            if c.get("type") == customer_type:
                for cd in cards:
                    if cd["customer_id"] == c["id"]:
                        type_cards.add(cd["id"])
        result = [t for t in result if t.get("from_card","") in type_cards
                  or t.get("to_card","") in type_cards
                  or t.get("from","") in type_cards or t.get("to","") in type_cards]

    # Sort
    if sort == "time_desc":
        result.sort(key=lambda x: x["time"], reverse=True)
    elif sort == "time_asc":
        result.sort(key=lambda x: x["time"])
    elif sort == "amount_desc":
        result.sort(key=lambda x: x["amount"], reverse=True)
    elif sort == "amount_asc":
        result.sort(key=lambda x: x["amount"])
    else:
        result.sort(key=lambda x: x["time"], reverse=True)

    # Summary
    summary = {"存款": {"count": 0, "total": 0}, "取款": {"count": 0, "total": 0},
               "转账": {"count": 0, "total": 0}, "贷款": {"count": 0, "total": 0},
               "还款": {"count": 0, "total": 0}}
    for t in result:
        if t["status"] == "成功" and t["type"] in summary:
            summary[t["type"]]["count"] += 1
            summary[t["type"]]["total"] += t["amount"]

    return jsonify({"success": True, "count": len(result), "data": result,
                    "summary": summary})

@app.route("/api/transactions/summary", methods=["GET"])
@login_required
def api_transaction_summary():
    transactions = read_transactions()
    summary = {"存款": {"count": 0, "total": 0}, "取款": {"count": 0, "total": 0},
               "转账": {"count": 0, "total": 0}, "贷款": {"count": 0, "total": 0},
               "还款": {"count": 0, "total": 0}}
    for t in transactions:
        if t["status"] == "成功" and t["type"] in summary:
            summary[t["type"]]["count"] += 1
            summary[t["type"]]["total"] += t["amount"]
    return jsonify({"success": True, "data": summary})

# ============================================================
# MODULE 6: QUEUE MANAGEMENT
# ============================================================

@app.route("/api/queue/take", methods=["POST"])
@login_required
def api_take_ticket():
    global vip_queue, normal_queue, ticket_counter
    data = request.get_json(silent=True) or {}
    customer_id = str(data.get("customer_id", "")).strip()
    service_type = data.get("service_type", "综合业务")

    customers = read_customers()
    customer = None
    for c in customers:
        if c["id"] == customer_id:
            customer = c
            break

    if not customer:
        # Try account
        accounts = read_accounts()
        for a in accounts:
            if a["id"] == customer_id:
                customer = {"id": a["id"], "name": a["name"], "type": "普通"}
                break

    if not customer:
        return jsonify({"success": False, "message": "客户不存在"}), 404

    ticket_counter += 1
    ticket = {
        "id": f"Q{ticket_counter % 10000:04d}",
        "customer_id": customer_id,
        "customer_name": customer["name"],
        "customer_type": customer.get("type", "普通"),
        "window_id": -1,
        "service_type": service_type,
        "arrive_time": now_str(),
        "start_time": "", "end_time": "",
        "rating": 0, "status": "等待中",
        "priority": 10 if customer.get("type") == "VIP" else 0
    }

    if customer.get("type") == "VIP":
        vip_queue.append(ticket)
    else:
        normal_queue.append(ticket)

    save_queue_state()
    return jsonify({"success": True, "message": f"取号成功! 票号: {ticket['id']}",
                    "data": ticket,
                    "vip_queue_size": len(vip_queue),
                    "normal_queue_size": len(normal_queue)})

@app.route("/api/queue/call", methods=["POST"])
@login_required
def api_call_ticket():
    global vip_queue, normal_queue
    data = request.get_json(silent=True) or {}
    window_type = data.get("window_type", "1")  # 1=VIP, 2=Normal
    completed = data.get("completed", True)
    rating = safe_int(data.get("rating"), 5)

    if window_type == "1":
        # VIP window: VIP first, then normal
        if vip_queue:
            ticket = vip_queue.pop(0)
        elif normal_queue:
            ticket = normal_queue.pop(0)
        else:
            return jsonify({"success": False, "message": "当前无等待客户"}), 404
    else:
        # Normal window: VIP first (cross-service), then normal
        if vip_queue:
            ticket = vip_queue.pop(0)
        elif normal_queue:
            ticket = normal_queue.pop(0)
        else:
            return jsonify({"success": False, "message": "当前无等待客户"}), 404

    ticket["window_id"] = 0 if window_type == "1" else 1
    ticket["start_time"] = now_str()
    ticket["status"] = "办理中"

    if completed:
        ticket["end_time"] = now_str()
        ticket["status"] = "已完成"
        ticket["rating"] = max(1, min(5, rating))

        # Save to history
        history = read_queue_history()
        history.append(ticket)
        write_queue_history(history)

        # Update daily stats
        daily = read_daily_stats()
        today = today_str()
        updated = False
        for ds in daily:
            if ds["date"] == today:
                ds["completed"] += 1
                total_rating = ds["avg_rating"] * (ds["completed"] - 1) + ticket["rating"]
                ds["avg_rating"] = total_rating / ds["completed"]
                updated = True
                break
        if not updated:
            daily.append({
                "date": today, "total_customers": 1,
                "vip_customers": 1 if ticket["customer_type"] == "VIP" else 0,
                "normal_customers": 0 if ticket["customer_type"] == "VIP" else 1,
                "avg_wait_time": 0, "avg_rating": ticket["rating"], "completed": 1
            })
        write_daily_stats(daily)
    else:
        # Re-queue
        ticket["status"] = "等待中"
        if ticket["customer_type"] == "VIP":
            vip_queue.append(ticket)
        else:
            normal_queue.append(ticket)

    save_queue_state()
    return jsonify({"success": True, "data": ticket,
                    "vip_queue_size": len(vip_queue),
                    "normal_queue_size": len(normal_queue)})

@app.route("/api/queue/status", methods=["GET"])
@login_required
def api_queue_status():
    return jsonify({
        "success": True,
        "vip_queue": vip_queue,
        "vip_queue_size": len(vip_queue),
        "normal_queue": normal_queue,
        "normal_queue_size": len(normal_queue)
    })

@app.route("/api/queue/history", methods=["GET"])
@login_required
def api_queue_history():
    history = read_queue_history()
    return jsonify({"success": True, "count": len(history), "data": history})

@app.route("/api/queue/stats", methods=["GET"])
@login_required
def api_daily_stats():
    stats = read_daily_stats()
    return jsonify({"success": True, "count": len(stats), "data": stats})

# ============================================================
# MODULE 7: BRANCH NAVIGATION (Dijkstra)
# ============================================================

@app.route("/api/branches", methods=["GET"])
@login_required
def api_list_branches():
    branches = read_branches()
    graph = read_branch_graph()
    return jsonify({"success": True, "count": len(branches),
                    "data": branches, "graph": graph})

@app.route("/api/branches", methods=["POST"])
@login_required
def api_add_branch():
    data = request.get_json(silent=True) or {}
    bid = str(data.get("id", "")).strip()
    if not bid:
        return jsonify({"success": False, "message": "网点编号不能为空"}), 400
    branches = read_branches()
    if any(b["id"] == bid for b in branches):
        return jsonify({"success": False, "message": "网点编号已存在"}), 400

    branch = {"id": bid, "name": data.get("name", ""), "address": data.get("address", ""),
              "phone": data.get("phone", ""), "hours": data.get("hours", "09:00-17:00"),
              "services": data.get("services", ""), "x": safe_float(data.get("x")),
              "y": safe_float(data.get("y"))}
    branches.append(branch)
    write_branches(branches)

    # Expand adjacency matrix
    graph = read_branch_graph()
    N = len(branches)
    if not graph:
        graph = [[0]]
    else:
        for row in graph:
            row.append(-1)
        graph.append([-1] * N)
        graph[N-1][N-1] = 0
    write_branch_graph(graph)
    return jsonify({"success": True, "message": f"网点 {branch['name']} 添加成功!", "data": branch})

@app.route("/api/branches/<bid>", methods=["PUT"])
@login_required
def api_modify_branch(bid):
    data = request.get_json(silent=True) or {}
    branches = read_branches()
    for b in branches:
        if b["id"] == bid:
            for key in ["name", "address", "phone", "hours", "services", "x", "y"]:
                if key in data:
                    b[key] = data[key] if key not in ("x", "y") else safe_float(data[key])
            write_branches(branches)
            return jsonify({"success": True, "message": "网点信息已更新!", "data": b})
    return jsonify({"success": False, "message": "网点不存在"}), 404

@app.route("/api/branches/<bid>", methods=["DELETE"])
@login_required
def api_delete_branch(bid):
    branches = read_branches()
    graph = read_branch_graph()
    idx = None
    for i, b in enumerate(branches):
        if b["id"] == bid:
            idx = i
            break
    if idx is None:
        return jsonify({"success": False, "message": "网点不存在"}), 404

    branches.pop(idx)
    N = len(branches)
    new_graph = [[-1]*N for _ in range(N)]
    for ni, i in enumerate(range(len(graph))):
        if i == idx: continue
        for nj, j in enumerate(range(len(graph))):
            if j == idx: continue
            new_graph[ni][nj] = graph[i][j]
    write_branches(branches)
    write_branch_graph(new_graph)
    return jsonify({"success": True, "message": "网点已删除"})

@app.route("/api/branches/path", methods=["POST"])
@login_required
def api_set_branch_path():
    data = request.get_json(silent=True) or {}
    src = str(data.get("src", "")).strip()
    dst = str(data.get("dst", "")).strip()
    dist = safe_float(data.get("distance"), -1)
    if dist < 0:
        return jsonify({"success": False, "message": "距离无效"}), 400

    graph = read_branch_graph()
    branches = read_branches()
    src_idx = next((i for i, b in enumerate(branches) if b["id"] == src), None)
    dst_idx = next((i for i, b in enumerate(branches) if b["id"] == dst), None)
    if src_idx is None or dst_idx is None:
        return jsonify({"success": False, "message": "网点不存在"}), 404

    graph[src_idx][dst_idx] = dist
    graph[dst_idx][src_idx] = dist
    write_branch_graph(graph)
    return jsonify({"success": True, "message": f"路径 {src}→{dst} = {dist}km 已设置"})

@app.route("/api/branches/<src_id>/path/<dst_id>", methods=["GET"])
@login_required
def api_shortest_path(src_id, dst_id):
    branches = read_branches()
    graph = read_branch_graph()
    N = len(branches)
    src = next((i for i, b in enumerate(branches) if b["id"] == src_id), None)
    dst = next((i for i, b in enumerate(branches) if b["id"] == dst_id), None)
    if src is None or dst is None:
        return jsonify({"success": False, "message": "网点不存在"}), 404

    dist, prev = dijkstra(graph, N, src)
    if dist[dst] >= 1e17:
        return jsonify({"success": False, "message": "两点之间无可达路径!"}), 404

    # Reconstruct path
    path_indices = []
    v = dst
    while v != -1:
        path_indices.append(v)
        v = prev[v]
    path_indices.reverse()

    path = []
    for i in range(len(path_indices)):
        node = {"name": branches[path_indices[i]]["name"],
                "address": branches[path_indices[i]]["address"],
                "id": branches[path_indices[i]]["id"]}
        if i > 0:
            node["segment_distance"] = graph[path_indices[i-1]][path_indices[i]]
        path.append(node)

    return jsonify({"success": True, "total_distance": dist[dst], "path": path})

@app.route("/api/branches/<src_id>/reachable", methods=["GET"])
@login_required
def api_reachable_branches(src_id):
    branches = read_branches()
    graph = read_branch_graph()
    N = len(branches)
    src = next((i for i, b in enumerate(branches) if b["id"] == src_id), None)
    if src is None:
        return jsonify({"success": False, "message": "网点不存在"}), 404

    dist, _ = dijkstra(graph, N, src)
    reachable = []
    for i in range(N):
        if i == src: continue
        reachable.append({
            "id": branches[i]["id"], "name": branches[i]["name"],
            "distance": dist[i] if dist[i] < 1e17 else -1,
            "reachable": dist[i] < 1e17
        })
    return jsonify({"success": True,
                    "from": branches[src]["name"],
                    "reachable": reachable})

# ============================================================
# MODULE 8: SMART MANAGEMENT
# ============================================================

@app.route("/api/smart/anomaly/large", methods=["GET"])
@login_required
def api_detect_large_amount():
    """Detect large-amount transactions."""
    threshold = safe_float(request.args.get("threshold", ""), LARGE_AMOUNT)
    transactions = read_transactions()
    anomalies = [t for t in transactions if t["amount"] > threshold]
    return jsonify({"success": True, "threshold": threshold,
                    "count": len(anomalies), "data": anomalies})

@app.route("/api/smart/anomaly/high_freq", methods=["GET"])
@login_required
def api_detect_high_frequency():
    """Detect high-frequency transactions (sliding window: >MULTI_TRANSACTION in 1 hour)."""
    transactions = read_transactions()
    # Group by from_card (or to_card for customers)
    cust_txns = {}
    for t in transactions:
        for key in [t.get("from_card",""), t.get("to_card",""), t.get("from",""), t.get("to","")]:
            if key and key != "系统" and key != "-":
                if key not in cust_txns:
                    cust_txns[key] = []
                cust_txns[key].append(t)

    anomalies = []
    for cust_id, txns in cust_txns.items():
        if len(txns) <= MULTI_TRANSACTION:
            continue
        txns_sorted = sorted(txns, key=lambda x: x["time"])
        for i in range(len(txns_sorted)):
            count = 1
            for j in range(i+1, len(txns_sorted)):
                if txns_sorted[i]["time"][:13] == txns_sorted[j]["time"][:13]:
                    count += 1
                else:
                    break
            if count > MULTI_TRANSACTION:
                anomalies.append({"customer_id": cust_id, "hour": txns_sorted[i]["time"][:13],
                                  "count": count})
                break

    return jsonify({"success": True, "threshold": MULTI_TRANSACTION,
                    "count": len(anomalies), "data": anomalies})

@app.route("/api/smart/interest", methods=["GET"])
@login_required
def api_calc_interest():
    """Calculate daily/monthly interest for all cards."""
    calc_type = request.args.get("type", "daily")
    cards = read_cards()
    results = []
    total_interest = 0
    for card in cards:
        if card.get("status") != "正常":
            continue
        if calc_type == "monthly":
            interest = calc_monthly_interest(card)
        else:
            interest = calc_daily_interest(card)
        principal = card.get("loan_balance", 0) if card["type"] == "信用卡" else card.get("balance", 0)
        results.append({
            "card_id": card["id"], "type": card["type"],
            "principal": principal,
            "interest_rate": card.get("interest_rate", 0.35),
            "interest": round(interest, 4)
        })
        total_interest += interest

    return jsonify({"success": True, "type": calc_type,
                    "count": len(results), "total_interest": round(total_interest, 2),
                    "data": results})

@app.route("/api/smart/customer-stats", methods=["GET"])
@login_required
def api_customer_stats():
    """Customer statistics and credit rating distribution."""
    customers = read_customers()
    active = [c for c in customers if c.get("is_active", True)]
    if not active:
        return jsonify({"success": False, "message": "无活跃客户"}), 404

    vip = [c for c in active if c.get("type") == "VIP"]
    normal = [c for c in active if c.get("type") != "VIP"]
    assets = [c.get("financial_assets", 0) for c in active]
    scores = [c.get("credit_score", 0) for c in active]

    avg_assets = sum(assets) / len(assets)
    avg_credit = sum(scores) / len(scores)
    variance = sum((s - avg_credit) ** 2 for s in scores) / len(scores)
    stddev = math.sqrt(variance)

    # Credit rating distribution
    rating_dist = {}
    for c in active:
        rating = get_credit_rating(c.get("credit_score", 600))
        rating_dist[rating] = rating_dist.get(rating, 0) + 1

    return jsonify({"success": True, "data": {
        "active_count": len(active),
        "vip_count": len(vip), "vip_ratio": round(100 * len(vip) / len(active), 1),
        "normal_count": len(normal), "normal_ratio": round(100 * len(normal) / len(active), 1),
        "avg_financial_assets": round(avg_assets, 2),
        "max_financial_assets": round(max(assets), 2),
        "min_financial_assets": round(min(assets), 2),
        "avg_credit_score": round(avg_credit, 2),
        "credit_stddev": round(stddev, 2),
        "rating_distribution": rating_dist
    }})

@app.route("/api/smart/risk/<cid>", methods=["GET"])
@login_required
def api_risk_approval(cid):
    """Risk assessment for a customer."""
    customers = read_customers()
    customer = None
    for c in customers:
        if c["id"] == cid:
            customer = c
            break
    if not customer:
        return jsonify({"success": False, "message": "客户不存在"}), 404

    cards = read_cards()
    total_balance = 0
    total_loan = 0
    total_credit_limit = 0
    for cd in cards:
        if cd["customer_id"] == cid:
            total_balance += cd.get("balance", 0)
            total_loan += cd.get("loan_balance", 0)
            if cd["type"] == "信用卡":
                total_credit_limit += cd.get("credit_limit", 0)

    denominator = total_balance + total_credit_limit
    debt_ratio = (total_loan / denominator) if denominator > 0 else 0

    risk_score = customer.get("credit_score", 600)
    if debt_ratio > 0.5: risk_score -= 100
    if debt_ratio > 0.8: risk_score -= 200
    if customer.get("financial_assets", 0) > 100000: risk_score += 50
    if customer.get("financial_assets", 0) > 500000: risk_score += 50

    if risk_score >= 700:
        conclusion = "通过 (低风险)"
    elif risk_score >= 600:
        conclusion = "条件通过 (中风险, 建议降低额度)"
    else:
        conclusion = "拒绝 (高风险)"

    return jsonify({"success": True, "data": {
        "customer_name": customer["name"],
        "customer_id": customer["id"],
        "customer_type": customer.get("type", ""),
        "credit_score": customer.get("credit_score", 600),
        "credit_rating": get_credit_rating(customer.get("credit_score", 600)),
        "financial_assets": customer.get("financial_assets", 0),
        "total_balance": round(total_balance, 2),
        "total_loan": round(total_loan, 2),
        "total_credit_limit": round(total_credit_limit, 2),
        "debt_ratio": round(debt_ratio * 100, 1),
        "risk_score": round(risk_score),
        "risk_rating": get_credit_rating(risk_score),
        "conclusion": conclusion
    }})

@app.route("/api/smart/investment/<cid>", methods=["GET"])
@login_required
def api_investment_advisor(cid):
    """Investment advisor based on credit score and available cash.
    Supports both customer.txt and account.txt users."""
    customer = None
    # Try customer.txt first
    for c in read_customers():
        if c["id"] == cid:
            customer = c
            break
    # Fallback: account.txt (student accounts)
    if not customer:
        for a in read_accounts():
            if a["id"] == cid:
                customer = {"id": a["id"], "name": a["name"],
                           "credit_score": 650, "type": "普通"}
                break
    if not customer:
        return jsonify({"success": False, "message": "客户不存在"}), 404

    cards = read_cards()
    cash = sum(cd.get("balance", 0) for cd in cards
               if cd["customer_id"] == cid and cd["type"] != "信用卡")
    # If no cards found, use customer's financial_assets or account balance
    if cash == 0:
        cash = safe_float(customer.get("financial_assets",
                         customer.get("balance", 0)))

    credit_score = customer.get("credit_score", 600)
    if credit_score > 750:
        risk_type = "进取型"
        config = {
            "股票型基金": round(cash * 0.4, 2),
            "债券型基金": round(cash * 0.2, 2),
            "定期存款": round(cash * 0.2, 2),
            "活期备用": round(cash * 0.2, 2)
        }
    elif credit_score > 600:
        risk_type = "稳健型"
        config = {
            "债券型基金": round(cash * 0.35, 2),
            "定期存款": round(cash * 0.35, 2),
            "货币基金": round(cash * 0.15, 2),
            "活期备用": round(cash * 0.15, 2)
        }
    else:
        risk_type = "保守型"
        config = {
            "定期存款": round(cash * 0.5, 2),
            "货币基金": round(cash * 0.25, 2),
            "活期备用": round(cash * 0.25, 2)
        }

    return jsonify({"success": True, "data": {
        "customer_name": customer["name"],
        "available_cash": round(cash, 2),
        "risk_preference": risk_type,
        "allocation": config
    }})

# ============================================================
# MODULE 9: IDENTITY VERIFICATION
# ============================================================

@app.route("/api/identity/verify-id", methods=["POST"])
@login_required
def api_verify_id_card():
    """Validate a Chinese 18-digit ID card (ISO 7064 MOD 11-2)."""
    data = request.get_json(silent=True) or {}
    id_card = str(data.get("id_card", "")).strip()

    valid, msg = validate_chinese_id_card(id_card)
    result = {"id_card": id_card, "valid": valid, "message": msg}

    if valid:
        info = extract_from_id_card(id_card)
        result.update(info)

    return jsonify({"success": True, "data": result})

@app.route("/api/identity/biometric/generate", methods=["POST"])
@login_required
def api_generate_biometric():
    """Generate biometric template from ID card."""
    data = request.get_json(silent=True) or {}
    id_card = str(data.get("id_card", "")).strip()
    if not id_card:
        return jsonify({"success": False, "message": "身份证号不能为空"}), 400
    template = generate_biometric_template(id_card)
    return jsonify({"success": True, "data": {"id_card": id_card, "template": template}})

@app.route("/api/identity/biometric/compare", methods=["POST"])
@login_required
def api_compare_biometric():
    """Compare two biometric templates."""
    data = request.get_json(silent=True) or {}
    t1 = str(data.get("template1", "")).strip()
    t2 = str(data.get("template2", "")).strip()
    similarity = compare_biometric_templates(t1, t2)
    threshold = 80.0
    passed = similarity >= threshold
    return jsonify({"success": True, "data": {
        "similarity": round(similarity, 2),
        "threshold": threshold,
        "passed": passed,
        "message": "验证通过" if passed else "验证不通过 (相似度不足)"
    }})

@app.route("/api/identity/verify-all", methods=["POST"])
@login_required
def api_verify_all_customers():
    """Batch verify all customer ID cards."""
    customers = read_customers()
    results = []
    for c in customers:
        if not c.get("id_card"):
            continue
        valid, msg = validate_chinese_id_card(c["id_card"])
        results.append({
            "customer_id": c["id"], "customer_name": c["name"],
            "id_card": c["id_card"], "valid": valid, "message": msg
        })
    total = len(results)
    valid_count = sum(1 for r in results if r["valid"])
    return jsonify({"success": True, "total": total,
                    "valid_count": valid_count,
                    "invalid_count": total - valid_count,
                    "data": results})

# ============================================================
# MODULE 10: SYSTEM / DASHBOARD / MISC
# ============================================================

@app.route("/api/dashboard", methods=["GET"])
@login_required
def api_dashboard():
    """Overview statistics for the dashboard (admin/employee)."""
    employees = read_employees()
    active_employees = [e for e in employees if e.get("is_active", True)]
    customers = read_customers()
    active_customers = [c for c in customers if c.get("is_active", True)]
    cards = read_cards()
    active_cards = [c for c in cards if c.get("status") == "正常"]
    transactions = read_transactions()
    total_txn_amount = sum(t["amount"] for t in transactions if t.get("status") == "成功")

    # Queue status
    queue_vip_size = len(vip_queue)
    queue_normal_size = len(normal_queue)

    # Large transaction anomalies
    large_anomalies = [t for t in transactions if t["amount"] > LARGE_AMOUNT]
    anomaly_large_count = len(large_anomalies)

    # High-frequency anomalies (sliding window: >MULTI_TRANSACTION in same hour)
    cust_txns = {}
    for t in transactions:
        for key in [t.get("from_card",""), t.get("to_card",""), t.get("from",""), t.get("to","")]:
            if key and key != "系统" and key != "-":
                cust_txns.setdefault(key, []).append(t)
    anomaly_freq_count = 0
    for cust_id, txns in cust_txns.items():
        if len(txns) <= MULTI_TRANSACTION:
            continue
        txns_sorted = sorted(txns, key=lambda x: x["time"])
        for i in range(len(txns_sorted)):
            count = 1
            for j in range(i+1, len(txns_sorted)):
                if txns_sorted[i]["time"][:13] == txns_sorted[j]["time"][:13]:
                    count += 1
                else:
                    break
            if count > MULTI_TRANSACTION:
                anomaly_freq_count += 1
                break

    # Total daily interest
    total_interest = 0
    for card in active_cards:
        total_interest += calc_daily_interest(card)

    # Branch count
    branches = read_branches()
    branch_count = len(branches)

    # Today's transactions
    today = datetime.now().strftime("%Y-%m-%d")
    today_txns = [t for t in transactions if t["time"].startswith(today)]
    today_txn_total = sum(t["amount"] for t in today_txns if t.get("status") == "成功")

    # Transaction type summary
    txn_summary = {"存款": {"count": 0, "total": 0}, "取款": {"count": 0, "total": 0},
                   "转账": {"count": 0, "total": 0}, "贷款": {"count": 0, "total": 0},
                   "还款": {"count": 0, "total": 0}}
    for t in transactions:
        ttype = t.get("type", "")
        if ttype in txn_summary:
            txn_summary[ttype]["count"] += 1
            if t.get("status") == "成功":
                txn_summary[ttype]["total"] += t["amount"]

    # Credit rating distribution
    rating_dist = {}
    for c in active_customers:
        rating = get_credit_rating(c.get("credit_score", 600))
        rating_dist[rating] = rating_dist.get(rating, 0) + 1

    # Recent 5 transactions
    txns_sorted = sorted(transactions, key=lambda x: x["time"], reverse=True)
    recent_txns = txns_sorted[:5]

    return jsonify({"success": True, "data": {
        "employee_count": len(active_employees),
        "customer_count": len(active_customers),
        "card_count": len(active_cards),
        "transaction_count": len(transactions),
        "total_transaction_amount": round(total_txn_amount, 2),
        "vip_count": sum(1 for c in active_customers if c.get("type") == "VIP"),
        "normal_count": sum(1 for c in active_customers if c.get("type") != "VIP"),
        "queue_vip": queue_vip_size,
        "queue_normal": queue_normal_size,
        "anomaly_large_count": anomaly_large_count,
        "anomaly_freq_count": anomaly_freq_count,
        "total_interest_daily": round(total_interest, 2),
        "branch_count": branch_count,
        "today_txn_count": len(today_txns),
        "today_txn_total": round(today_txn_total, 2),
        "txn_summary": txn_summary,
        "rating_distribution": rating_dist,
        "recent_txns": recent_txns
    }})

@app.route("/api/dashboard/customer", methods=["GET"])
@login_required
def api_dashboard_customer():
    """Customer-specific dashboard aggregates."""
    customer_id = session.get("user", {}).get("id", "")
    cards = read_cards()
    transactions = read_transactions()

    # My cards
    my_cards = [c for c in cards if c.get("customer_id") == customer_id]
    total_balance = sum(c.get("balance", 0) for c in my_cards if c.get("status") == "正常")
    card_count = len([c for c in my_cards if c.get("status") == "正常"])

    # Daily interest for my cards
    my_interest = sum(calc_daily_interest(c) for c in my_cards if c.get("status") == "正常")

    # My transactions (check sdufe card IDs + legacy account ID + from/to fields)
    my_card_ids = [c["id"] for c in my_cards] + [customer_id]  # include legacy student ID
    my_txns = [t for t in transactions if
               t.get("from_card") in my_card_ids or t.get("to_card") in my_card_ids or
               t.get("from") == customer_id or t.get("to") == customer_id]
    my_txn_count = len(my_txns)

    # Recent 5
    my_txns_sorted = sorted(my_txns, key=lambda x: x["time"], reverse=True)
    recent_txns = my_txns_sorted[:5]

    # This month's transaction count
    this_month = datetime.now().strftime("%Y-%m")
    month_txns = [t for t in my_txns if t["time"].startswith(this_month)]

    # Investment allocation
    customer = None
    for c in read_customers():
        if c["id"] == customer_id:
            customer = c
            break
    cards_for_invest = read_cards()
    if customer:
        available_cash = sum(cd.get("balance", 0) for cd in cards_for_invest
                           if cd["customer_id"] == customer_id and cd["type"] != "信用卡")
        credit_score = customer.get("credit_score", 600)
        if credit_score > 750:
            risk = "进取型"
            allocation = {"股票型基金": round(available_cash * 0.4, 2),
                          "债券型基金": round(available_cash * 0.2, 2),
                          "定期存款": round(available_cash * 0.2, 2),
                          "活期备用": round(available_cash * 0.2, 2)}
        elif credit_score > 600:
            risk = "稳健型"
            allocation = {"债券型基金": round(available_cash * 0.35, 2),
                          "定期存款": round(available_cash * 0.35, 2),
                          "货币基金": round(available_cash * 0.15, 2),
                          "活期备用": round(available_cash * 0.15, 2)}
        else:
            risk = "保守型"
            allocation = {"定期存款": round(available_cash * 0.5, 2),
                          "货币基金": round(available_cash * 0.25, 2),
                          "活期备用": round(available_cash * 0.25, 2)}
    else:
        available_cash = total_balance
        risk = "保守型"
        allocation = {"定期存款": round(total_balance * 0.5, 2),
                      "货币基金": round(total_balance * 0.25, 2),
                      "活期备用": round(total_balance * 0.25, 2)}

    return jsonify({"success": True, "data": {
        "total_balance": round(total_balance, 2),
        "card_count": card_count,
        "my_cards": my_cards,
        "daily_interest": round(my_interest, 4),
        "transaction_count": my_txn_count,
        "month_txn_count": len(month_txns),
        "recent_txns": recent_txns,
        "available_cash": round(available_cash, 2),
        "risk_preference": risk,
        "allocation": allocation
    }})

@app.route("/api/accounts", methods=["GET"])
@login_required
def api_legacy_accounts():
    """Legacy: list all account.txt accounts (50 students)."""
    accounts = read_accounts()
    return jsonify({"success": True, "count": len(accounts), "data": accounts})

# ============================================================
# STARTUP
# ============================================================

def open_browser():
    webbrowser.open("http://127.0.0.1:5000")

if __name__ == "__main__":
    init_all_data()
    print("")
    print("  ╔══════════════════════════════════════════╗")
    print("  ║  银行智能管理系统 v3.0 Full-Stack      ║")
    print("  ║  C++ 10大模块 → Web全功能版            ║")
    print("  ╚══════════════════════════════════════════╝")
    print("")
    print("  前端: http://127.0.0.1:5000")
    print("  测试: 管理员 E001/000000 | 客户 C000001/123456")
    print("  学员登录: 202418440201/0201")
    print("")
    print("  按 Ctrl+C 停止")
    print("")
    threading.Timer(1.0, open_browser).start()
    app.run(host="127.0.0.1", port=5000, debug=False)
