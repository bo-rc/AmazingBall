/* Microchip Technology Inc. and its subsidiaries.  You may use this software
 * and any derivatives exclusively with Microchip products.
 *
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
 * TERMS.
 */

/*
 * File:   flextouch.h
 * Author: Bo Liu
 * Comments:
 * Revision history:
 */
#ifndef FLEXTOUCH_H
#define	FLEXTOUCH_H
#include <p33Fxxxx.h>
//do not change the order of the following 3 definitions
#define FCY 12800000UL
#include <stdio.h>
#include <libpic30.h>

#include <stdio.h>
#include <libpic30.h>
#include "types.h"

#define DIM_X 13
#define DIM_Y 23

#ifdef	__cplusplus
extern "C" {
#endif

extern void Delay_ms(uint16_t time_ms);

extern void init_adc1();

extern void touch_init();

extern void touch_select_dim(uint8_t dim);

extern uint16_t touch_adc();

extern uint16_t touch_read(uint8_t dim);

#ifdef	__cplusplus
}
#endif

#endif	/* FLEXTOUCH_H */
