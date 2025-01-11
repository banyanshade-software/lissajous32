/*
 * pwmdma.c
 *
 *  Created on: Jan 9, 2025
 *      Author: danielbraun
 */



#include "cmsis_os.h"
#include "main.h"
#include "pwmdma.h"
#include "misc.h"
#include "itm_debug.h"

#include <math.h>

#define USE_DMA 0
#define MAXBUF 1024
#define BASEFREQ  50000
#define BASEFREQF (1.0*BASEFREQ)
#define VALUEONE 631

// full buffer (1024 samples) at 50kHZ -> 20.48ms -> 48Hz

typedef struct sigbuf {
	uint16_t buf[MAXBUF];
	uint16_t buflen;
	volatile uint16_t ridx;
} sigbuf_t;

#define NCHAN 2
static sigbuf_t gen[NCHAN];



///
///
_UNUSED_ static void  fillramp(sigbuf_t *buf)
{
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

// each sample is 1/50000 s (20us)
// f=1 2*PI -> 50000
// f=10 2*PI -> 5000 sample  sin(2*PI*10*n/50000)
#define EPSILON 1E-5
static int adjust_freq(float fs, float f, float *fa)
{
	*fa = f;
    int P;
    float minnpf = 1.0;
    int minisup = 0;
    int bestp = 0;
    float N = fs/f;
    for (P=1; P*N<MAXBUF; P++) {
    	float np = N*P;
    	float npf = np - floor(np);
    	//printf("test %d -> %f npf=%f %f\n", P, np, npf, 1.0-npf);
    	if (npf < EPSILON) {
    		//printf("P=%d N=%f N*P=%f\n", P, N, N*P);
    		return np;
    	}
    	if (npf <= minnpf) {
    		minnpf = npf;
    		bestp = P;
    		minisup = 0;
    	}
    	if (1.0-npf <= minnpf) {
    		minnpf = 1.0-npf;
    		bestp = P;
    		minisup = 1;
    	}
    }
    //printf("bestp %d %f %s\n", bestp, minnpf, minisup ? "up" : "dn");
    // N = fs/f   f = fs/N
    int npa = floor(N*bestp);
    if (minisup) npa++;
    float fadj = bestp*fs/npa;
    //printf("adj %f\n", fadj);
    *fa = fadj;
    return npa;
}


static void fillsine(sigbuf_t *buf, float freq)
{
	float freqa;
	int n = adjust_freq(BASEFREQF, freq, &freqa);
	if (n>=MAXBUF) {
		Error_Handler();
	}
	float w = (2.*M_PI*freqa)/BASEFREQF;
	for (int ns=0; ns<n; ns++) {
		float v = sin(w*ns);
		float v1 = (v+1.0)/2.0;
		uint16_t k = (uint16_t) (v1*(VALUEONE));
		buf->buf[ns] = k;
	}
	buf->buflen = n;
}


static void fillbuf(void)
{
	fillsine(&gen[0], 3000.);
	fillsine(&gen[1], 2500.);
	//fillramp(&gen[1]);
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
		if (gen[i].ridx >= gen[i].buflen) {
			gen[i].ridx = 0;
		}
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
