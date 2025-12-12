#ifndef __CONFIG_H
#define __CONFIG_H

/*=========================
   Application configuration
 *=========================*/

#define CONFIG_SYSTEM_SAVE_FILE_PATH          "S:/SystemSave.json"
#define CONFIG_SYSTEM_LANGUAGE_DEFAULT        {'e','n','-','G','B'}
#define CONFIG_SYSTEM_GMT_OFFSET_DEFAULT      8 // GMT+ 8

#define CONFIG_WEIGHT_DEFAULT                 65 // kg

#define CONFIG_GPS_REFR_PERIOD                1000 // ms
#define CONFIG_GPS_LNG_DEFAULT                116.391332
#define CONFIG_GPS_LAT_DEFAULT                39.907415

#define CONFIG_TRACK_FILTER_OFFSET_THRESHOLD  2 // pixel
#define CONFIG_TRACK_RECORD_FILE_DIR_NAME     "Track"

#define CONFIG_MAP_USE_WGS84_DEFAULT          false

#define CONFIG_MAP_DIR_PATH                   {'/','M','A','P'}//"/MAP"

#define CONFIG_ARROW_THEME_DEFAULT            "default"

#define CONFIG_LIVE_MAP_LEVEL_DEFAULT         16
#define CONFIG_LIVE_MAP_VIEW_WIDTH            LV_HOR_RES
#define CONFIG_LIVE_MAP_VIEW_HEIGHT           LV_VER_RES

/* Simulator */
#define CONFIG_TRACK_VIRTUAL_GPX_FILE_PATH    "S:/TRK_20210801_203324.gpx"

/*=========================
   Hardware Configuration
 *=========================*/

#define NULL_PIN                    PD0

/* Screen */

#define CONFIG_SCREEN_BLK_PIN       -1 //15
#define CONFIG_SCREEN_HOR_RES       128
#define CONFIG_SCREEN_VER_RES       64
#define CONFIG_SCREEN_BUFFER_SIZE   (CONFIG_SCREEN_HOR_RES * CONFIG_SCREEN_VER_RES /2)

/* Battery */
#define CONFIG_BAT_DET_PIN          -1
#define CONFIG_BAT_CHG_DET_PIN      19

/* Button Configuration */
#define CONFIG_MENU_OK_PIN         32  // GPIO 32 - Menu/OK 按鈕
#define CONFIG_UP_BACK_PIN         33  // GPIO 33 - UP/Back 按鈕
#define CONFIG_DOWN_FN_PIN         34  // GPIO 34 - Down/Fn 按鈕 (輸入專用，需外部上拉)
#define CONFIG_PTT_PIN             35  // GPIO 35 - PTT 按鈕

/* IMU */
//#define CONFIG_IMU_INT1_PIN         32 Alex1017
//#define CONFIG_IMU_INT2_PIN         33

/* I2C */
#define CONFIG_MCU_SDA_PIN          21
#define CONFIG_MCU_SCL_PIN          22
#define CONFIG_BMP388_ID          22

#define CONFIG_OLED_I2C_ADDRESS     0x3C
#define CONFIG_BMP388_I2C_ADDRESS   0x76
#define CONFIG_QMC5883C_I2C_ADDRESS 0x0D  
#define CONFIG_BMA400_I2C_ADDRESS   0x68
#define CONFIG_AHT20_I2C_ADDRESS    0x38
#define CONFIG_CW2015_I2C_ADDRESS   0x60
#define CONFIG_RDA5807M_I2C_ADDRESS  0x11

/* Power */
#define CONFIG_POWER_EN_PIN         -1 //21

/* Debug USART */
#define CONFIG_DEBUG_SERIAL         Serial

/* Stack Info */
#define CONFIG_USE_STACK_INFO       0

/* SA818 Configuration */
#define CONFIG_SA818_RX_PIN            16   // ESP32 RX2 -> SA818 TX pin 17
#define CONFIG_SA818_TX_PIN            17   // ESP32 TX2 -> SA818 RX pin 16
#define CONFIG_SA818_PD_PIN            26   // SA818 PD pin 6 (Power Down control)
#define CONFIG_SA818_HL_PIN            18   // SA818 H/L pin 1 (High/Low Power control)
#define CONFIG_SA818_SQ_PIN            36
#define CONFIG_SA818_PTT_PIN           13   // Push To Talk button
#define CONFIG_SA818_BAUD              9600 // Fixed baud rate
#define CONFIG_SA818_BAUD_ALT          115200 // Alternative baud rate (unused)


#endif
