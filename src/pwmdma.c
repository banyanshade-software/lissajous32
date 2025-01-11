/*
 * pwmdma.c
 *
 *  Created on: Jan 9, 2025
 *      Author: danielbraun
 */



#include "cmsis_os.h"
#include "main.h"
#include "pwmdma.h"
#include "itm_debug.h"



#define USE_DMA 0
#define MAXBUF 1024
#define BASEFREQ 50000
#define VALUEONE 631

typedef struct sigbuf {
	uint16_t buf[MAXBUF];
	uint16_t buflen;
	volatile uint16_t ridx;
} sigbuf_t;

#define NCHAN 2
sigbuf_t gen[NCHAN];




static void fillsine(sigbuf_t *buf, uint16_t freq)
{
	uint16_t buflen;

	// number of samples needed for a full cycle
	// f=25000 -> 2 samples at 50kHz sampling
	buflen = 50000/freq;
	if (buflen>=MAXBUF) buflen=MAXBUF;
	int li = 0;
	for (int i=0; i<MAXBUF; i++) {
		uint16_t k = i%(VALUEONE+1);
		//...
		buf->buf[i]=k;
		if (k==VALUEONE) li = i;
	}
	buf->buflen = li ? li : MAXBUF;
	buf->ridx = 0;

}

static void fillbuf(void)
{
	fillsine(&gen[0], 3000);
	fillsine(&gen[1], 4000);
}

static void pwmdma_start(void)
{
#if USE_DMA
	HAL_TIM_Base_Start(&htim1);
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, buf1, buflen1);
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_2, buf2, buflen2);
#else
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
#endif

}

void pwm_tim_it(void)
{
	static int cnt = 0;
	cnt++;
	if (0==(cnt%50000)) {
		itm_debug1(DBG_TIM, "hop", cnt);
	}
	for (int i=0; i<NCHAN; i++) {
		uint16_t v = gen[i].buf[gen[i].ridx];
		gen[i].ridx++;
		//pwm_timer->Instance->CCR4 = CNT_ON(t);
		switch (i) {
		case 0:
			htim1.Instance->CCR1 = v;
			break;
		case 1:
			htim1.Instance->CCR2 = v;
			break;
		case 2:
			htim1.Instance->CCR3 = v;
			break;
		case 3:
			htim1.Instance->CCR4 = v;
			break;
		}
	}
}

void sigGenTask(void const * argument)
{
	itm_debug1(DBG_ERR, "go", 0);
	fillbuf();
	pwmdma_start();
	itm_debug1(DBG_ERR, "go1", 0);
	for (;;) {
		osDelay(3000);
	}
}
