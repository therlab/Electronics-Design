# 循迹小车按键系统重设计 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task.

**Goal:** 去掉4秒倒计时，新增 KEY4/5/6 (PB20-22) 实现启停 + 双路调参

**Architecture:** 3个文件修改——ti_msp_dl_config.h/c 添加 GPIO 定义和初始化，empty.c 改状态机和按键逻辑

**Tech Stack:** MSPM0G3507, TI DriverLib, Keil MDK

## Global Constraints

- 所有按键内部上拉，按下=0，消抖 10ms
- KEY4=PB20(PINCM47), KEY5=PB21(PINCM48), KEY6=PB22(PINCM49)
- OLED 4行显示，8x16 字体为主
- 上电 stopped=1，KEY4 toggle 启停
- W25Q64 保存逻辑不变

---

### Task 1: 添加 KEY4/5/6 GPIO 宏定义

**Files:**
- Modify: `ti_msp_dl_config.h`

- [ ] **Step 1: 在 ti_msp_dl_config.h 的 GPIO_KEY 段添加 KEY4/5/6 定义**

找到现有 KEY1-3 的定义（约 218-226 行），在后面添加：

```c
/* Defines for KEY4: GPIOB.20 with pinCMx 47 on package pin 17 */
#define GPIO_KEY_KEY4_PIN                                       (DL_GPIO_PIN_20)
#define GPIO_KEY_KEY4_IOMUX                                      (IOMUX_PINCM47)
/* Defines for KEY5: GPIOB.21 with pinCMx 48 on package pin 18 */
#define GPIO_KEY_KEY5_PIN                                       (DL_GPIO_PIN_21)
#define GPIO_KEY_KEY5_IOMUX                                      (IOMUX_PINCM48)
/* Defines for KEY6: GPIOB.22 with pinCMx 49 on package pin 19 */
#define GPIO_KEY_KEY6_PIN                                       (DL_GPIO_PIN_22)
#define GPIO_KEY_KEY6_IOMUX                                      (IOMUX_PINCM49)
```

- [ ] **Step 2: 编译验证** — 确认宏定义无语法错误

---

### Task 2: 添加 KEY4/5/6 GPIO 初始化

**Files:**
- Modify: `ti_msp_dl_config.c`

- [ ] **Step 1: 在 SYSCFG_DL_GPIO_init() 中添加 3 个按键的输入初始化**

找到现有的 KEY1-3 初始化代码（约 167-177 行），在后面添加：

```c
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY4_IOMUX,
                 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
                 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY5_IOMUX,
                 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
                 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);

    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY6_IOMUX,
                 DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_NONE,
                 DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
```

- [ ] **Step 2: 编译验证** — 确认 GPIO 初始化无语法错误

---

### Task 3: 修改 empty.c — 去掉倒计时，重构按键系统，优化 OLED

**Files:**
- Modify: `empty.c`

**Interfaces:**
- Consumes: GPIO_KEY_KEY4/5/6_PIN from Task 1, GPIO init from Task 2
- Produces: 6键状态机 + 启停 toggle + OLED 4行显示

- [ ] **Step 1: 修改 keys_init() 添加 KEY4/5/6 初始化**

找到 `keys_init()` 函数，追加 3 个新按键：

```c
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY4_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY5_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
    DL_GPIO_initDigitalInputFeatures(GPIO_KEY_KEY6_IOMUX,
        DL_GPIO_INVERSION_DISABLE, DL_GPIO_RESISTOR_PULL_UP,
        DL_GPIO_HYSTERESIS_DISABLE, DL_GPIO_WAKEUP_DISABLE);
```

- [ ] **Step 2: 删除 4 秒倒计时代码块**

删除 `main()` 中从 `/* ══════ 4秒倒计时 ══════ */` 到 `OLED_Clear();` 的整段代码。

- [ ] **Step 3: 修改 stopped 初始值为 1，添加 STOP 状态 OLED 显示**

在 `OLED_Clear()` 之后，while(1) 之前，将 `uint8_t stopped = 0;` 改为 `uint8_t stopped = 1;`，并在进入主循环前显示 "STOP" 画面：

```c
    uint8_t  stopped = 1;

    /* 显示初始 STOP 画面 */
    OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
    snprintf(t, sizeof(t), " KP: %.1f", (double)g_kp);
    OLED_ShowString(0, 1, (uint8_t*)t, 16);
    snprintf(t, sizeof(t), " KD: %.1f", (double)g_kd);
    OLED_ShowString(0, 2, (uint8_t*)t, 16);
    snprintf(t, sizeof(t), " KI: %.2f", (double)g_ki);
    OLED_ShowString(0, 3, (uint8_t*)t, 16);

    while (1)
    {
```

- [ ] **Step 4: 重写按键处理段 — 添加 KEY4/5/6 逻辑**

将主循环中原来的按键处理段（`/* ══════ 按键处理 */` 到下一个 `}` 之间）替换为：

```c
        /* ══════ 按键处理 (每20次循环≈100ms) ══════ */
        static uint8_t key_tick = 0;
        if (++key_tick >= 20) {
            key_tick = 0;

            uint8_t k1 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY1_PIN);
            uint8_t k2 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY2_PIN);
            uint8_t k3 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY3_PIN);
            uint8_t k4 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY4_PIN);
            uint8_t k5 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY5_PIN);
            uint8_t k6 = key_debounced(GPIO_KEY_PORT, GPIO_KEY_KEY6_PIN);

            /* KEY4 启停 toggle */
            static uint8_t k4_was = 0;
            if (k4 && !k4_was) {
                stopped = !stopped;
                if (stopped) {
                    motor_control(0, 0);
                    g_pid_int = 0; g_pid_last = 0;
                    OLED_Clear();
                    OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
                } else {
                    OLED_Clear();
                }
            }
            k4_was = k4;

            /* 只在 STOP 状态下响应参数调节按键 */
            if (stopped) {
                /* KEY1 短按: 切换参数 */
                static uint8_t k1_was = 0;
                if (k1 && !k1_was) { pidx = (pidx + 1) % 4; }
                k1_was = k1;

                /* KEY1 长按: 恢复默认 (~1s) */
                static uint8_t k1_hold = 0;
                if (k1) {
                    if (++k1_hold > 10) {
                        k1_hold = 0;
                        g_kp = GRAY_KP; g_kd = GRAY_KD;
                        g_ki = GRAY_KI; g_spd = BASE_SPEED;
                        g_pid_int = 0; g_pid_last = 0;
                    }
                } else { k1_hold = 0; }

                /* KEY2 长按: 保存 (~2s) */
                static uint8_t k2_hold = 0;
                if (k2) {
                    if (++k2_hold > 20) {
                        k2_hold = 0;
                        w25_params_save();
                        OLED_ShowString(0, 0, (uint8_t*)"SAVED!", 16);
                        delay_ms(800);
                        OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
                    }
                } else { k2_hold = 0; }

                /* KEY2 / KEY5 短按: 参数+ */
                static uint8_t k2_was = 0, k5_was = 0;
                if ((k2 && !k2_was) || (k5 && !k5_was)) {
                    switch (pidx) {
                    case 0: g_kp += 0.5f; if (g_kp > 20) g_kp = 20; break;
                    case 1: g_kd += 0.5f; if (g_kd > 20) g_kd = 20; break;
                    case 2: g_ki += 0.05f; if (g_ki > 5) g_ki = 5; break;
                    case 3: g_spd += 25; if (g_spd > 800) g_spd = 800; break;
                    }
                    g_pid_int = 0; g_pid_last = 0;
                }
                k2_was = k2; k5_was = k5;

                /* KEY3 / KEY6 短按: 参数- */
                static uint8_t k3_was = 0, k6_was = 0;
                if ((k3 && !k3_was) || (k6 && !k6_was)) {
                    switch (pidx) {
                    case 0: g_kp -= 0.5f; if (g_kp < 0.5f) g_kp = 0.5f; break;
                    case 1: g_kd -= 0.5f; if (g_kd < 0) g_kd = 0; break;
                    case 2: g_ki -= 0.05f; if (g_ki < 0) g_ki = 0; break;
                    case 3: g_spd -= 25; if (g_spd < 50) g_spd = 50; break;
                    }
                    g_pid_int = 0; g_pid_last = 0;
                }
                k3_was = k3; k6_was = k6;
            }
        }
```

- [ ] **Step 5: 修改 COLOR HIT 处理 — 改为可以重新启动**

将现有的色标处理段替换为：

```c
        if (!stopped && color)
        {
            motor_control(0, 0);
            stopped = 1;
            OLED_Clear();
            OLED_ShowString(0, 0, (uint8_t*)"COLOR HIT!", 16);      /* 8x16 大字体 */
            OLED_ShowString(0, 1, (uint8_t*)"STOPPED", 16);
            snprintf(t, sizeof(t), "KP%.1f KD%.1f", (double)g_kp, (double)g_kd);
            OLED_ShowString(0, 2, (uint8_t*)t, 8);
            snprintf(t, sizeof(t), "KI:%.2f SPD:%d", (double)g_ki, g_spd);
            OLED_ShowString(0, 3, (uint8_t*)t, 8);
        }
```

关键修改：COLOR HIT 第0行用 size=16(8x16大字)，stopped=1 后可 KEY4 重新启动。

- [ ] **Step 6: 修改 RUN 状态 OLED 显示 — 第0行显示 "RUN"**

将 RUN 状态的 OLED 刷新段第0行改为：

```c
            OLED_ShowString(0, 0, (uint8_t*)" RUN", 16);
            snprintf(t, sizeof(t), "%cKP: %.1f", tag0, (double)g_kp);
            OLED_ShowString(0, 1, (uint8_t*)t, 16);
            snprintf(t, sizeof(t), "%cKD: %.1f", tag1, (double)g_kd);
            OLED_ShowString(0, 2, (uint8_t*)t, 16);
            snprintf(t, sizeof(t), "%cKI: %.2f", tag2, (double)g_ki);
            OLED_ShowString(0, 3, (uint8_t*)t, 16);
```

注意：4行都改为 size=16，但 SPD 不单独显示一行（通过 pidx 切换看到，或再加一行）。实际上 OLED 只有4行，KP/KD/KI/SPD 四个参数需要通过 `>` 轮转显示。这里改为 KP/KD/KI 各占一行，SPD 在 pidx=3 时显示替换 KI 行：

```c
        /* ══════ OLED (RUN模式) ══════ */
        if (count % OLED_DIV == 0 && !stopped)
        {
            char tag0 = (pidx == 0) ? '>' : ' ';
            char tag1 = (pidx == 1) ? '>' : ' ';
            char tag2 = (pidx == 2) ? '>' : ' ';
            char tag3 = (pidx == 3) ? '>' : ' ';
            OLED_ShowString(0, 0, (uint8_t*)" RUN", 16);
            snprintf(t, sizeof(t), "%cKP: %.1f", tag0, (double)g_kp);
            OLED_ShowString(0, 1, (uint8_t*)t, 16);
            snprintf(t, sizeof(t), "%cKD: %.1f", tag1, (double)g_kd);
            OLED_ShowString(0, 2, (uint8_t*)t, 16);
            /* 第3行: KI 或 SPD (轮转) */
            if (pidx == 2 || pidx < 2) {
                snprintf(t, sizeof(t), "%cKI: %.2f", tag2, (double)g_ki);
            } else {
                snprintf(t, sizeof(t), "%cSPD: %d", tag3, g_spd);
            }
            OLED_ShowString(0, 3, (uint8_t*)t, 16);
        }
```

- [ ] **Step 7: 添加 STOP 状态的 OLED 刷新**

在主循环中，色标处理后、RUN OLED 刷新之前，添加 STOP 状态显示：

```c
        /* ══════ OLED (STOP模式) ══════ */
        if (count % OLED_DIV == 0 && stopped) {
            /* 只在 stop 时刷新（不覆盖 COLOR HIT 的显示） */
            static uint8_t was_stopped_by_color = 0;
            if (color && stopped && !was_stopped_by_color) {
                was_stopped_by_color = 1;  /* COLOR HIT 已显示，不覆盖 */
            } else if (!color) {
                was_stopped_by_color = 0;
            }

            if (!was_stopped_by_color) {
                char tag0 = (pidx == 0) ? '>' : ' ';
                char tag1 = (pidx == 1) ? '>' : ' ';
                char tag2 = (pidx == 2) ? '>' : ' ';
                char tag3 = (pidx == 3) ? '>' : ' ';
                OLED_ShowString(0, 0, (uint8_t*)"STOP KEY4->GO", 16);
                snprintf(t, sizeof(t), "%cKP: %.1f", tag0, (double)g_kp);
                OLED_ShowString(0, 1, (uint8_t*)t, 16);
                snprintf(t, sizeof(t), "%cKD: %.1f", tag1, (double)g_kd);
                OLED_ShowString(0, 2, (uint8_t*)t, 16);
                if (pidx == 2 || pidx < 2) {
                    snprintf(t, sizeof(t), "%cKI: %.2f", tag2, (double)g_ki);
                } else {
                    snprintf(t, sizeof(t), "%cSPD: %d", tag3, g_spd);
                }
                OLED_ShowString(0, 3, (uint8_t*)t, 16);
            }
        }
```

- [ ] **Step 8: 编译完整项目** — Rebuild all，确认 0 error 0 warning

---

### 验证清单

- [ ] 编译 0 error
- [ ] 上电 OLED 显示 "STOP KEY4->GO"
- [ ] 按 KEY4 启动，OLED 显示 "RUN" + 参数
- [ ] 按 KEY4 停止，电机停转
- [ ] KEY5/KEY6 能加减参数
- [ ] 色标 PA9 触发 → "COLOR HIT!" 大字体 + "STOPPED"
- [ ] COLOR HIT 后按 KEY4 可重新启动
