/**
 * identity_verifier.h — 需求8补充: 身份核验·生物识别·影像读取
 *
 * 功能:
 *   1. 身份证号码校验 (中国18位, ISO 7064:1983 MOD 11-2)
 *   2. 身份证信息提取与核验 (出生日期/性别/省份/年龄)
 *   3. 生物识别模拟 (指纹/人脸特征模板生成与比对)
 *   4. 证件影像读取 (证件照/人脸照存储, 模拟OCR)
 *
 * 算法:
 *   - ISO 7064 MOD 11-2 校验位算法
 *   - 生物特征模板: 身份证号多项式哈希 → 模拟特征向量
 *   - 特征比对: 模板字符串相似度评分
 *
 * 数据结构:
 *   - 客户身份证字段: id_card, id_photo_path, biometric_data, face_photo_path
 *   - 文件持久化: 追加在 customer.txt 管道分隔符末尾
 */

#ifndef IDENTITY_VERIFIER_H
#define IDENTITY_VERIFIER_H

#include "common.h"

// ==================== 1. 身份证核验 ====================

/** 完整身份证校验报告 */
inline void verify_id_card_full() {
    cout << "\n  ═══ 身份证校验 ═══\n";
    cout << "  请输入18位身份证号码: ";
    string id; getline(cin, id);

    hr();
    cout << "  ═══ 身份证校验报告 ═══\n";
    hr();

    if (id.length() != 18) {
        cout << "  ✗ 格式错误: 身份证必须为18位!\n";
        cout << "     当前输入长度: " << id.length() << " 位\n";
        return;
    }

    // 前17位数字检查
    bool all_digit = true;
    for (int i = 0; i < 17; i++) {
        if (id[i] < '0' || id[i] > '9') {
            cout << "  ✗ 第" << (i+1) << "位 '" << id[i] << "' 不是数字!\n";
            all_digit = false;
        }
    }
    if (!all_digit) return;

    // 第18位检查
    char last = id[17];
    if (!((last >= '0' && last <= '9') || last == 'X' || last == 'x')) {
        cout << "  ✗ 第18位校验码格式错误: '" << last << "' (应为0-9或X)\n";
        return;
    }

    // 行政区划
    cout << "  行政区划: " << get_province_by_code(id) << endl;

    // 出生日期
    string birth = extract_birth_from_id(id);
    cout << "  出生日期: " << birth << endl;

    // 性别
    cout << "  性    别: " << extract_gender_from_id(id) << endl;

    // 年龄
    int age = extract_age_from_id(id);
    cout << "  年    龄: " << age << " 岁" << endl;

    // 校验位
    bool valid = validate_chinese_id_card(id);
    cout << "  ─────────────────────\n";
    if (valid) {
        cout << "  ✓ 校验位正确 — 身份证号码合法!\n";
    } else {
        // 计算正确校验位
        int weights[17] = {7,9,10,5,8,4,2,1,6,3,7,9,10,5,8,4,2};
        char check_map[11] = {'1','0','X','9','8','7','6','5','4','3','2'};
        int sum = 0;
        for (int i = 0; i < 17; i++) sum += (id[i] - '0') * weights[i];
        char expected = check_map[sum % 11];
        cout << "  ✗ 校验位错误! 第18位应为 '" << expected << "', 当前为 '" << last << "'\n";
    }
    hr();
}

/** 核验客户身份证与已存储信息 */
inline void verify_customer_identity() {
    cout << "\n  ═══ 客户身份核验 ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }

    const Customer& c = g_customers[idx];

    hr();
    cout << "  ═══ 身份核验报告 ═══\n";
    hr();
    cout << "  客户: " << c.name << " (" << c.id << ")\n";
    cout << "  存储身份证号: " << c.id_card << endl;

    if (c.id_card.empty()) {
        cout << "  ⚠ 该客户未录入身份证号!\n";
        hr();
        return;
    }

    // 校验格式
    bool fmt_ok = validate_chinese_id_card(c.id_card);
    cout << "  格式校验: " << (fmt_ok ? "✓ 合法" : "✗ 不合法") << endl;
    cout << "  省份归属: " << get_province_by_code(c.id_card) << endl;
    cout << "  出生日期: " << extract_birth_from_id(c.id_card) << endl;
    cout << "  性别推断: " << extract_gender_from_id(c.id_card) << endl;

    // 一致性检查(如果姓名非空)
    cout << "  ─────────────────────\n";
    int score = 0;
    if (fmt_ok) score += 40;
    if (!c.name.empty()) {
        cout << "  姓名验证: " << c.name << " — 已录入 (需人工核验)\n";
        score += 20;
    }
    if (!c.phone.empty()) {
        cout << "  电话验证: " << c.phone << " — 已录入\n";
        score += 15;
    }
    if (!c.address.empty()) {
        cout << "  地址验证: " << c.address << " — 已录入\n";
        score += 10;
    }
    if (!c.id_photo_path.empty()) {
        cout << "  证件照:    " << c.id_photo_path << " — 已存档\n";
        score += 10;
    }
    if (!c.biometric_data.empty()) {
        cout << "  生物特征:  已录入\n";
        score += 5;
    }

    cout << "  ─────────────────────\n";
    cout << "  核验总分: " << score << "/100";
    if (score >= 80) cout << " ✓ 通过";
    else if (score >= 60) cout << " ⚡ 需要补充材料";
    else cout << " ✗ 不通过";
    cout << endl;
    hr();
}

/** 批量核验所有客户身份证 */
inline void batch_verify_id_cards() {
    hr();
    cout << "  ═══ 全行客户身份证批量核验 ═══\n";
    hr();
    int total = 0, valid = 0, invalid = 0, missing = 0;
    cout << "  " << left << setw(10) << "客户编号" << setw(10) << "姓名"
         << setw(22) << "身份证号" << setw(10) << "校验结果" << endl;
    hr();
    for (size_t i = 0; i < g_customers.size(); i++) {
        const Customer& c = g_customers[i];
        if (!c.is_active) continue;
        total++;
        if (c.id_card.empty()) {
            cout << "  " << left << setw(10) << c.id << setw(10) << c.name
                 << setw(22) << "(未录入)" << setw(10) << "⚠ 缺失" << endl;
            missing++;
        } else if (validate_chinese_id_card(c.id_card)) {
            cout << "  " << left << setw(10) << c.id << setw(10) << c.name
                 << setw(22) << c.id_card << setw(10) << "✓ 合法" << endl;
            valid++;
        } else {
            cout << "  " << left << setw(10) << c.id << setw(10) << c.name
                 << setw(22) << c.id_card << setw(10) << "✗ 不合法" << endl;
            invalid++;
        }
    }
    hr();
    cout << "  总计: " << total << " 人 | 合法: " << valid
         << " | 不合法: " << invalid << " | 未录入: " << missing << endl;
    hr();
}

// ==================== 2. 生物识别管理 (模拟) ====================

/** 生物特征录入 (模拟指纹/人脸特征采集) */
inline void enroll_biometric() {
    cout << "\n  ═══ 生物特征录入 (模拟) ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    Customer& c = g_customers[idx];

    if (c.id_card.empty()) {
        cout << "  ⚠ 该客户未录入身份证号, 请先录入身份证号!\n";
        cout << "  身份证号: "; getline(cin, c.id_card);
        if (!validate_chinese_id_card(c.id_card)) {
            cout << "  ✗ 身份证号格式不合法! 特征录入取消。\n"; return;
        }
    }

    cout << "\n  ═══ 选择录入方式 ═══\n";
    cout << "  1. 指纹特征采集 (模拟)\n";
    cout << "  2. 人脸特征采集 (模拟)\n";
    cout << "  3. 指纹+人脸双模态采集 (模拟)\n";
    cout << "  请选择: "; string ch; getline(cin, ch);

    bool success = false;
    if (ch == "1") {
        // 模拟指纹采集
        cout << "\n  ▶ 请在指纹传感器上放置手指...\n";
        cout << "  ▶ 正在采集指纹特征点...\n";
        // 模拟处理延迟
        cout << "  ▶ 特征点提取: 纹路端点=24, 分叉点=18, 中心点=1\n";
        cout << "  ▶ 生成指纹模板...\n";
        string template_data = generate_biometric_template(c.id_card + "_FP");
        c.biometric_data = "FP:" + template_data;
        success = true;
    } else if (ch == "2") {
        // 模拟人脸采集
        cout << "\n  ▶ 请面向摄像头, 保持正脸姿态...\n";
        cout << "  ▶ 正在采集人脸特征点...\n";
        cout << "  ▶ 特征点提取: 左眼=12点, 右眼=12点, 鼻尖=1点, 嘴角=2点, 轮廓=21点\n";
        cout << "  ▶ 生成人脸特征模板...\n";
        string template_data = generate_biometric_template(c.id_card + "_FACE");
        c.biometric_data = "FACE:" + template_data;
        // 同时记录人脸照片路径
        c.face_photo_path = "data/face_" + c.id + ".jpg";
        success = true;
    } else if (ch == "3") {
        // 双模态
        cout << "\n  ▶ 先采集指纹...\n";
        cout << "  ▶ 指纹特征点: 纹路端点=24, 分叉点=18\n";
        cout << "  ▶ 再采集人脸...\n";
        cout << "  ▶ 人脸特征点: 48个关键点\n";
        cout << "  ▶ 融合双模态特征模板...\n";
        string fp_tmpl = generate_biometric_template(c.id_card + "_FP");
        string fc_tmpl = generate_biometric_template(c.id_card + "_FACE");
        c.biometric_data = "BIO:" + fp_tmpl + fc_tmpl;
        c.face_photo_path = "data/face_" + c.id + ".jpg";
        success = true;
    } else {
        cout << "  ✗ 无效选择!\n"; return;
    }

    if (success) {
        save_customers();
        cout << "\n  ✓ 生物特征录入成功!\n";
        cout << "  客户: " << c.name << " (" << c.id << ")\n";
        cout << "  模板长度: " << c.biometric_data.length() << " 字节\n";
    }
}

/** 生物特征验证 (模拟比对) */
inline void verify_biometric() {
    cout << "\n  ═══ 生物特征验证 (模拟) ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    const Customer& c = g_customers[idx];

    if (c.biometric_data.empty()) {
        cout << "  ✗ 该客户未录入生物特征, 请先录入!\n"; return;
    }

    cout << "\n  ▶ 请再次采集生物特征进行比对...\n";
    cout << "  验证方式 (1.指纹 2.人脸 3.双模态): ";
    string ch; getline(cin, ch);

    string input_template;
    if (ch == "1") {
        input_template = "FP:" + generate_biometric_template(c.id_card + "_FP");
    } else if (ch == "2") {
        input_template = "FACE:" + generate_biometric_template(c.id_card + "_FACE");
    } else if (ch == "3") {
        string fp_tmpl = generate_biometric_template(c.id_card + "_FP");
        string fc_tmpl = generate_biometric_template(c.id_card + "_FACE");
        input_template = "BIO:" + fp_tmpl + fc_tmpl;
    } else {
        cout << "  ✗ 无效选择!\n"; return;
    }

    double similarity = compare_biometric_templates(c.biometric_data, input_template);
    double threshold = (ch == "3") ? 90.0 : 80.0;

    hr();
    cout << "  ═══ 生物特征验证报告 ═══\n";
    hr();
    cout << "  客户: " << c.name << " (" << c.id << ")\n";
    cout << "  存储模板: " << c.biometric_data.substr(0, 32) << "...\n";
    cout << "  采集模板: " << input_template.substr(0, 32) << "...\n";
    cout << "  相似度: " << double_to_str(similarity) << "%\n";
    cout << "  匹配阈值: " << double_to_str(threshold) << "%\n";
    cout << "  ─────────────────────\n";
    if (similarity >= threshold) {
        cout << "  ✓ 生物特征验证通过! 确认为本人。\n";
    } else if (similarity >= 50.0) {
        cout << "  ⚡ 相似度偏低, 建议人工复核! \n";
    } else {
        cout << "  ✗ 生物特征不匹配! 验证失败。\n";
    }
    hr();
}

/** 生物特征管理总览 */
inline void biometric_overview() {
    hr();
    cout << "  ═══ 全行生物特征录入统计 ═══\n";
    hr();
    int total = 0, enrolled = 0;
    for (size_t i = 0; i < g_customers.size(); i++) {
        if (!g_customers[i].is_active) continue;
        total++;
        if (!g_customers[i].biometric_data.empty()) enrolled++;
    }
    cout << "  活跃客户: " << total << " 人\n";
    cout << "  已录入生物特征: " << enrolled << " 人 ("
         << double_to_str(total > 0 ? 100.0 * enrolled / total : 0) << "%)\n";
    cout << "  未录入: " << (total - enrolled) << " 人\n";
    hr();
    if (enrolled > 0) {
        cout << "  已录入客户列表:\n";
        for (size_t i = 0; i < g_customers.size(); i++) {
            if (!g_customers[i].is_active) continue;
            if (!g_customers[i].biometric_data.empty()) {
                string bt = g_customers[i].biometric_data;
                string mode = bt.substr(0, bt.find(':'));
                cout << "    " << g_customers[i].id << " " << g_customers[i].name
                     << " — 模态: " << mode
                     << " — 模板长度: " << g_customers[i].biometric_data.length() << "B\n";
            }
        }
        hr();
    }
}

// ==================== 3. 证件影像读取 (模拟) ====================

/** 证件照采集/存储 */
inline void capture_id_photo() {
    cout << "\n  ═══ 证件照采集 (模拟) ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    Customer& c = g_customers[idx];

    cout << "\n  采集方式:\n";
    cout << "  1. 高拍仪拍摄身份证正面\n";
    cout << "  2. 扫描仪扫描身份证\n";
    cout << "  3. 手动输入照片路径\n";
    cout << "  请选择: "; string ch; getline(cin, ch);

    string photo_path;
    if (ch == "1") {
        photo_path = "data/id_photo_" + c.id + "_front.jpg";
        cout << "  ▶ 启动高拍仪...\n";
        cout << "  ▶ 正在拍摄身份证正面...\n";
        cout << "  ▶ 图像预处理: 去噪/纠偏/裁剪...\n";
    } else if (ch == "2") {
        photo_path = "data/id_scan_" + c.id + ".png";
        cout << "  ▶ 启动扫描仪...\n";
        cout << "  ▶ 正在扫描... 分辨率: 300DPI\n";
    } else if (ch == "3") {
        cout << "  照片文件路径: "; getline(cin, photo_path);
    } else {
        cout << "  ✗ 无效选择!\n"; return;
    }

    c.id_photo_path = photo_path;
    save_customers();
    cout << "  ✓ 证件照已存档: " << photo_path << endl;
}

/** 人脸照片采集 */
inline void capture_face_photo() {
    cout << "\n  ═══ 人脸照片采集 (模拟) ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    Customer& c = g_customers[idx];

    cout << "\n  ▶ 启动摄像头...\n";
    cout << "  ▶ 请面向摄像头, 保持正脸姿态...\n";
    cout << "  ▶ 活体检测: 请缓慢眨眼... ✓\n";
    cout << "  ▶ 活体检测: 请缓慢张嘴... ✓\n";
    cout << "  ▶ 正在拍摄人脸照片...\n";

    c.face_photo_path = "data/face_" + c.id + ".jpg";
    save_customers();
    cout << "  ✓ 人脸照片已存档: " << c.face_photo_path << endl;
}

/** 模拟OCR证件信息读取 */
inline void ocr_read_document() {
    cout << "\n  ═══ 证件影像OCR识别 (模拟) ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    const Customer& c = g_customers[idx];

    if (c.id_photo_path.empty()) {
        cout << "  ⚠ 该客户未录入证件照, 请先采集证件照!\n";
        return;
    }

    cout << "\n  ▶ 加载影像文件: " << c.id_photo_path << endl;
    cout << "  ▶ 图像预处理: 灰度化/二值化/倾斜校正...\n";
    cout << "  ▶ OCR引擎: Tesseract 5.x + PaddleOCR\n";
    cout << "  ▶ 文字区域检测: 姓名/证件号/地址/出生日期\n";
    cout << "  ▶ 关键信息提取...\n";

    hr();
    cout << "  ═══ OCR识别结果 ═══\n";
    hr();
    cout << "  影像文件:    " << c.id_photo_path << endl;
    cout << "  识别姓名:    " << c.name << " (置信度: 98.5%)\n";
    cout << "  识别证件号:  " << c.id_card << " (置信度: 99.2%)\n";
    cout << "  识别地址:    " << c.address << " (置信度: 95.1%)\n";

    // 自动比对OCR结果与已存储信息
    cout << "\n  ─────────────────────\n";
    cout << "  ═══ 信息比对结果 ═══\n";
    cout << "  ─────────────────────\n";
    int match_count = 0, total_fields = 3;
    if (!c.name.empty()) {
        cout << "  姓名比对: OCR=" << c.name << " DB=" << c.name << " ✓ 一致\n";
        match_count++;
    }
    if (!c.id_card.empty()) {
        cout << "  证件号比对: OCR=" << c.id_card << " DB=" << c.id_card;
        bool id_ok = validate_chinese_id_card(c.id_card);
        cout << (id_ok ? " ✓ 一致且合法\n" : " ⚠ 一致但格式有问题\n");
        match_count++;
    }
    if (!c.address.empty()) {
        cout << "  地址比对: OCR=" << c.address << " DB=" << c.address << " ✓ 一致\n";
        match_count++;
    }
    cout << "  ─────────────────────\n";
    cout << "  综合: " << match_count << "/" << total_fields << " 字段一致 — "
         << (match_count == total_fields ? "✓ 人证合一" : "⚠ 需要人工复核") << endl;
    hr();
}

/** 客户证件信息补录/修改 */
inline void update_customer_id_info() {
    cout << "\n  ═══ 客户证件信息补录 ═══\n";
    cout << "  客户编号: "; string cid; getline(cin, cid);
    int idx = find_customer(cid);
    if (idx == -1) { cout << "  ✗ 客户不存在!\n"; return; }
    Customer& c = g_customers[idx];

    cout << "  当前身份证号 [" << c.id_card << "]: ";
    string s; getline(cin, s);
    if (!s.empty()) {
        if (!validate_chinese_id_card(s)) {
            cout << "  ⚠ 身份证号格式不合法! 确认录入? (y/n): ";
            string cf; getline(cin, cf);
            if (cf != "y" && cf != "Y") {
                cout << "  已取消。\n"; return;
            }
        }
        c.id_card = s;
        cout << "  ✓ 身份证号已更新\n";
        cout << "  自动解析: " << get_province_by_code(s)
             << " " << extract_gender_from_id(s)
             << " " << extract_birth_from_id(s) << endl;
    }

    cout << "  当前证件照路径 [" << c.id_photo_path << "]: ";
    getline(cin, s);
    if (!s.empty()) { c.id_photo_path = s; cout << "  ✓ 证件照路径已更新\n"; }

    cout << "  当前人脸照路径 [" << c.face_photo_path << "]: ";
    getline(cin, s);
    if (!s.empty()) { c.face_photo_path = s; cout << "  ✓ 人脸照路径已更新\n"; }

    save_customers();
    cout << "  ✓ 证件信息已保存!\n";
}

// ==================== 集成: 身份核验管理菜单 ====================

inline void identity_verifier_menu() {
    while (true) {
        cls();
        cout << "\n  ╔══════════════════════════════════╗\n";
        cout << "  ║  需求8补充: 身份核验·生物识别   ║\n";
        cout << "  ╠══════════════════════════════════╣\n";
        cout << "  ║  [身份证核验]                    ║\n";
        cout << "  ║  1. 身份证号码校验               ║\n";
        cout << "  ║  2. 客户身份核验                 ║\n";
        cout << "  ║  3. 批量核验全行客户身份证       ║\n";
        cout << "  ║  [生物识别]                      ║\n";
        cout << "  ║  4. 生物特征录入 (指纹/人脸)     ║\n";
        cout << "  ║  5. 生物特征验证 (比对)          ║\n";
        cout << "  ║  6. 全行生物特征统计             ║\n";
        cout << "  ║  [影像读取]                      ║\n";
        cout << "  ║  7. 证件照采集                   ║\n";
        cout << "  ║  8. 人脸照片采集                 ║\n";
        cout << "  ║  9. 证件OCR识别 (模拟)           ║\n";
        cout << "  ║  [信息管理]                      ║\n";
        cout << "  ║  A. 客户证件信息补录/修改         ║\n";
        cout << "  ║  0. 返回上级菜单                 ║\n";
        cout << "  ╚══════════════════════════════════╝\n";
        cout << "  请选择: ";
        string ch; getline(cin, ch);

        if (ch == "1") { verify_id_card_full(); pause(); }
        else if (ch == "2") { verify_customer_identity(); pause(); }
        else if (ch == "3") { batch_verify_id_cards(); pause(); }
        else if (ch == "4") { enroll_biometric(); pause(); }
        else if (ch == "5") { verify_biometric(); pause(); }
        else if (ch == "6") { biometric_overview(); pause(); }
        else if (ch == "7") { capture_id_photo(); pause(); }
        else if (ch == "8") { capture_face_photo(); pause(); }
        else if (ch == "9") { ocr_read_document(); pause(); }
        else if (ch == "A" || ch == "a") { update_customer_id_info(); pause(); }
        else if (ch == "0") break;
    }
}

#endif // IDENTITY_VERIFIER_H
