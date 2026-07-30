' MSPM0G3507 Pinout PPT Generator
Option Explicit

Dim ppt, pres, s

Set ppt = CreateObject("PowerPoint.Application")
Set pres = ppt.Presentations.Add()

' Colors (BGR hex strings for MsoThemeColorSchemeIndex)
Const COLOR_BG     = &H2E1A1A  ' Dark navy
Const COLOR_ACCENT = &HFFD200   ' Cyan accent
Const COLOR_WHITE  = &HFFFFFF
Const COLOR_GRAY   = &HCCCCCC
Const COLOR_GREEN  = &H76E600
Const COLOR_ORANGE = &H40ABFF
Const COLOR_DARK   = &H1A0D0D

Dim sw, sh
sw = pres.PageSetup.SlideWidth
sh = pres.PageSetup.SlideHeight

' ==== SLIDE 1: Title ====
Set s = pres.Slides.Add(1, 1) ' ppLayoutBlank
s.Shapes.AddShape(1, 0, 0, sw, sh).Fill.ForeColor.RGB = COLOR_BG
s.Shapes.AddShape(1, 40, 80, 8, 200).Fill.ForeColor.RGB = COLOR_ACCENT

Dim tb
Set tb = s.Shapes.AddTextbox(1, 70, 100, 860, 60)
tb.TextFrame.TextRange.Text = "MSPM0G3507 引脚分配"
tb.TextFrame.TextRange.Font.Size = 40
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_WHITE
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set tb = s.Shapes.AddTextbox(1, 70, 170, 860, 40)
tb.TextFrame.TextRange.Text = "Amy 64pin 4轮循迹小车"
tb.TextFrame.TextRange.Font.Size = 22
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set tb = s.Shapes.AddTextbox(1, 70, 230, 860, 30)
tb.TextFrame.TextRange.Text = "UART0=Zigbee · UART2=MaixCAM2 · I2C0=OLED · TIMA0=4路PWM"
tb.TextFrame.TextRange.Font.Size = 14
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set tb = s.Shapes.AddTextbox(1, 70, 340, 860, 20)
tb.TextFrame.TextRange.Text = "Electronics-Design  |  2025-07"
tb.TextFrame.TextRange.Font.Size = 13
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

' ==== SLIDE 2: Motor ====
Set s = pres.Slides.Add(1, 1)
s.Shapes.AddShape(1, 0, 0, sw, sh).Fill.ForeColor.RGB = COLOR_BG

Set tb = s.Shapes.AddTextbox(1, 30, 15, 900, 35)
tb.TextFrame.TextRange.Text = "电机驱动 — TB6612 (4路PWM, TIMA0)"
tb.TextFrame.TextRange.Font.Size = 22
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GREEN
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

AddTable s, 30, 65, Array(110, 140, 60, 60, 210), Array(_
    Array("电机","PWM通道","IN1","IN2","方向 (前进时)"),_
    Array("F_L 前左","PA15 CH2","PA7","PA12","对调: IN1=0,IN2=1"),_
    Array("F_R 前右","PB14 CH0","PA13","PA16","默认: IN1=1,IN2=0"),_
    Array("B_L 后左","PB9  CH1","PB16","PA14","默认: IN1=1,IN2=0"),_
    Array("B_R 后右","PA23 CH3","PB13","PB15","默认: IN1=1,IN2=0")), COLOR_ACCENT, COLOR_DARK

Set tb = s.Shapes.AddTextbox(1, 30, 260, 900, 30)
tb.TextFrame.TextRange.Text = "API: motor_control(left, right)  范围 -1000 ~ +1000"
tb.TextFrame.TextRange.Font.Size = 15
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_ACCENT
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set tb = s.Shapes.AddTextbox(1, 30, 295, 900, 25)
tb.TextFrame.TextRange.Text = "左侧=F_L+B_L同速  |  右侧=F_R+B_R同速  (滑移转向)"
tb.TextFrame.TextRange.Font.Size = 13
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

' Car diagram
Dim box
Set box = s.Shapes.AddShape(1, 200, 370, 170, 50)
box.Fill.ForeColor.RGB = COLOR_DARK
box.Line.ForeColor.RGB = COLOR_ACCENT
Set tb = s.Shapes.AddTextbox(1, 210, 377, 150, 35)
tb.TextFrame.TextRange.Text = "F_L     车头     F_R" & vbCrLf & "左 <--    --> 右"
tb.TextFrame.TextRange.Font.Size = 10
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set box = s.Shapes.AddShape(1, 200, 440, 170, 50)
box.Fill.ForeColor.RGB = COLOR_DARK
box.Line.ForeColor.RGB = COLOR_ACCENT
Set tb = s.Shapes.AddTextbox(1, 210, 447, 150, 35)
tb.TextFrame.TextRange.Text = "B_L     车尾     B_R" & vbCrLf & "左 <--    --> 右"
tb.TextFrame.TextRange.Font.Size = 10
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

' ==== SLIDE 3: Sensors ====
Set s = pres.Slides.Add(1, 1)
s.Shapes.AddShape(1, 0, 0, sw, sh).Fill.ForeColor.RGB = COLOR_BG

Set tb = s.Shapes.AddTextbox(1, 30, 15, 900, 35)
tb.TextFrame.TextRange.Text = "传感器 & 编码器"
tb.TextFrame.TextRange.Font.Size = 22
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GREEN
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set tb = s.Shapes.AddTextbox(1, 30, 60, 200, 22)
tb.TextFrame.TextRange.Text = "编码器"
tb.TextFrame.TextRange.Font.Size = 15
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_ACCENT
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

AddTable s, 30, 88, Array(70, 70, 200), Array(_
    Array("信号","引脚","备注"),_
    Array("L_A","PB24",""),_
    Array("L_B","PA25","MM_PER_COUNT"),_
    Array("R_A","PB25","= 10.53 mm"),_
    Array("R_B","PA26","简化计数,不判方向")), COLOR_ACCENT, COLOR_DARK

Set tb = s.Shapes.AddTextbox(1, 410, 60, 200, 22)
tb.TextFrame.TextRange.Text = "传感器"
tb.TextFrame.TextRange.Font.Size = 15
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_ORANGE
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

AddTable s, 410, 88, Array(110, 65, 230), Array(_
    Array("传感器","引脚","说明"),_
    Array("灰度 CLK","PB7","8路灰度,串口协议"),_
    Array("灰度 DAT","PB8","1=白线,0=黑底"),_
    Array("色标 LEFT","PA8","左侧触发"),_
    Array("色标 RIGHT","PA9","→ 停车+倒车+转弯序列"),_
    Array("陀螺仪 SCL","PA0","JY61P  软件I2C"),_
    Array("陀螺仪 SDA","PA1","6轴姿态, 地址0x50")), COLOR_ORANGE, COLOR_DARK

Set tb = s.Shapes.AddTextbox(1, 30, 350, 200, 22)
tb.TextFrame.TextRange.Text = "按键"
tb.TextFrame.TextRange.Font.Size = 15
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

AddTable s, 30, 380, Array(80, 80), Array(_
    Array("按键","引脚"),_
    Array("KEY1","PB17"),_
    Array("KEY2","PB18"),_
    Array("KEY3","PB19")), COLOR_GRAY, COLOR_DARK

' ==== SLIDE 4: Communication ====
Set s = pres.Slides.Add(1, 1)
s.Shapes.AddShape(1, 0, 0, sw, sh).Fill.ForeColor.RGB = COLOR_BG

Set tb = s.Shapes.AddTextbox(1, 30, 15, 900, 35)
tb.TextFrame.TextRange.Text = "通信接口"
tb.TextFrame.TextRange.Font.Size = 22
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GREEN
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

AddTable s, 30, 70, Array(85, 100, 105, 170, 80, 130), Array(_
    Array("接口","TX","RX","用途","波特率","协议"),_
    Array("UART0","PA10","PA11","Zigbee 遥控","9600","文本帧"),_
    Array("UART2","PA21","PA24","MaixCAM2 钢珠","115200","0xAA 0x55 9字节"),_
    Array("I2C0","PA31(SCL)","PA28(SDA)","OLED 0.96寸","400k","硬件I2C"),_
    Array("GPIO I2C","PA0(SCL)","PA1(SDA)","JY61P 陀螺仪","~100k","软件模拟I2C")), COLOR_ACCENT, COLOR_DARK

Set tb = s.Shapes.AddTextbox(1, 30, 280, 900, 30)
tb.TextFrame.TextRange.Text = "Zigbee 数据格式"
tb.TextFrame.TextRange.Font.Size = 16
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_ACCENT
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

Set tb = s.Shapes.AddTextbox(1, 30, 315, 900, 70)
tb.TextFrame.TextRange.Text = "心跳: HB T:<tx> R:<rx> E:<echo>  (每秒1次)" & vbCrLf & _
    "Echo: 收到任意字节 → 原样回发" & vbCrLf & _
    "旧协议: Z<tx> S<state> D<dist> Y<yaw>"
tb.TextFrame.TextRange.Font.Size = 12
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Consolas"

Set tb = s.Shapes.AddTextbox(1, 30, 440, 900, 25)
tb.TextFrame.TextRange.Text = "资源占用: 28 GPIO · TIMA0 4/4 · I2C0 1/1 · UART0+UART2 2/4 · 软件I2C 1"
tb.TextFrame.TextRange.Font.Size = 13
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

' ==== SLIDE 5: Full pin list ====
Set s = pres.Slides.Add(1, 1)
s.Shapes.AddShape(1, 0, 0, sw, sh).Fill.ForeColor.RGB = COLOR_BG

Set tb = s.Shapes.AddTextbox(1, 30, 12, 900, 35)
tb.TextFrame.TextRange.Text = "完整引脚列表 (按端口排序)"
tb.TextFrame.TextRange.Font.Size = 20
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GREEN
tb.TextFrame.TextRange.Font.Bold = True
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

AddTable s, 15, 55, Array(55, 330), Array(_
    Array("PA","功能"),_
    Array("PA0","JY61P 陀螺仪 SCL (软件I2C)"),_
    Array("PA1","JY61P 陀螺仪 SDA (软件I2C)"),_
    Array("PA7","F_L 电机 IN1"),_
    Array("PA8","色标传感器 LEFT"),_
    Array("PA9","色标传感器 RIGHT"),_
    Array("PA10","UART0 TX  -> Zigbee"),_
    Array("PA11","UART0 RX  <- Zigbee"),_
    Array("PA12","F_L 电机 IN2"),_
    Array("PA13","F_R 电机 IN1"),_
    Array("PA14","B_L 电机 IN2"),_
    Array("PA15","F_L 电机 PWM  TIMA0_CH2"),_
    Array("PA16","F_R 电机 IN2"),_
    Array("PA21","UART2 TX  -> MaixCAM2"),_
    Array("PA23","B_R 电机 PWM  TIMA0_CH3"),_
    Array("PA24","UART2 RX  <- MaixCAM2"),_
    Array("PA25","编码器 L_B"),_
    Array("PA26","编码器 R_B"),_
    Array("PA28","OLED I2C0 SDA"),_
    Array("PA31","OLED I2C0 SCL")), COLOR_ACCENT, COLOR_DARK

AddTable s, 440, 55, Array(55, 270), Array(_
    Array("PB","功能"),_
    Array("PB7","灰度传感器 CLK"),_
    Array("PB8","灰度传感器 DAT"),_
    Array("PB9","B_L 电机 PWM  TIMA0_CH1"),_
    Array("PB13","B_R 电机 IN1"),_
    Array("PB14","F_R 电机 PWM  TIMA0_CH0"),_
    Array("PB15","B_R 电机 IN2"),_
    Array("PB16","B_L 电机 IN1"),_
    Array("PB17","KEY1 按键"),_
    Array("PB18","KEY2 按键"),_
    Array("PB19","KEY3 按键"),_
    Array("PB24","编码器 L_A"),_
    Array("PB25","编码器 R_A")), COLOR_ORANGE, COLOR_DARK

Set tb = s.Shapes.AddTextbox(1, 440, 460, 400, 25)
tb.TextFrame.TextRange.Text = "PA: 20  ·  PB: 12  ·  合计: 32  (28已分配)"
tb.TextFrame.TextRange.Font.Size = 12
tb.TextFrame.TextRange.Font.Color.RGB = COLOR_GRAY
tb.TextFrame.TextRange.Font.Name = "Microsoft YaHei"

' ==== Save ====
Dim outPath
outPath = "C:\msys64\home\wtl\Electronics-Design\MSPM0G3507_Pinout.pptx"
pres.SaveAs outPath
pres.Close
ppt.Quit

Set pres = Nothing
Set ppt = Nothing

MsgBox "PPT saved: " & outPath, vbInformation, "Done"

' Helper: Add table
Sub AddTable(sld, left, top, widths, data, hdrColor, bodyColor)
    Dim rows, cols, r, c, tw, tbl, cell
    rows = UBound(data) + 1
    cols = UBound(data(0)) + 1

    tw = 0
    For c = 0 To cols - 1
        tw = tw + widths(c)
    Next

    Set tbl = sld.Shapes.AddTable(rows, cols, left, top, tw, rows * 28)

    For r = 0 To rows - 1
        For c = 0 To cols - 1
            Set cell = tbl.Table.Cell(r + 1, c + 1)
            cell.Shape.TextFrame.TextRange.Text = data(r)(c)
            cell.Shape.TextFrame.TextRange.Font.Size = 11
            cell.Shape.TextFrame.TextRange.Font.Name = "Microsoft YaHei"
            cell.Shape.TextFrame.TextRange.ParagraphFormat.Alignment = 1 ' center

            If r = 0 Then
                cell.Shape.TextFrame.TextRange.Font.Bold = True
                cell.Shape.TextFrame.TextRange.Font.Color.RGB = COLOR_WHITE
                cell.Shape.Fill.ForeColor.RGB = hdrColor
            Else
                cell.Shape.TextFrame.TextRange.Font.Color.RGB = COLOR_WHITE
                cell.Shape.Fill.ForeColor.RGB = bodyColor
            End If
        Next
    Next

    For c = 0 To cols - 1
        tbl.Table.Columns(c + 1).Width = widths(c)
    Next
End Sub
