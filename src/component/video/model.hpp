#pragma once
#include "material.hpp"
#include "mesh.hpp"
#include "node.hpp"
#include "program.hpp"
#include <adapters/video.hpp>
#include <vector>

struct ModelConfiguration {

  bool depthMask = false;
  bool cullFrontFace = false;
  GLuint renderMode = GL_TRIANGLES;
  GLuint polygonMode = GL_FILL;
  int pointSize = 5;
  int lineWidth = 5;

  uint32_t skipRenderMask = 0;
};

struct Model : public ModelConfiguration,
               public shambhala::video::DrawCallArgs {
  Program *program = nullptr;
  Mesh *mesh = nullptr;
  Material *material = nullptr;
  Node *node = nullptr;
  int zIndex = 0;

  int hint_class = 0;
  bool hint_raycast = false;
  bool hint_editor = false;
  bool hint_selectionpass = false;
  int hint_modelid = 0;
  Material *hint_selection_material = nullptr;

  bool operator<(const Model &model) const;

  virtual void draw();
  bool ready() const;
  bool isEnabled();
  Node *getNode();
  void setNode(Node *node);
};
