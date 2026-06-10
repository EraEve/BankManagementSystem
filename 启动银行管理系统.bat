@echo off
chcp 65001 >nul
title 银行智能管理系统 v2.1 — 第十组

cd /d "%~dp0"

echo.
echo   ╔══════════════════════════════════════════╗
echo   ║     🏦 银行智能管理系统  v2.1          ║
echo   ║     数据结构与算法课程设计 · 第十组      ║
echo   ╚══════════════════════════════════════════╝
echo.

echo [1/3] 检查 Python 环境...
where python >nul 2>&1
if %errorlevel% neq 0 (
    echo   ❌ 未找到 Python！请先安装 Python 3.x
    echo   下载地址：https://www.python.org/downloads/
    pause
    exit /b 1
)
echo   ✅ Python 已就绪

echo [2/3] 安装依赖...
pip install Flask flask-cors -q 2>&1
echo   ✅ 依赖已就绪

echo [3/3] 启动 Flask 后端...
echo.
echo   ┌─────────────────────────────────────────┐
echo   │  🔑 测试账号                            │
echo   │  100001 / 123456  (张三, 管理员)        │
echo   │  100002 / 654321  (李四, 管理员)        │
echo   │  888888 / 888888  (管理员)              │
echo   │  📍 浏览器将在 1 秒后自动打开            │
echo   │  🛑 关闭本窗口即可停止服务               │
echo   └─────────────────────────────────────────┘
echo.

python app.py
pause
