
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

#include "rgb_led.h"

#define BT_SERVICE_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xb97417ae, 0x4ceb, 0x4baa, 0xab93, 0x654943bc4f5a))
#define BT_HEARTBEAT_CHARACTERISTIC_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x78b8e891, 0x0135, 0x475c, 0xab67, 0x814a37725cf3))
#define BT_DATA_CHARACTERISTIC_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x1ddd389c, 0xe639, 0x4603, 0xa145, 0xcd3867f7715b))
#define BT_CMD_CHARACTERISTIC_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0xbd06ba71, 0x9321, 0x409c, 0x8b82, 0xcc6ed61deb73))

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
