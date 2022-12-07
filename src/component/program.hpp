#include "core/resource.hpp"
using SHuint = unsigned int;
using SHenum = unsigned int;
using GLint = int;

enum ShaderType {
  FRAGMENT_SHADER = 0,
  VERTEX_SHADER,
  GEOMETRY_SHADER,
  TESS_EVALUATION_SHADER,
  TESS_CONTROL_SHADER,
  SHADER_TYPE_COUNT
};

struct Program;
struct Shader {
  ResourceHandler file;
  bool use(GLint type);

private:
  SHuint gl_shader = -1;
  friend Program;
};

struct Program {
  Shader *shaders[SHADER_TYPE_COUNT] = {0};
  bool hint_skybox = false;

  void use();
  void bind(Material *material);
  void bind(const char *uniformName, Uniform value);

  inline int getCompilationCount() const { return compilationCount; }
  SHuint gl();

private:
  int compilationCount = 0;
  bool errored = false;
  SHuint gl_shaderProgram = -1;
};
