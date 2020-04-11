#ifndef __GATT_NUS_SERVICE_H__
#define __GATT_NUS_SERVICE_H__

#include <data/json.h>

#define BT_NUS_SERVICE_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x6E400001, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E))
#define BT_RX_CHARACTERISTIC_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x6E400002, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E))
#define BT_TX_CHARACTERISTIC_UUID BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x6E400003, 0xB5A3, 0xF393, 0xE0A9, 0xE50E24DCCA9E))

struct cmd_msg
{
  const char *color;
};

struct status_msg
{
  const char *color;
  int r;
  int g;
  int b;
};

static const struct json_obj_descr cmd_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct cmd_msg, color, JSON_TOK_STRING),
};

static const struct json_obj_descr status_msg_descr[] = {
    JSON_OBJ_DESCR_PRIM(struct status_msg, color, JSON_TOK_STRING),
    JSON_OBJ_DESCR_PRIM(struct status_msg, r, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct status_msg, g, JSON_TOK_NUMBER),
    JSON_OBJ_DESCR_PRIM(struct status_msg, b, JSON_TOK_NUMBER),
};

void gatt_nus_service_init(void);
void gatt_nus_service_data_notify();

#endif