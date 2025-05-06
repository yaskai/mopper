#ifndef CONFIG_H_
#define CONFIG_H_

typedef struct {
	int ww, wh;
	int fps;
	float mouse_sensitivity;
} Config;

void LoadConfig(Config *conf, char *file_path);

#endif
