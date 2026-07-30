# 循迹小车按键系统重设计

**日期**: 2026-07-30  
**项目**: Amy 64pin 循迹小车 (Electronics-Design)  
**MCU**: MSPM0G3507

---

## 目标

1. 去掉 4 秒倒计时，改为按键启停控制
2. 新增 3 个按键：启停 + 两个辅助调参键
3. 修复 COLOR HIT 显示为清晰的大字体

---

## 引脚分配

新增 3 个按键紧邻现有 KEY1-3 (PB17-19)，同排走线：

| 按键 | GPIO | PinCM | 封装脚 | 功能 |
|------|------|-------|--------|------|
| KEY1 | PB17 | PINCM43 | 14 | 短按切换参数 / 长按恢复默认 |
| KEY2 | PB18 | PINCM44 | 15 | 短按+ / 长按保存 W25Q64 |
| KEY3 | PB19 | PINCM45 | 16 | 短按- |
| **KEY4** | **PB20** | PINCM47 | 17 | **启停 toggle** |
| **KEY5** | **PB21** | PINCM48 | 18 | **参数+** |
| **KEY6** | **PB22** | PINCM49 | 19 | **参数-** |

所有按键：内部上拉，按下 = 0，消抖 10ms。

---

## 状态机

```
上电 → STOPPED (stopped=1, 显示参数等 KEY4)
  KEY4 短按 → RUNNING (stopped=0, 循迹)
  KEY4 短按 → STOPPED (电机停, 显参数)
  色标触发 → STOPPED (电机停, OLED显示COLOR HIT!)
```

- 去掉 4 秒倒计时代码块
- `stopped` 变量初始值改为 `1`（上电即停）
- KEY4 按下时 toggle：`stopped = !stopped`，切换时清除 OLED

---

## 按键功能详表

| 按键 | 短按 | 长按 |
|------|------|------|
| KEY1 | 切换选中参数 pidx++ | 恢复默认值 (config.h) |
| KEY2 | 参数+ (同 KEY5) | 保存到 W25Q64 |
| KEY3 | 参数- (同 KEY6) | — |
| KEY4 | 启停 toggle | — |
| KEY5 | 参数+ (同 KEY2 短按) | — |
| KEY6 | 参数- (同 KEY3 短按) | — |

KEY2 和 KEY5 加减逻辑完全一致（复用同一段代码）。

---

## OLED 显示

### 停止状态 (STOP)
```
STOP  KEY4->GO        (第0行, 8x16字体)
>KP: 6.0              (第1行, 8x16)
 KD: 2.0              (第2行)
 KI: 0.10             (第3行)
```

### 运行状态 (RUN)
```
 RUN                  (第0行, 8x16字体)
>KP: 6.0              (第1行, 8x16)
 KD: 2.0              (第2行)
 KI: 0.10             (第3行)
```
SPD 显示通过 `>` 切换可见（pidx 轮转 KP→KD→KI→SPD）。

### 色标触发 (COLOR HIT)
```
COLOR HIT!            (第0行, 8x16字体)
STOPPED               (第1行, 8x16)
KP:6.0 KD:2.0 KI:0.10 (第2行, 6x8字体)
SPD:300               (第3行, 6x8字体)
```
色标触发后等同于 STOP 状态，按 KEY4 可重新启动。

---

## 代码修改范围

1. **ti_msp_dl_config.h** — 添加 KEY4/5/6 的 GPIO 宏定义
2. **ti_msp_dl_config.c** — `SYSCFG_DL_GPIO_init()` 添加 3 个按键初始化
3. **empty.c** — 去掉倒计时、添加 KEY4/5/6 逻辑、改 stopped 初始值、优化 OLED 显示

---

## spec self-review

- [x] 无 TBD/占位符
- [x] 引脚不冲突（PB20-22 空闲）
- [x] 按键行为无歧义
- [x] OLED 四行内容明确
- [x] 与 W25Q64 保存逻辑兼容
