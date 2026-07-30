"""
send_ball_zigbee.py — 钢珠检测 + UART 输出 9 字节帧 → DL-20 Zigbee

接线:
  MaixCAM2 A21 (TX) → DL-20 RX
  MaixCAM2 GND      → DL-20 GND
  MaixCAM2 3.3V     → DL-20 VCC

帧格式 (9 字节, 发给小车 MSPM0 解析):
  0xAA 0x55  num  cxL  cxH  cyL  cyH  conf  XOR_CHK
  - num:  钢珠数量 (0=无钢珠)
  - cx:   最大钢珠中心X (0~224, 小端序)
  - cy:   最大钢珠中心Y (0~224, 小端序)
  - conf: 置信度百分比 (0~100)
  - XOR:  byte2^byte3^byte4^byte5^byte6^byte7

用法: 把此文件复制到 MaixCAM2 上替换 main.py 运行
"""

from maix import camera, display, image, nn, app, time, uart, pinmap, sys

# ── 参数 ──
CONF_TH   = 0.4       # 检测置信度阈值
IOU_TH    = 0.4       # NMS IoU 阈值
SEND_MS   = 300       # 发送间隔 (毫秒), 9600下不要太快

# ── 初始化 UART (接 DL-20 Zigbee, 9600bps) ──
if sys.device_id() == "maixcam2":
    pinmap.set_pin_function("A21", "UART4_TX")
    pinmap.set_pin_function("A22", "UART4_RX")
    uart_dev = "/dev/ttyS4"
else:
    pinmap.set_pin_function("A21", "UART2_TX")
    pinmap.set_pin_function("A22", "UART2_RX")
    uart_dev = "/dev/ttyS2"

ser = uart.UART(uart_dev, 9600)
time.sleep_ms(200)

def send_frame(num, cx, cy, conf):
    """打包并发送 9 字节协议帧"""
    cx_l = cx & 0xFF
    cx_h = (cx >> 8) & 0xFF
    cy_l = cy & 0xFF
    cy_h = (cy >> 8) & 0xFF
    chk = num ^ cx_l ^ cx_h ^ cy_l ^ cy_h ^ conf
    frame = bytes([0xAA, 0x55, num, cx_l, cx_h, cy_l, cy_h, conf, chk])
    ser.write(frame)

# ── IoU ──
def iou(a, b):
    x1 = max(a[0], b[0]); y1 = max(a[1], b[1])
    x2 = min(a[0] + a[2], b[0] + b[2])
    y2 = min(a[1] + a[3], b[1] + b[3])
    iw = max(0, x2 - x1); ih = max(0, y2 - y1)
    inter = iw * ih
    union = a[2] * a[3] + b[2] * b[3] - inter
    return inter / union if union > 0 else 0

# ── 主程序 ──
def main():
    d = nn.YOLOv5(model="/root/models/model_296904.mud", dual_buff=True)
    labels = d.labels
    cam = camera.Camera(d.input_width(), d.input_height(), d.input_format())
    disp = display.Display()

    tracks = []
    last_send = 0

    while not app.need_exit():
        img = cam.read()
        objs = d.detect(img, conf_th=CONF_TH, iou_th=IOU_TH)

        # ── 匹配 & 平滑 (同 main.py) ──
        matched = [False] * len(objs)
        for t in tracks:
            t[6] += 1
            best_iou, best_j = 0, -1
            for j, obj in enumerate(objs):
                if matched[j]:
                    continue
                val = iou([obj.x, obj.y, obj.w, obj.h],
                         [t[0], t[1], t[2], t[3]])
                if val > best_iou:
                    best_iou, best_j = val, j
            if best_iou > 0.15:
                obj = objs[best_j]
                matched[best_j] = True
                t[0] = 0.25 * obj.x + 0.75 * t[0]
                t[1] = 0.25 * obj.y + 0.75 * t[1]
                t[2] = 0.25 * obj.w + 0.75 * t[2]
                t[3] = 0.25 * obj.h + 0.75 * t[3]
                t[4] = labels[obj.class_id]
                t[5] = obj.score
                t[6] = 0

        for j, obj in enumerate(objs):
            if not matched[j]:
                tracks.append([float(obj.x), float(obj.y), float(obj.w),
                              float(obj.h), labels[obj.class_id], obj.score, 0])

        tracks = [t for t in tracks if t[6] <= 5]

        # ── 找到置信度最高的钢珠 ──
        best_t = None
        for t in tracks:
            if best_t is None or t[5] > best_t[5]:
                best_t = t

        # ── 定时发送 (通过 DL-20 Zigbee) ──
        now = time.ticks_ms()
        if now - last_send >= SEND_MS:
            last_send = now
            if best_t and best_t[5] >= CONF_TH:
                num = len(tracks)
                cx = int(best_t[0] + best_t[2] / 2)   # 中心 X
                cy = int(best_t[1] + best_t[3] / 2)   # 中心 Y
                conf = min(int(best_t[5] * 100), 100)  # 置信度 0~100
            else:
                num, cx, cy, conf = 0, 0, 0, 0
            send_frame(num, cx, cy, conf)

        # ── 屏幕显示 (保留调试用) ──
        for t in tracks:
            sx, sy, sw, sh = int(t[0]), int(t[1]), int(t[2]), int(t[3])
            score = t[5]
            color = image.COLOR_GREEN if score >= 0.7 else image.COLOR_YELLOW
            img.draw_rect(sx, sy, sw, sh, color, 2)
            img.draw_string(sx, sy - 14, f"{t[4]}:{score:.2f}", color, 1.5)
            cx, cy = sx + sw // 2, sy + sh // 2
            img.draw_line(cx - 5, cy, cx + 5, cy, image.COLOR_RED, 1)
            img.draw_line(cx, cy - 5, cx, cy + 5, image.COLOR_RED, 1)

        status = "TX" if (best_t and best_t[5] >= CONF_TH) else "--"
        img.draw_string(2, 2, f"FPS:{time.fps():.1f} N:{len(tracks)} {status}",
                        image.COLOR_WHITE, 1.2)
        disp.show(img)


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        import traceback as _tb
        msg = _tb.format_exc()
        print(msg)
        ei = image.Image(224, 224, image.Format.FMT_RGB888)
        ei.draw_rect(0, 0, 224, 224, image.COLOR_RED, -1)
        ei.draw_string(0, 0, msg, image.COLOR_WHITE, 0.5)
        display.Display().show(ei)
        time.sleep(20)
