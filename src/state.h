#ifndef __STATE_H__
#define __STATE_H__

#include <data/json.h>

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

static struct status_msg current_status = {
    .color = "000000",
    .r = 0,
    .g = 0,
    .b = 0,
};

void update_color_rgb(u32_t r, u32_t g, u32_t b);
void update_color(const char *);

#endif