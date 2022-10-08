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

struct RenderTarget {
  RenderCamera *renderCamera;
  RenderShot shot;
};

struct EditorState {
  std::unordered_map<std::string, RenderTarget> tabs;

  void renderTabs(int frame) {
    for (auto &tab : tabs) {
      const char *name = tab.first.c_str();

      ImGui::Begin(name);

      ImVec2 windowsize = ImGui::GetWindowSize();
      tab.second.renderCamera->setSize(windowsize.x, windowsize.y);
      tab.second.shot.currentFrame = frame;
      ImGui::RenderCamera(tab.second.renderCamera->render(tab.second.shot));
      ImGui::End();
    }
  }
};

struct EditorWindow {
  bool enabled = true;
  const char *name;

  virtual void render(int frame) = 0;
  void draw(int frame) {
    if (ImGui::Begin(name, &enabled)) {
      render(frame);
      ImGui::End();
    }
  }
};

struct NodeTreeWindow : public EditorWindow {

  Node *selected_node = nullptr;
  Node *clicked = nullptr;
  int id = 0;
  ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth;

  NodeTreeWindow() { EditorWindow::name = "Nodes"; }

  void recursiveTreeRender(Node *node, int depth = 0) {
    ImGuiTreeNodeFlags node_flags = base_flags;
    if (node == selected_node)
      node_flags |= ImGuiTreeNodeFlags_Selected;

    if (ImGui::TreeNodeEx((void *)(intptr_t)id++, node_flags,
                          node->getName().c_str())) {

      if (ImGui::IsItemClicked() || ImGui::IsItemToggledOpen()) {
        clicked = node;
      }

      for (Node *child : node->children) {
        recursiveTreeRender(child, depth + 1);
      }

      ImGui::TreePop();
    }
  }

  void renderNodeWindow() {
    if (ImGui::Begin("NodeEditor")) {
      ImGui::Text("Editor node: %s \n", selected_node->getName().c_str());
      ImGui::InputFloat3("X: ", &selected_node->transformMatrix[0][0]);
      ImGui::InputFloat3("Y: ", &selected_node->transformMatrix[1][0]);
      ImGui::InputFloat3("Z: ", &selected_node->transformMatrix[2][0]);
      ImGui::InputFloat3("Offset: ", &selected_node->transformMatrix[3][0]);
      ImGui::End();
      selected_node->setDirty();
    }
  }
  void render(int frame) override {
    id = 0;
    recursiveTreeRender(shambhala::getRootNode());

    if (clicked != nullptr) {
      selected_node = clicked;
      clicked = nullptr;
    }

    if (selected_node != nullptr)
      renderNodeWindow();
  }
};

EditorState editorState;
EditorWindow *nodeWindow;

void editor::editorRender(int frame) {
  editorState.renderTabs(frame);
  nodeWindow->draw(frame);
}
void editor::editorStep() {}
void editor::enableEditor(bool pEnable) {}

void editor::addEditorTab(RenderCamera *renderTarget, RenderShot shot,
                          const std::string &name) {
  editorState.tabs[name] = {renderTarget, shot};
}
void editor::editorBeginContext() {}
void editor::editorEndContext() {}
void editor::editorInit() {
  static NodeTreeWindow nodes = NodeTreeWindow{};
  nodeWindow = &nodes;
}
