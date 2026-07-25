# Amy 64pin 项目状态总结

## 硬件接线

### 电机 (TB6612, 4路)
| 电机 | PWM | IN1 | IN2 | 方向宏 |
|------|-----|-----|-----|--------|
| F_L 前左 | PA15 (CH2) | PA7 | PA12 | 对调 (IN1=0,IN2=1=前进) |
| F_R 前右 | PB14 (CH0) | PA13 | PA16 | 默认 (IN1=1,IN2=0=前进) |
| B_L 后左 | PB9 (CH1) | PB16 | PA14 | 默认 |
| B_R 后右 | PA23 (CH3) | PB13 | PB15 | 默认 |

### 编码器
| 信号 | 引脚 |
|------|------|
| L_A | PB24 |
| L_B | PA25 |
| R_A | PB25 |
| R_B | PA26 |
- MM_PER_COUNT = 10.53 (实测校准)
- 简化计数：A相跳变+1，不判方向

### 传感器
| 传感器 | 引脚 |
|--------|------|
| 灰度 CLK | PB7 |
| 灰度 DAT | PB8 |
| 色标 LEFT | PA8 |
| 色标 RIGHT | PA9 |
| 陀螺仪 JY61P | I2C (PA28=SDA, PA31=SCL) |

### 通信
| 接口 | TX | RX | 用途 | 波特率 |
|------|-----|-----|------|--------|
| UART0 | PA10 | PA11 | **Zigbee** (原蓝牙已替换) | 9600 |
| UART2 (OPENMV) | PA21 | PA24 | MaixCAM2 钢珠检测 | 115200 |
| I2C OLED | PA28 | PA31 | OLED (0.96寸) | - |

---

## 运行逻辑 (empty.c 当前)

```
上电→4秒倒计时→

STATE_GRAY: 灰度PID巡线
  ├─ 速度: BASE_SPEED (350)
  ├─ 走够 GRAY_DIST_MM (750mm) → 切 STATE_GYRO

STATE_GYRO: 陀螺仪PID直走 + 色标检测
  ├─ 速度: GYRO_SPEED (190)
  ├─ 检测色标(PA9, 1次触发)
  └─ 检测到 → ①停车2s → ②倒退50mm → ③停车2s → ④右转90° → 距离归零 → 回 STATE_GRAY
```

### 并发功能
- **Zigbee**: 每1秒发心跳包 `Z<tx>S<state>D<dist>Y<yaw>\r\n`，主循环轮询接收
- **MaixCAM2**: 中断接收 9 字节钢珠帧 (`0xAA 0x55 ...`)，g_ball_new 通知主循环
- **OLED**: 4行 — 状态+Zigbee计数 / 钢珠数据 / 距离+航向 / 灰度8位+色标

---

## 当前参数 (config.h)

### 速度
- BASE_SPEED = 350 (灰度循迹)
- GYRO_SPEED = 190 (陀螺仪直走)

### 灰度循迹 PID
- GRAY_KP=2.5, GRAY_KI=0, GRAY_KD=0.8
- 权重: W1=-210, W2=-150, W3=-90, W4=-50, W5=50, W6=90, W7=150, W8=210

### 陀螺仪直走 PID
- YAW_KP=6.0, YAW_KI=0.05, YAW_KD=0.5, YAW_INT_MAX=350
- 注意: correction 符号已在代码中反过一次(陀螺仪方向反了)

### 陀螺仪转弯 PID
- 粗调: KP=3.8, KI=0.01, KD=1.5, SPD=400
- 精调: KP=1.0, KI=0, KD=0.6, SPD=190
- TURN_FWD_BIAS=-10, TURN_LR_COMP=50
- 切换角度 20°, 退出阈值 1.5°, 连续 3 次

### 编码器
- MM_PER_COUNT=10.53
- SPEED_PI 已关闭 (KP=KI=0)

---

## 代码结构

```
Amy_64pin_official_1/
├── empty.c                    ★ 主状态机 (灰度→陀螺仪+色标→转弯 循环)
├── config.h                   ★ 所有可调参数
├── ti_msp_dl_config.c/.h     SysConfig 生成 (不改)
├── BSP/
│   ├── ZIGBEE/
│   │   ├── bsp_zigbee.h       ★ Zigbee 通信库头文件 (UART0, 环形缓冲区)
│   │   └── bsp_zigbee.c       ★ Zigbee 通信库实现
│   ├── MOTOR/
│   │   ├── bsp_tb6612.h       电机驱动头文件 (方向宏)
│   │   └── bsp_tb6612.c       电机驱动实现 (4路 PWM)
│   ├── ENCODER/
│   │   ├── encoder.h          编码器头文件
│   │   └── encoder.c          编码器实现 (简化计数)
│   ├── PID/
│   │   ├── pid.h              PID 头文件 (灰度/航向/速度)
│   │   └── pid.c              PID 实现
│   ├── JY61P_IIC/
│   │   ├── bsp_gyro.h         陀螺仪头文件 (I2C 模拟)
│   │   └── bsp_gyro.c         陀螺仪实现
│   ├── grayscale/
│   │   ├── gray_serial.h      8路灰度传感器头文件
│   │   └── delay.c            延时函数
│   ├── OLED/
│   │   └── oled.c/h           OLED 软件 I2C (旧)
│   ├── OLED_NEW/
│   │   └── oled_hardware_i2c.c/h  OLED 硬件 I2C (当前使用)
│   └── BLUE/
│       └── bsp_bluetooth.c/h  蓝牙驱动 (已废弃，UART0 改为 Zigbee)
└── myused/
    ├── uart.c/h               UART0 底层驱动 (printf 重定向, RX 中断无 echo)
    ├── usart_openmv.c/h       ★ MaixCAM2 摄像头串口库 (9字节协议)
    ├── fun.c/h                (未使用)
    └── tim.c/h                SysTick (tick_ms)
```

---

## 下次开发方向
1. Zigbee 命令解析 (遥控/调参) — 框架已就绪，在 empty.c 的 `while(zigbee_available())` 中添加
2. 钢珠坐标 → 调整巡线或触发动作 — g_ball 数据已可用
3. 多段路线 (使用 SEG1~5_DIST)
4. 编码器方向判断恢复
