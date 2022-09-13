#pragma once
#include <core/core.hpp>
#include <iostream>

/**
 * @class Debug
 * @brief Serves as global interface for collecting debug data generated for
 * every frame and displaying them
 */
class Debug {
public:
  static int materialSwaps;
  static int materialInstanceSwaps;
  static int meshSwaps;
  static int textureSwaps;
  static int uniformsFlush;
  static int lightFlush;

  static int missingUniforms;
  static float lastTime;
  static float currentTime;

  static int watchesAdded;
  static int watchesRemoved;

  inline static void reset(float time) {
    materialSwaps = 0;
    materialInstanceSwaps = 0;
    meshSwaps = 0;
    textureSwaps = 0;
    uniformsFlush = 0;
    lightFlush = 0;
    lastTime = currentTime;
    currentTime = time;
  }

  inline static void print(int verbose_level = 0) {

    std::cerr << "Frame debug information: " << std::endl;
    std::cerr << "Frame rate: " << (1.0f / (currentTime - lastTime))
              << std::endl;
    std::cerr << "Material swaps :" << materialSwaps << std::endl;
    std::cerr << "Matrial instance swap: " << materialInstanceSwaps
              << std::endl;
    std::cerr << "Uniform flushs: " << uniformsFlush << std::endl;
    std::cerr << "Mesh swaps :" << meshSwaps << std::endl;
    std::cerr << "Texture swaps :" << textureSwaps << std::endl;
    std::cerr << "Light flush :" << lightFlush << std::endl;
    std::cerr << "----" << std::endl;
    std::cerr << "Missing uniforms: " << missingUniforms << std::endl;
    std::cerr << "----" << std::endl;

    if (verbose_level > 0) {
      std::cerr << "Watches added " << watchesAdded << std::endl;
      std::cerr << "Watches removed " << watchesRemoved << std::endl;
      std::cerr << "Total Watches: " << (watchesAdded - watchesRemoved)
                << std::endl;
    }
  }

  static void glError(GLenum source, GLenum type, GLuint id, GLenum severity,
                      GLsizei length, const GLchar *message,
                      const void *userParam);

  static int getFrameRate() { return int(1.0f / (currentTime - lastTime)); }
};

// Debug macros, disable debug logging in non debug builds
/*
#ifndef NDEBUG
#ifndef DEBUG
#define DEBUG
#endif
#endif
*/

// Register frame metadata macros
#ifdef DEBUG
#define REGISTER_MISSED_UNIFORM() Debug::missingUniforms++
#define REGISTER_FRAME(time) Debug::reset(time)
#define REGISTER_MATERIAL_SWAP() Debug::materialSwaps++
#define REGISTER_MESH_SWAP() Debug::meshSwaps++
#define REGISTER_TEXTURE_SWAP() Debug::textureSwaps++
#define REGISTER_MATERIAL_INSTANCE_SWAP() Debug::materialInstanceSwaps++
#define REGISTER_UNIFORM_FLUSH() Debug::uniformsFlush++
#define REGISTER_LIGHT_FLUSH() Debug::lightFlush++
#define REGISTER_WATCH_ADDITION() Debug::watchesAdded++
#define REGISTER_WATCH_REMOVAL() Debug::watchesRemoved++
#define LOG_FRAME() Debug::print()
#else
#define REGISTER_MISSED_UNIFORM()
#define REGISTER_FRAME(time)
#define REGISTER_MATERIAL_SWAP()
#define REGISTER_MESH_SWAP()
#define REGISTER_TEXTURE_SWAP()
#define REGISTER_MATERIAL_INSTANCE_SWAP()
#define REGISTER_UNIFORM_FLUSH()
#define REGISTER_LIGHT_FLUSH()
#define REGISTER_WATCH_ADDITION()
#define REGISTER_WATCH_REMOVAL()
#define LOG_FRAME()
#endif

// Debug macros

#ifdef DEBUG
#define ENGINE_ASSERT(x)                                                       \
  do {                                                                         \
    if (!(x)) {                                                                \
      std::cerr << __FILE__ << ": " << __LINE__                                \
                << " -> Assetion failed : " << #x << std::endl                 \
                << std::flush;                                                 \
      exit(1);                                                                 \
    }                                                                          \
  } while (0)
#else
#define ENGINE_ASSERT(x)
#endif
