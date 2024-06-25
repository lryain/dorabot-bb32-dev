/*****************************************************************
Motor Encoder Information:

  Pluse Per Rotation: 11
  Reduction Ratio: 30
  Motor Voltage: 12V
  Hall Encoder Voltage: 5.0V
  Total pluse per Rotation:
    11(PPR) * 2(Hall) * 2(CHANGE) * 30(Reduction Ratio) =1320

*****************************************************************/
// #define MOTOR_SPWM_L 18
// #define MOTOR_SDIR_L 5
// #define MOTOR_SPWM_R 16         // -> 25
// #define MOTOR_SDIR_R 17         // -> 19
#define LEFT_ENCODER_A 18 // Pin for Left Encoder A Pin
#define LEFT_ENCODER_B 5  // Pin for Left Encoder B Pin DIR

#define RIGHT_ENCODER_A 16 // Pin for Right Encoder A Pin
#define RIGHT_ENCODER_B 17 // Pin for Right Encoder B Pin

#define LEFT 0
#define RIGHT 1

long leftPosition = 0;  // Global variable for storing the encoder position
long rightPosition = 0; // Global variable for storing the encoder position

long current_leftPosition = 0;  // Global variable for storing the current encoder position
long current_rightPosition = 0; // Global variable for storing the current encoder position

void Left_encoder_isr()
{

    // check channel B to see which way encoder is turning
    if (digitalRead(LEFT_ENCODER_B) == HIGH)
    {
        current_leftPosition++; // CW
    }
    if (digitalRead(LEFT_ENCODER_B) == LOW)
    {
        current_leftPosition--; // CW
    }
}

void Right_encoder_isr()
{
    // check channel B to see which way encoder is turning
    if (digitalRead(RIGHT_ENCODER_B) == HIGH)
    {
        current_rightPosition++; // CW
    }
    if (digitalRead(RIGHT_ENCODER_B) == LOW)
    {
        current_rightPosition--; // CW
    }
}

long readEncoder(int i)
{
    if (i == LEFT)
    {
        return (current_leftPosition - leftPosition);
    }
    else
    {
        return (current_rightPosition - rightPosition);
    }
}

void resetEncoder(int i)
{
    if (i == LEFT)
    {
        leftPosition = current_leftPosition;
    }
    else
    {
        rightPosition = current_rightPosition;
    }
}

void resetEncoders()
{
    resetEncoder(LEFT);
    resetEncoder(RIGHT);
}

void Init_Encoder()
{
    resetEncoders();
    pinMode(LEFT_ENCODER_A, INPUT);
    pinMode(LEFT_ENCODER_B, INPUT);

    pinMode(RIGHT_ENCODER_A, INPUT);
    pinMode(RIGHT_ENCODER_B, INPUT);

    // Attaching the ISR to encoder Left
    attachInterrupt(digitalPinToInterrupt(LEFT_ENCODER_A), Left_encoder_isr, CHANGE);

    // Attaching the ISR to encoder Rigth
    attachInterrupt(digitalPinToInterrupt(RIGHT_ENCODER_A), Right_encoder_isr, CHANGE);
}