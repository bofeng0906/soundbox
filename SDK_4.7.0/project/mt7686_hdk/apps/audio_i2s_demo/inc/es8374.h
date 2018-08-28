/*
 * Copyright 2005 Openedhand Ltd.
 *
 * Author: Richard Purdie <richard@openedhand.com>
 *
 * Based on ES8374.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _ES8374_H
#define _ES8374_H

#define ES8374_I2C_ADDR                               0x11


/**  This enum define the HAL interface return value. */
typedef enum {
    AUCODEC_STATUS_ERROR = -1,   /**<  An error occurred and the transaction failed. */
    AUCODEC_STATUS_OK = 0        /**<  No error occurred during the function call. */
} AUCODEC_STATUS_e;

/**  This enum defines the nau8810 DAI mode. */
typedef enum  {
    eRightJustified = 0,  /**<  Right Justified. */
    eLeftJustified = 1,   /**<  Left Justified  . */
    eI2S = 2,             /**<  I2S . */
    ePCMA = 3,            /**< PCM mode A.*/
    ePCMB = 4,            /**< PCM mode B.*/
} AUCODEC_DAI_FORMAT_e;

/**  This enum defines the nau8810 DAI word length. */
typedef enum  {
    e16Bit = 0,          /**<  16-bit . */
    e20Bit = 1,          /**<  20-bit . */
    e24Bit = 2,          /**<  24-bit . */
    e32Bit = 3,          /**<  32-bit . */
} AUCODEC_DAI_WLEN_e;

/**  This enum defines the nau8810 output path for DAI input. */
typedef enum  {
    eLineOut = 0,       /**<  Line output . */
    eSpkOut = 1,        /**<  Speaker output . */
    eNoneOut = 2,       /**<  No output . */
} AUCODEC_OUTPUT_SEL_e;


/**  This enum defines the nau8810 input path for DAI output. */
typedef enum  {
    eMicIn = 0,         /**<  Microphone input . */
    eNoneIn = 1,        /**<  No input . */
} AUCODEC_INPUT_SEL_e;

/**  This enum defines the nau8810 soft mute. */
typedef enum  {
    eUnmute = 0,  // Soft unmute DAC
    eMute = 1,  // Soft mute DAC
} AUCODEC_MUTE_e;

/**  This enum defines the nau8810 BCLK Polarity. */
typedef enum  {
    eBCLK_NO_INV = 0,   /**<  BCLK Polarity Normal . */
    eBCLK_INV = 1,      /**<  BCLK Polarity Inverted . */
} AUCODEC_BCLK_INV_e;

/**  This enum defines the nau8810 sample rate. */
typedef enum  {
    eSR48KHz = 0,  // Sampling frequency 48KHz
    eSR44K1Hz = 1,  // Sampling frequency 44.1KHz
    eSR32KHz = 2,  // Sampling frequency 32KHz
    eSR16KHz = 3,  // Sampling frequency 16KHz
    eSR8KHz = 4,  // Sampling frequency 8KHz
    eSR22_05KHz = 5, // Sampling frequency 22.05KHz mp3
    eSR11_025KHz = 6, // Sampling frequency 11.025KHz mp3
    eSR24KHz = 7, // Sampling frequency 24KHz mp3
    eSR12KHz = 8, // Sampling frequency 8KHz mp3
} AUCODEC_SAMPLERATE_SEL_e;

/**  This enum defines the nau8810 sample rate. */
typedef enum  {
    eMCLK8KBASE = 24576000,
    eMCLK11_025KBASE = 22579200,
} AUCODEC_MCLKRATE_e;


/**  This enum defines the nau8810 clock mode. */
typedef enum  {
    eSLAVE = 0,   /**<  Digital audio interface slave mode . */
    eMASTER = 1,  /**<  Digital audio interface master mode . */
} AUCODEC_DAI_ROLE_e;

/**  This enum defines the nau8810 BCLK rate. */
typedef enum  {
    e256xFS = 0,  /**<  the frequency of BCLK = 256xFS  . */
    e128xFS = 1,  /**<  the frequency of BCLK = 128xFS  . */
    e64xFS = 2,   /**<  the frequency of BCLK = 64xFS  . */
    e32xFS = 3,   /**<  the frequency of BCLK = 32xFS  . */
    e16xFS = 4,   /**<  the frequency of BCLK = 16xFS  . */
    e8xFS = 5,    /**<  the frequency of BCLK = 8xFS  . */
} AUCODEC_BCLK_RATE_e;
/**  This enum defines the nau8810 PLL mode. */
typedef enum  {
    ePLLDisable = 0,  /**<   PLL enable . */
    ePLLEnable = 1,   /**<   PLL disable . */
} AUCODEC_PLL_e;
struct pll_ {
    uint32_t pre_div : 1;
    uint32_t n : 4;
    uint32_t k;
};

#endif
