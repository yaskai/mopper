#include <stdint.h>
#include <string.h>
#include "raylib.h"
#include "audioplayer.h"

void AudioPlayerInit(AudioPlayer *ap) {
	SoundEffect sfx[ap->sfx_count] = {
	};

	Track tracks[ap->track_count] = {
	};

	memcpy(ap->sfx, sfx, sizeof(SoundEffect) * ap->sfx_count);
	memcpy(ap->tracks, tracks, sizeof(Track) * ap->track_count);
}

void AudioPlayerClose(AudioPlayer *ap) {
	for(uint8_t i = 0; i < ap->sfx_count; 	i++)	UnloadSound(ap->sfx[i].sound); 
	for(uint8_t i = 0; i < ap->track_count; i++) 	UnloadMusicStream(ap->tracks[i].music);
}
