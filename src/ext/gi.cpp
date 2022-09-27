#include "gi.hpp"
#include "lightmapper/lightmapper.hpp"
#include "shambhala.hpp"
#include <cstdio>
#include <cstring>
#include <ext.hpp>
#include <iostream>
#include <standard.hpp>

using namespace shambhala;

lm_context *getLightMapperContext() {
  static lm_context *ctx = nullptr;

  if (ctx == nullptr) {
    ctx = lmCreate(64, 0.001f, 100.0f, 1.0f, 1.0f, 1.0f, 2, 0.01f, 0.00f);
  }
  return ctx;
}
Program *getAmbientOcclusionProgram() {

  const char *vp =
      "#version 330  core\n"
      "layout (location = 0) in vec3 aPosition ;\n"
      "layout (location = 2) in vec2 aUV;\n"
      "uniform mat4 uViewMatrix;\n"
      "uniform mat4 uProjectionMatrix;\n"
      "out vec2 v_texcoord;\n"

      "void main()\n"
      "{\n"
      "gl_Position = uProjectionMatrix * (uViewMatrix * vec4(aPosition, 1.0));\n"
      "v_texcoord = aUV;\n"
      "}\n";

  const char *fp = "#version 330  core\n"
                   "in vec2 v_texcoord;\n"
                   "uniform sampler2D uLightmap;\n"
                   "out vec4 o_color;\n"

                   "void main()\n"
                   "{\n"
                   "o_color = vec4(texture(uLightmap, v_texcoord).rgb, "
                   "gl_FrontFacing ? 1.0 : 0.0);\n"
                   "}\n";

  static Program *ao = nullptr;
  if (ao == nullptr) {
    ao = shambhala::createProgram();
    ao->shaders[FRAGMENT_SHADER].file =
        resource::createFromNullTerminatedString(fp, "internal:ao_bake.fs");
    ao->shaders[VERTEX_SHADER].file =
        resource::createFromNullTerminatedString(vp, "internal:ao_bake.vs");
  }
  return ao;
}
Texture *gi::bakeAmbientOcclusion(ModelList *modelList, int size, int bounces) {
  static glm::mat4 originTransform = glm::mat4(1.0);

  lm_context *ctx = getLightMapperContext();
  Program *aoProgram = getAmbientOcclusionProgram();
  Texture *lightmapResult = shambhala::createTexture();
  TextureResource *lightmapResource = new TextureResource;
  lightmapResult->addTextureResource(lightmapResource);

  const static int components = 4;
  int imageSize = size * size * components;

  float *lightmap = new float[imageSize];
  float *tmp = new float[imageSize];

  lightmapResource->textureBuffer = (uint8_t *)lightmap;
  lightmapResource->width = 1;
  lightmapResource->height = 1;
  lightmapResource->components = components;
  lightmapResource->hdrSpace = false;

  static unsigned char emissive[] = {0, 0, 0, 255};
  lightmapResource->textureBuffer = emissive;

  struct AOBinding {
    short positionAttribute;
    short normalAttribute;
    short uvAttribute;

    short positionOffset;
    short normalOffset;
    short uvOffset;
  };

  AOBinding bindings[modelList->models.size()];
  for (int i = 0; i < modelList->size(); i++) {
    VertexBuffer *vbo = modelList->get(i)->mesh->vbo;
    int offset = 0;
    for (int j = 0; j < vbo->attributes.size(); j++) {
      if (vbo->attributes[j].index == Standard::aPosition) {
        bindings[i].positionAttribute = j;
        bindings[i].positionOffset = offset;
      }
      if (vbo->attributes[j].index == Standard::aNormal) {
        bindings[i].normalAttribute = j;
        bindings[i].normalOffset = offset;
      }
      if (vbo->attributes[j].index == Standard::aUV) {
        bindings[i].uvAttribute = j;
        bindings[i].uvOffset = offset;
      }

      offset += vbo->attributes[j].size;
    }
  }

  for (int i = 0; i < bounces; i++) {

    memset(lightmap, 0, imageSize * sizeof(float));
    lmSetTargetLightmap(ctx, lightmap, size, size, components);

    for (int j = 0; j < modelList->size(); j++) {

      Model *model = modelList->get(j);
      VertexBuffer *vbo = model->mesh->vbo;
      IndexBuffer *ebo = model->mesh->ebo;

      float *transform = (float *)&model->node->getTransformMatrix();
      int pOffset = sizeof(float) * bindings[j].positionOffset;
      int nOffset = sizeof(float) * bindings[j].normalOffset;
      int uvOffset = sizeof(float) * bindings[j].uvOffset;

      dprintf(2, "Position attr %d, Normal attr %d, uv attr %d\n", bindings[j].positionAttribute , bindings[j].normalAttribute, bindings[j].uvAttribute);
      dprintf(2, "Position offset %d, Normal offset %d, uv Offset %d\n", pOffset,nOffset, uvOffset);
      dprintf(2, "Vertexbuffer size %d\n", int(vbo->vertexBuffer.size()));

      uint8_t *position = vbo->vertexBuffer.bindata() + pOffset;
      uint8_t *normal = vbo->vertexBuffer.bindata() + nOffset;
      uint8_t *uv = vbo->vertexBuffer.bindata() + uvOffset;

      int stride = vbo->vertexSize();

      lmSetGeometry(ctx, transform, LM_FLOAT, position, stride,
                    LM_FLOAT, normal, stride, LM_FLOAT, uv, stride,
                    ebo->indexBuffer.size(), LM_UNSIGNED_SHORT,
                    ebo->indexBuffer.bindata());

      int vp[4];
      glm::mat4 view, proj;
      int frame = 0;
      while (lmBegin(ctx, vp, &view[0][0], &proj[0][0])) {
        // RENDER LOOP
        glViewport(vp[0], vp[1], vp[2], vp[3]);
        shambhala::engine_clearState();
        shambhala::engine_prepareRender();
        shambhala::device::useProgram(aoProgram);
        shambhala::device::useMesh(model->mesh);
        shambhala::device::useUniform(Standard::uLightmap,
                                      DynamicTexture{lightmapResult});
        shambhala::device::useUniform(Standard::uProjectionMatrix, proj);
        shambhala::device::useUniform(Standard::uViewMatrix, view);
        shambhala::device::drawCall();
        frame++;
        lmEnd(ctx);
      }
    }

    lmImageDilate(lightmap, tmp, size, size, components);
    lmImageDilate(tmp, lightmap, size, size, components);
    lightmapResource->forceUpdate();
    lightmapResource->textureBuffer = (uint8_t *)lightmap;
    lightmapResource->width = size;
    lightmapResource->height = size;
    lightmapResource->hdrSpace = true;
  }

  lmImagePower(lightmap, size, size, 4, 1.0f / 2.2f);
  lmImageSaveTGAf("test.tga", lightmap, size, size, 4);

  delete[] tmp;
  return lightmapResult;
}
