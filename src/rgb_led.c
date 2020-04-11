#include <zephyr.h>
#include <device.h>
#include <drivers/pwm.h>
#include <logging/log.h>

LOG_MODULE_REGISTER(ble_rgb_light_led, LOG_LEVEL_DBG);

#define PERIOD (USEC_PER_SEC / 100)
#define PWM_DEV DT_ALIAS_PWM_0_LABEL
#define RED_PIN DT_ALIAS_PWM_0_CH0_PIN
#define GREEN_PIN DT_ALIAS_PWM_0_CH1_PIN
#define BLUE_PIN DT_ALIAS_PWM_0_CH2_PIN

struct device *pwm_dev;

void rgb_led_init()
{

  pwm_dev = device_get_binding(PWM_DEV);

  if (!pwm_dev)
  {
    LOG_ERR("Cannot find PWM device!\n");
    return;
  }
}

void rgb_led_set(u32_t r, u32_t g, u32_t b)
{
  pwm_pin_set_usec(pwm_dev, RED_PIN, PERIOD, ((255 - (r & 0xff)) * PERIOD) >> 8, PWM_POLARITY_NORMAL);
  k_sleep(K_MSEC(1));
  pwm_pin_set_usec(pwm_dev, GREEN_PIN, PERIOD, ((255 - (g & 0xff)) * PERIOD) >> 8, PWM_POLARITY_NORMAL);
  k_sleep(K_MSEC(1));
  pwm_pin_set_usec(pwm_dev, BLUE_PIN, PERIOD, ((255 - (b & 0xff)) * PERIOD) >> 8, PWM_POLARITY_NORMAL);
  k_sleep(K_MSEC(1));
}