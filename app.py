from flask import Flask, jsonify, request
from flask_cors import CORS
import os

app = Flask(__name__)
CORS(app)

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
ACCOUNT_FILE = os.path.join(BASE_DIR, "data", "account.txt")
TRANSACTION_FILE = os.path.join(BASE_DIR, "data", "transaction.txt")

@app.route('/')
def home():
    return "✅ 银行智能管理系统后端运行成功！<br>接口正常工作～"

def text_to_bool(value):
    return value == "1"

def safe_float(value):
    try:
        return float(value)
    except ValueError:
        return 0.0

def read_accounts(include_password=False):
    accounts = []

    if not os.path.exists(ACCOUNT_FILE):
        return accounts

    with open(ACCOUNT_FILE, "r", encoding="utf-8") as file:
        for line in file:
            line = line.strip()

            if not line:
                continue

            parts = line.split("|")

            if len(parts) != 6:
                continue

            account = {
                "account_id": parts[0],
                "name": parts[1],
                "balance": safe_float(parts[3]),
                "is_locked": text_to_bool(parts[4]),
                "is_admin": text_to_bool(parts[5])
            }

            if include_password:
                account["password"] = parts[2]

            accounts.append(account)

    return accounts

def read_transactions():
    transactions = []

    if not os.path.exists(TRANSACTION_FILE):
        return transactions

    with open(TRANSACTION_FILE, "r", encoding="utf-8") as file:
        for line in file:
            line = line.strip()

            if not line:
                continue

            parts = line.split("|")

            if len(parts) != 7:
                continue

            transaction = {
                "transaction_id": parts[0],
                "from_account": parts[1],
                "to_account": parts[2],
                "type": parts[3],
                "amount": safe_float(parts[4]),
                "time": parts[5],
                "status": parts[6]
            }

            transactions.append(transaction)

    return transactions

def find_account(account_id, include_password=False):
    accounts = read_accounts(include_password=include_password)

    for account in accounts:
        if account["account_id"] == account_id:
            return account

    return None

@app.route("/api/accounts", methods=["GET"])
def get_accounts():
    accounts = read_accounts(include_password=False)

    return jsonify({
        "success": True,
        "count": len(accounts),
        "data": accounts
    })

@app.route("/api/account/<account_id>", methods=["GET"])
def get_account_detail(account_id):
    account = find_account(account_id, include_password=False)

    if account is None:
        return jsonify({
            "success": False,
            "message": "账户不存在"
        }), 404

    return jsonify({
        "success": True,
        "data": account
    })

@app.route("/api/transactions", methods=["GET"])
def get_transactions():
    transactions = read_transactions()

    return jsonify({
        "success": True,
        "count": len(transactions),
        "data": transactions
    })

@app.route("/api/transactions/<account_id>", methods=["GET"])
def get_account_transactions(account_id):
    transactions = read_transactions()
    result = []

    for transaction in transactions:
        if (transaction["from_account"] == account_id or
                transaction["to_account"] == account_id):
            result.append(transaction)

    return jsonify({
        "success": True,
        "count": len(result),
        "data": result
    })

@app.route("/api/login", methods=["POST"])
def login():
    form_data = request.get_json(silent=True) or {}

    account_id = str(form_data.get("account_id", "")).strip()
    password = str(form_data.get("password", "")).strip()

    if len(account_id) != 6 or not account_id.isdigit():
        return jsonify({
            "success": False,
            "message": "账号格式错误，账号必须是6位数字"
        }), 400

    if len(password) != 6 or not password.isdigit():
        return jsonify({
            "success": False,
            "message": "密码格式错误，密码必须是6位数字"
        }), 400

    account = find_account(account_id, include_password=True)

    if account is None:
        return jsonify({
            "success": False,
            "message": "账户不存在"
        }), 404

    if account["is_locked"]:
        return jsonify({
            "success": False,
            "message": "账户已锁定，请联系后续管理员模块处理"
        }), 403

    if account["password"] != password:
        return jsonify({
            "success": False,
            "message": "密码错误"
        }), 401

    account.pop("password", None)

    return jsonify({
        "success": True,
        "message": "登录成功",
        "data": account
    })

# -------------------- 【修复 2：自动创建 data 文件夹，避免报错】 --------------------
if not os.path.exists(os.path.join(BASE_DIR, "data")):
    os.makedirs(os.path.join(BASE_DIR, "data"))

if __name__ == "__main__":
    app.run(host="127.0.0.1", port=5000, debug=True)