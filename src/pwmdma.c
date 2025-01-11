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

static uint16_t buf1[MAXBUF];
static uint16_t buf2[MAXBUF];
static uint16_t buflen1;
static uint16_t buflen2;


static uint16_t fillsine(uint16_t *buf, uint16_t freq)
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
		buf1[i]=k;
		if (k==VALUEONE) li = i;
	}
	if (!li) return buflen;
	return li;
}

static void fillbuf(void)
{
	buflen1 = fillsine(buf1, 5*100);
	buflen2 = fillsine(buf2, 7*100);
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
}

void sigGenTask(void const * argument)
{
	itm_debug1(DBG_ERR, "gogogogogogogogogo", 0);
	itm_debug1(DBG_ERR, "1234567890", 0);
	itm_debug1(DBG_ERR, "go", 0);
	itm_debug1(DBG_ERR, "go", 0);
	itm_debug1(DBG_ERR, "go", 0);
	itm_debug1(DBG_ERR, "go", 0);
	itm_debug1(DBG_ERR, "go", 0);
	itm_debug1(DBG_ERR, "go", 0);
	itm_debug1(DBG_ERR, "go", 0);
	fillbuf();
	pwmdma_start();
	itm_debug1(DBG_ERR, "go1", 0);
	for (;;) {
		osDelay(3000);
	}
}
