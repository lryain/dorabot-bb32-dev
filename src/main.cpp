#include <Arduino.h>
#include <HardwareSerial.h>
#include <string>
#include "encoder_driver.h"
#include "motor_driver.h"

// ESP32 原始串口定义
// Serial0 UART0 RX-3 TX-1
// Serial1 UART1 RX-9 TX-10
// Serial2 UART2 RX16 TX17

// 串口 0 连接到 USB 转串口芯片，主要用于调试
// 在这里可以作为和 ROS 通信的串口
// 串口 1 RX9 TX10 被 flash 暂用，需要重新映射端口
// 串口 2 RX16 TX17

// 新增加 ASR 静音控制 ESP32 PIN 
// const int ASR_MUTE_PIN = 12; // 12 RES 保留为 MTDI 不可以用
// MTDI = IO12 RES
// MTDO = IO15

const int ASR_RX_PIN = 13; // PB5-TX 13 MO/SDA
const int ASR_TX_PIN = -1; // 
const int ROS_RX_PIN = 14; // 14 DC -> ROS UART2_TX
const int ROS_TX_PIN = 15; // 15 CLK SLK TFT_CLK -> 25 MPU_INT2
const int BAT_ADC    = 34; // 电压检测

// for driver board Serial Ports
// Serial0 RX-3 TX-1                用于调试
// Serial1 RX--1 TX-13        用于和 ASRPRO 通信 PB5-TX-13(SDA)
// Serial2 RX-14 TX-15        用于和 ROS 通信

const int BAUD_RATE = 115200; // 波特率
char cmd_buffer[2048];
String recv_str;
int recv_cnt;
char ch = 0, l_ch = 0, ll_ch = 0, lll_ch = 0;
int l_pwm_set = 0;
int r_pwm_set = 0; 
#define PWM_RATIO 1.5 

// 电机 PWM 更新
int motor_pwm0 = 10; // 死区占空比，需要测

#pragma pack(1)
typedef struct
{
    // sizeof(McuData): 36
    // Sent: 53 54 24 0 D2 4 2E 16 1 0 2 0 3 0 4 0 5 0 6 0 7 0 8 0 9 0 74 E 1 0 0 55 56 D A 0
    // Serial.print(dataPtr[i], HEX); Serial.print(" ");
    //       53 54 24 00 D2 04 2E 16 01 00 02 00 03 00 04 00 05 00 06 00 07 00 08 00 09 00 74 0E 01 00 08 5556 0D 0A 00 = 36
    unsigned char head1;       // 数据头 1 'S' 0x53
    unsigned char head2;       // 数据头 2 'T' 0x54
    unsigned char struct_size; // 结构体长度
    // 1+1+1 2+2 2+2+2 2+1+1 1 1+1+1+1
    // 3 + 4 + 6 + 4 + 1+ 4 = 22
    short encoder1; // 编码器当前值 1
    short encoder2; // 编码器当前值 2

    short gyro[3];      // MPU6050 角速度
    short accel[3];     // MPU6050 线加速度
    short angle_100[3]; // 通过 DMP 模块读出来的四元数转的角度

    short vbat_mv;              // 电池电压 mV
    unsigned char charging;     // 是否正在充电
    unsigned char full_charged; // 是否充满电

    unsigned char asr_id; // 语音命令 ID

    unsigned char end1; // 数据尾 1 'U' 0x55
    unsigned char end2; // 数据尾 2 'V' 0x56
    unsigned char end3; // 数据尾 3 '\r' 0x0d
    unsigned char end4; // 数据尾 4 '\n' 0x0a
} McuData;

// Define the CmdData struct
typedef struct
{
    unsigned char head1; // 数据头 1 'S' 0x53
    unsigned char head2; // 数据头 2 'T' 0x54
    // struct_size=12
    unsigned char struct_size; // 结构体长度

    short pwm1;                 // 油门 PWM1
    short pwm2;                 // 油门 PWM2
    unsigned char enable_sound; // 是否开启声音

    unsigned char end1; // 数据尾 1 'U' 0x55
    unsigned char end2; // 数据尾 2 'V' 0x56
    unsigned char end3; // 数据尾 3 '\r' 0x0d
    unsigned char end4; // 数据尾 4 '\n' 0x0a
} CmdData;

#pragma pack()

McuData mcuData;
CmdData cmdData;

unsigned char hex_asr_id = 0; // 确保数组足够大以存储结果

void sendMcuData(const McuData &mcuData)
{
    const unsigned char *dataPtr = (const unsigned char *)&mcuData;
    // Sent to ROS
    Serial1.write(dataPtr, mcuData.struct_size);
    // struct_size: 36

    // Print for debug
    // Serial.print("ESP32 -> ROS [McuData]: ");
    // for (int i = 0; i < mcuData.struct_size; i++)
    // {
    //     Serial.print(dataPtr[i], HEX);
    //     Serial.printf("%#X ", dataPtr[i]);
    // }
    // Serial.println();
}

void sendMcuDataTask(void *parameter)
{
    while (true)
    {
        // Dummy     数据填充
        mcuData.head1 = 0x53; // 'S'
        mcuData.head2 = 0x54; // 'T'
        // mcuData.struct_size = static_cast<unsigned char>(sizeof(McuData));
        mcuData.struct_size = sizeof(McuData);

        mcuData.encoder1 = (short)readEncoder(LEFT);
        mcuData.encoder2 = (short)readEncoder(RIGHT);

        mcuData.gyro[0] = 0x1111;
        mcuData.gyro[1] = 0x2222;
        mcuData.gyro[2] = 0x3333;

        mcuData.accel[0] = 0x4444;
        mcuData.accel[1] = 0x5555;
        mcuData.accel[2] = 0x6666;

        mcuData.angle_100[0] = 0x7777;
        mcuData.angle_100[1] = 0x8888;
        mcuData.angle_100[2] = 0x9999;
        // 3700 => 0E74
        mcuData.vbat_mv = 0x0E74;
        mcuData.charging = 0x00;
        mcuData.full_charged = 0x00;

        // 格式化为 16 进制字符串
        // sprintf(hex_str, "%02x", hex_asr_id);
        mcuData.asr_id = hex_asr_id;

        mcuData.end1 = 0x55; // 'U'
        mcuData.end2 = 0x56; // 'V'
        mcuData.end3 = 0x0D; // '\r'
        mcuData.end4 = 0x0A; // '\n'

        sendMcuData(mcuData);
        // 发送完成后对 asr_id 进行复位
        hex_asr_id = 0x00;
        resetEncoder(LEFT);
        resetEncoder(RIGHT);
        vTaskDelay(10 / portTICK_PERIOD_MS); // 延迟 1 秒
    }
}

// Task to handle receiving CmdData from UART
void receiveCmdDataTask(void *pvParameters)
{
    bool recv_ok_flag = false;

    // Serial.printf("sizeof(cmd_buffer): [%d]", sizeof(cmd_buffer)); // =1024
    while (!recv_ok_flag)
    {
        // Serial.println("-> recv_ok_flag not ok!!");
        if (Serial1.available())
        {
            // Read bytes from serial port into CmdData struct
            Serial1.readBytes((char *)&cmdData, sizeof(CmdData));
            // // Process received CmdData here
            if (cmdData.head1 == 'S' && cmdData.head2 == 'T' &&
                cmdData.end1 == 'U' && cmdData.end2 == 'V' &&
                cmdData.end3 == '\r' && cmdData.end4 == '\n')
            {
                l_pwm_set = map(cmdData.pwm1, 0, 6400, 0, 250) * PWM_RATIO;
                r_pwm_set = map(cmdData.pwm2, 0, 6400, 0, 250) * PWM_RATIO;

                // Serial.printf("ROS -> ESP32 [CmdData]: pwm1: %d pwm2: %d lpwm: %d rpwm: %d \n", cmdData.pwm1, cmdData.pwm2, l_pwm_set, r_pwm_set);
                // // resetEncoders();
                // // CmdData is valid, process it
                // Serial.print("Received [CmdData] from ROS: ");
                // Serial.print("PWM1=");
                // Serial.print(cmdData.pwm1);
                // Serial.print(", PWM2=");
                // Serial.print(cmdData.pwm2);
                // Serial.print(", enable_sound=");
                // Serial.println(cmdData.enable_sound);

            }else{
                // Invalid CmdData, discard it
                // Serial.println("!!!!!!!!!!!!!!!!!!Invalid CmdData received.");
                l_pwm_set = 0;
                r_pwm_set = 0;
            }
        }else{
            // 串口没有数据的情况
            // resetEncoders();
            // Serial.println("-----串口没有数据的情况----");
            l_pwm_set = 0;
            r_pwm_set = 0;
        }
        Serial.printf("l_pwm_set= %d  , r_pwm_set= %d\n", l_pwm_set, r_pwm_set);
        setSpeeds(l_pwm_set, r_pwm_set);
        vTaskDelay(10 / portTICK_PERIOD_MS); // Delay to prevent tight loop
    }

}

// Task to handle receiving id message from UART
void receiveASRIdTask(void *pvParameters)
{
    while (1)
    {
        // 检查是否有数据可用
        if (Serial2.available() > 0)
        {
            // 读取字符串
            int intId = Serial2.readStringUntil('\n').toInt();
            hex_asr_id = (unsigned char)intId;

            Serial.print("Received intId: ");
            Serial.println(intId);
            Serial.print("Received hex_asr_id: ");
            Serial.println(hex_asr_id);
        }
        vTaskDelay(100 / portTICK_PERIOD_MS); // Delay
    }
}

void setup()
{
    Serial.begin(BAUD_RATE);                                      // 用于调试
    Serial1.begin(BAUD_RATE, SERIAL_8N1, ROS_RX_PIN, ROS_TX_PIN); // 用于和 ROS 通信
    Serial2.begin(BAUD_RATE, SERIAL_8N1, ASR_RX_PIN, ASR_TX_PIN); // 用于和 ASR_PRO 通信
    // pinMode(ASR_MUTE_PIN, output);
    // digitalWrite(ASR_MUTE_PIN, LOW);
    // 初始化编码器
    Init_Encoder();
    delay(1000); // Wait for serial to initialize
    Serial.println("UART Initialized");

    // 创建发送数据的任务
    xTaskCreate(
        sendMcuDataTask,   // 任务函数
        "sendMcuDataTask", // 任务名称
        2048,              // 任务堆栈大小
        NULL,              // 任务参数
        1,                 // 任务优先级
        NULL               // 任务句柄
    );

    // 创建接收数据的任务
    // 与 xTaskCreate 函数相比，xTaskCreatePinnedToCore 函数具有额外的参数，可以指定任务运行在哪个核心上。在 ESP32 上有两个主要的 CPU 核心：核心 0 和核心 1。
    xTaskCreate(
        receiveCmdDataTask,
        "receiveCmdDataTask",
        2048,
        NULL,
        1,
        NULL);

    // ASRID 接受任务
    xTaskCreate(receiveASRIdTask, "receiveASRIdTask", 1024, NULL, 1, NULL);
}

void loop()
{
    // 主循环不做任何事，所有工作在任务中完成
    // setSpeeds(l_pwm_set, r_pwm_set);
    // delay(10); // Wait for serial to initialize
    // l_pwm_set = 0;
    // r_pwm_set = 0;
    // // Serial.println("ccw ");
    // setSpeeds(-30, -30);
    // delay(2000); // Wait for serial to initialize
    // vTaskDelay(pdMS_TO_TICKS(1000));
}
