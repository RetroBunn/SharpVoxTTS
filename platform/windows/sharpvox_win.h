#ifndef SHARPVOX_WIN_H
#define SHARPVOX_WIN_H

#include <stdint.h>

#if defined(_WIN32)
  #if defined(SHARPVOX_BUILD_DLL)
    #define SHARPVOX_API __declspec(dllexport)
  #elif defined(SHARPVOX_USE_DLL)
    #define SHARPVOX_API __declspec(dllimport)
  #else
    #define SHARPVOX_API
  #endif
#else
  #define SHARPVOX_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct SharpVoxHandle_* SharpVoxHandle;

typedef enum SharpVoxStatus {
    SHARPVOX_OK = 0,
    SHARPVOX_ERR_NULL_HANDLE = 1,
    SHARPVOX_ERR_NULL_TEXT = 2,
    SHARPVOX_ERR_EXCEPTION = 3,
} SharpVoxStatus;

typedef enum SharpVoxPreset {
    SHARPVOX_PRESET_BASELINE = 0,
    SHARPVOX_PRESET_WHISPER = 1,
    SHARPVOX_PRESET_CUSTOM = 2,
} SharpVoxPreset;

// PCM chunk callback: 16-bit mono samples at the speaker's current SampleRate.
// Fired synchronously from within SharpVox_Speak, on the calling thread.
typedef void (*SharpVoxAudioCallback)(const int16_t* pcm, int32_t sampleCount, void* userdata);

// Fired for each phoneme event whose onset starts a new word, with its absolute
// offset (seconds) from the start of the utterance. Intended for driving a screen
// reader's word-boundary/index-reached tracking during an utterance.
typedef void (*SharpVoxWordBoundaryCallback)(float timeSeconds, void* userdata);

SHARPVOX_API SharpVoxHandle SharpVox_Create(void);
SHARPVOX_API void SharpVox_Destroy(SharpVoxHandle handle);

// Synthesizes text synchronously, invoking onAudio for each PCM chunk and
// onWordBoundary for each word-start event as they are produced. Either callback
// may be NULL. Blocks the calling thread until the utterance finishes or
// SharpVox_Stop is called from another thread.
SHARPVOX_API SharpVoxStatus SharpVox_Speak(SharpVoxHandle handle,
                                            const char* utf8Text,
                                            SharpVoxAudioCallback onAudio,
                                            SharpVoxWordBoundaryCallback onWordBoundary,
                                            void* userdata);

// Requests that an in-flight SharpVox_Speak (running on another thread) stop
// delivering further audio/word-boundary callbacks. The underlying synthesis call
// still runs to completion internally, but its output is discarded from this point
// on -- there is no true mid-utterance abort in the engine, so callers needing low
// cancel latency should keep utterances reasonably short (e.g. one sentence).
SHARPVOX_API void SharpVox_Stop(SharpVoxHandle handle);

// Commits any parameter changes made via the setters below into the underlying
// synthesis engine. Setters only stage values; call this once after a batch of
// changes and before the next SharpVox_Speak.
SHARPVOX_API void SharpVox_ApplyVoice(SharpVoxHandle handle);

SHARPVOX_API int32_t SharpVox_GetSampleRate(SharpVoxHandle handle);
SHARPVOX_API void SharpVox_SetSampleRate(SharpVoxHandle handle, int32_t sampleRate);

SHARPVOX_API int32_t SharpVox_GetRate(SharpVoxHandle handle);
SHARPVOX_API void SharpVox_SetRate(SharpVoxHandle handle, int32_t rate);

SHARPVOX_API int32_t SharpVox_GetPitchHz(SharpVoxHandle handle);
SHARPVOX_API void SharpVox_SetPitchHz(SharpVoxHandle handle, int32_t pitchHz);

SHARPVOX_API float SharpVox_GetVolume(SharpVoxHandle handle);
SHARPVOX_API void SharpVox_SetVolume(SharpVoxHandle handle, float volume);

SHARPVOX_API SharpVoxPreset SharpVox_GetPreset(SharpVoxHandle handle);
SHARPVOX_API void SharpVox_SetPreset(SharpVoxHandle handle, SharpVoxPreset preset);

SHARPVOX_API int32_t SharpVox_GetFemale(SharpVoxHandle handle);
SHARPVOX_API void SharpVox_SetFemale(SharpVoxHandle handle, int32_t isFemale);

#ifdef __cplusplus
}
#endif

#endif  // SHARPVOX_WIN_H
