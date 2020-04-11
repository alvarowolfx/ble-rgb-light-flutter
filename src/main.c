#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <usb/usb_device.h>

#include <zephyr/types.h>
#include <stddef.h>
#include <sys/util.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>

#define LOG_LEVEL 4
#include <logging/log.h>
LOG_MODULE_REGISTER(ble_rgb_light);

#include "rgb_led.h"
#include "gatt_led_service.h"
#include "gatt_nus_service.h"

#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

#define LED_PORT DT_ALIAS_LED0_GPIOS_CONTROLLER
#define LED DT_ALIAS_LED0_GPIOS_PIN

/* 1000 msec = 1 sec */
#define SLEEP_TIME 1000

/*
 * Set Advertisement data. Based on the Eddystone specification:
 * https://github.com/google/eddystone/blob/master/protocol-specification.md
 * https://github.com/google/eddystone/tree/master/eddystone-url
 */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR),
    BT_DATA_BYTES(BT_DATA_UUID128_ALL,
                  0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12,
                  0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12)};

static struct device *dev;
static u8_t is_connected = 0;

static void connected(struct bt_conn *conn, u8_t err)
{
  if (err)
  {
    LOG_INF("Connection failed (err %u)\n", err);
    rgb_led_set(0x7f, 0, 0);
    gpio_pin_set(dev, LED, 0);
    is_connected = 0;
  }
  else
  {
    LOG_INF("Connected\n");
    rgb_led_set(0, 0, 0);
    gpio_pin_set(dev, LED, 1);
    is_connected = 1;
  }
}

static void disconnected(struct bt_conn *conn, u8_t reason)
{
  LOG_INF("Disconnected (reason %u)\n", reason);
  rgb_led_set(0x7f, 0, 0);
  is_connected = 0;
}

static struct bt_conn_cb conn_callbacks = {
    .connected = connected,
    .disconnected = disconnected,
};

static void bt_ready(int err)
{
  if (err)
  {
    LOG_ERR("Bluetooth init failed (err %d)\n", err);
    return;
  }

  LOG_INF("Bluetooth initialized\n");

  gatt_service_init();
  gatt_nus_service_init();

  /* Start advertising */
  err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad),
                        NULL, 0);
  if (err)
  {
    LOG_ERR("Advertising failed to start (err %d)\n", err);
    return;
  }

  LOG_INF("Gatt service started\n");
}

void main(void)
{

  rgb_led_init();
  rgb_led_set(0x7f, 0x00, 0x00);

  int ret;

  ret = usb_enable(NULL);
  if (ret != 0)
  {
    LOG_ERR("Failed to enable USB");
    return;
  }

  u32_t cnt = 0;

  dev = device_get_binding(LED_PORT);
  /* Set LED pin as output */
  gpio_pin_configure(dev, LED, GPIO_DIR_OUT);

  int err;

  LOG_INF("Starting Beacon Demo\n");

  /* Initialize the Bluetooth Subsystem */
  err = bt_enable(bt_ready);
  if (err)
  {
    LOG_ERR("Bluetooth init failed (err %d)\n", err);
  }

  bt_conn_cb_register(&conn_callbacks);

  while (1)
  {
    k_sleep(SLEEP_TIME);
    cnt++;

    if (!is_connected)
    {
      rgb_led_set(0, 0, cnt % 2 ? 0x7f : 0);
      gpio_pin_set(dev, LED, 0);
    }

    gatt_service_heartbeat_notify(cnt++);
    gatt_service_data_notify();
    gatt_nus_service_data_notify(NULL);

    LOG_INF("Hello - %d\n", cnt);
  }
}