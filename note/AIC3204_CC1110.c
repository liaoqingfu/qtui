/* 
******************************************************************************************************** 
* Copyright (C) 2015, Changsha Spon Electronic Co., Ltd 
******************************************************************************************************** 
* Filename     : AIC3204_CC1110.c
* Author       : PanLinFeng <954048047@qq.com>
* Created      : 2015/11/15
* Description  : CC1110 AIC3204驱动文件
******************************************************************************************************** 
* Modify       : 
******************************************************************************************************** 
*/
#include "Common.h"
#include "AIC3204_CC1110.h"
#include "I2C_CC1110.h"
#include "I2S_CC1110.h"
#include "Timer.h"
#include "RF_CC1110.h"
#include "DMA_CC1110.h"
#include <iocc1110.h>

//----------------- 1. 宏及结构本声明 -----------------
#define AIC3204_ADDR            0x30
/*
* Bias levels
*
* @ON:      Bias is fully on for audio playback and capture operations.
* @PREPARE: Prepare for audio operations. Called before DAPM switching for
*           stream start and stop operations.
* @STANDBY: Low power standby state when no playback/capture operations are
*           in progress. NOTE: The transition time between STANDBY and ON
*           should be as fast as possible and no longer than 10ms.
* @OFF:     Power Off. No restrictions on transition times.
*/
typedef enum {
    SND_SOC_BIAS_OFF = 0,
    SND_SOC_BIAS_STANDBY = 1,
    SND_SOC_BIAS_PREPARE = 2,
    SND_SOC_BIAS_ON = 3,
} SND_SOC_BIAS_LEVEL;

/*
* DAI hardware clock masters.
*
* This is wrt the codec, the inverse is true for the interface
* i.e. if the codec is clk and FRM master then the interface is
* clk and frame slave.
*/
 #define SND_SOC_DAIFMT_CBM_CFM          (1) /* codec clk & FRM master */
 #define SND_SOC_DAIFMT_CBS_CFM          (2) /* codec clk slave & FRM master */
 #define SND_SOC_DAIFMT_CBM_CFS          (3) /* codec clk master & frame slave */
 #define SND_SOC_DAIFMT_CBS_CFS          (4) /* codec clk & FRM slave */
 
/*
 * DAI hardware audio formats.
 *
 * Describes the physical PCM data formating and clocking. Add new formats
 * to the end.
 */
#define SND_SOC_DAIFMT_I2S              (1) /* I2S mode */
#define SND_SOC_DAIFMT_RIGHT_J          (2) /* Right Justified mode */
#define SND_SOC_DAIFMT_LEFT_J           (3) /* Left Justified mode */
#define SND_SOC_DAIFMT_DSP_A            (4) /* L data MSB after FRM LRC */
#define SND_SOC_DAIFMT_DSP_B            (5) /* L data MSB during FRM LRC */
#define SND_SOC_DAIFMT_AC97             (6) /* AC97 */
#define SND_SOC_DAIFMT_PDM              (7) /* Pulse density modulation */


/* tlv320aic32x4 register space (in decimal to match datasheet) */

#define AIC32X4_PAGE1		(1 << 7)
#define AIC32X4_PAGE44		(44 << 7)
#define AIC32X4_PAGE45		(45 << 7)
#define AIC32X4_PAGE46		(46 << 7)

#define	AIC32X4_PSEL		(0)
#define	AIC32X4_RESET		(1)
#define	AIC32X4_CLKMUX		(4)
	#define HIGH_PLL_CLOCK_RANGE	(0x01 << 6)
	#define PLL_2_CLKIN		(0x03 << 0)
#define	AIC32X4_PLLPR		(5)
	#define AIC32X4_PLLEN			(0x01 << 7)
#define	AIC32X4_PLLJ		(6)
#define	AIC32X4_PLLDMSB		(7)
#define	AIC32X4_PLLDLSB		(8)
#define	AIC32X4_NDAC		(11)
	#define AIC32X4_NDACEN			(0x01 << 7)
#define	AIC32X4_MDAC		(12)
	#define AIC32X4_MDACEN			(0x01 << 7)
#define AIC32X4_DOSRMSB		(13)
#define AIC32X4_DOSRLSB		(14)
#define	AIC32X4_NADC		(18)
	#define AIC32X4_NADCEN			(0x01 << 7)
#define	AIC32X4_MADC		(19)
	#define AIC32X4_MADCEN			(0x01 << 7)
#define AIC32X4_AOSR		(20)
#define AIC32X4_CLKMUX2		(25)
	#define PLL_2_CDIV_CLKIN	(0x03)
#define AIC32X4_CLKOUTM		(26)
	#define CLKOUTM_EN_M_EQU_1		((0x01 << 7) | (0x01 << 0)
#define AIC32X4_IFACE1		(27)
	#define AIC32X4_I2S_MODE				(0x00 << 0)
	#define AIC32X4_DSP_MODE				(0x01 << 0)
	#define AIC32X4_RIGHT_JUSTIFIED_MODE	(0x02 << 0)
	#define AIC32X4_LEFT_JUSTIFIED_MODE		(0x03 << 0)
	#define AIC32X4_WORD_LEN_16BITS		(0x00 << 4)
	#define AIC32X4_WORD_LEN_20BITS		(0x01 << 4)
	#define AIC32X4_WORD_LEN_24BITS		(0x02 << 4)
	#define AIC32X4_WORD_LEN_32BITS		(0x03 << 4)
	#define AIC32X4_BCLKMASTER		(0x01 << 3)
	#define AIC32X4_WCLKMASTER		(0x01 << 2)
	#define DOUT_HIGH_IMPENDANCE_OUTPUT (0x01 << 1)
#define AIC32X4_IFACE2		(28)
#define AIC32X4_IFACE3		(29)
	#define LOOPBACK	((0X01 << 5))
	#define	DACMOD_2_BCLK		(0x01)
#define AIC32X4_BCLKN		(30)
	#define AIC32X4_BCLKEN			(0x01 << 7)
#define AIC32X4_IFACE4		(31)
#define AIC32X4_IFACE5		(32)
#define AIC32X4_IFACE6		(33)
#define AIC32X4_DOUTCTL		(53)
#define AIC32X4_DINCTL		(54)
#define AIC32X4_DACSPB		(60)
	#define PRB_P1   (1)
	#define PRB_P2   (2)
	#define PRB_P8	 (8)
	#define PRB_P25	 (0x19)
#define AIC32X4_ADCSPB		(61)
	#define PRB_R1	 (1)	
#define AIC32X4_DACSETUP	(63)
	#define LDAC_POWERUP	(0X01 << 7)
	#define RDAC_POWERUP	(0X01 << 6)
	#define AIC32X4_RDAC2LCHN		(0x02 << 2)
	#define AIC32X4_LDAC2RCHN		(0x02 << 4)
	#define AIC32X4_LDAC2LCHN		(0x01 << 4)
	#define AIC32X4_RDAC2RCHN		(0x01 << 2)	
#define AIC32X4_DACMUTE		(64)
	#define AIC32X4_MUTE			(0x03 << 2)
	#define AIC32X4_UNMUTE			(0)
	#define AIC32X4_VOL_L_CONTOL_R		(0X01 << 0)
#define AIC32X4_LDACVOL		(65)
#define AIC32X4_RDACVOL		(66)
#define AIC32X4_DRCCTRL1    (68)
    #define AIC32X4_LDRCEN          (0x01 << 6)
    #define AIC32X4_RDRCEN          (0x01 << 5)
    #define AIC32X4_DRCTHR3DB       (0x00 << 2) // Threshold
    #define AIC32X4_DRCTHR6DB       (0x01 << 2) // Threshold
    #define AIC32X4_DRCTHR9DB       (0x02 << 2) // Threshold
    #define AIC32X4_DRCTHR12DB      (0x03 << 2) // Threshold
    #define AIC32X4_DRCTHR24DB      (0x07 << 2) // Threshold
    #define AIC32X4_DRCHYS3DB       (0x03 << 2) // Hysteresis
#define AIC32X4_DRCCTRL2    (69)
    
#define AIC32X4_ADCSETUP	(81)
	#define LADC_POWERUP	(0X01 << 7)
	#define RADC_POWERUP	(0X01 << 6)
#define	AIC32X4_ADCFGA		(82)
	#define ADC_UN_MUTED	(0)
	#define ADC_MUTED		((0X01 << 7) | (0X01 << 3))
#define AIC32X4_LADCVOL		(83)
#define AIC32X4_RADCVOL		(84)
#define AIC32X4_LAGC1		(86)
#define AIC32X4_LAGC2		(87)
#define AIC32X4_LAGC3		(88)
#define AIC32X4_LAGC4		(89)
#define AIC32X4_LAGC5		(90)
#define AIC32X4_LAGC6		(91)
#define AIC32X4_LAGC7		(92)
#define AIC32X4_RAGC1		(94)
#define AIC32X4_RAGC2		(95)
#define AIC32X4_RAGC3		(96)
#define AIC32X4_RAGC4		(97)
#define AIC32X4_RAGC5		(98)
#define AIC32X4_RAGC6		(99)
#define AIC32X4_RAGC7		(100)


#define AIC32X4_PWRCFG		(AIC32X4_PAGE1 + 1)
	#define AIC32X4_AVDDWEAKDISABLE		0x08
#define AIC32X4_LDOCTL		(AIC32X4_PAGE1 + 2)
	#define ANALOG_ENABLE		(0x01 << 3)
	#define AIC32X4_LDOCTLEN	(0x01 << 0)
#define AIC32X4_OUTPWRCTL	(AIC32X4_PAGE1 + 9)
	#define HPL_POWERUP 	(0x01 << 5)
	#define HPR_POWERUP		(0x01 << 4)
	#define LOL_POWERUP		(0X01 << 3)
	#define LOR_POWERUP		(0x01 << 2)
	#define MAL_POWERUP		(0X01 << 1)
	#define MAR_POWERUP		(0x01 << 0)
#define AIC32X4_CMMODE		(AIC32X4_PAGE1 + 10)	
	#define AIC32X4_LDOIN_18_36		(0x01 << 0)
	#define AIC32X4_LDOIN2HP		(0x01 << 1)
	#define CM_LOUT_1_65_LDOIN (0X01 << 3)
	#define CM_HP_1_65			(0X03 << 4)
#define AIC32X4_HPLROUTE	(AIC32X4_PAGE1 + 12)
	#define LDAC_2_HPL	(0x01 << 3)
	#define IN1L_2_HPL	(0x01 << 2)
	#define MAL_2_HPL	(0x01 << 1)
	#define MAR_2_HPL	(0x01 << 0)
#define AIC32X4_HPRROUTE	(AIC32X4_PAGE1 + 13)
	#define LDAC_2_HPR	(0x01 << 4)
	#define RDAC_2_HPR	(0x01 << 3)
	#define IN1R_2_HPR	(0x01 << 2)
	#define MAR_2_HPR	(0x01 << 1)
	#define HPL_2_HPR	(0x01 << 0)
#define AIC32X4_LOLROUTE	(AIC32X4_PAGE1 + 14)	
	#define RDAC_2_LOL	(0x01 << 4)
	#define LDAC_2_LOL	(0X01 << 3)
	#define MAL_2_LOL	(0X01 << 1)
	#define LOR_2_LOL	(0x01 << 0)
#define AIC32X4_LORROUTE	(AIC32X4_PAGE1 + 15)
	#define RDAC_2_LOR	(0X01 << 3)
	#define MAR_2_LOR	(0X01 << 1)
#define	AIC32X4_HPLGAIN		(AIC32X4_PAGE1 + 16)
#define	AIC32X4_HPRGAIN		(AIC32X4_PAGE1 + 17)
#define	AIC32X4_LOLGAIN		(AIC32X4_PAGE1 + 18)
#define	AIC32X4_LORGAIN		(AIC32X4_PAGE1 + 19)
#define AIC32X4_HEADSTART	(AIC32X4_PAGE1 + 20)
#define AIC32X4_MICBIAS		(AIC32X4_PAGE1 + 51)
	#define AIC32X4_MICBIAS_AVDD		(0x00 << 3)
	#define AIC32X4_MICBIAS_LDOIN		(0x01 << 3)
	#define AIC32X4_MICBIAS_1025V		(0x01 << 4)
	#define AIC32X4_MICBIAS_205V		(0x02 << 4)
	#define AIC32X4_MICBIAS_2075V		(0x03 << 4)
	#define MICBIAS_POWERUP				(0x01 << 6)
#define AIC32X4_LMICPGAPIN	(AIC32X4_PAGE1 + 52)
	#define AIC32X4_LMICPGAPIN_IN1L_10K	(0x01 << 6)
	#define AIC32X4_LMICPGAPIN_IN2L_10K	(0x01 << 4)
	#define AIC32X4_LMICPGAPIN_IN3L_10K	(0x01 << 2)
	#define AIC32X4_LMICPGAPIN_IN1R_10K	(0x01 << 0)
#define AIC32X4_LMICPGANIN	(AIC32X4_PAGE1 + 54)
	#define AIC32X4_LMICPGANIN_CM1L_10K	(0x01 << 6)
	#define AIC32X4_LMICPGANIN_IN2R_10K	(0x01 << 4)
	#define AIC32X4_LMICPGANIN_IN3R_10K	(0x01 << 2)
	#define AIC32X4_LMICPGANIN_CM2L_10K	(0x01 << 0)
#define AIC32X4_RMICPGAPIN	(AIC32X4_PAGE1 + 55)
	#define AIC32X4_RMICPGAPIN_IN1R_10K	(0x01 << 6)
	#define AIC32X4_RMICPGAPIN_IN2R_10K	(0x01 << 4)
	#define AIC32X4_RMICPGAPIN_IN3R_10K	(0x01 << 2)
	#define AIC32X4_RMICPGAPIN_IN2L_10K	(0x01 << 0)
#define AIC32X4_RMICPGANIN	(AIC32X4_PAGE1 + 57)
	#define AIC32X4_RMICPGANIN_CM1R_10K	(0x01 << 6)
	#define AIC32X4_RMICPGANIN_IN1L_10K	(0x01 << 4)
	#define AIC32X4_RMICPGANIN_IN3L_10K	(0x01 << 2)
	#define AIC32X4_RMICPGANIN_CM2R_10K	(0x01 << 0)
#define AIC32X4_FLOATINGINPUT	(AIC32X4_PAGE1 + 58)
#define AIC32X4_LMICPGAVOL	(AIC32X4_PAGE1 + 59)
	#define AIC32X4_LMICPGAVOL_NOGAIN	(0x01 << 7)
#define AIC32X4_RMICPGAVOL	(AIC32X4_PAGE1 + 60)
	#define AIC32X4_RMICPGAVOL_NOGAIN	(0x01 << 7)
#define AIC32X4_INPUTCHARGING	(AIC32X4_PAGE1 + 71)
	#define AIC32X4_ANALOG_INPUT_CHARGING_1_6MS	0X33
#define AIC32X4_REFCHARGING	(AIC32X4_PAGE1 + 123)
	#define AIC32X4_REF_CHARGING_40MS	(0X01 << 0)
	#define AIC32X4_REF_FORCE	(0X01 << 2)

#define AIC32X4_FREQ_12000000 12
#define AIC32X4_FREQ_13000000 13
#define AIC32X4_FREQ_24000000 24
#define AIC32X4_FREQ_25000000 25

#define AIC32X4_DACSPBLOCK_MASK		0x1f
#define AIC32X4_ADCSPBLOCK_MASK		0x1f

#define AIC32X4_PLLJ_SHIFT			6
#define AIC32X4_DOSRMSB_SHIFT		4

#define AIC32X4_SSTEP2WCLK		0x01

typedef struct tagAIC32X4_RATE_DIVS {
	int         nMclk;
	UINT16      nRate;
	BYTE        ucP_val;
	BYTE        ucR_val;
	BYTE        ucPLL_j;
	WORD        wPLL_d;
	WORD        wDosr;
	BYTE        ucNdac;
	BYTE        ucMdac;
	BYTE        ucAosr;
	BYTE        ucNadc;
	BYTE        ucMadc;
	BYTE        ucBlck_N;
} AIC32X4_RATE_DIVS;

//----------------- 2. 变量定义 -----------------------
__code static const AIC32X4_RATE_DIVS  g_sAIC32X4_DIVS[] = {
    // 2.8MHz < DOSR * DAC_FS < 6.2MHz
    // AOSR: 128 or 64 or 32
	// 8k rate
	{AIC32X4_FREQ_12000000, 8000, 1, 1, 7, 6800, 768, 5, 3, 128, 5, 18, 24},
	{AIC32X4_FREQ_13000000, 8000, 1, 1, 6, 1440, 624, 4, 4, 128, 6, 13, 24},
	{AIC32X4_FREQ_24000000, 8000, 2, 1, 7, 6800, 768, 15, 1, 64, 45, 4, 24},
	{AIC32X4_FREQ_25000000, 8000, 2, 1, 7, 3728, 768, 15, 1, 64, 45, 4, 24},
	// 11.025k rate
	{AIC32X4_FREQ_12000000, 11025, 1, 1, 7, 5264, 512, 8, 2, 128, 8, 8, 16},
	{AIC32X4_FREQ_13000000, 11025, 1, 2, 4, 2336, 416, 6, 4, 128, 6, 13, 16},
	{AIC32X4_FREQ_24000000, 11025, 2, 1, 7, 5264, 512, 16, 1, 64, 32, 4, 16},
	// 16k rate
	{AIC32X4_FREQ_12000000, 16000, 1, 1, 7, 6800, 384, 5, 3, 128, 5, 9, 12},
	{AIC32X4_FREQ_13000000, 16000, 1, 1, 6, 1440, 312, 4, 4, 128, 3, 13, 12},
	{AIC32X4_FREQ_24000000, 16000, 2, 1, 7, 6800, 384, 15, 1, 64, 18, 5, 12},
	{AIC32X4_FREQ_25000000, 16000, 2, 1, 7, 3728, 384, 15, 1, 64, 18, 5, 12},
	// 22.05k rate
	{AIC32X4_FREQ_12000000, 22050, 1, 1, 7, 5264, 256, 4, 4, 128, 4, 8, 8},
	{AIC32X4_FREQ_13000000, 22050, 1, 2, 4, 2336, 208, 6, 4, 128, 3, 13, 8},
	{AIC32X4_FREQ_24000000, 22050, 2, 1, 7, 5264, 256, 16, 1, 64, 16, 4, 8},
	{AIC32X4_FREQ_25000000, 22050, 2, 1, 7, 2253, 256, 16, 1, 64, 16, 4, 8},
	// 32k rate
	{AIC32X4_FREQ_12000000, 32000, 1, 1, 7, 1680, 192, 2, 7, 64, 2, 21, 6},
	{AIC32X4_FREQ_13000000, 32000, 1, 1, 6, 1440, 156, 4, 4, 64, 3, 13, 6},
	{AIC32X4_FREQ_24000000, 32000, 2, 1, 7, 1680, 192, 7, 2, 64, 7, 6, 6},
	// 44.1k rate
	{AIC32X4_FREQ_12000000, 44100, 1, 1, 7, 5264, 128, 2, 8, 128, 2, 8, 4},
	{AIC32X4_FREQ_13000000, 44100, 1, 2, 4, 2336, 104, 6, 4, 64, 3, 13, 4},
	{AIC32X4_FREQ_24000000, 44100, 2, 1, 7, 5264, 128, 8, 2, 64, 8, 4, 4},
	{AIC32X4_FREQ_25000000, 44100, 2, 1, 7, 2253, 128, 8, 2, 64, 8, 4, 4},
	// 48k rate
	{AIC32X4_FREQ_12000000, 48000, 1, 1, 8, 1920, 128, 2, 8, 128, 2, 8, 4},
	{AIC32X4_FREQ_13000000, 48000, 1, 2, 4, 6080, 104, 6, 4, 64, 3, 13, 4},
	{AIC32X4_FREQ_24000000, 48000, 2, 1, 8, 1920, 128, 8, 2, 64, 8, 4, 4},
	{AIC32X4_FREQ_25000000, 48000, 2, 1, 7, 8643, 128, 8, 2, 64, 8, 4, 4}
};

// 音量6为0DB
__code static const BYTE           g_pOutputVolume[AIC3204_VOLUME_MAX + 1] = { // -63.5 ~ +24dB
                       /*0x81,*/0x81, 0xB0, 0xC4, 0xD8, // -63.5dB, -40, -30, -20...
                                0xEC, 0xF8, 0x00, 0x04, // -10, -4, 0
                                0x08, 0x0C, 0x10, 0x14, 
                                0x18, 0x1C, 0x20, 0x24, // +18dB
                                };
__code static const BYTE           g_pPHLOVolume[AIC3204_VOLUME_MAX + 1] = { // -6 ~ +29dB
                                0x3A, 0x3B, 0x3C, 0x3D, // -6dB...
                                0x3E, 0x3F, 0x00, 0x02, 
                                0x04, 0x06, 0x08, 0x0A, 
                                0x0C, 0x0E, 0x10, 0x12, // +18dB
                                };
__code static const BYTE           g_pInputVolume[AIC3204_VOLUME_MAX + 1] = { // -12 ~ +20dB
                                0x68, 0x6C, 0x70, 0x74, // -12dB...
                                0x78, 0x7C, 0x00, 0x04, 
                                0x08, 0x0C, 0x10, 0x14, 
                                0x18, 0x1C, 0x20, 0x24, // +18dB
                                };
__code static const BYTE           g_pMICPGAVolume[AIC3204_VOLUME_MAX + 1] = { // 0 ~ +47.5dB
                                0x00, 0x04, 0x08, 0x0C, // 0dB...
                                0x10, 0x14, 0x18, 0x1C, 
                                0x20, 0x24, 0x28, 0x2C, 
                                0x30, 0x34, 0x38, 0x3C, // +30dB
                                };

__code static const AIC3204_VOLUME     g_sVolumeDefault = {
    FALSE,              // bMuteHPR
    FALSE,              // bMuteHPL
    FALSE,              // bMuteLOR
    FALSE,              // bMuteLOL
    FALSE,              // bMuteInput
    FALSE,              // bMuteOutput
    FALSE,              // bMuteLMICPGA
    TRUE,              // bMuteRMICPGA
    
    AIC3204_VOLUME_0DB, // ucVolumeHPR
    AIC3204_VOLUME_0DB, // ucVolumeHPL
    AIC3204_VOLUME_0DB, // ucVolumeLOR
    AIC3204_VOLUME_0DB, // ucVolumeLOL
    AIC3204_VOLUME_0DB, // ucVolumeInput
    AIC3204_VOLUME_0DB, // ucVolumeOutput
    12,                  // ucVolumeLMICPGA
    0,                  // ucVolumeRMICPGA
    
    TRUE,               // bEnableAGC
    
    FALSE,              // bByPass
};

static BOOL					g_bDMARxCheck;
static BOOL					g_bAIC3204Enable;
static BYTE                 g_ucCurPage;

int                         g_nCurRate;
AIC3204_VOLUME              g_sVolumeCur;
int                         g_nLackFrame;

//----------------- 3. 函数声明 -----------------------
static BOOL     AIC3204_Write(DWORD dwAddr, BYTE ucData);                   // 写寄存器数据
static BYTE     AIC3204_Read(DWORD dwAddr);                                 // 读寄存器数据
static BOOL     AIC3204_ChangePage(BYTE ucPage);                            // 寄存器页面切换
static void     AIC3204_SoftwareReset(void);                                // 软复位
#define         AIC3204_I2SRxHandler RF_SetSendData                         // I2S DMA接收处理
static void     AIC3204_I2STxHandler(BYTE *pBuf, BYTE ucLen);               // I2S DMA发送处理

static void     AIC3204_SetAnalog(void);
static void     AIC3204_SetInputRouting(void);
static void     AIC3204_SetOutputRouting(void);
static void     AIC3204_SetBiasLevel(SND_SOC_BIAS_LEVEL eLevel);
static void     AIC3204_SetDaiFmt(BYTE ucAudioMode, BYTE ucMasterFace);
static int      AIC3204_GetDivs(int nMclk, int nRate);

//----------------- 4. 函数定义 -----------------------
// 初始化AIC3204
void AIC3204_Init(void)
{
    g_ucCurPage = 0;
    g_nCurRate = 0;
    g_sVolumeCur = g_sVolumeDefault;
    g_nLackFrame = 0;
    
    I2C_Init();
    I2S_Init();
    DMA_I2SInit(AIC3204_I2SRxHandler, AIC3204_I2STxHandler);
    
    AIC3204_SoftwareReset();
    AIC3204_SetRate(SAMPLES_RATE);
    AIC3204_Enable();
    AIC3204_SetByPass(TRUE);
    AIC3204_SetAGC(FALSE);
    I2S_Enable();
}

// 寄存器页面切换
static BOOL AIC3204_ChangePage(BYTE ucPage)
{
    BYTE    pBuf[3];
	pBuf[0] = AIC3204_ADDR;
	pBuf[1] = 0;
	pBuf[2] = ucPage;
    g_ucCurPage = ucPage;
	return (I2C_Send(pBuf, 3) == 0) ? FALSE :TRUE;
}

// 写寄存器数据
static BOOL AIC3204_Write(DWORD dwAddr, BYTE ucData)
{
    BYTE    pBuf[3];
    BYTE    ucPage = dwAddr >> 7;
    BYTE    ucAddr = dwAddr & 0x7F;
    if (ucPage != g_ucCurPage)
        AIC3204_ChangePage(ucPage);
	pBuf[0] = AIC3204_ADDR;
	pBuf[1] = ucAddr;
	pBuf[2] = ucData;
	return (I2C_Send(pBuf, 3) == 0) ? FALSE :TRUE;
}

// 读寄存器数据
static BYTE AIC3204_Read(DWORD dwAddr)
{
    BYTE    ucDevice, ucData;
    BYTE    ucPage = dwAddr >> 7;
    BYTE    ucAddr = dwAddr & 0x7F;
    if (ucPage != g_ucCurPage)
        AIC3204_ChangePage(ucPage);
    ucDevice = AIC3204_ADDR;
    I2C_Start();
    I2C_SendByte(ucDevice);
    I2C_SendByte(ucAddr);
    ucDevice = AIC3204_ADDR | 1;
    I2C_Start();
    I2C_SendByte(ucDevice);
    I2C_RecvByte(&ucData, FALSE);
    I2C_Stop();    
	return ucData;
}

// 软复位
static void AIC3204_SoftwareReset(void)
{
    AIC3204_Write(0, 0);            // 选择页面0
    AIC3204_Write(1, 1);            // 软复位
    DelayCyc(5000);                 // 硬复位后延时1ms
    g_ucCurPage = 0;
    AIC3204_Write(AIC32X4_RDACVOL, 0x81);
    AIC3204_SetAnalog();
    AIC3204_SetInputRouting();
    AIC3204_SetOutputRouting();
    AIC3204_SetBiasLevel(SND_SOC_BIAS_ON);
#ifdef MCU_I2S_MASTER
    AIC3204_SetDaiFmt(SND_SOC_DAIFMT_LEFT_J, SND_SOC_DAIFMT_CBS_CFS);
#else
    AIC3204_SetDaiFmt(SND_SOC_DAIFMT_I2S, SND_SOC_DAIFMT_CBM_CFM);
#endif
}

static void AIC3204_SetAnalog(void)
{
    // Power platform configuration
    AIC3204_Write(AIC32X4_PWRCFG, AIC32X4_AVDDWEAKDISABLE);
    AIC3204_Write(AIC32X4_LDOCTL, AIC32X4_LDOCTLEN);
    AIC3204_Write(AIC32X4_REFCHARGING, AIC32X4_REF_FORCE | AIC32X4_REF_CHARGING_40MS);
    AIC3204_Write(AIC32X4_INPUTCHARGING, AIC32X4_ANALOG_INPUT_CHARGING_1_6MS);

    AIC3204_Write(AIC32X4_MICBIAS, AIC32X4_MICBIAS_LDOIN | AIC32X4_MICBIAS_205V | MICBIAS_POWERUP);
    AIC3204_Write(AIC32X4_CMMODE, AIC32X4_LDOIN_18_36 | AIC32X4_LDOIN2HP | CM_LOUT_1_65_LDOIN | CM_HP_1_65);
}

static void AIC3204_SetInputRouting(void)
{
    // Mic PGA routing
    AIC3204_Write(AIC32X4_LMICPGAPIN, 0);
    AIC3204_Write(AIC32X4_LMICPGANIN, 0);
    AIC3204_Write(AIC32X4_RMICPGAPIN, 0);
    AIC3204_Write(AIC32X4_RMICPGANIN, 0);
    AIC3204_Write(AIC32X4_ADCSETUP, LADC_POWERUP | RADC_POWERUP);
}

static void AIC3204_SetOutputRouting(void)
{
	AIC3204_Write(AIC32X4_DACSETUP, AIC32X4_LDAC2LCHN | AIC32X4_RDAC2RCHN | RDAC_POWERUP | LDAC_POWERUP);// AIC32X4_RDAC2LCHN
	AIC3204_Write(AIC32X4_OUTPWRCTL, HPL_POWERUP | HPR_POWERUP | LOL_POWERUP | LOR_POWERUP | MAL_POWERUP | MAR_POWERUP);
	AIC3204_Write(AIC32X4_LOLROUTE, LDAC_2_LOL);
	AIC3204_Write(AIC32X4_LORROUTE, RDAC_2_LOR);
	AIC3204_Write(AIC32X4_HPLROUTE, LDAC_2_HPL);
	AIC3204_Write(AIC32X4_HPRROUTE, RDAC_2_HPR);
}

static void AIC3204_SetBiasLevel(SND_SOC_BIAS_LEVEL eLevel)
{
	switch (eLevel)
    {
	case SND_SOC_BIAS_ON:
		AIC3204_Write(AIC32X4_PLLPR, AIC32X4_PLLEN);    // Switch on PLL
		AIC3204_Write(AIC32X4_NDAC, AIC32X4_NDACEN);    // Switch on NDAC Divider
		AIC3204_Write(AIC32X4_MDAC, AIC32X4_MDACEN);    // Switch on MDAC Divider
		AIC3204_Write(AIC32X4_NADC, AIC32X4_NADCEN);    // Switch on NADC Divider
		AIC3204_Write(AIC32X4_MADC, AIC32X4_MADCEN);    // Switch on MADC Divider
		AIC3204_Write(AIC32X4_BCLKN, AIC32X4_BCLKEN);   // Switch on BCLK_N Divider
		break;
	case SND_SOC_BIAS_OFF:
		AIC3204_Write(AIC32X4_PLLPR, 0);                // Switch off PLL
		AIC3204_Write(AIC32X4_NDAC, 0);                 // Switch off NDAC Divider
		AIC3204_Write(AIC32X4_MDAC, 0);                 // Switch off MDAC Divider
		AIC3204_Write(AIC32X4_NADC, 0);                 // Switch off NADC Divider
		AIC3204_Write(AIC32X4_MADC, 0);                 // Switch off MADC Divider
		AIC3204_Write(AIC32X4_BCLKN, 0);                // Switch off BCLK_N Divider
		break;
	case SND_SOC_BIAS_PREPARE:
		break;
	case SND_SOC_BIAS_STANDBY:
		break;
	}
}

static void AIC3204_SetDaiFmt(BYTE ucAudioMode, BYTE ucMasterFace)
{
	BYTE    ucFaceReg1;
	BYTE    ucFaceReg2;
	BYTE    ucFaceReg3;

	ucFaceReg1 = AIC3204_Read(AIC32X4_IFACE1);
	ucFaceReg1 = ucFaceReg1 & ~(3 << 6 | 3 << 2);
	ucFaceReg2 = AIC3204_Read(AIC32X4_IFACE2);
#ifdef MCU_I2S_MASTER
	ucFaceReg2 = 1;
#else
    ucFaceReg2 = 0;
#endif
	ucFaceReg3 = AIC3204_Read(AIC32X4_IFACE3);
	ucFaceReg3 = ucFaceReg3 & ~(1 << 3);

	// set master/slave audio interface
	switch (ucMasterFace)
    {
	case SND_SOC_DAIFMT_CBM_CFM:
		ucFaceReg1 |= AIC32X4_BCLKMASTER | AIC32X4_WCLKMASTER;
		break;
	case SND_SOC_DAIFMT_CBS_CFS:
		break;
	default:
		break;
	}

	switch (ucAudioMode)
    {
	case SND_SOC_DAIFMT_I2S:
		ucFaceReg1 |= AIC32X4_WORD_LEN_16BITS;
		break;
	case SND_SOC_DAIFMT_DSP_A:
		ucFaceReg1 |= (AIC32X4_DSP_MODE << AIC32X4_PLLJ_SHIFT);
		ucFaceReg3 |= (1 << 3);                 // invert bit clock
		ucFaceReg2 = 0x01;                      // add offset 1
		break;
	case SND_SOC_DAIFMT_DSP_B:
		ucFaceReg1 |= (AIC32X4_DSP_MODE << AIC32X4_PLLJ_SHIFT);
		ucFaceReg3 |= (1 << 3);                 // invert bit clock
		break;
	case SND_SOC_DAIFMT_RIGHT_J:
		ucFaceReg1 |= (AIC32X4_RIGHT_JUSTIFIED_MODE << AIC32X4_PLLJ_SHIFT);
		break;
	case SND_SOC_DAIFMT_LEFT_J:
		ucFaceReg1 |= (AIC32X4_LEFT_JUSTIFIED_MODE << AIC32X4_PLLJ_SHIFT);
		break;
	default:
		break;
	}

	AIC3204_Write(AIC32X4_IFACE1, ucFaceReg1);
	AIC3204_Write(AIC32X4_IFACE2, ucFaceReg2);
	AIC3204_Write(AIC32X4_IFACE3, ucFaceReg3);
	
	AIC3204_Write(AIC32X4_ADCSPB, PRB_R1);
	AIC3204_Write(AIC32X4_DACSPB, PRB_P1);
}

static int AIC3204_GetDivs(int nMclk, int nRate)
{
	int     nFind = -1, i;

    i = sizeof(g_sAIC32X4_DIVS) / sizeof(AIC32X4_RATE_DIVS) - 1;
	for (; nFind < 0 && i >= 0; i--)
    {
		if ((g_sAIC32X4_DIVS[i].nRate == nRate)&& (g_sAIC32X4_DIVS[i].nMclk == nMclk))
            nFind = i;
	}
    return nFind;
}

// 禁用AIC3204
void AIC3204_Disable(void)
{	
//    I2SDisable();
	g_bAIC3204Enable = FALSE;
}	

// 启用AIC3204
void AIC3204_Enable(void)
{	
//	I2SEnable();
    g_bDMARxCheck = TRUE;
	g_bAIC3204Enable = TRUE;
}

// AIC3204 DMA数据接收检查
void AIC3204_DMARxCheck(void)
{
    if (g_bAIC3204Enable)
    {
        if (g_bDMARxCheck == FALSE)
        {
            AIC3204_SoftwareReset();
            AIC3204_SetRate(g_nCurRate);
            AIC3204_Enable();
        }
    }
    g_bDMARxCheck = FALSE;
}

// 输入输出是否直通
void AIC3204_SetByPass(BOOL bByPass)
{
    g_sVolumeCur.bByPass = bByPass;
}

// 设置采样频率
BOOL AIC3204_SetRate(UINT16 nRate)
{
    BOOL    bRet = FALSE;
	BYTE    ucData = 0;
	int     nDivs;
    BOOL    bAIC3204Enable;

    if (nRate >= 48000)
        nRate = 48000;
    else if (nRate >= 44100)
        nRate = 44100;
    else if (nRate >= 32000)
        nRate = 32000;
    else if (nRate >= 22050)
        nRate = 22050;
    else if (nRate >= 16000)
        nRate = 16000;
    else if (nRate >= 11025)
        nRate = 11025;
    else
        nRate = 8000;
	nDivs = AIC3204_GetDivs(AIC32X4_FREQ_13000000, nRate);
	if (nDivs >= 0)
    {
        g_nCurRate = nRate;
        bAIC3204Enable = g_bAIC3204Enable;
        if (bAIC3204Enable)
            AIC3204_Disable();
        
        AIC3204_SetMuteHPR(g_sVolumeCur.bMuteHPR);
        AIC3204_SetMuteHPL(g_sVolumeCur.bMuteHPL);
        AIC3204_SetMuteLOR(g_sVolumeCur.bMuteLOR);
        AIC3204_SetMuteLOL(g_sVolumeCur.bMuteLOL);
        AIC3204_SetMuteInput(g_sVolumeCur.bMuteInput);
        AIC3204_SetMuteOutput(g_sVolumeCur.bMuteOutput);
        AIC3204_SetMuteLMICPGA(g_sVolumeCur.bMuteLMICPGA);
        AIC3204_SetMuteRMICPGA(g_sVolumeCur.bMuteRMICPGA);
        AIC3204_SetVolumeHPR(g_sVolumeCur.ucVolumeHPR);
        AIC3204_SetVolumeHPL(g_sVolumeCur.ucVolumeHPL);
        AIC3204_SetVolumeLOR(g_sVolumeCur.ucVolumeLOR);
        AIC3204_SetVolumeLOL(g_sVolumeCur.ucVolumeLOL);
        AIC3204_SetVolumeInput(g_sVolumeCur.ucVolumeInput);
//        AIC3204_SetVolumeOutput(g_sVolumeCur.ucVolumeOutput);
        AIC3204_SetVolumeLMICPGA(g_sVolumeCur.ucVolumeLMICPGA);
        AIC3204_SetVolumeRMICPGA(g_sVolumeCur.ucVolumeRMICPGA);
        
        // Use PLL as CODEC_CLKIN and DAC_MOD_CLK as BDIV_CLKIN
        AIC3204_Write(AIC32X4_CLKMUX, PLL_2_CLKIN);
        ucData = AIC3204_Read(AIC32X4_IFACE3);
        ucData &= ~(0x03);
        AIC3204_Write(AIC32X4_IFACE3, ucData | DACMOD_2_BCLK);

        // We will fix R value to 1 and will make P & J=K.D as varialble
        ucData = AIC3204_Read(AIC32X4_PLLPR);
        ucData &= ~(7 << 4);
        AIC3204_Write(AIC32X4_PLLPR, (ucData | (g_sAIC32X4_DIVS[nDivs].ucP_val << 4) | g_sAIC32X4_DIVS[nDivs].ucR_val));

        AIC3204_Write(AIC32X4_PLLJ, g_sAIC32X4_DIVS[nDivs].ucPLL_j);

        AIC3204_Write(AIC32X4_PLLDMSB, (g_sAIC32X4_DIVS[nDivs].wPLL_d >> 8));
        AIC3204_Write(AIC32X4_PLLDLSB, (g_sAIC32X4_DIVS[nDivs].wPLL_d & 0xff));

        // NDAC divider value
        ucData = AIC3204_Read(AIC32X4_NDAC);
        ucData &= ~(0x7f);
        AIC3204_Write(AIC32X4_NDAC, ucData | g_sAIC32X4_DIVS[nDivs].ucNdac);

        // MDAC divider value
        ucData = AIC3204_Read(AIC32X4_MDAC);
        ucData &= ~(0x7f);
        AIC3204_Write(AIC32X4_MDAC, ucData | g_sAIC32X4_DIVS[nDivs].ucMdac);

        // DOSR MSB & LSB values
        AIC3204_Write(AIC32X4_DOSRMSB, g_sAIC32X4_DIVS[nDivs].wDosr >> 8);
        AIC3204_Write(AIC32X4_DOSRLSB, (g_sAIC32X4_DIVS[nDivs].wDosr & 0xff));

        // NADC divider value
        ucData = AIC3204_Read(AIC32X4_NADC);
        ucData &= ~(0x7f);
        AIC3204_Write(AIC32X4_NADC, ucData | g_sAIC32X4_DIVS[nDivs].ucNadc);

        // MADC divider value
        ucData = AIC3204_Read(AIC32X4_MADC);
        ucData &= ~(0x7f);
        AIC3204_Write(AIC32X4_MADC, ucData | g_sAIC32X4_DIVS[nDivs].ucMdac);

        // AOSR value
        AIC3204_Write(AIC32X4_AOSR, g_sAIC32X4_DIVS[nDivs].ucAosr);

        // BCLK N divider
        ucData = AIC3204_Read(AIC32X4_BCLKN);
        ucData &= ~(0x7f);
        AIC3204_Write(AIC32X4_BCLKN, ucData | g_sAIC32X4_DIVS[nDivs].ucBlck_N);

        ucData = AIC3204_Read(AIC32X4_IFACE1);
        ucData = ucData & ~(3 << 4);
        AIC3204_Write(AIC32X4_IFACE1, ucData);
        
//        AIC3204_I2SInit(nRate);
        if (bAIC3204Enable)
            AIC3204_Enable();
        
        bRet = TRUE;
    }
    return bRet;
}

// 耳机输出左声道是否静音
void AIC3204_SetMuteHPL(BOOL bMute)
{
    BYTE    ucData = 0;
    ucData = AIC3204_Read(AIC32X4_HPLGAIN);
    if (bMute)
        ucData |= 1 << 6;
    else
        ucData &= ~(1 << 6);
    AIC3204_Write(AIC32X4_HPLGAIN, ucData);
    g_sVolumeCur.bMuteHPL = bMute;
}

// 耳机输出右声道是否静音
void AIC3204_SetMuteHPR(BOOL bMute)
{
    BYTE    ucData = 0;
    ucData = AIC3204_Read(AIC32X4_HPRGAIN);
    if (bMute)
        ucData |= 1 << 6;
    else
        ucData &= ~(1 << 6);
    AIC3204_Write(AIC32X4_HPRGAIN, ucData);
    g_sVolumeCur.bMuteHPR = bMute;
}

// 线路输出左声道是否静音
void AIC3204_SetMuteLOL(BOOL bMute)
{
    BYTE    ucData = 0;
    ucData = AIC3204_Read(AIC32X4_LOLGAIN);
    if (bMute)
        ucData |= 1 << 6;
    else
        ucData &= ~(1 << 6);
    AIC3204_Write(AIC32X4_LOLGAIN, ucData);
    g_sVolumeCur.bMuteLOL = bMute;
}

// 线路输出右声道是否静音
void AIC3204_SetMuteLOR(BOOL bMute)
{
    BYTE    ucData = 0;
    ucData = AIC3204_Read(AIC32X4_LORGAIN);
    if (bMute)
        ucData |= 1 << 6;
    else
        ucData &= ~(1 << 6);
    AIC3204_Write(AIC32X4_LORGAIN, ucData);
    g_sVolumeCur.bMuteLOR = bMute;
}

// 输入是否静音
void AIC3204_SetMuteInput(BOOL bMute)
{
    if (bMute)
        AIC3204_Write(AIC32X4_ADCFGA, ADC_MUTED);
    else
        AIC3204_Write(AIC32X4_ADCFGA, ADC_UN_MUTED);
    g_sVolumeCur.bMuteInput = bMute;
}

// 总输出是否静音
void AIC3204_SetMuteOutput(BOOL bMute)
{
    if (bMute)
        AIC3204_Write(AIC32X4_DACMUTE, AIC32X4_MUTE);
    else
        AIC3204_Write(AIC32X4_DACMUTE, AIC32X4_UNMUTE);
    g_sVolumeCur.bMuteOutput = bMute;
}

// LMICPGA输入是否静音
void AIC3204_SetMuteLMICPGA(BOOL bMute)
{
    if (bMute)
    {
        AIC3204_Write(AIC32X4_LMICPGAPIN, 0);
        AIC3204_Write(AIC32X4_LMICPGANIN, 0);
    }
    else
    {
        AIC3204_Write(AIC32X4_LMICPGAPIN, AIC32X4_LMICPGAPIN_IN2L_10K);
        AIC3204_Write(AIC32X4_LMICPGANIN, AIC32X4_LMICPGANIN_IN2R_10K);// AIC32X4_LMICPGANIN_IN2R_10K AIC32X4_LMICPGANIN_CM2L_10K
//        AIC3204_Write(AIC32X4_LMICPGAPIN, AIC32X4_LMICPGAPIN_IN1L_10K | AIC32X4_LMICPGAPIN_IN1R_10K);
//        AIC3204_Write(AIC32X4_LMICPGANIN, AIC32X4_LMICPGANIN_CM1L_10K);
    }
    g_sVolumeCur.bMuteLMICPGA = bMute;
}

// RMICPGA输入是否静音
void AIC3204_SetMuteRMICPGA(BOOL bMute)
{
    if (bMute)
    {
        AIC3204_Write(AIC32X4_RMICPGAPIN, 0);
        AIC3204_Write(AIC32X4_RMICPGANIN, 0);
    }
    else
    {
        AIC3204_Write(AIC32X4_RMICPGAPIN, 0);
        AIC3204_Write(AIC32X4_RMICPGANIN, 0);
    }
    g_sVolumeCur.bMuteRMICPGA = bMute;
}

// 耳机输出左声道音量
void AIC3204_SetVolumeHPL(BYTE ucVolume)
{
    BYTE    ucData = 0;
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeHPL = ucVolume;
    ucVolume = g_pPHLOVolume[ucVolume];
    ucData = AIC3204_Read(AIC32X4_HPLGAIN);
    ucData &= 0xC0;
    ucData |= ucVolume;
    AIC3204_Write(AIC32X4_HPLGAIN, ucData);
}

// 耳机输出右声道音量
void AIC3204_SetVolumeHPR(BYTE ucVolume)
{
    BYTE    ucData = 0;
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeHPR = ucVolume;
    ucVolume = g_pPHLOVolume[ucVolume];
    ucData = AIC3204_Read(AIC32X4_HPRGAIN);
    ucData &= 0xC0;
    ucData |= ucVolume;
    AIC3204_Write(AIC32X4_HPRGAIN, ucData);
}

// 线路输出左声道音量
void AIC3204_SetVolumeLOL(BYTE ucVolume)
{
    BYTE    ucData = 0;
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeLOL = ucVolume;
    ucVolume = g_pPHLOVolume[ucVolume];
    ucData = AIC3204_Read(AIC32X4_LOLGAIN);
    ucData &= 0xC0;
    ucData |= ucVolume;
    AIC3204_Write(AIC32X4_LOLGAIN, ucData);
}

// 线路输出右声道音量
void AIC3204_SetVolumeLOR(BYTE ucVolume)
{
    BYTE    ucData = 0;
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeLOR = ucVolume;
    ucVolume = g_pPHLOVolume[ucVolume];
    ucData = AIC3204_Read(AIC32X4_LORGAIN);
    ucData &= 0xC0;
    ucData |= ucVolume;
    AIC3204_Write(AIC32X4_LORGAIN, ucData);
}

// 输入音量
void AIC3204_SetVolumeInput(BYTE ucVolume)
{
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeInput = ucVolume;
    ucVolume = g_pInputVolume[ucVolume];
    AIC3204_Write(AIC32X4_LADCVOL, ucVolume);
    AIC3204_Write(AIC32X4_RADCVOL, ucVolume);
}

// 总输出音量
void AIC3204_SetVolumeOutput(BYTE ucVolume)
{
    AIC3204_Write(AIC32X4_RDACVOL, ucVolume);    

//    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
//    g_sVolumeCur.ucVolumeOutput = ucVolume;
//    ucVolume = g_pOutputVolume[ucVolume];
//    AIC3204_Write(AIC32X4_LDACVOL, ucVolume);
//    AIC3204_Write(AIC32X4_RDACVOL, ucVolume);
//    
//    if(g_sVolumeCur.ucVolumeOutput <= 3)
//    {
//        AIC3204_Write(AIC32X4_DRCCTRL1, 0);
//    }
//    else
//    {
//        AIC3204_Write(AIC32X4_DRCCTRL1, AIC32X4_LDRCEN | AIC32X4_RDRCEN | AIC32X4_DRCTHR24DB | AIC32X4_DRCHYS3DB);
//    }
}

// LMICPGA输入音量
void AIC3204_SetVolumeLMICPGA(BYTE ucVolume)
{
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeLMICPGA = ucVolume;
    ucVolume = g_pMICPGAVolume[ucVolume];
    AIC3204_Write(AIC32X4_LMICPGAVOL, ucVolume);
}

// RMICPGA输入音量
void AIC3204_SetVolumeRMICPGA(BYTE ucVolume)
{
    ucVolume = MIN(ucVolume, AIC3204_VOLUME_MAX);
    g_sVolumeCur.ucVolumeRMICPGA = ucVolume;
    ucVolume = g_pMICPGAVolume[ucVolume];
    AIC3204_Write(AIC32X4_RMICPGAVOL, ucVolume);
}

// I2S DMA发送处理
static void AIC3204_I2STxHandler(BYTE *pBuf, BYTE ucLen)
{
#ifdef USE_U_LAW
    BYTE        i;
    static BYTE ucLast = 0;
    if (RF_GetRecvData(pBuf, ucLen) == 0)
    {
        g_nLackFrame++;
        for (i = 0; i < ucLen; i++)
            *pBuf++ = ucLast;
    }
    else
        ucLast = pBuf[ucLen - 1];
#else
    BYTE        i;
    short       *pshBuf = (short *)pBuf;
    static short shLast = 0;
    if (RF_GetRecvData(pBuf, ucLen) == 0)
    {
        g_nLackFrame++;
        ucLen >>= 1;
        for (i = 0; i < ucLen; i++)
            *pshBuf++ = shLast;
    }
    else
        shLast = pshBuf[(ucLen >> 1) - 1];
#endif
}

#define AIC32X4_LAGC_CTRL1                      (86)
#define AIC32X4_RAGC_CTRL1                      (94)
    #define AIC32X4_AGC_DISABLE                 (0x00 << 7)
    #define AIC32X4_AGC_ENABLE                  (0x01 << 7)
    #define AIC32X4_AGC_TARGET_LEVEL_12dB       (0x03 << 4)
    #define AIC32X4_AGC_GAIN_HYSTERESIS_1dB     (0x02 << 0)
#define AIC32X4_LAGC_CTRL2                      (87)
#define AIC32X4_RAGC_CTRL2                      (95)
    #define AIC32X4_AGC_HYSTERESIS_1dB          (0x00 << 6)
    #define AIC32X4_AGC_NOISE_THRESHOLD_90dB    (((63 - 28) / 2) << 1)
#define AIC32X4_LAGC_CTRL3                      (88)
#define AIC32X4_RAGC_CTRL3                      (96)
    #define AIC32X4_AGC_MAX_GAIN                (9 << 1)
#define AIC32X4_LAGC_CTRL4                      (89)
#define AIC32X4_RAGC_CTRL4                      (97)
    #define AIC32X4_AGC_ATTACK_TIME             (2 << 3)
    #define AIC32X4_AGC_ATTACK_SCALE            (2 << 3)
#define AIC32X4_LAGC_CTRL5                      (90)
#define AIC32X4_RAGC_CTRL5                      (98)
    #define AIC32X4_AGC_DECAY_TIME              (2 << 3)
    #define AIC32X4_AGC_DECAY_SCALE             (2 << 3)
#define AIC32X4_LAGC_CTRL6                      (91)
#define AIC32X4_RAGC_CTRL6                      (99)
    #define AIC32X4_AGC_NOISE_DEBOUNCE_TIME     (50)
#define AIC32X4_LAGC_CTRL7                      (92)
#define AIC32X4_RAGC_CTRL7                      (100)
    #define AIC32X4_AGC_SIGNAL_DEBOUNCE_TIME    (0)
// AGC配置
void AIC3204_SetAGC(BOOL bEnable)
{
    if (bEnable)
    {
//        AIC3204_Write(AIC32X4_LAGC_CTRL1, AIC32X4_AGC_ENABLE | AIC32X4_AGC_TARGET_LEVEL_12dB | AIC32X4_AGC_GAIN_HYSTERESIS_1dB);
//        AIC3204_Write(AIC32X4_RAGC_CTRL1, AIC32X4_AGC_ENABLE | AIC32X4_AGC_TARGET_LEVEL_12dB | AIC32X4_AGC_GAIN_HYSTERESIS_1dB);
//        AIC3204_Write(AIC32X4_LAGC_CTRL2, AIC32X4_AGC_HYSTERESIS_1dB | AIC32X4_AGC_NOISE_THRESHOLD_90dB);
//        AIC3204_Write(AIC32X4_RAGC_CTRL2, AIC32X4_AGC_HYSTERESIS_1dB | AIC32X4_AGC_NOISE_THRESHOLD_90dB);
        AIC3204_Write(AIC32X4_LAGC_CTRL3, AIC32X4_AGC_MAX_GAIN);
        AIC3204_Write(AIC32X4_RAGC_CTRL3, AIC32X4_AGC_MAX_GAIN);
        AIC3204_Write(AIC32X4_LAGC_CTRL4, AIC32X4_AGC_ATTACK_TIME | AIC32X4_AGC_ATTACK_SCALE);
        AIC3204_Write(AIC32X4_RAGC_CTRL4, AIC32X4_AGC_ATTACK_TIME | AIC32X4_AGC_ATTACK_SCALE);
        AIC3204_Write(AIC32X4_LAGC_CTRL5, AIC32X4_AGC_DECAY_TIME | AIC32X4_AGC_DECAY_SCALE);
        AIC3204_Write(AIC32X4_RAGC_CTRL5, AIC32X4_AGC_DECAY_TIME | AIC32X4_AGC_DECAY_SCALE);
        AIC3204_Write(AIC32X4_LAGC_CTRL6, AIC32X4_AGC_NOISE_DEBOUNCE_TIME);
        AIC3204_Write(AIC32X4_RAGC_CTRL6, AIC32X4_AGC_NOISE_DEBOUNCE_TIME);
        AIC3204_Write(AIC32X4_LAGC_CTRL7, AIC32X4_AGC_SIGNAL_DEBOUNCE_TIME);
        AIC3204_Write(AIC32X4_RAGC_CTRL7, AIC32X4_AGC_SIGNAL_DEBOUNCE_TIME);
    }
    else
    {
        AIC3204_Write(AIC32X4_LAGC_CTRL1, AIC32X4_AGC_DISABLE);
        AIC3204_Write(AIC32X4_RAGC_CTRL1, AIC32X4_AGC_DISABLE);
    }
    g_sVolumeCur.bEnableAGC = bEnable;
}

// 读取当前AGC增益
void AIC3204_GetAGCGain(int *pnGainL, int *pnGainR)
{
    if (pnGainL != NULL)
        *pnGainL = AIC3204_Read(93);
    if (pnGainR != NULL)
        *pnGainR = AIC3204_Read(101);
}




