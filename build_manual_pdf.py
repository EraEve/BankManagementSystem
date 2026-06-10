# -*- coding: utf-8 -*-
"""USER_MANUAL.md → PDF 转换脚本 (fpdf2 + 中文字体)"""
import os, re, textwrap

from fpdf import FPDF
from fpdf.enums import XPos, YPos

SRC = os.path.expanduser("~/Desktop/BankManagementSystem_clone/USER_MANUAL.md")
DST = os.path.expanduser("~/Desktop/银行智能管理系统_使用说明文档.pdf")

# ── 中文字体注册 ──────────────────────────────────
FONT_DIR = "C:/Windows/Fonts"
FONT_HEI  = os.path.join(FONT_DIR, "simhei.ttf")   # 黑体 → 标题
FONT_SONG = os.path.join(FONT_DIR, "simsun.ttc")    # 宋体 → 正文
FONT_KAI  = os.path.join(FONT_DIR, "simkai.ttf")    # 楷体 → 代码/表格

class ManualPDF(FPDF):
    def __init__(self):
        super().__init__("P", "mm", "A4")
        self.add_font("Hei",  "", FONT_HEI)
        self.add_font("Song", "", FONT_SONG)
        self.add_font("Kai",  "", FONT_KAI)
        self.set_auto_page_break(True, 18)
        self._in_code = False
        self._in_table = False
        self._table_data = []

    # ── helpers ────────────────────────────────────
    def clean(self, txt):
        """Replace glyphs missing in SimSun font"""
        return (txt.replace('✓','[OK]').replace('✗','[X]')
                    .replace('⚠','[!]').replace('⚡','[!!]')
                    .replace('−','-'))

    def h1(self, txt):
        txt = self.clean(txt)
        self.ln(6)
        self.set_font("Hei", "", 20)
        self.set_text_color(8, 66, 152)
        self.cell(0, 12, txt, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.line(self.l_margin, self.y + 1, self.w - self.r_margin, self.y + 1)
        self.ln(5)

    def h2(self, txt):
        txt = self.clean(txt)
        self.ln(3)
        self.set_font("Hei", "", 15)
        self.set_text_color(13, 110, 253)
        self.cell(0, 9, txt, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.set_draw_color(13, 110, 253)
        self.line(self.l_margin, self.y + 1, self.l_margin + 60, self.y + 1)
        self.ln(3)

    def h3(self, txt):
        txt = self.clean(txt)
        self.ln(2)
        self.set_font("Hei", "", 12)
        self.set_text_color(31, 41, 55)
        self.cell(0, 7, txt, new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(1)

    def body(self, txt):
        if not txt.strip():
            self.ln(3)
            return
        self.set_font("Song", "", 10.5)
        self.set_text_color(31, 41, 55)
        txt = self.clean(txt)
        parts = re.split(r'(\*\*[^*]+\*\*)', txt)
        for p in parts:
            if p.startswith("**") and p.endswith("**"):
                self.set_font("Hei", "", 10.5)
                self.write(8, p[2:-2])
                self.set_font("Song", "", 10.5)
            else:
                self.write(8, p)
        self.ln()

    def code_block(self, lines):
        self.set_font("Kai", "", 9)
        self.set_text_color(51, 51, 51)
        self.set_fill_color(245, 247, 251)
        self.ln(1)
        for l in lines:
            clean_l = self.clean(l[:100])
            self.cell(self.w - self.l_margin - self.r_margin, 5.5, clean_l,
                      new_x=XPos.LMARGIN, new_y=YPos.NEXT, fill=True)
        self.set_text_color(31, 41, 55)
        self.ln(2)

    def table_row(self, cells, is_header=False):
        self.set_font("Hei" if is_header else "Song", "", 9)
        col_w = (self.w - self.l_margin - self.r_margin) / len(cells)
        max_h = 0
        for c in cells:
            lines = self.multi_cell(col_w, 5.5, c, split_only=True)
            h = len(lines) * 5.5
            if h > max_h: max_h = h
        y_before = self.y
        for i, c in enumerate(cells):
            x = self.l_margin + i * col_w
            self.set_xy(x, y_before)
            if is_header:
                self.set_fill_color(8, 66, 152)
                self.set_text_color(255, 255, 255)
            else:
                self.set_fill_color(250, 250, 252)
                self.set_text_color(31, 41, 55)
            self.cell(col_w, max_h, c, border=1, fill=True)
        self.set_y(y_before + max_h)
        self.set_text_color(31, 41, 55)

    def bullet(self, txt):
        txt = self.clean(txt)
        self.set_font("Song", "", 10.5)
        self.set_text_color(31, 41, 55)
        self.cell(6, 7, "•")
        self.write(7, txt)
        self.ln()

    def hr(self):
        self.ln(2)
        self.set_draw_color(200, 200, 200)
        self.line(self.l_margin, self.y, self.w - self.r_margin, self.y)
        self.ln(2)

    def conver_title(self):
        self.add_page()
        self.set_font("Hei", "", 24)
        self.set_text_color(8, 66, 152)
        self.cell(0, 12, "银行智能管理系统", align="C",
                  new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.set_font("Hei", "", 16)
        self.cell(0, 10, "使用说明文档 v2.2", align="C",
                  new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(4)
        self.set_font("Song", "", 10)
        self.set_text_color(100, 100, 100)
        self.cell(0, 7, "数据结构与算法课程设计 · 第十组", align="C",
                  new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.cell(0, 7, "2026-06-10  |  GitHub: EraEve/BankManagementSystem", align="C",
                  new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.ln(8)
        self.set_text_color(31, 41, 55)

    # ── TOC skeleton ───────────────────────────────
    def write_toc(self, toc_lines):
        self.set_font("Song", "", 10)
        for l in toc_lines:
            m = re.match(r'^\d+\.\s+(.+?)(\d+)?$', l)
            if m:
                indent = 0
                if l.startswith("    "): indent = 8
                self.cell(indent, 6, "")
                self.cell(0, 6, l.strip(), new_x=XPos.LMARGIN, new_y=YPos.NEXT)
        self.add_page()


def parse_and_build():
    with open(SRC, "r", encoding="utf-8") as f:
        lines = f.readlines()

    pdf = ManualPDF()
    pdf.set_margin(20)

    # ── Title page ─────────────────────────────────
    pdf.conver_title()

    # ── Parse markdown ─────────────────────────────
    i = 0
    toc_lines = []
    in_toc = False
    in_code = False
    code_buf = []
    in_table = False
    table_buf = []
    after_title = False
    past_title_block = False
    skip_until_body = False

    while i < len(lines):
        line = lines[i].rstrip()

        # Detect end of title block
        if line.startswith("# 银行智能管理系统") and not past_title_block:
            past_title_block = True
            while i < len(lines) and not lines[i].startswith("---"):
                i += 1
            i += 1
            continue

        if not past_title_block:
            i += 1
            continue

        # Skip the first --- after title
        if line == "---" and not after_title:
            after_title = True
            i += 1
            while i < len(lines) and lines[i].strip() == "":
                i += 1
            continue

        # TOC
        if line.startswith("## 目录") or line.startswith("## Table of Contents"):
            in_toc = True
            i += 1
            continue
        if in_toc:
            if line.strip() == "":
                i += 1
                continue
            if line.startswith("## "):
                in_toc = False
                pdf.write_toc(toc_lines)
                continue
            if line.strip().startswith("- ["):
                toc_lines.append(line.strip()[3:].replace("](#", " — "))
            i += 1
            continue

        # Skip code blocks - not needed for TOC
        if line.startswith("```") and not in_code:
            in_code = True
            code_buf = []
            i += 1
            continue
        if in_code:
            if line.startswith("```"):
                in_code = False
                pdf.code_block(code_buf)
                i += 1
                continue
            code_buf.append(line)
            i += 1
            continue

        # Headers
        if line.startswith("## "):
            if line.startswith("### "):
                pdf.h3(line[4:].strip())
            else:
                txt = line[3:].strip()
                # remove leading number like "1. " or "10. "
                txt = re.sub(r'^\d+\.\s*', '', txt)
                # remove "模块" prefix for cleaner h2
                if txt.startswith("模块"):
                    txt = txt
                pdf.h2(txt)
            i += 1
            continue

        # Tables
        if "|" in line and line.strip().startswith("|"):
            if not in_table:
                in_table = True
                table_buf = []
            table_buf.append(line)
            i += 1
            continue
        if in_table:
            # flush table
            in_table = False
            rows = []
            for r in table_buf:
                if re.match(r'^\|[\s\-:|]+\|$', r): continue  # separator row
                cells = [c.strip() for c in r.split("|")[1:-1]]
                rows.append(cells)
            for ri, cells in enumerate(rows):
                pdf.table_row(cells, is_header=(ri == 0))
            pdf.ln(3)
            table_buf = []
            # don't skip current line, process it normally
            continue

        # Empty lines
        if line.strip() == "":
            pdf.body("")
            i += 1
            continue

        # Bold-only or simple text
        if line.startswith("**") and line.endswith("**"):
            pdf.set_font("Hei", "", 11)
            pdf.cell(0, 7, line[2:-2], new_x=XPos.LMARGIN, new_y=YPos.NEXT)
            i += 1
            continue

        # Horizontal rules
        if line.strip() == "---":
            pdf.hr()
            i += 1
            continue

        # Bullets
        if line.strip().startswith("- ") or line.strip().startswith("* "):
            pdf.bullet(line.strip()[2:])
            i += 1
            continue

        # Numbered list
        if re.match(r'^\d+\.\s', line.strip()):
            pdf.bullet(line.strip())
            i += 1
            continue

        # Blockquotes (skip >)
        txt = line.strip()
        if txt.startswith("> "):
            txt = txt[2:]
        if txt.startswith(">"):
            txt = txt[1:]

        # Regular body text - skip if empty or looks like navigation
        if txt and not txt.startswith("```") and not txt.startswith("<!--"):
            pdf.body(txt)
        i += 1

    # ── Footer ─────────────────────────────────────
    pdf.ln(10)
    pdf.set_font("Song", "", 9)
    pdf.set_text_color(130, 130, 130)
    pdf.cell(0, 6, "— 文档结束 —", align="C", new_x=XPos.LMARGIN, new_y=YPos.NEXT)
    pdf.cell(0, 6, "银行智能管理系统 v2.2  |  第十组  |  2026-06-10", align="C",
             new_x=XPos.LMARGIN, new_y=YPos.NEXT)

    pdf.output(DST)
    return DST, pdf.pages_count

if __name__ == "__main__":
    path, pages = parse_and_build()
    print(f"[OK] PDF generated: {path}")
    print(f"  Pages: {pages}")
    size_kb = os.path.getsize(path) / 1024
    print(f"  Size: {size_kb:.0f} KB")
