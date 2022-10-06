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

struct EditorState {
  std::unordered_map<std::string, RenderCamera *> tabs;

  void renderTabs(int frame) {
    for (auto &tab : tabs) {
      const char *name = tab.first.c_str();

      ImGui::Begin(name);

      ImVec2 windowsize = ImGui::GetWindowSize();
      tab.second->setSize(windowsize.x, windowsize.y);
      ImGui::RenderCamera(tab.second->render(frame));
      ImGui::End();
    }
  }
};

EditorState editorState;

void editor::editorRender(int frame) { editorState.renderTabs(frame); }
void editor::editorStep() {}
void editor::enableEditor(bool pEnable) {}

void editor::addEditorTab(RenderCamera *renderTarget, const std::string &name) {
  editorState.tabs[name] = renderTarget;
}
void editor::editorBeginContext() {}
void editor::editorEndContext() {}
void editor::editorInit() {}
