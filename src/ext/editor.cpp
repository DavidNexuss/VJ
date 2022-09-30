#include "editor.hpp"
#include "ext/worldmat.hpp"
#include <ext.hpp>
#include <imgui/imgui.h>
#include <shambhala.hpp>
#include <standard.hpp>

using namespace shambhala::editor;
using namespace shambhala;

namespace ImGui {
void RenderCamera(RenderCamera *renderCamera) {
  ImVec2 size{float(renderCamera->getWidth()),
              float(renderCamera->getHeight())};
  ImGui::Image((void *)(intptr_t)renderCamera->frameBuffer->colorAttachments[0],
               size);
}
} // namespace ImGui

struct RenderCameraTab {
  worldmats::DebugCamera *debugCamera;
  RenderCamera *camera;

  RenderCameraTab() {}
  RenderCameraTab(RenderCamera *camera) {
    this->camera = camera;
    debugCamera = new worldmats::DebugCamera;
  }

  void render(int frame, bool isRoot = false) const {
    camera->addNextMaterial(debugCamera);
    camera->render(frame, isRoot);
    camera->popNextMaterial();
  }
};

struct EditorState {
  std::unordered_map<std::string, RenderCameraTab> tabs;

  void renderTabs(int frame) {
    for (const auto &tab : tabs) {
      const char *name = tab.first.c_str();
      tab.second.render(frame);

      ImGui::Begin(name);
      ImGui::RenderCamera(tab.second.camera);
      ImGui::End();
    }
  }
};

EditorState editorState;

void editor::editorRender(int frame) { editorState.renderTabs(frame); }
void editor::editorStep() {}
void editor::enableEditor(bool pEnable) {}

void editor::addEditorTab(RenderCamera *renderCamera, const std::string &name) {
  editorState.tabs[name] = RenderCameraTab{renderCamera};
}
void editor::editorBeginContext() {}
void editor::editorEndContext() {}
void editor::editorInit() {}
