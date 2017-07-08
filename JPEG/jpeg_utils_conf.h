/*
 * jpeg_utils_conf.h
 *
 * Copyright (C) 1991-1997, Thomas G. Lane.
 * Modified 1997-2011 by Guido Vollbeding.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains additional configuration options that customize the
 * JPEG HW configuration.  Most users will not need to touch this file.
 */

 /* Define to prevent recursive inclusion -------------------------------------*/

#ifndef  __JPEG_UTILS_CONF_H__
#define  __JPEG_UTILS_CONF_H__

/* Includes ------------------------------------------------------------------*/
//#if defined (STM32F767xx) || (STM32F769xx) || (STM32F777xx) || (STM32F779xx) 
#if 1

#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_jpeg.h"


/*
#if defined (STM32H7xxxx)
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_jpeg.h"
*/

#endif

/* Private define ------------------------------------------------------------*/
/** @addtogroup JPEG_Private_Defines
  * @{
  */
/* RGB Color format definition for JPEG encoding/Decoding : Should not be modified*/
#define JPEG_ARGB8888            0  /* ARGB8888 Color Format */
#define JPEG_RGB888              1  /* RGB888 Color Format   */
#define JPEG_RGB565              2  /* RGB565 Color Format   */

/*
 * Define USE_JPEG_DECODER 
 */

#define USE_JPEG_DECODER 0 /* 1 or 0 ********* Value different from default value : 1 ********** */
/*
 * Define USE_JPEG_ENCODER 
 */

#define USE_JPEG_ENCODER 1 /* 1 or 0 ********* Value different from default value : 1 ********** */			

/*
 * Define JPEG_RGB_FORMAT 
 */
#define JPEG_RGB_FORMAT JPEG_RGB888 /* JPEG_ARGB8888, JPEG_RGB888, JPEG_RGB565 ********* Value different from default value : 0 ********** */
            	
/*
 * Define JPEG_SWAP_RG 
 */
#define JPEG_SWAP_RG 0 /* 0 or 1 ********* Value different from default value : 0 ********** */
		
		
		
#define JPEG_CHROMA_SAMPLING     JPEG_420_SUBSAMPLING   /* Select Chroma Sampling: JPEG_420_SUBSAMPLING, JPEG_422_SUBSAMPLING, JPEG_444_SUBSAMPLING   */
#define JPEG_COLOR_SPACE         JPEG_YCBCR_COLORSPACE  /* Select Color Space: JPEG_YCBCR_COLORSPACE, JPEG_GRAYSCALE_COLORSPACE, JPEG_CMYK_COLORSPACE */
#define JPEG_IMAGE_QUALITY       75                     /* Set Image Quality for Jpeg Encoding */
#define MAX_INPUT_WIDTH          800                    /* Set the Maximum of RGB input images Width to be encoded */
#define MAX_INPUT_LINES          16                     /* Set Input buffer lines to 16 for YCbCr420, and 8 for YCbCr422 and YCbCr444 (to save RAM space) */

/* Exported macro ------------------------------------------------------------*/

/* Exported functions ------------------------------------------------------- */

#endif /* __JPEG_UTILS_CONF_H__ */
