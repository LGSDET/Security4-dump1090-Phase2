#ifndef RTL_SDR_H
#define RTL_SDR_H

// 실제 타입은 opaque pointer지만, 분석만 목적이므로 dummy struct로 대체
typedef struct rtlsdr_dev_t rtlsdr_dev_t;

#define rtlsdr_get_device_count(...) (0)
#define rtlsdr_get_device_usb_strings(...) (-1)
#define rtlsdr_open(...) (-1)
#define rtlsdr_set_tuner_gain_mode(...) (-1)
#define rtlsdr_get_tuner_gains(...) (-1)
#define rtlsdr_set_tuner_gain(...) (-1)
#define rtlsdr_set_freq_correction(...) (-1)
#define rtlsdr_set_agc_mode(...) (-1)
#define rtlsdr_set_center_freq(...) (-1)
#define rtlsdr_set_sample_rate(...) (-1)
#define rtlsdr_reset_buffer(...) (-1)
#define rtlsdr_get_tuner_gain(...) (-1)
#define rtlsdr_read_async(...) (-1)
#define rtlsdr_close(...) (-1)

#endif
