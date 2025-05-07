#ifndef AUDIOPLAYER_H_
#define AUDIOPLAYER_H_

#include <stdint.h>
#include <sys/types.h>
#include "raylib.h"

#define SFX_MAX 	32
#define TRACK_MAX	16

#define FILE_LOADED		0x01
#define SPATIAL			0x02
#define POS_SET			0x04

enum SFX_ID : uint8_t {
	SFX_CLEAN			= 0,
	SFX_PLAYER_WALK		= 1,
	SFX_USE_INHALER		= 2,
	SFX_REFILL_INHALER	= 3,
	SFX_PLAYER_RUN		= 4,
	SFX_PFG_WALK		= 5,
	SFX_PFG_ATTACK		= 6,
	SFX_PFG_DIAG_0		= 7,
	SFX_PHARM_DIAG		= 8,
};

enum TRACK_ID : uint8_t {
	TRACK_DEFAULT_MUSIC 	= 0,
	TRACK_PHARM_MUSIC		= 1,
};

typedef struct {
	uint8_t flags;
	Sound sound;
	
	Vector3 position;	// Audio source position
} SoundEffect;

typedef struct {
	uint8_t flags;
	Music music;
	
	Vector3 position;
} Track;

typedef struct {
	uint8_t sfx_count;
	uint8_t track_count;

	SoundEffect sfx[SFX_MAX];
	Track tracks[TRACK_MAX];
} AudioPlayer;

void AudioPlayerInit(AudioPlayer *ap);
void AudioPlayerClose(AudioPlayer *ap);

SoundEffect LoadSoundEffect(uint8_t flags, char *file_path);
Track LoadTrack(uint8_t flags, char *file_path);

void EffectSetPos(AudioPlayer *ap, uint8_t id, Vector3 position);
void TrackSetPos(AudioPlayer *ap, uint8_t id, Vector3 position);

#endif // !AUDIOPLAYER_H_
