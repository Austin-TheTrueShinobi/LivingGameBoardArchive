#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "servo.h"

uint slice_num = 0;
uint chan = 0;

void motor_init(uint gpio){
    gpio_set_function(gpio, GPIO_FUNC_PWM);
    slice_num = pwm_gpio_to_slice_num(gpio);
    chan =  pwm_gpio_to_channel(gpio);
    pwm_set_irq_enabled(slice_num, false);
    pwm_config config = pwm_get_default_config();
    // pwm_config_set_clkdiv_int(&config, 40);
    pwm_config_set_clkdiv(&config, 40.f);
    pwm_config_set_wrap(&config, 0xF424U);
    pwm_set_chan_level(slice_num, chan, 0xC35U);
    pwm_init(slice_num, &config, false);
}

void turn_on_motor(uint gpio){
    slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_enabled(slice_num, true);
}

void turn_off_motor(uint gpio){
    slice_num = pwm_gpio_to_slice_num(gpio);
    pwm_set_enabled(slice_num, false);
}


void set_motor_angle(uint gpio, uint8_t angle){
    slice_num = pwm_gpio_to_slice_num(gpio);
    chan =  pwm_gpio_to_channel(gpio);
    pwm_set_chan_level(slice_num, chan, (0xC35/2)+(0xC35*2*angle/(0xff)));
}
