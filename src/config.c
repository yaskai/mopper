#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "config.h"

void LoadConfig(Config *conf, char *file_path) {
	conf->flags = 0;

	FILE *file = fopen(file_path, "r");

	if(file != NULL) {
		puts("READING CONFIGURATION FILE...");

		uint8_t line_id = 0;
		char line[256];
		
		while(fgets(line, sizeof(line), file)) {
			if(line[0] == '#') continue;	// Skip comments		
	
			// Parse
			switch(line_id) {
				case 0: sscanf(line, "resolution: %dx%d", &conf->ww, &conf->wh);		 break;
				case 1: sscanf(line, "target framerate: %d", &conf->fps);				 break;
				case 2: sscanf(line, "mouse sensitivity: %f", &conf->mouse_sensitivity); break;
			}

			line_id++;
		}

		printf("resolution: %dx%d\n", conf->ww, conf->wh);
		printf("framerate: %d\n", conf->fps);
		printf("mouse sensitivity: %f\n", conf->mouse_sensitivity);
		
		conf->flags |= CONF_LOADED;
		fclose(file);
	} else puts("ERROR: COULD NOT LOCATE CONFIGURATION FILE");
}
