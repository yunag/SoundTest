#include "AL/al.h"
#include "AL/alc.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#define alcCall(function, device, ...)                                         \
  alcCallImpl(__FILE__, __LINE__, function, device, __VA_ARGS__)

#define alCall(function, ...)                                                  \
  alCallImpl(__FILE__, __LINE__, function, __VA_ARGS__)

bool check_al_errors(const std::string &filename,
                     const std::uint_fast32_t line) {
  ALenum error = alGetError();
  if (error != AL_NO_ERROR) {
    std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
    switch (error) {
    case AL_INVALID_NAME:
      std::cerr << "AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL "
                   "function";
      break;
    case AL_INVALID_ENUM:
      std::cerr << "AL_INVALID_ENUM: an invalid enum value was passed to an "
                   "OpenAL function";
      break;
    case AL_INVALID_VALUE:
      std::cerr << "AL_INVALID_VALUE: an invalid value was passed to an OpenAL "
                   "function";
      break;
    case AL_INVALID_OPERATION:
      std::cerr << "AL_INVALID_OPERATION: the requested operation is not valid";
      break;
    case AL_OUT_OF_MEMORY:
      std::cerr << "AL_OUT_OF_MEMORY: the requested operation resulted in "
                   "OpenAL running out of memory";
      break;
    default:
      std::cerr << "UNKNOWN AL ERROR: " << error;
    }
    std::cerr << std::endl;
    return false;
  }
  return true;
}

template <typename alFunction, typename... Params>
auto alCallImpl(const char *filename, const std::uint_fast32_t line,
                alFunction function, Params... params) ->
    typename std::enable_if<
        !std::is_same<void, decltype(function(params...))>::value,
        decltype(function(params...))>::type {
  auto ret = function(std::forward<Params>(params)...);
  check_al_errors(filename, line);
  return ret;
}

template <typename alFunction, typename... Params>
auto alCallImpl(const char *filename, const std::uint_fast32_t line,
                alFunction function, Params... params) ->
    typename std::enable_if<
        std::is_same<void, decltype(function(params...))>::value, bool>::type {
  function(std::forward<Params>(params)...);
  return check_al_errors(filename, line);
}

bool check_alc_errors(const std::string &filename,
                      const std::uint_fast32_t line, ALCdevice *device) {
  ALCenum error = alcGetError(device);
  if (error != ALC_NO_ERROR) {
    std::cerr << "***ERROR*** (" << filename << ": " << line << ")\n";
    switch (error) {
    case ALC_INVALID_VALUE:
      std::cerr << "ALC_INVALID_VALUE: an invalid value was passed to an "
                   "OpenAL function";
      break;
    case ALC_INVALID_DEVICE:
      std::cerr << "ALC_INVALID_DEVICE: a bad device was passed to an OpenAL "
                   "function";
      break;
    case ALC_INVALID_CONTEXT:
      std::cerr << "ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL "
                   "function";
      break;
    case ALC_INVALID_ENUM:
      std::cerr << "ALC_INVALID_ENUM: an unknown enum value was passed to an "
                   "OpenAL function";
      break;
    case ALC_OUT_OF_MEMORY:
      std::cerr << "ALC_OUT_OF_MEMORY: an unknown enum value was passed to an "
                   "OpenAL function";
      break;
    default:
      std::cerr << "UNKNOWN ALC ERROR: " << error;
    }
    std::cerr << std::endl;
    return false;
  }
  return true;
}

template <typename alcFunction, typename... Params>
auto alcCallImpl(const char *filename, const std::uint_fast32_t line,
                 alcFunction function, ALCdevice *device, Params... params) ->
    typename std::enable_if<
        std::is_same<void, decltype(function(params...))>::value, bool>::type {
  function(std::forward<Params>(params)...);
  return check_alc_errors(filename, line, device);
}

template <typename alcFunction, typename ReturnType, typename... Params>
auto alcCallImpl(const char *filename, const std::uint_fast32_t line,
                 alcFunction function, ReturnType &returnValue,
                 ALCdevice *device, Params... params) ->
    typename std::enable_if<
        !std::is_same<void, decltype(function(params...))>::value, bool>::type {
  returnValue = function(std::forward<Params>(params)...);
  return check_alc_errors(filename, line, device);
}

bool get_available_devices(std::vector<std::string> &devicesVec,
                           ALCdevice *device) {
  const ALCchar *devices;
  if (!alcCall(alcGetString, devices, device, nullptr, ALC_DEVICE_SPECIFIER))
    return false;

  const char *ptr = devices;

  devicesVec.clear();

  do {
    devicesVec.push_back(std::string(ptr));
    ptr += devicesVec.back().size() + 1;
  } while (*(ptr + 1) != '\0');

  return true;
}

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("usage:\n\t%s <filename>", argv[0]);
    return EXIT_FAILURE;
  }

  char *filename = argv[1];

  ALCdevice *openALDevice = alcOpenDevice(nullptr);
  if (!openALDevice)
    return 0;

  ALCcontext *openALContext;
  if (!alcCall(alcCreateContext, openALContext, openALDevice, openALDevice,
               nullptr) ||
      !openALContext) {
    printf("Error: Could not create audio context");
    return EXIT_FAILURE;
  }

  ALCboolean contextMadeCurrent = false;
  if (!alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice,
               openALContext) ||
      contextMadeCurrent != ALC_TRUE) {
    printf("Error: could not make audio context current");
    return EXIT_FAILURE;
  }

  drwav wav;
  if (!drwav_init_file(&wav, filename, nullptr)) {
    printf("Error: could not load wav at path %s", filename);
    return EXIT_FAILURE;
  }

  uint32_t channels = wav.channels;
  uint32_t sampleRate = wav.sampleRate;
  uint64_t framesOut = wav.totalPCMFrameCount;
  uint8_t bitsPerSample = wav.bitsPerSample;

  ALsizei soundDataSize = framesOut * channels * bitsPerSample / CHAR_BIT;

  printf("Audio info:\n"
         "\tchannels: %u\n"
         "\tsample rate: %u\n"
         "\tPCMFrameCount: %llu\n"
         "\tbps: %hhu\n\n",
         channels, sampleRate, framesOut, bitsPerSample);

  char *pDecodedInterleavedPCMFrames = (char *)malloc(soundDataSize);

  size_t nsamplesDecoded =
      drwav_read_pcm_frames(&wav, framesOut, pDecodedInterleavedPCMFrames);

  ALuint buffer;
  alCall(alGenBuffers, 1, &buffer);

  ALenum format;
  if (channels == 1 && bitsPerSample == 8)
    format = AL_FORMAT_MONO8;
  else if (channels == 1 && bitsPerSample == 16)
    format = AL_FORMAT_MONO16;
  else if (channels == 2 && bitsPerSample == 8)
    format = AL_FORMAT_STEREO8;
  else if (channels == 2 && bitsPerSample == 16)
    format = AL_FORMAT_STEREO16;
  else {
    printf("Error: unrecognised wave format:\n"
           "\tchannels: %u\n"
           "\tbps: %u\n",
           channels, bitsPerSample);
    return EXIT_FAILURE;
  }

  alCall(alBufferData, buffer, format, pDecodedInterleavedPCMFrames,
         soundDataSize, sampleRate);

  free(pDecodedInterleavedPCMFrames);
  drwav_uninit(&wav);

  ALuint source;
  alCall(alGenSources, 1, &source);
  alCall(alSourcef, source, AL_PITCH, 1);
  alCall(alSourcef, source, AL_GAIN, 1.0f);
  alCall(alSource3f, source, AL_POSITION, 0, 0, 0);
  alCall(alSource3f, source, AL_VELOCITY, 0, 0, 0);
  alCall(alSourcei, source, AL_LOOPING, AL_FALSE);
  alCall(alSourcei, source, AL_BUFFER, buffer);

  alCall(alSourcePlay, source);

  ALint state = AL_PLAYING;

  printf("Music is playing!\n");
  while (state == AL_PLAYING) {
    alCall(alGetSourcei, source, AL_SOURCE_STATE, &state);
  }

  alCall(alDeleteSources, 1, &source);
  alCall(alDeleteBuffers, 1, &buffer);

  alcCall(alcMakeContextCurrent, contextMadeCurrent, openALDevice, nullptr);
  alcCall(alcDestroyContext, openALDevice, openALContext);

  ALCboolean closed;
  alcCall(alcCloseDevice, closed, openALDevice, openALDevice);

  return EXIT_SUCCESS;
}
