$ErrorActionPreference = "Stop"
$ppt = New-Object -ComObject PowerPoint.Application
$pres = $ppt.Presentations.Add()

$sw = $pres.PageSetup.SlideWidth
$sh = $pres.PageSetup.SlideHeight
$C_BG = 0x1A1A2E; $C_ACC = 0x00D2FF; $C_W = 0xFFFFFF; $C_G = 0xCCCCCC
$C_GRN = 0x00E676; $C_ORG = 0xFFAB40; $C_DK = 0x0D0D1A

function B($s,$l,$t,$w,$h,$c){$b=$s.Shapes.AddShape(1,$l,$t,$w,$h);$b.Fill.ForeColor.RGB=$c;$b.Line.Visible=$false}
function T($s,$l,$t,$w,$h,$x,$f,$c,$bold=$false){
    $tb=$s.Shapes.AddTextbox(1,$l,$t,$w,$h);$r=$tb.TextFrame.TextRange
    $r.Text=$x;$r.Font.Size=$f;$r.Font.Color.RGB=$c;$r.Font.Bold=$bold;$r.Font.Name="Microsoft YaHei"
    $r.ParagraphFormat.Alignment=1
}
function M($s,$l,$t,$data,$ws,$hc,$bc){
    $rs=$data.Count;$cs=$data[0].Count;$tw=($ws|Measure -Sum).Sum
    $tb=$s.Shapes.AddTable($rs,$cs,$l,$t,$tw,$rs*28)
    for($r=0;$r-lt$rs;$r++){for($c=0;$c-lt$cs;$c++){
        $cl=$tb.Table.Cell($r+1,$c+1);$cl.Shape.TextFrame.TextRange.Text=$data[$r][$c]
        $cl.Shape.TextFrame.TextRange.Font.Size=11;$cl.Shape.TextFrame.TextRange.Font.Name="Microsoft YaHei"
        $cl.Shape.TextFrame.TextRange.ParagraphFormat.Alignment=1
        if($r-eq0){$cl.Shape.TextFrame.TextRange.Font.Bold=$true;$cl.Shape.TextFrame.TextRange.Font.Color.RGB=$C_W
                   $cl.Shape.Fill.ForeColor.RGB=$hc}
        else{$cl.Shape.TextFrame.TextRange.Font.Color.RGB=$C_W;$cl.Shape.Fill.ForeColor.RGB=$bc}
    }}
    for($c=0;$c-lt$cs;$c++){$tb.Table.Columns($c+1).Width=$ws[$c]}
}

# Slide 1 - Title
$s=$pres.Slides.Add(1,1); B $s 0 0 $sw $sh $C_BG; B $s 40 80 8 200 $C_ACC
T $s 70 100 860 60 "MSPM0G3507 Pin Map" 40 $C_W $true
T $s 70 170 860 40 "Amy 64pin 4-Wheel Line-Following Robot" 22 $C_G
T $s 70 230 860 30 "UART0=Zigbee . UART2=MaixCAM2 . I2C0=OLED . TIMA0=4ch PWM" 14 $C_G
T $s 70 340 860 20 "Electronics-Design | 2025-07" 13 $C_G

# Slide 2 - Motor
$s=$pres.Slides.Add(1,1); B $s 0 0 $sw $sh $C_BG
T $s 30 15 900 35 "Motor Driver - TB6612 (4ch PWM, TIMA0)" 22 $C_GRN $true
M $s 30 65 @(
    @("Motor","PWM Ch","IN1","IN2","Direction (forward)"),
    @("F_L FrontL","PA15 CH2","PA7","PA12","Swapped: IN1=0,IN2=1"),
    @("F_R FrontR","PB14 CH0","PA13","PA16","Normal: IN1=1,IN2=0"),
    @("B_L BackL","PB9  CH1","PB16","PA14","Normal: IN1=1,IN2=0"),
    @("B_R BackR","PA23 CH3","PB13","PB15","Normal: IN1=1,IN2=0")
) @(110,120,60,60,220) $C_ACC $C_DK
T $s 30 260 900 30 "API: motor_control(left, right)  range -1000 ~ +1000" 15 $C_ACC $true
T $s 30 295 900 25 "Left side=F_L+B_L same speed | Right side=F_R+B_R same speed (skid steer)" 13 $C_G

# Car diagram
$bx = $s.Shapes.AddShape(1,200,370,170,50); $bx.Fill.ForeColor.RGB=$C_DK; $bx.Line.ForeColor.RGB=$C_ACC
T $s 210 377 150 35 "F_L    FRONT    F_R`nL <--      --> R" 10 $C_G
$bx = $s.Shapes.AddShape(1,200,440,170,50); $bx.Fill.ForeColor.RGB=$C_DK; $bx.Line.ForeColor.RGB=$C_ACC
T $s 210 447 150 35 "B_L     REAR    B_R`nL <--      --> R" 10 $C_G

# Slide 3 - Sensors & Encoders
$s=$pres.Slides.Add(1,1); B $s 0 0 $sw $sh $C_BG
T $s 30 15 900 35 "Sensors & Encoders" 22 $C_GRN $true

T $s 30 60 200 22 "Encoders" 15 $C_ACC $true
M $s 30 88 @(
    @("Signal","Pin","Note"),
    @("L_A","PB24",""),
    @("L_B","PA25","MM_PER_COUNT"),
    @("R_A","PB25","= 10.53 mm"),
    @("R_B","PA26","simple counting")
) @(70,70,200) $C_ACC $C_DK

T $s 410 60 200 22 "Sensors" 15 $C_ORG $true
M $s 410 88 @(
    @("Sensor","Pin","Description"),
    @("Gray CLK","PB7","8ch grayscale serial"),
    @("Gray DAT","PB8","1=white line, 0=black"),
    @("Color LEFT","PA8","left trigger"),
    @("Color RIGHT","PA9","-> stop+back+turn seq"),
    @("Gyro SCL","PA0","JY61P bit-bang I2C"),
    @("Gyro SDA","PA1","6-axis, addr 0x50")
) @(110,65,235) $C_ORG $C_DK

T $s 30 350 200 22 "Buttons" 15 $C_G $true
M $s 30 380 @(@("Key","Pin"),@("KEY1","PB17"),@("KEY2","PB18"),@("KEY3","PB19")) @(80,80) $C_G $C_DK

# Slide 4 - Communication
$s=$pres.Slides.Add(1,1); B $s 0 0 $sw $sh $C_BG
T $s 30 15 900 35 "Communication Interfaces" 22 $C_GRN $true
M $s 30 70 @(
    @("Interface","TX","RX","Usage","Baud","Protocol"),
    @("UART0","PA10","PA11","Zigbee remote/status","9600","text frames"),
    @("UART2","PA21","PA24","MaixCAM2 ball detect","115200","0xAA 0x55 9-byte"),
    @("I2C0","PA31(SCL)","PA28(SDA)","OLED 0.96in 4-line","400k","HW I2C"),
    @("GPIO I2C","PA0(SCL)","PA1(SDA)","JY61P gyro","~100k","bit-bang I2C")
) @(85,105,105,160,75,130) $C_ACC $C_DK

T $s 30 280 900 30 "Zigbee Data Format" 16 $C_ACC $true
T $s 30 315 900 80 "Heartbeat: HB T:<tx> R:<rx> E:<echo>  (every 1s)`nEcho mode: receive any byte -> send back unchanged`nOld protocol: Z<tx> S<state> D<dist> Y<yaw>" 12 $C_G
T $s 30 440 900 25 "Resources: 28 GPIO . TIMA0 4/4ch . I2C0 1/1 . UART0+UART2 2/4 . SW I2C 1" 13 $C_G

# Slide 5 - Full pin list
$s=$pres.Slides.Add(1,1); B $s 0 0 $sw $sh $C_BG
T $s 30 12 900 35 "Full Pin List (by port)" 20 $C_GRN $true

M $s 15 55 @(
    @("PA","Function"),
    @("PA0","Gyro SCL (bit-bang I2C)"),
    @("PA1","Gyro SDA (bit-bang I2C)"),
    @("PA7","F_L motor IN1"),
    @("PA8","Color sensor LEFT"),
    @("PA9","Color sensor RIGHT"),
    @("PA10","UART0 TX -> Zigbee"),
    @("PA11","UART0 RX <- Zigbee"),
    @("PA12","F_L motor IN2"),
    @("PA13","F_R motor IN1"),
    @("PA14","B_L motor IN2"),
    @("PA15","F_L motor PWM TIMA0_CH2"),
    @("PA16","F_R motor IN2"),
    @("PA21","UART2 TX -> MaixCAM2"),
    @("PA23","B_R motor PWM TIMA0_CH3"),
    @("PA24","UART2 RX <- MaixCAM2"),
    @("PA25","Encoder L_B"),
    @("PA26","Encoder R_B"),
    @("PA28","OLED I2C0 SDA"),
    @("PA31","OLED I2C0 SCL")
) @(55,340) $C_ACC $C_DK

M $s 450 55 @(
    @("PB","Function"),
    @("PB7","Grayscale sensor CLK"),
    @("PB8","Grayscale sensor DAT"),
    @("PB9","B_L motor PWM TIMA0_CH1"),
    @("PB13","B_R motor IN1"),
    @("PB14","F_R motor PWM TIMA0_CH0"),
    @("PB15","B_R motor IN2"),
    @("PB16","B_L motor IN1"),
    @("PB17","KEY1 button"),
    @("PB18","KEY2 button"),
    @("PB19","KEY3 button"),
    @("PB24","Encoder L_A"),
    @("PB25","Encoder R_A")
) @(55,280) $C_ORG $C_DK

T $s 450 460 400 25 "PA:20 . PB:12 . Total:32 (28 assigned)" 12 $C_G

# Save
$out = "C:\msys64\home\wtl\Electronics-Design\MSPM0G3507_Pinout.pptx"
$pres.SaveAs($out)
$pres.Close()
[System.Runtime.InteropServices.Marshal]::ReleaseComObject($pres) | Out-Null
[System.Runtime.InteropServices.Marshal]::ReleaseComObject($ppt) | Out-Null
Write-Output "DONE: $out"
