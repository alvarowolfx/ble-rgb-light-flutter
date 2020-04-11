
#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <sys/byteorder.h>
#include <zephyr.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/conn.h>
#include <bluetooth/uuid.h>
#include <bluetooth/gatt.h>

#include "gatt_led_service.h"
#include "rgb_led.h"

static u8_t notify_data;
static u8_t notify_heartbeat;
static u8_t latest_color_data[3];

static void heartbeat_ccc_cfg_changed(const struct bt_gatt_attr *attr,
                                      u16_t value)
{
  notify_heartbeat = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static void data_ccc_cfg_changed(const struct bt_gatt_attr *attr,
                                 u16_t value)
{
  notify_data = (value == BT_GATT_CCC_NOTIFY) ? 1 : 0;
}

static ssize_t write_cmd(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         const void *buf, u16_t len, u16_t offset,
                         u8_t flags);

BT_GATT_SERVICE_DEFINE(led_cvs,
                       BT_GATT_PRIMARY_SERVICE(BT_SERVICE_UUID),

                       BT_GATT_CHARACTERISTIC(BT_HEARTBEAT_CHARACTERISTIC_UUID,
                                              BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_NONE, NULL, NULL, NULL),
                       BT_GATT_CCC(heartbeat_ccc_cfg_changed,
                                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

                       BT_GATT_CHARACTERISTIC(BT_DATA_CHARACTERISTIC_UUID,
                                              BT_GATT_CHRC_NOTIFY,
                                              BT_GATT_PERM_NONE, NULL, NULL, NULL),
                       BT_GATT_CCC(data_ccc_cfg_changed,
                                   BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),

                       BT_GATT_CHARACTERISTIC(BT_CMD_CHARACTERISTIC_UUID,
                                              BT_GATT_CHRC_WRITE,
                                              BT_GATT_PERM_WRITE, NULL, write_cmd, NULL));

static ssize_t write_cmd(struct bt_conn *conn,
                         const struct bt_gatt_attr *attr,
                         const void *buf, u16_t len, u16_t offset,
                         u8_t flags)
{
  u8_t *values = (u8_t *)buf;

  if (len == 0)
  {
    return len;
  }

  if (len == 3)
  {
    rgb_led_set(values[0], values[1], values[2]);
    memcpy(latest_color_data, values, sizeof(latest_color_data));
  }

  return len;
}

s8_t initDone = 0;

void gatt_service_init(void)
{
  initDone = 1;
}

void gatt_service_data_notify()
{
  if (!initDone)
  {
    return;
  }

  /*if (!notify_data)
  {
    return;
  }*/

  bt_gatt_notify(NULL, &led_cvs.attrs[3], &latest_color_data, sizeof(latest_color_data));
}

void gatt_service_heartbeat_notify(u32_t count)
{
  if (!initDone)
  {
    return;
  }

  if (!notify_heartbeat)
  {
    return;
  }

  bt_gatt_notify(NULL, &led_cvs.attrs[1], &count, 2);
}
