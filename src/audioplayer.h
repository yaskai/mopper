#ifndef AUDIOPLAYER_H_
#define AUDIOPLAYER_H_

#include <stdint.h>
#include "raylib.h"

#define SFX_MAX 	32
#define TRACK_MAX	16

#define FILE_LOADED		0x01
#define SPATIAL			0x02
#define POS_SET			0x04

enum SFX_ID : uint8_t {
	SFX_CLEAN,
	SFX_USE_INHALER,
	SFX_REFILL_INHALER,
	SFX_PLAYER_RUN,		 
	SFX_PFG_WALK,
	SFX_PFG_ATTACK,
	SFX_PFG_DIAG_0,
	SFX_PHARM_DIAG,
	SFX_BREATHE,
};

enum TRACK_ID : uint8_t {
	TRACK_DEFAULT_MUSIC,
	TRACK_PHARM_MUSIC,
	TRACK_PLAYER_WALK,
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

void PlayEffect(AudioPlayer *ap, uint8_t id);
void PlayTrack(AudioPlayer *ap, uint8_t id);

void StopEffect(AudioPlayer *ap, uint8_t id);
void StopTrack(AudioPlayer *ap, uint8_t id);

#endif // !AUDIOPLAYER_H_
