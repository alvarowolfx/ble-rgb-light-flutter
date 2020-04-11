#include <zephyr/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include <logging/log.h>
LOG_MODULE_REGISTER(state);

#include "state.h"

static struct status_msg current_status = {
    .color = "000000",
    .r = 0,
    .g = 0,
    .b = 0,
};

void update_color_rgb(int r, int g, int b)
{
  char buf[10];
  sprintf(buf, "%x%x%x", r, g, b);
  current_status.color = buf;
  current_status.r = r;
  current_status.g = g;
  current_status.b = b;
}

void update_color(const char *color)
{
  int hex_value = (int)strtol(color, NULL, 16);
  int r = (hex_value >> 16) & 0xFF;
  int g = (hex_value >> 8) & 0xFF;
  int b = hex_value & 0xFF;

  current_status.color = color;
  current_status.r = r;
  current_status.g = g;
  current_status.b = b;
}

struct status_msg get_current_status()
{
  return current_status;
}
