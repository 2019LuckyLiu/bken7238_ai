/**
 ******************************************************************************
 * @file    BkDriverPwm.h
 * @brief   This file provides all the headers of PWM operation functions.
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2017 BEKEN Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 */
#include "include.h"
#include "rtos_pub.h"
#include "BkDriverRng.h"
#include "drv_model_pub.h"
#include "error.h"
#include "uart_pub.h"
#if !(SOC_BK7252N == CFG_SOC_NAME)
#include "irda_pub.h"
#else
#include "irda_pub_bk7252n.h"
#endif
#include <stdlib.h>
#include "mem_pub.h"

#if (SOC_BK7231 == CFG_SOC_NAME)
int bk_rand(void)
{    
	int i = (int)prandom_get();
	return (i & RAND_MAX);
}

#else
int bk_rand(void)
{    
	int i = 0;
	
	BkRandomNumberRead(&i, sizeof(i));
	return (i & RAND_MAX);
}

OSStatus BkRandomNumberRead( void *inBuffer, int inByteCount )
{
	uint32_t i;
	uint32_t param = 0;


	ASSERT(inBuffer);
	for (i = 0; i < inByteCount; i += sizeof(param)) {
		sddev_control(IRDA_DEV_NAME, TRNG_CMD_GET, &param);
		os_memcpy((uint8_t *)inBuffer+i, (uint8_t *)&param, sizeof(param) > (inByteCount - i) ? (inByteCount - i) : sizeof(param));
	}

	return 0;
}
#endif
// eof

