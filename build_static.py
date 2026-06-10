import json

accounts = []
with open(r'C:\Users\Lenovo\Desktop\BankManagementSystem_clone\data\account.txt', 'r', encoding='utf-8') as f:
    for line in f:
        line = line.strip()
        if not line: continue
        parts = line.split('|')
        if len(parts) != 6: continue
        accounts.append({'id':parts[0],'name':parts[1],'password':parts[2],'balance':float(parts[3]),'is_locked':parts[4]=='1','is_admin':parts[5]=='1'})

tx_map = {'100001':'202418440201','100002':'202418440202','100003':'202418440203','100004':'202418440204'}
transactions = []
with open(r'C:\Users\Lenovo\Desktop\BankManagementSystem_clone\data\transaction.txt', 'r', encoding='utf-8') as f:
    for line in f:
        line = line.strip()
        if not line: continue
        parts = line.split('|')
        if len(parts) != 7: continue
        transactions.append({'tid':parts[0],'from':tx_map.get(parts[1],parts[1]),'to':tx_map.get(parts[2],parts[2]),'type':parts[3],'amount':float(parts[4]),'time':parts[5],'status':parts[6]})

acc_json = json.dumps(accounts, ensure_ascii=False)
tx_json = json.dumps(transactions, ensure_ascii=False)
print(f"Accounts: {len(accounts)}, Transactions: {len(transactions)}")

# Build complete HTML
html = f'''<!DOCTYPE html>
<html lang="zh-CN">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>银行智能管理系统 v2.1 | 第十组</title>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.2/css/all.min.css">
    <style>
        :root {{ --bank-blue: #0d6efd; --bank-deep-blue: #084298; --bank-light-blue: #eef5ff; --soft-gray: #f5f7fb; --text-main: #1f2937; }}
        * {{ box-sizing: border-box; }}
        body {{ margin: 0; min-height: 100vh; font-family: "Microsoft YaHei", Arial, sans-serif; background: linear-gradient(135deg, #eef5ff 0%, #f8fbff 45%, #e8f1ff 100%); color: var(--text-main); }}
        .hidden {{ display: none !important; }}
        .login-wrapper {{ min-height: 100vh; display: flex; align-items: center; justify-content: center; padding: 24px; }}
        .login-card {{ width: 100%; max-width: 460px; border: none; border-radius: 24px; overflow: hidden; box-shadow: 0 18px 45px rgba(13,110,253,0.18); }}
        .login-header {{ padding: 32px 32px 22px; background: linear-gradient(135deg, #0d6efd, #084298); color: #fff; }}
        .login-header .brand-icon {{ width: 58px; height: 58px; border-radius: 18px; display: flex; align-items: center; justify-content: center; font-size: 28px; background: rgba(255,255,255,0.18); margin-bottom: 16px; }}
        .login-body {{ background: #fff; padding: 30px 32px 34px; }}
        .form-control {{ min-height: 48px; border-radius: 14px; }}
        .btn-rounded {{ min-height: 48px; border-radius: 14px; font-weight: 600; }}
        .topbar {{ background: linear-gradient(135deg, #0d6efd, #084298); color: #fff; box-shadow: 0 8px 24px rgba(8,66,152,0.22); }}
        .navbar-brand {{ font-weight: 700; letter-spacing: 0.5px; }}
        .navbar .nav-link {{ color: rgba(255,255,255,0.9); border-radius: 12px; padding-left: 14px; padding-right: 14px; }}
        .navbar .nav-link:hover, .navbar .nav-link.active {{ color: #fff; background: rgba(255,255,255,0.16); }}
        .page-area {{ padding-top: 30px; padding-bottom: 48px; }}
        .hero-card, .info-card, .table-card, .about-card {{ border: none; border-radius: 22px; box-shadow: 0 12px 28px rgba(31,41,55,0.08); }}
        .hero-card {{ color: #fff; background: linear-gradient(135deg, #0d6efd, #3d8bfd); overflow: hidden; }}
        .hero-card .hero-icon {{ font-size: 84px; opacity: 0.2; }}
        .stat-card {{ height: 100%; border: none; border-radius: 20px; background: #fff; box-shadow: 0 12px 28px rgba(31,41,55,0.08); transition: transform 0.2s ease, box-shadow 0.2s ease; }}
        .stat-card:hover {{ transform: translateY(-3px); box-shadow: 0 16px 34px rgba(31,41,55,0.12); }}
        .stat-icon {{ width: 52px; height: 52px; border-radius: 16px; display: flex; justify-content: center; align-items: center; font-size: 22px; color: var(--bank-blue); background: var(--bank-light-blue); }}
        .section-title {{ font-weight: 700; margin-bottom: 18px; }}
        .section-subtitle {{ color: #6b7280; margin-bottom: 22px; }}
        .data-chip {{ display: inline-flex; align-items: center; gap: 6px; border-radius: 999px; padding: 6px 12px; background: #eef5ff; color: #0d6efd; font-size: 14px; font-weight: 600; }}
        .profile-row {{ padding: 14px 0; border-bottom: 1px solid #edf0f5; }}
        .profile-row:last-child {{ border-bottom: none; }}
        .table thead th {{ white-space: nowrap; background: #eef5ff; color: #084298; }}
        .table tbody td {{ vertical-align: middle; }}
        .status-success {{ background: #d1e7dd; color: #0f5132; }}
        .status-failed {{ background: #f8d7da; color: #842029; }}
        .page-section {{ animation: fadeIn 0.25s ease; }}
        @keyframes fadeIn {{ from {{ opacity: 0; transform: translateY(8px); }} to {{ opacity: 1; transform: translateY(0); }} }}
        .footer-note {{ color: #64748b; font-size: 14px; }}
        @media (max-width: 768px) {{ .login-header, .login-body {{ padding-left: 22px; padding-right: 22px; }} .page-area {{ padding-top: 22px; }} }}
    </style>
</head>
<body>

<section id="login_area" class="login-wrapper">
    <div class="card login-card">
        <div class="login-header">
            <div class="brand-icon"><i class="fa-solid fa-building-columns"></i></div>
            <h2 class="mb-2 fw-bold">银行智能管理系统</h2>
            <p class="mb-0 opacity-75">数据结构与算法课程设计 · 第十组 · GitHub Pages</p>
        </div>
        <div class="login-body">
            <div class="mb-4">
                <span class="data-chip"><i class="fa-solid fa-circle-info"></i> 纯前端 · 数据内嵌 · 即开即用</span>
            </div>
            <form id="login_form" novalidate>
                <div class="mb-3">
                    <label for="account_id" class="form-label fw-semibold">账号（学号）</label>
                    <input type="text" id="account_id" class="form-control" placeholder="请输入12位学号" maxlength="20">
                    <div class="invalid-feedback">账号必须为纯数字。</div>
                </div>
                <div class="mb-3">
                    <label for="password" class="form-label fw-semibold">密码（学号后4位）</label>
                    <input type="password" id="password" class="form-control" placeholder="请输入密码" maxlength="20">
                    <div class="invalid-feedback">密码必须为纯数字。</div>
                </div>
                <div id="login_message" class="alert alert-primary py-2 px-3 small hidden" role="alert"></div>
                <button type="submit" class="btn btn-primary btn-rounded w-100">
                    <i class="fa-solid fa-right-to-bracket me-2"></i> 登录
                </button>
            </form>
            <p class="small text-secondary mb-0 mt-4">
                账号为学号（12位），密码为学号后4位。管理员：尾号0218/0219/0221
            </p>
        </div>
    </div>
</section>

<section id="system_area" class="system-shell hidden">
    <nav class="navbar navbar-expand-lg navbar-dark topbar">
        <div class="container">
            <a class="navbar-brand" href="javascript:void(0);"><i class="fa-solid fa-building-columns me-2"></i>Bank v2.1</a>
            <button class="navbar-toggler" type="button" data-bs-toggle="collapse" data-bs-target="#main_nav">
                <span class="navbar-toggler-icon"></span>
            </button>
            <div class="collapse navbar-collapse" id="main_nav">
                <ul class="navbar-nav me-auto mb-2 mb-lg-0">
                    <li class="nav-item"><a class="nav-link active" href="javascript:void(0);" data-page="dashboard_page">首页仪表盘</a></li>
                    <li class="nav-item"><a class="nav-link" href="javascript:void(0);" data-page="account_page">账户管理</a></li>
                    <li class="nav-item"><a class="nav-link" href="javascript:void(0);" data-page="transaction_page">交易记录</a></li>
                    <li class="nav-item"><a class="nav-link" href="javascript:void(0);" data-page="about_page">系统说明</a></li>
                </ul>
                <div class="d-flex align-items-center gap-3">
                    <span id="top_user_text" class="small"></span>
                    <button id="logout_btn" class="btn btn-light btn-sm rounded-pill px-3"><i class="fa-solid fa-arrow-right-from-bracket me-1"></i>登出</button>
                </div>
            </div>
        </div>
    </nav>

    <main class="container page-area">
        <section id="dashboard_page" class="page-section">
            <div class="card hero-card mb-4">
                <div class="card-body p-4 p-lg-5">
                    <div class="row align-items-center">
                        <div class="col-lg-8">
                            <h1 class="fw-bold mb-3">银行智能管理系统</h1>
                            <p class="mb-3 fs-5">v2.1 · 纯前端单页应用 · 内嵌50人花名册 · GitHub Pages 托管</p>
                            <span class="badge rounded-pill text-bg-light text-primary px-3 py-2">第十组</span>
                        </div>
                        <div class="col-lg-4 text-lg-end mt-4 mt-lg-0"><i class="fa-solid fa-chart-line hero-icon"></i></div>
                    </div>
                </div>
            </div>
            <div class="row g-4">
                <div class="col-md-4"><div class="card stat-card"><div class="card-body p-4"><div class="d-flex align-items-center justify-content-between"><div><p class="text-secondary mb-2">总账户数</p><h2 id="account_count" class="fw-bold mb-0">0</h2></div><div class="stat-icon"><i class="fa-solid fa-users"></i></div></div></div></div></div>
                <div class="col-md-4"><div class="card stat-card"><div class="card-body p-4"><div class="d-flex align-items-center justify-content-between"><div><p class="text-secondary mb-2">交易记录数</p><h2 id="transaction_count" class="fw-bold mb-0">0</h2></div><div class="stat-icon"><i class="fa-solid fa-receipt"></i></div></div></div></div></div>
                <div class="col-md-4"><div class="card stat-card"><div class="card-body p-4"><div class="d-flex align-items-center justify-content-between"><div><p class="text-secondary mb-2">总交易额</p><h2 id="transaction_amount" class="fw-bold mb-0">&yen;0.00</h2></div><div class="stat-icon"><i class="fa-solid fa-sack-dollar"></i></div></div></div></div></div>
            </div>
            <div class="card info-card mt-4"><div class="card-body p-4"><h3 class="section-title">系统功能</h3><div class="row g-3"><div class="col-md-4"><div class="p-3 rounded-4 bg-light h-100"><h5 class="fw-bold"><i class="fa-solid fa-right-to-bracket me-2 text-primary"></i>登录验证</h5><p class="mb-0 text-secondary">学号+后4位密码登录，管理员/普通用户权限区分。</p></div></div><div class="col-md-4"><div class="p-3 rounded-4 bg-light h-100"><h5 class="fw-bold"><i class="fa-solid fa-database me-2 text-primary"></i>数据内嵌</h5><p class="mb-0 text-secondary">50人花名册数据直接内嵌页面，无需后端服务器。</p></div></div><div class="col-md-4"><div class="p-3 rounded-4 bg-light h-100"><h5 class="fw-bold"><i class="fa-solid fa-cloud me-2 text-primary"></i>GitHub Pages</h5><p class="mb-0 text-secondary">纯静态托管，全球 CDN 加速，打开即用。</p></div></div></div></div></div>
        </section>

        <section id="account_page" class="page-section hidden">
            <h2 class="section-title">账户管理</h2>
            <p class="section-subtitle">个人信息与全班账户列表。</p>
            <div class="row g-4">
                <div class="col-lg-4"><div class="card info-card h-100"><div class="card-body p-4"><h4 class="fw-bold mb-4"><i class="fa-solid fa-id-card me-2 text-primary"></i>当前登录账户</h4><div class="profile-row"><div class="text-secondary small">学号</div><div id="profile_account_id" class="fw-semibold">-</div></div><div class="profile-row"><div class="text-secondary small">姓名</div><div id="profile_name" class="fw-semibold">-</div></div><div class="profile-row"><div class="text-secondary small">余额</div><div id="profile_balance" class="fw-semibold">&yen;0.00</div></div><div class="profile-row"><div class="text-secondary small">状态</div><div id="profile_status" class="fw-semibold">-</div></div><div class="profile-row"><div class="text-secondary small">权限</div><div id="profile_role" class="fw-semibold">-</div></div></div></div></div>
                <div class="col-lg-8"><div class="card table-card"><div class="card-body p-4"><div class="d-flex flex-wrap justify-content-between align-items-center gap-2 mb-3"><h4 class="fw-bold mb-0"><i class="fa-solid fa-list me-2 text-primary"></i>全班账户列表</h4><span class="data-chip">密码已隐藏</span></div><div class="table-responsive" style="max-height:500px;overflow-y:auto;"><table class="table table-hover align-middle mb-0"><thead><tr><th>学号</th><th>姓名</th><th>余额</th><th>状态</th><th>权限</th></tr></thead><tbody id="account_table_body"><tr><td colspan="5" class="text-center text-secondary py-4">暂无数据</td></tr></tbody></table></div></div></div></div>
            </div>
        </section>

        <section id="transaction_page" class="page-section hidden">
            <h2 class="section-title">交易记录</h2>
            <p class="section-subtitle">当前账户的交易明细。</p>
            <div class="card table-card"><div class="card-body p-4"><div class="d-flex flex-wrap justify-content-between align-items-center gap-2 mb-3"><h4 class="fw-bold mb-0"><i class="fa-solid fa-money-check-dollar me-2 text-primary"></i>我的交易记录</h4><span id="my_transaction_count" class="data-chip">共 0 条</span></div><div class="table-responsive"><table class="table table-hover align-middle mb-0"><thead><tr><th>交易ID</th><th>类型</th><th>金额</th><th>交易对象</th><th>时间</th><th>状态</th></tr></thead><tbody id="transaction_table_body"><tr><td colspan="6" class="text-center text-secondary py-4">暂无交易记录</td></tr></tbody></table></div></div></div>
        </section>

        <section id="about_page" class="page-section hidden">
            <h2 class="section-title">系统说明</h2>
            <p class="section-subtitle">数据结构与算法课程设计 · 第十组</p>
            <div class="card about-card"><div class="card-body p-4"><div class="row g-4"><div class="col-md-6"><h4 class="fw-bold mb-3"><i class="fa-solid fa-code me-2 text-primary"></i>技术栈</h4><ul class="list-unstyled mb-0"><li><i class="fa-solid fa-check text-success me-2"></i> 前端：HTML5 + CSS3 + Bootstrap 5 + vanilla JS</li><li><i class="fa-solid fa-check text-success me-2"></i> 托管：GitHub Pages（全球CDN加速）</li><li><i class="fa-solid fa-check text-success me-2"></i> 数据：JSON内嵌（50人花名册）</li><li><i class="fa-solid fa-check text-success me-2"></i> C++核心：10大模块完整实现</li><li><i class="fa-solid fa-check text-success me-2"></i> 图标：Font Awesome 6</li></ul></div><div class="col-md-6"><h4 class="fw-bold mb-3"><i class="fa-solid fa-list-check me-2 text-primary"></i>功能模块</h4><ul class="list-unstyled mb-0"><li><i class="fa-solid fa-check text-success me-2"></i> 登录验证（学号+后4位密码）</li><li><i class="fa-solid fa-check text-success me-2"></i> 仪表盘（账户/交易统计）</li><li><i class="fa-solid fa-check text-success me-2"></i> 账户管理（个人信息+全班列表）</li><li><i class="fa-solid fa-check text-success me-2"></i> 交易记录（按账户筛选）</li><li><i class="fa-solid fa-check text-success me-2"></i> 管理员/普通用户权限区分</li></ul></div></div><hr class="my-4"><div class="text-center footer-note"><p class="mb-0">&copy; 2025-2026 银行智能管理系统 · 第十组 · GitHub Pages</p></div></div></div>
        </section>
    </main>
</section>

<script src="https://cdn.jsdelivr.net/npm/@popperjs/core@2.11.8/dist/umd/popper.min.js"></script>
<script src="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/js/bootstrap.min.js"></script>
<script>
var ACCOUNTS = {acc_json};
var TRANSACTIONS = {tx_json};

var currentUser = null;
var loginForm = document.getElementById('login_form');
var loginArea = document.getElementById('login_area');
var systemArea = document.getElementById('system_area');
var loginMessage = document.getElementById('login_message');
var accountIdInput = document.getElementById('account_id');
var passwordInput = document.getElementById('password');
var logoutBtn = document.getElementById('logout_btn');
var topUserText = document.getElementById('top_user_text');
var navLinks = document.querySelectorAll('.nav-link[data-page]');
var pageSections = document.querySelectorAll('.page-section');

function showMessage(el, text, type) {{
    type = type || 'primary';
    el.textContent = text;
    el.classList.remove('hidden');
    el.className = 'alert alert-' + type + ' py-2 px-3 small';
    clearTimeout(el._msgTimer);
    el._msgTimer = setTimeout(function() {{ el.classList.add('hidden'); }}, 4000);
}}

function formatMoney(amount) {{
    return '&yen;' + Number(amount || 0).toFixed(2);
}}

function statusBadge(isLocked) {{
    return isLocked ? '<span class="badge bg-danger">已锁定</span>' : '<span class="badge bg-success">正常</span>';
}}

function roleBadge(isAdmin) {{
    return isAdmin ? '<span class="badge bg-primary">管理员</span>' : '<span class="badge bg-secondary">普通用户</span>';
}}

function txStatusBadge(status) {{
    return status === '成功' ? '<span class="badge status-success">成功</span>' : '<span class="badge status-failed">失败</span>';
}}

loginForm.addEventListener('submit', function(e) {{
    e.preventDefault();
    var accountId = accountIdInput.value.trim();
    var password = passwordInput.value.trim();
    var isAccountValid = /^\\d{{4,}}$/.test(accountId);
    var isPasswordValid = /^\\d{{4,}}$/.test(password);

    if (!isAccountValid) {{ accountIdInput.classList.add('is-invalid'); return; }}
    accountIdInput.classList.remove('is-invalid');
    if (!isPasswordValid) {{ passwordInput.classList.add('is-invalid'); return; }}
    passwordInput.classList.remove('is-invalid');

    var user = null;
    for (var i = 0; i < ACCOUNTS.length; i++) {{
        if (ACCOUNTS[i].id === accountId && ACCOUNTS[i].password === password) {{
            user = ACCOUNTS[i];
            break;
        }}
    }}

    if (!user) {{
        showMessage(loginMessage, '账号或密码错误，请重试！', 'danger');
        return;
    }}

    if (user.is_locked) {{
        showMessage(loginMessage, '账户已锁定，请联系管理员', 'danger');
        return;
    }}

    currentUser = user;
    loginArea.classList.add('hidden');
    systemArea.classList.remove('hidden');
    topUserText.textContent = '当前登录：' + user.name + ' (' + user.id + ')';
    renderDashboard();
    renderAccountPage();
    renderTransactionPage();
    showMessage(loginMessage, '登录成功！欢迎 ' + user.name, 'success');
}});

accountIdInput.addEventListener('input', function() {{ if (/^\\d{{4,}}$/.test(this.value.trim())) this.classList.remove('is-invalid'); }});
passwordInput.addEventListener('input', function() {{ if (/^\\d{{4,}}$/.test(this.value.trim())) this.classList.remove('is-invalid'); }});

logoutBtn.addEventListener('click', function() {{
    currentUser = null;
    systemArea.classList.add('hidden');
    loginArea.classList.remove('hidden');
    loginForm.reset();
    showMessage(loginMessage, '已成功登出', 'info');
}});

navLinks.forEach(function(link) {{
    link.addEventListener('click', function() {{
        navLinks.forEach(function(l) {{ l.classList.remove('active'); }});
        this.classList.add('active');
        var targetPage = this.getAttribute('data-page');
        pageSections.forEach(function(s) {{ s.classList.add('hidden'); if (s.id === targetPage) s.classList.remove('hidden'); }});
    }});
}});

function renderDashboard() {{
    document.getElementById('account_count').textContent = ACCOUNTS.length;
    document.getElementById('transaction_count').textContent = TRANSACTIONS.length;
    var total = TRANSACTIONS.reduce(function(s, t) {{ return s + (Number(t.amount) || 0); }}, 0);
    document.getElementById('transaction_amount').textContent = formatMoney(total);
}}

function renderAccountPage() {{
    if (!currentUser) return;
    document.getElementById('profile_account_id').textContent = currentUser.id;
    document.getElementById('profile_name').textContent = currentUser.name;
    document.getElementById('profile_balance').textContent = formatMoney(currentUser.balance);
    document.getElementById('profile_status').innerHTML = statusBadge(currentUser.is_locked);
    document.getElementById('profile_role').innerHTML = roleBadge(currentUser.is_admin);
    var tbody = document.getElementById('account_table_body');
    tbody.innerHTML = '';
    ACCOUNTS.forEach(function(acc) {{
        var tr = document.createElement('tr');
        tr.innerHTML = '<td>' + acc.id + '</td><td>' + acc.name + '</td><td>' + formatMoney(acc.balance) + '</td><td>' + statusBadge(acc.is_locked) + '</td><td>' + roleBadge(acc.is_admin) + '</td>';
        tbody.appendChild(tr);
    }});
}}

function renderTransactionPage() {{
    if (!currentUser) return;
    var tbody = document.getElementById('transaction_table_body');
    var countEl = document.getElementById('my_transaction_count');
    var myTx = TRANSACTIONS.filter(function(tx) {{ return tx.from === currentUser.id || tx.to === currentUser.id; }});
    countEl.textContent = '共 ' + myTx.length + ' 条';
    tbody.innerHTML = '';
    if (myTx.length === 0) {{ tbody.innerHTML = '<tr><td colspan="6" class="text-center text-secondary py-4">暂无交易记录</td></tr>'; return; }}
    myTx.forEach(function(tx) {{
        var icon = tx.type === '存款' ? 'arrow-down' : tx.type === '取款' ? 'arrow-up' : 'arrow-right-left';
        var target = tx.type === '存款' ? tx.from : tx.to;
        var tr = document.createElement('tr');
        tr.innerHTML = '<td>' + tx.tid + '</td><td><i class="fa-solid fa-' + icon + ' text-primary me-1"></i>' + tx.type + '</td><td>' + formatMoney(tx.amount) + '</td><td>' + target + '</td><td>' + tx.time + '</td><td>' + txStatusBadge(tx.status) + '</td>';
        tbody.appendChild(tr);
    }});
}}
</script>
</body>
</html>'''

output_path = r'C:\Users\Lenovo\Desktop\BankManagementSystem_clone\index.html'
with open(output_path, 'w', encoding='utf-8') as f:
    f.write(html)

print(f'Written: {output_path}')
print(f'Size: {len(html.encode("utf-8"))} bytes')
