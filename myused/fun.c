#include "fun.h"
uint8_t run_flag = 0;
uint8_t circle_mode = 1;

void key_fun(void) {
    static uint8_t k1_last=1, k3_last=1;
    uint8_t k1=DL_GPIO_readPins(GPIO_KEY_PORT,GPIO_KEY_KEY1_PIN)?1:0;
    uint8_t k3=DL_GPIO_readPins(GPIO_KEY_PORT,GPIO_KEY_KEY3_PIN)?1:0;
    if(k1==0&&k1_last==1){circle_mode++;if(circle_mode>5)circle_mode=1;}
    if(k3==0&&k3_last==1)run_flag=!run_flag;
    k1_last=k1;k3_last=k3;
}
