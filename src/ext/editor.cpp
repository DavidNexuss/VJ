#include "editor.hpp"
#include "ext/worldmat.hpp"
#include <ext.hpp>
#include <imgui/imgui.h>
#include <shambhala.hpp>
#include <standard.hpp>

using namespace shambhala::editor;
using namespace shambhala;

namespace shambhala {
namespace gui {

ImVec2 vec2(glm::vec2 v) { return ImVec2{v.x, v.y}; }

void renderCamera(RenderCamera *renderCamera) {
  ImVec2 size{float(renderCamera->getWidth()),
              float(renderCamera->getHeight())};
  ImGui::Image((void *)(intptr_t)renderCamera->frameBuffer->colorAttachments[0],
               size);
}

void texture(Texture *texture, glm::vec2 uv, glm::vec2 uv2, glm::vec2 size) {
  auto id = (void *)(intptr_t)texture->_textureID;
  std::swap(uv.y, uv2.y);
  ImGui::Image(id, vec2(size), vec2(uv), vec2(uv2));
}

void toggleButton(const char *name, bool *enable) {
  static float b = 1.0f;
  static float c = 0.5f;
  static int i = 3;

  if (*enable) {
    ImGui::PushID(name);
    ImColor color;
    color.SetHSV(2.8f, 0.8f, 0.6);
    ImGui::PushStyleColor(ImGuiCol_Button, color.Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color.Value);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color.Value);
    ImGui::Button(name);
    if (ImGui::IsItemClicked(0)) {
      *enable = !*enable;
    }
    ImGui::PopStyleColor(3);
    ImGui::PopID();
  } else if (ImGui::Button(name)) {
    *enable = true;
  }
}

} // namespace gui
} // namespace shambhala

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
      gui::renderCamera(tab.second.renderCamera->render(tab.second.shot));
      ImGui::End();
    }
  }
};

struct EditorWindow {
  bool enabled = true;
  const char *name;

  virtual void render(int frame) = 0;
  void draw(int frame) {
    if (enabled) {
      if (ImGui::Begin(name, &enabled)) {
        render(frame);
        ImGui::End();
      }
    }
  }
};

struct ComponentWindow : public EditorWindow {
  LogicComponent *selected = nullptr;
  ImGuiTreeNodeFlags base_flags =
      ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_Leaf;

  ComponentWindow() { EditorWindow::name = "Components"; }
  void renderComponentWindow(LogicComponent *selected) {
    selected->editorStep(shambhala::getStepInfo());
    selected->editorRender();
  }
  void renderWindow() {
    int id = 0;
    for (int i = 0; i < shambhala::componentCount(); i++) {
      ImGuiTreeNodeFlags node_flags = base_flags;
      LogicComponent *comp = shambhala::getComponent(i);
      if (comp == selected) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }

      ImGui::TreeNodeEx((void *)(intptr_t)id++, node_flags,
                        comp->getName().c_str());

      if (ImGui::IsItemClicked() || ImGui::IsItemToggledOpen()) {
        if (selected == comp) {
          selected = nullptr;
        } else
          selected = comp;
      }
      ImGui::TreePop();
    }
  }

  void render(int frame) override {
    renderWindow();
    if (selected != nullptr) {
      renderComponentWindow(selected);
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
      ImGui::Checkbox("Enabled", &selected_node->enabled);
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

static EditorState editorState;
static EditorWindow *nodeWindow;
static EditorWindow *componentWindow;
static int toolbarSize = 30;
static void dockspace() {
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(
      ImVec2(viewport->Pos.x, toolbarSize + viewport->Pos.y));
  ImGui::SetNextWindowSize(
      ImVec2(-viewport->Size.x, viewport->Size.y - toolbarSize));
  ImGuiWindowFlags window_flags =
      0 | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar |
      ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBringToFrontOnFocus |
      ImGuiWindowFlags_NoNavFocus;

  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
  ImGui::Begin("Master DockSpace", NULL, window_flags);
  ImGuiID dockMain = ImGui::GetID("MyDockspace");

  // Save off menu bar height for later.

  gui::toggleButton("Nodes", &nodeWindow->enabled);
  gui::toggleButton("Comps", &componentWindow->enabled);
  ImGui::End();
  ImGui::PopStyleVar(3);
}
static void toolbar() {
  ImGuiViewport *viewport = ImGui::GetMainViewport();
  ImGui::SetNextWindowPos(ImVec2(viewport->Pos.x, viewport->Pos.y));
  ImGui::SetNextWindowSize(ImVec2(viewport->Size.x, toolbarSize));

  ImGuiWindowFlags window_flags =
      0 | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
      ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
      ImGuiWindowFlags_NoSavedSettings;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0);
  ImGui::Begin("TOOLBAR", NULL, window_flags);
  ImGui::PopStyleVar();

  ImGui::End();
}

void editor::editorRender(int frame) {
  editorState.renderTabs(frame);
  nodeWindow->draw(frame);
  componentWindow->draw(frame);

  dockspace();
  toolbar();

  ImGuiIO &io = ImGui::GetIO();
  viewport()->enableInput(!io.WantCaptureMouse);
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
  static ComponentWindow components = ComponentWindow{};
  nodeWindow = &nodes;
  componentWindow = &components;
}
