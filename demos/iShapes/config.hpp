#ifndef ISOCPP_DEMO_ISHAPES_CONFIG_HPP
#define ISOCPP_DEMO_ISHAPES_CONFIG_HPP

/**
 * @addtogroup demos_iShapes
 */
/** @{*/
/** @file */
/** @}*/

#define ISHAPES_IOP_DEMO  1
#define ISHAPES_WIDE_FORMAT 1
#define REFRESH_PERIOD 40 //ms

// Durability Settings
#define PERSISTENT_TOPIC 1
#define DS_HISTORY 100
#define DS_MAX_SAMPLES 8192
#define DS_MAX_INSTANCES 4196
#define DS_MAX_SAMPLES_X_INSTANCE 8192

/*
 painter.drawPixmap(15, 300, logo_);
    painter.drawPixmap(200, 10, simd_);
 */
#if  (ISHAPES_WIDE_FORMAT == 1)
#  define IS_WIDTH 501
#  define IS_HEIGHT 361
#  define ISOCPP_LOGO_X 340
#  define ISOCPP_LOGO_Y 10
#  define BRAND_LOGO_X 200
#  define BRAND_LOGO_Y 310
#else
#  define IS_WIDTH  321
#  define IS_HEIGHT 361
#  define ISOCPP_LOGO_X 160
#  define ISOCPP_LOGO_Y 7
#  define BRAND_LOGO_X 15
#  define BRAND_LOGO_Y 300
#endif

#endif /* ISOCPP_DEMO_ISHAPES_CONFIG_HPP */
