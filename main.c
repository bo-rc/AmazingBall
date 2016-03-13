#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

#include <time.h>
#include "lcd.h"
#include "flextouch.h"
#include "flexmotor.h"
#include "algorithm.h"
#include "pid_controller.h"
#include "Debouncer.h"

#define ADC_SAMPLES (5)

#define KP_X (0.04)
#define KI_X (0.02)
#define KD_X (0.02)

#define KP_Y (0.02)
#define KI_Y (0.01)
#define KD_Y (0.01)

#define Set_x (1500)
#define Set_y (1500)

#define BOUNDARY_BUF (50)

/* Initial configuration by EE */
// Primary (XT, HS, EC) Oscillator with PLL
_FOSCSEL(FNOSC_PRIPLL);

// OSC2 Pin Function: OSC2 is Clock Output - Primary Oscillator Mode: XT Crystal
_FOSC(OSCIOFNC_OFF & POSCMD_XT);

// Watchdog Timer Enabled/disabled by user software
_FWDT(FWDTEN_OFF);

// Disable Code Protection
_FGS(GCP_OFF);

void init_timer3()
{
   CLEARBIT(T3CONbits.TON);   // Disable Timer
   CLEARBIT(T3CONbits.TCS);   //Select internal instruction cycle clock (12.8 MHz)
   CLEARBIT(T3CONbits.TGATE); //Disable Gated Timer mode
   TMR3 = 0x00;               //Clear Timer Register
   T3CONbits.TCKPS = 0b10;    //Set Prescaler (1:64)
   PR3 = 10000;                //Set Period
   IPC2bits.T3IP = 0x01;      //Set IPL
   CLEARBIT(IFS0bits.T3IF);   //Clear IF
   SETBIT(IEC0bits.T3IE);     //Enable INT
   SETBIT(T3CONbits.TON);     //Enable Timer
}

void init_timer1()
{
   CLEARBIT(T1CONbits.TON);   // Disable Timer
   CLEARBIT(T1CONbits.TCS);   //Select internal instruction cycle clock (12.8 MHz)
   CLEARBIT(T1CONbits.TGATE); //Disable Gated Timer mode
   TMR1 = 0x00;               //Clear Timer Register
   T1CONbits.TCKPS = 0b10;    //Set Prescaler (1:64)
   PR1 = 10000;                //Set Period
   IPC0bits.T1IP = 0x01;      //Set IPL
   CLEARBIT(IFS0bits.T1IF);   //Clear IF
   SETBIT(IEC0bits.T1IE);     //Enable INT
   SETBIT(T1CONbits.TON);     //Enable Timer
}

uint16_t samples[ADC_SAMPLES];
uint16_t i = 0;
uint16_t x_set = Set_x;
uint16_t x_current = Set_x;
uint16_t y_set = Set_y;
uint16_t y_current = Set_y;
uint16_t js_x;
uint16_t js_y;

pid_controller_t controller_x;
pid_controller_t controller_y;
uint16_t duty_x, duty_y;

void
__attribute__ (( __interrupt__, no_auto_psv )) _T3Interrupt(void)
{
    touch_select_dim(DIM_X);
    for (i = 0; i < ADC_SAMPLES; ++i) {
        Delay_ms(1);
        samples[i] = touch_adc();
    }

    x_current = find_median(samples, ADC_SAMPLES);

    //motor_set_duty(CHANNEL_X, feed_back(&controller_x, x_current, x_set));

    duty_x = feed_back(&controller_x, x_current, x_set);
    motor_set_duty(CHANNEL_X, duty_x);

    CLEARBIT(IFS0bits.T3IF);
}

void
__attribute__ (( __interrupt__, no_auto_psv )) _T1Interrupt(void)
{
    touch_select_dim(DIM_Y);
    for (i = 0; i < ADC_SAMPLES; ++i) {
        Delay_ms(1);
        samples[i] = touch_adc();
    }

    y_current = find_median(samples, ADC_SAMPLES);

    // motor_set_duty(CHANNEL_Y, feed_back(&controller_y, y_current, y_set));

    duty_y = feed_back(&controller_y, y_current, y_set);
    motor_set_duty(CHANNEL_Y, duty_y);

    CLEARBIT(IFS0bits.T1IF);
}

void init_trigger(){
    SETBIT(TRISEbits.TRISE8); // input
    SETBIT(AD1PCFGHbits.PCFG20);
    SETBIT(IEC1bits.INT1IE); // enable interrupt
    SETBIT(INTCON2bits.INT1EP); // enable FALLING edge-triggered external interrupt 1
    IPC5bits.INT1IP1 = 0x01; // interrupt priority
}

void __attribute__ ((__interrupt__, no_auto_psv)) _INT1Interrupt(void){

    x_set = js_x;
    y_set = js_y;

   CLEARBIT(IFS1bits.INT1IF);
}

int main(){
	//Init LCD
	__C30_UART=1;
	lcd_initialize();
	lcd_clear();
	lcd_locate(0,0);

	lcd_printf("Lab08");

    init_adc1(); // for flextouch
    touch_init();
    
    init_adc2(); // for joystick
    init_trigger();

    Debouncer thumb; // thumb
    uint16_t thumbStatus = UNSTABLE;

    // calibrate joystick and flextouch
    SETBIT(TRISDbits.TRISD10); // set thumb as togle

    uint16_t X;
    uint16_t Xmin = 0xffff;
    uint16_t Xmax = 0x0000;

    uint16_t Y;
    uint16_t Ymin = 0xffff;
    uint16_t Ymax = 0x0000;

    uint16_t TX;
    uint16_t TXmin = 0xffff;
    uint16_t TXmax = 0x0000;

    uint16_t TY;
    uint16_t TYmin = 0xffff;
    uint16_t TYmax = 0x0000;

    uint8_t calibrate_flag = 1;

    lcd_locate(0,2);
    lcd_printf("Calibrate joystick:");
    lcd_locate(0,3);
    lcd_printf("move from corner to corner");

    while(calibrate_flag) {
        // calibrate x
        AD2CHS0bits.CH0SA = 0x004; //set ADC to Sample AN4 pin, Joystick X-axis
        SETBIT(AD2CON1bits.SAMP); //start to sample
        while(!AD2CON1bits.DONE); // wait for conversion to finish
        CLEARBIT(AD2CON1bits.DONE); // MUST HAVE! clear conversion done bit
        X = ADC2BUF0;

        Xmax = (Xmax > X) ? Xmax : X;

        AD2CHS0bits.CH0SA = 0x005; //set ADC to Sample AN5 pin, Joystick Y-axis
        SETBIT(AD2CON1bits.SAMP); //start to sample
        while(!AD2CON1bits.DONE); // wait for conversion to finish
        CLEARBIT(AD2CON1bits.DONE); // MUST HAVE! clear conversion done bit
        Y = ADC2BUF0;

        Ymax = (Ymax > Y) ? Ymax : Y;

        button_sample(&thumb, PORTDbits.RD10);
        thumbStatus = button_debounced(&thumb);
        if (thumbStatus == 0)
        {
            calibrate_flag = 0;
        }

    }

    calibrate_flag = 1;
    thumbStatus = UNSTABLE;

    lcd_locate(0,5);
    lcd_printf("Calibrate touchscreen:");
    lcd_locate(0,6);
    lcd_printf("move ball from corner to corner");

    while(calibrate_flag) {
        // calibrate x
        touch_select_dim(DIM_X);

        for (i = 0; i < ADC_SAMPLES; ++i) {
            Delay_ms(1);
            samples[i] = touch_adc();
        }

        TX = find_median(samples, ADC_SAMPLES);

        TXmax = (TXmax > TX) ? TXmax : TX;

        touch_select_dim(DIM_Y);

        for (i = 0; i < ADC_SAMPLES; ++i) {
            Delay_ms(1);
            samples[i] = touch_adc();
        }

        TY = find_median(samples, ADC_SAMPLES);

        TYmax = (TYmax > TY) ? TYmax : TY;

        button_sample(&thumb, PORTDbits.RD10);
        thumbStatus = button_debounced(&thumb);
        if (thumbStatus == 0)
        {
            calibrate_flag = 0;
        }

    }

    // initialization the motor and timer interrupts for controls
    lcd_clear();
    lcd_locate(0,0);
    lcd_printf("Lab08");

    motor_init(CHANNEL_X);
    motor_set_duty(CHANNEL_X, MID);
    motor_init(CHANNEL_Y);
    motor_set_duty(CHANNEL_Y, MID);

    init_timer3(); // for timer interrupt
    init_timer1();

    pid_controller_init(&controller_x, 290, 1000, 0.05, KP_X, KI_X, KD_X);
    pid_controller_init(&controller_y, 290, 1500, 0.05, KP_Y, KI_Y, KD_Y);

    uint16_t loopCounter = 0;
    // display_arena();

    while(1) {      

        if (loopCounter++ >= 2000) {
            loopCounter = 0;

            // sample Joystick
            AD2CHS0bits.CH0SA = 0x004; //set ADC to Sample AN4 pin, Joystick X-axis
            SETBIT(AD2CON1bits.SAMP); //start to sample
            while(!AD2CON1bits.DONE); // wait for conversion to finish
            CLEARBIT(AD2CON1bits.DONE); // MUST HAVE! clear conversion done bit
            // TODO: need to do a mapping from ADC2BUF0 to js_x
            js_x = ((float)(Xmax + BOUNDARY_BUF - ADC2BUF0)/(Xmax + BOUNDARY_BUF - Xmin))*(TXmax - TXmin) + TXmin;

            AD2CHS0bits.CH0SA = 0x005; //set ADC to Sample AN5 pin, Joystick Y-axis
            SETBIT(AD2CON1bits.SAMP); //start to sample
            while(!AD2CON1bits.DONE); // wait for conversion to finish
            CLEARBIT(AD2CON1bits.DONE); // MUST HAVE! clear conversion done bit
            // TODO: need to do a mapping from ADC2BUF0 to js_y
            js_y = ((float)(Ymax + BOUNDARY_BUF - ADC2BUF0)/(Ymax + BOUNDARY_BUF - Ymin))*(TYmax - TYmin) + TYmin;

            lcd_locate(0,1);
            lcd_printf("js_x:%d     ", js_x);
            lcd_locate(0,2);
            lcd_printf("js_y:%d     ", js_y);

            lcd_locate(0,3);
            lcd_printf("x_set:%d    ", x_set);
            lcd_locate(0,4);
            lcd_printf("y_set:%d    ", y_set);

            lcd_locate(0,5);
            lcd_printf("x:%d    ", x_current);
            lcd_locate(0,6);
            lcd_printf("y:%d    ", y_current);

        // display(x_set, y_set, js_x, js_y);
        }
        
    }

    return 0;
}
