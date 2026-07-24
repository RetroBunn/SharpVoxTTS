#include "sharpvox_win.h"

#include <atomic>

#include "../lib/SharpVox.h"

using SharpVox::PhonemeEvent;
using SharpVox::SharpVoxSpeaker;
using SharpVox::VoicePreset;

struct SharpVoxHandle_ {
    SharpVoxSpeaker speaker;
    std::atomic<bool> stopRequested{false};
};

namespace {

struct SpeakCtx {
    SharpVoxHandle_* inst;
    SharpVoxAudioCallback onAudio;
    SharpVoxWordBoundaryCallback onWordBoundary;
    void* userdata;
};

void ChunkTrampoline(SharpVoxSpeaker* /*speaker*/, const int16_t* buf, int32_t len,
                     const PhonemeEvent* events, int32_t count, void* ud) {
    auto* ctx = static_cast<SpeakCtx*>(ud);
    if (ctx->inst->stopRequested.load(std::memory_order_relaxed)) {
        return;
    }
    if (ctx->onAudio && len > 0) {
        ctx->onAudio(buf, len, ctx->userdata);
    }
    if (ctx->onWordBoundary) {
        for (int32_t i = 0; i < count; ++i) {
            if (events[i].IsWordStart) {
                ctx->onWordBoundary(events[i].TimeSeconds, ctx->userdata);
            }
        }
    }
}

}  // namespace

extern "C" {

SharpVoxHandle SharpVox_Create(void) {
    try {
        return new SharpVoxHandle_();
    } catch (...) {
        return nullptr;
    }
}

void SharpVox_Destroy(SharpVoxHandle handle) {
    delete handle;
}

SharpVoxStatus SharpVox_Speak(SharpVoxHandle handle, const char* utf8Text,
                              SharpVoxAudioCallback onAudio,
                              SharpVoxWordBoundaryCallback onWordBoundary,
                              void* userdata) {
    if (!handle) return SHARPVOX_ERR_NULL_HANDLE;
    if (!utf8Text) return SHARPVOX_ERR_NULL_TEXT;

    handle->stopRequested.store(false, std::memory_order_relaxed);
    try {
        SpeakCtx ctx{handle, onAudio, onWordBoundary, userdata};
        handle->speaker.SpeakWithEvents(utf8Text, ChunkTrampoline, &ctx);
        return SHARPVOX_OK;
    } catch (...) {
        return SHARPVOX_ERR_EXCEPTION;
    }
}

void SharpVox_Stop(SharpVoxHandle handle) {
    if (!handle) return;
    handle->stopRequested.store(true, std::memory_order_relaxed);
}

void SharpVox_ApplyVoice(SharpVoxHandle handle) {
    if (!handle) return;
    try {
        handle->speaker.ApplyVoiceInPlace();
    } catch (...) {
    }
}

int32_t SharpVox_GetSampleRate(SharpVoxHandle handle) {
    return handle ? handle->speaker.SampleRate : 0;
}
void SharpVox_SetSampleRate(SharpVoxHandle handle, int32_t sampleRate) {
    if (handle) handle->speaker.SampleRate = sampleRate;
}

int32_t SharpVox_GetRate(SharpVoxHandle handle) {
    return handle ? handle->speaker.Rate : 0;
}
void SharpVox_SetRate(SharpVoxHandle handle, int32_t rate) {
    if (handle) handle->speaker.Rate = rate;
}

int32_t SharpVox_GetPitchHz(SharpVoxHandle handle) {
    return handle ? handle->speaker.PitchHz : 0;
}
void SharpVox_SetPitchHz(SharpVoxHandle handle, int32_t pitchHz) {
    if (handle) handle->speaker.PitchHz = pitchHz;
}

float SharpVox_GetVolume(SharpVoxHandle handle) {
    return handle ? handle->speaker.AudioVolume : 0.0f;
}
void SharpVox_SetVolume(SharpVoxHandle handle, float volume) {
    if (handle) handle->speaker.AudioVolume = volume;
}

SharpVoxPreset SharpVox_GetPreset(SharpVoxHandle handle) {
    if (!handle) return SHARPVOX_PRESET_BASELINE;
    return static_cast<SharpVoxPreset>(handle->speaker.GetPreset());
}
void SharpVox_SetPreset(SharpVoxHandle handle, SharpVoxPreset preset) {
    if (!handle) return;
    handle->speaker.SetPreset(static_cast<VoicePreset>(preset));
}

int32_t SharpVox_GetFemale(SharpVoxHandle handle) {
    return handle && handle->speaker.GetFemale() ? 1 : 0;
}
void SharpVox_SetFemale(SharpVoxHandle handle, int32_t isFemale) {
    if (handle) handle->speaker.SetFemale(isFemale != 0);
}

}  // extern "C"
