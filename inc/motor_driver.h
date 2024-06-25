// 电机 PWM 控制信号
// #define MOTOR_PWM_L1_CHANNEL 0
// #define MOTOR_PWM_L1 2 // -> PWML
// #define MOTOR_PWM_L2_CHANNEL 1
// #define MOTOR_PWM_L2 4 // -> DIRL
// #define MOTOR_PWM_R1_CHANNEL 2
// #define MOTOR_PWM_R1 22 // -> PWMR
// #define MOTOR_PWM_R2_CHANNEL 3
// #define MOTOR_PWM_R2 23 // -> DIRR

// //电机 PWM 更新
// int motor_pwm0 = 200;//死区占空比，需要测
// float pid0_out = 0;//速度外环 PID 输出
// float pid1_out = 0;//角度内环 PID 输出
// float pid2_out = 0;//转向环 PID 输出
// float mannual_set_l = 0;//自定义左电机输入 PWM
// float mannual_set_r = 0;//自定义右电机输入 PWM
// int l_pwm_set = 0;
// int r_pwm_set = 0;

// //电机测速更新
// float s_motor_l = 0;//左电机速度
// float s_motor_r = 0;//右电机速度
// volatile int s_counter_l = 0;//左电机速度信号捕获
// volatile int s_counter_r = 0;//右电机速度信号捕获

// 左边电机转动方向控制位 引脚
// 左边电机转动方向控制位 引脚
#define Back_Left_D1 2
#define Back_Left_D1_B 4

// 右边电机转动方向控制位 引脚
// 右边电机转动方向控制位 引脚
#define Back_Right_D1 22
#define Back_Right_D1_B 23

void Init_Motors()
{
    // 电机控制信号初始化
    // ledcSetup(MOTOR_PWM_L1_CHANNEL, 10000, 10); // 设置 LEDC 通道 8 频率为 10kHz，分辨率为 16 位，即占空比可选 0~1024
    // ledcSetup(MOTOR_PWM_L2_CHANNEL, 10000, 10);
    // ledcSetup(MOTOR_PWM_R1_CHANNEL, 10000, 10);
    // ledcSetup(MOTOR_PWM_R2_CHANNEL, 10000, 10);
    // ledcAttachPin(MOTOR_PWM_L1, MOTOR_PWM_L1_CHANNEL); // 将 LEDC 通道绑定到指定 IO 口上以实现输出
    // ledcAttachPin(MOTOR_PWM_L2, MOTOR_PWM_L2_CHANNEL);
    // ledcAttachPin(MOTOR_PWM_R1, MOTOR_PWM_R1_CHANNEL);
    // ledcAttachPin(MOTOR_PWM_R2, MOTOR_PWM_R2_CHANNEL);
    // ledcWrite(MOTOR_PWM_L1_CHANNEL, 500); // 指定通道输出一定占空比波形
    // ledcWrite(MOTOR_PWM_L2_CHANNEL, 500);
    // ledcWrite(MOTOR_PWM_R1_CHANNEL, 500);
    // ledcWrite(MOTOR_PWM_R2_CHANNEL, 500);

    pinMode(Back_Left_D1, OUTPUT);
    pinMode(Back_Left_D1_B, OUTPUT);

    pinMode(Back_Right_D1, OUTPUT);
    pinMode(Back_Right_D1_B, OUTPUT);
}

void setSpeeds(int m1Speed, int m2Speed)
{
    // 控制左侧电机
    if (m1Speed > 0)
    {
        analogWrite(Back_Left_D1, m1Speed);
        analogWrite(Back_Left_D1_B, LOW);
    }
    else
    {
        analogWrite(Back_Left_D1, LOW);
        analogWrite(Back_Left_D1_B, -m1Speed);
    }

    // 控制右侧电机
    if (m2Speed > 0)
    {
        analogWrite(Back_Right_D1, m2Speed);
        analogWrite(Back_Right_D1_B, LOW);
    }
    else
    {
        analogWrite(Back_Right_D1, LOW);
        analogWrite(Back_Right_D1_B, -m2Speed);
    }
}

// void motor_duty_update(int m1Speed, int m2Speed)
// {
//     // 执行电机测速更新  motor_speed_update();
//     s_motor_l = s_counter_l;
//     s_motor_r = s_counter_r;
//     s_counter_l = 0;
//     s_counter_r = 0;
//     // 执行电机 PWM 更新 motor_duty_update();
//     l_pwm_set = m1Speed + m2Speed + mannual_set_l;
//     r_pwm_set = m1Speed - m2Speed + mannual_set_r;

//     l_pwm_set = constrain(l_pwm_set, -500 + motor_pwm0, 500 - motor_pwm0);
//     r_pwm_set = constrain(r_pwm_set, -500 + motor_pwm0, 500 - motor_pwm0);

//     if ((l_pwm_set) < 0)
//     {
//         l_pwm_set -= motor_pwm0;
//         ledcWrite(MOTOR_PWM_L1_CHANNEL, +abs(l_pwm_set) + 500);
//         ledcWrite(MOTOR_PWM_L2_CHANNEL, -abs(l_pwm_set) + 500);
//     }
//     else
//     {
//         l_pwm_set += motor_pwm0;
//         ledcWrite(MOTOR_PWM_L1_CHANNEL, -abs(l_pwm_set) + 500);
//         ledcWrite(MOTOR_PWM_L2_CHANNEL, +abs(l_pwm_set) + 500);
//     }

//     if ((r_pwm_set) < 0)
//     {
//         r_pwm_set -= motor_pwm0;
//         ledcWrite(MOTOR_PWM_R1_CHANNEL, -abs(r_pwm_set) + 500);
//         ledcWrite(MOTOR_PWM_R2_CHANNEL, +abs(r_pwm_set) + 500);
//     }
//     else
//     {
//         r_pwm_set += motor_pwm0;
//         ledcWrite(MOTOR_PWM_R1_CHANNEL, +abs(r_pwm_set) + 500);
//         ledcWrite(MOTOR_PWM_R2_CHANNEL, -abs(r_pwm_set) + 500);
//     }
// }