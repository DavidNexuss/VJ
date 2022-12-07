#include "material.hpp"
#include <adapters/video.hpp>
using namespace shambhala;
bool Uniform::bind(GLuint program, GLuint glUniformID) const {

  // TODO: This switch should be deleted...
  switch (type) {
  case UniformType::VEC2:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC2,
                       (void *)&VEC2[0]);
    break;
  case UniformType::VEC3:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)&VEC3[0]);
    break;
  case UniformType::VEC2PTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)VEC2PTR, count);
    break;
  case UniformType::FLOATPTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_FLOAT,
                       (void *)FLOATPTR, count);
    break;
  case UniformType::VEC3PTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC3,
                       (void *)VEC3PTR, count);
    break;
  case UniformType::VEC4:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_VEC4,
                       (void *)&VEC4[0]);
    break;
  case UniformType::MAT2:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_MAT2,
                       (void *)&MAT2[0][0]);
    break;
  case UniformType::MAT3:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_MAT3,
                       (void *)&MAT3[0][0]);
    break;
  case UniformType::MAT4:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_MAT4,
                       (void *)&MAT4[0][0]);
    break;
  case UniformType::FLOAT:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_FLOAT,
                       (void *)&FLOAT);
    break;
  case UniformType::BOOL:
  case UniformType::INT:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_INT,
                       (void *)&INT);
    break;
  case UniformType::INTPTR:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_INT, INTPTR,
                       count);
    break;
  case UniformType::UTEXTURE:
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_SAMPLER,
                       (void *)&UTEXTURE.textureID);
    break;
  case UniformType::ITEXTURE:
    GLuint textureID = ITEXTURE->gl();
    video::bindUniform(program, glUniformID, video::SH_UNIFORM_SAMPLER,
                       &textureID);
    break;
  }

  return true;
}
