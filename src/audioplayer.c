#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "raylib.h"
#include "raymath.h"
#include "audioplayer.h"

void AudioPlayerInit(AudioPlayer *ap) {
	ap->sfx_count = 8;
	ap->track_count = 3;
	
	char *sfx_pref = "audio/sfx";


	ap->tracks[TRACK_DEFAULT_MUSIC] = LoadTrack(0, "music_default.ogg");
	ap->tracks[TRACK_PHARM_MUSIC]	= LoadTrack((SPATIAL), "pharmacy_music.ogg");
	ap->tracks[TRACK_PLAYER_WALK]	= LoadTrack(0, "player_walk.mp3");
		
	ap->sfx[SFX_CLEAN] 		 	= LoadSoundEffect(0, "clean.mp3");
	ap->sfx[SFX_USE_INHALER] 	= LoadSoundEffect(0, "use_inhaler.mp3");
	ap->sfx[SFX_BREATHE]	 	= LoadSoundEffect(0, "breathe.mp3");
	
	// -- CHECK HEALTH --
	for(uint8_t i = 0; i < ap->sfx_count; i++) {
		if((ap->sfx[i].flags & FILE_LOADED) == 0) continue;

		if(ap->sfx[i].flags & SPATIAL) 
			if(!(Vector3Equals(Vector3Zero(), ap->sfx[i].position))) ap->sfx[i].flags |= POS_SET;

		if((ap->sfx[i].flags & SPATIAL) && (ap->sfx[i].flags & POS_SET) == 0) 
			printf("WARING: SFX[%d] POSITION NOT SET\n", i);
	}

	for(uint8_t i = 0; i < ap->track_count; i++) {
		if((ap->tracks[i].flags & FILE_LOADED) == 0) continue;

		if(ap->tracks[i].flags & SPATIAL) 
			if(!(Vector3Equals(Vector3Zero(), ap->tracks[i].position))) ap->tracks[i].flags |= POS_SET;

		if((ap->tracks[i].flags & SPATIAL) && (ap->tracks[i].flags & POS_SET) == 0) 
			printf("WARING: TRACKS[%d] POSITION NOT SET\n", i);
	}

	PlayMusicStream(ap->tracks[TRACK_DEFAULT_MUSIC].music);
	//PlayMusicStream(ap->tracks[TRACK_PHARM_MUSIC].music);
}

void AudioPlayerClose(AudioPlayer *ap) {
	for(uint8_t i = 0; i < ap->sfx_count; i++)
		if(ap->sfx[i].flags & FILE_LOADED) UnloadSound(ap->sfx[i].sound); 

	for(uint8_t i = 0; i < ap->track_count; i++) 	
		if(ap->tracks[i].flags & FILE_LOADED) UnloadMusicStream(ap->tracks[i].music);
}

SoundEffect LoadSoundEffect(uint8_t flags, char *file_name) {
	char *file_pref = "audio/sfx/";		

	SoundEffect effect;
	effect.flags = flags;
	effect.flags &= ~POS_SET;

	effect.sound = LoadSound(TextFormat("%s%s", file_pref, file_name));
	effect.flags |= FILE_LOADED;

	return effect;
}

Track LoadTrack(uint8_t flags, char *file_name) {
	char *file_pref = "audio/tracks/";		

	Track track;
	track.flags = flags;
	track.flags &= ~POS_SET;

	track.music = LoadMusicStream(TextFormat("%s%s", file_pref, file_name));
	track.flags |= FILE_LOADED;

	return track;
}

void PlayEffect(AudioPlayer *ap, uint8_t id) {
	// TODO:
	if(ap->sfx[id].flags & SPATIAL) {
		
	}
		
	PlaySound(ap->sfx[id].sound);
}

void PlayTrack(AudioPlayer *ap, uint8_t id) {
	// TODO:
	if(ap->tracks[id].flags & SPATIAL) {
		
	}
	
	if(!IsMusicStreamPlaying(ap->tracks[id].music)) PlayMusicStream(ap->tracks[id].music);
	UpdateMusicStream(ap->tracks[id].music);
}

void StopEffect(AudioPlayer *ap, uint8_t id) {
	if(IsSoundPlaying(ap->sfx[id].sound)) StopSound(ap->sfx[id].sound);
}

