#include "editor.hpp"
#include <ext.hpp>
#include <imgui/imgui.h>
#include <shambhala.hpp>
#include <standard.hpp>

using namespace shambhala::editor;
using namespace shambhala;

struct MaterialRenderCamera : public RenderCamera {
  MaterialRenderCamera(RenderCamera *child) { addDummyInput(child); }
  void render(int frame, bool isRoot = false) override {
    getDummyInput(0)->beginRender(frame, isRoot);
    shambhala::setWorldMaterial(Standard::clas_worldMatRenderCamera, this);
    getDummyInput(0)->endRender(frame, isRoot);
  }
};

struct EditorRenderCamera : public MaterialRenderCamera {
  worldmats::DebugCamera *editorCamera;
  EditorRenderCamera(RenderCamera *child) : MaterialRenderCamera(child) {
    editorCamera = new worldmats::DebugCamera;
    addNextMaterial(editorCamera);
  }
};

namespace ImGui {
void RenderCamera(RenderCamera *renderCamera) {
  ImVec2 size{float(renderCamera->getWidth()),
              float(renderCamera->getHeight())};
  ImGui::Image((void *)(intptr_t)renderCamera->frameBuffer->colorAttachments[0],
               size);
}
} // namespace ImGui

struct EditorState {
  std::unordered_map<std::string, RenderCamera *> tabs;

  void renderTabs(int frame) {
    for (const auto &tab : tabs) {
      const char *name = tab.first.c_str();
      RenderCamera *renderCamera = tab.second;
      renderCamera->render(frame);

      ImGui::Begin(name);
      ImGui::RenderCamera(renderCamera->getDummyInput(0));
      ImGui::End();
    }
  }
};

EditorState editorState;

void editor::editorRender(int frame) { editorState.renderTabs(frame); }
void editor::editorStep() {}
void editor::enableEditor(bool pEnable) {}

void editor::addEditorTab(RenderCamera *renderCamera, const std::string &name) {
  editorState.tabs[name] = new EditorRenderCamera(renderCamera);
}
void editor::editorBeginContext() {}
void editor::editorEndContext() {}
void editor::editorInit() {}
