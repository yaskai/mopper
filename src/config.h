#ifndef CONFIG_H_
#define CONFIG_H_

#include <stdint.h>

#define CONF_LOADED		0x01

typedef struct {
	uint8_t flags;
	int ww, wh;
	int fps;
	float mouse_sensitivity;
} Config;

void LoadConfig(Config *conf, char *file_path);

#endif
