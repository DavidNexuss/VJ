#pragma once
#include "texture.hpp"
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <vector>

struct Program;

struct UTexture {

  UTexture() {}
  UTexture(GLuint textureID) { this->textureID = textureID; }
  UTexture(GLuint textureID, GLuint textureMode) {
    this->textureID = textureID;
    this->textureMode = textureMode;
  }

  GLuint textureID;
  GLenum textureMode;
};

#define UNIFORMS_LIST(o)                                                       \
  o(VEC2, glm::vec2) o(VEC3, glm::vec3) o(VEC4, glm::vec4) o(MAT2, glm::mat2)  \
      o(MAT3, glm::mat3) o(MAT4, glm::mat4) o(FLOAT, float) o(BOOL, bool)      \
          o(INT, int) o(ITEXTURE, ITexture *) o(VEC3PTR, const glm::vec3 *)    \
              o(INTPTR, int *) o(FLOATPTR, float *)                            \
                  o(VEC2PTR, const glm::vec2 *) o(UTEXTURE, UTexture)

enum UniformType {
#define UNIFORMS_ENUMS_DECLARATION(v, T) v,
  UNIFORMS_LIST(UNIFORMS_ENUMS_DECLARATION)
#undef UNIFORMS_ENUMS_DECLARATION
};

struct Uniform {
  union {
#define UNIFORMS_UNION_DECLARATION(v, T) T v;
    UNIFORMS_LIST(UNIFORMS_UNION_DECLARATION)
#undef UNIFORMS_UNION_DECLARATION
  };

  UniformType type;
  int count = 1;

  Uniform() {}
#define UNIFORMS_CONSTRUCTOR(v, T)                                             \
  Uniform(T _##v) : v(_##v), type(UniformType::v) {}
  UNIFORMS_LIST(UNIFORMS_CONSTRUCTOR)
#undef UNIFORMS_CONSTRUCTOR
#define UNIFORMS_CONSTRUCTOR(v, T)                                             \
  Uniform(T _##v, int _count) : v(_##v), type(UniformType::v), count(_count) {}
  UNIFORMS_LIST(UNIFORMS_CONSTRUCTOR)
#undef UNIFORMS_CONSTRUCTOR

  bool bind(GLuint program, GLuint glUniformID) const;
};

struct Material {

#define UNIFORMS_FUNC_DECLARATION(v, T)                                        \
  inline Uniform &set(const std::string &name, T a) {                          \
    Uniform val;                                                               \
    val.v = a;                                                                 \
    val.type = UniformType::v;                                                 \
    return uniforms[name] = val;                                               \
  }

  UNIFORMS_LIST(UNIFORMS_FUNC_DECLARATION)
#undef UNIFORMS_FUNC_DECLARATION

  // Uniforms collection
  std::unordered_map<std::string, Uniform> uniforms;
  bool has(const std::string &uniformName);

  void setSetupProgram(Program *program);

  void addMaterial(Material *);
  void popNextMaterial();

  // Various hints

  bool hint_isCamera = false;

protected:
  friend Program;
  virtual void bind(Program *program) {}
  std::vector<Material *> childMaterials;

private:
  // This is for cretaing a uniform collection template from a shader
  Program *setupProgram = nullptr;
  int setupProgramCompilationCount = 0;
};
