#ifndef AUDIOPLAYER_H_
#define AUDIOPLAYER_H_

#include <stdint.h>
#include "raylib.h"

enum SFX_ID : uint8_t {
	SFX_NONE = 0,
};

enum TRACK_ID : uint8_t {
	TRACK_NONE = 0,
};

typedef struct {
	bool spatial;	// Use distance based volume setting
	Sound sound;
	
	Vector3 position;	// Audio source position
} SoundEffect;

typedef struct {
	bool spatial;
	Music music;
	
	Vector3 position;
} Track;

typedef struct {
	uint8_t sfx_count;
	uint8_t track_count;

	SoundEffect *sfx;
	Track *tracks;
} AudioPlayer;

void AudioPlayerInit(AudioPlayer *ap);
void AudioPlayerClose(AudioPlayer *ap);

#endif // !AUDIOPLAYER_H_
