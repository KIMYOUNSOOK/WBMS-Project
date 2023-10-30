/**
 * @brief   Custom functions for debugging targeted for IFX AURIX TC387
 * @author  ADK SW team
 * @date    2022-06-17
 */

#include "adi_wil_example_debug_functions.h"

void DBG_GPIO_INIT(void){
    /**
     * @remark : GPIO int and test
     */
    // IfxPort_setPinMode(DBG_GPIO_0, IfxPort_Mode_outputPushPullGeneral);    /* @remark  : Initialize GPIO port pin */
    // IfxPort_setPinMode(DBG_GPIO_1, IfxPort_Mode_outputPushPullGeneral);    /* @remark  : Initialize GPIO port pin */
    // IfxPort_setPinMode(DBG_GPIO_2, IfxPort_Mode_outputPushPullGeneral);    /* @remark  : Initialize GPIO port pin */
    // IfxPort_setPinMode(DBG_GPIO_3, IfxPort_Mode_outputPushPullGeneral);    /* @remark  : Initialize GPIO port pin */
    // IfxPort_setPinMode(DBG_GPIO_4, IfxPort_Mode_outputPushPullGeneral);    /* @remark  : Initialize GPIO port pin */
    // IfxPort_setPinMode(DBG_GPIO_5, IfxPort_Mode_outputPushPullGeneral);    /* @remark  : Initialize GPIO port pin */
}

void DBG_GPIO_HIGH(uint8_t ch){

    switch (ch)
    {
        case 0:
            IfxPort_setPinState(DBG_GPIO_0, IfxPort_State_high);     /* @remark  : DBG_GPIO_0 high */
            break;
        case 1:
            IfxPort_setPinState(DBG_GPIO_1, IfxPort_State_high);     /* @remark  : DBG_GPIO_1 high */
            break;
        case 2:
            IfxPort_setPinState(DBG_GPIO_2, IfxPort_State_high);     /* @remark  : DBG_GPIO_2 high */
            break;
        case 3:
            IfxPort_setPinState(DBG_GPIO_3, IfxPort_State_high);     /* @remark  : DBG_GPIO_3 high */
            break;
        case 4:
            IfxPort_setPinState(DBG_GPIO_4, IfxPort_State_high);     /* @remark  : DBG_GPIO_4 high */
            break;
        case 5:
            IfxPort_setPinState(DBG_GPIO_5, IfxPort_State_high);     /* @remark  : DBG_GPIO_5 high */
            break;
        default:
            break;
    }
    
}

void DBG_GPIO_LOW(uint8_t ch){

    switch (ch)
    {
        case 0:
            IfxPort_setPinState(DBG_GPIO_0, IfxPort_State_low);     /* @remark  : DBG_GPIO_0 low */
            break;
        case 1:
            IfxPort_setPinState(DBG_GPIO_1, IfxPort_State_low);     /* @remark  : DBG_GPIO_1 low */
            break;
        case 2:
            IfxPort_setPinState(DBG_GPIO_2, IfxPort_State_low);     /* @remark  : DBG_GPIO_2 low */
            break;
        case 3:
            IfxPort_setPinState(DBG_GPIO_3, IfxPort_State_low);     /* @remark  : DBG_GPIO_3 low */
            break;
        case 4:
            IfxPort_setPinState(DBG_GPIO_4, IfxPort_State_low);     /* @remark  : DBG_GPIO_4 low */
            break;
        case 5:
            IfxPort_setPinState(DBG_GPIO_5, IfxPort_State_low);     /* @remark  : DBG_GPIO_5 low */
            break;
        default:
            break;
    }
    
}


void DBG_GPIO_SPIKE_UP(uint8_t ch){
    DBG_GPIO_HIGH(ch);
    DBG_GPIO_LOW(ch);
}

void DBG_GPIO_SPIKE_DOWN(uint8_t ch){
    DBG_GPIO_LOW(ch);
    DBG_GPIO_HIGH(ch);
}


