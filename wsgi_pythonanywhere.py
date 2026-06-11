"""
WSGI entry point for PythonAnywhere deployment.
=================================================
Place this file at: /var/www/eraeve_pythonanywhere_com_wsgi.py
(or reference it from the PythonAnywhere Web tab → WSGI configuration file)

On PythonAnywhere:
  1. Web tab → Code: set source directory to /home/eraeve/BankManagementSystem
  2. Web tab → WSGI configuration file: point to this file
  3. Web tab → Virtualenv: /home/eraeve/.virtualenvs/bankenv
  4. Reload the web app after pulling new code

The PythonAnywhere environment automatically sets:
  - PYTHONANYWHERE_SITE = 'eraeve.pythonanywhere.com'
  - PYTHONANYWHERE_DOMAIN = 'pythonanywhere.com'
"""

import sys
import os

# --- Project directory ---
project_home = '/home/eraeve/BankManagementSystem'
if project_home not in sys.path:
    sys.path.insert(0, project_home)

# Set working directory (for data/ file I/O)
os.chdir(project_home)

# --- Import Flask app ---
from app import app as application
