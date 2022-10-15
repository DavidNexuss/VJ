#include "editor.hpp"
#include "ext/worldmat.hpp"
#include <cstring>
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

int tree(int id, const char *name) {
  static ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth;
  ImGui::TreeNodeEx((void *)intptr_t(id), base_flags, name);
  return id + 1;
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

io_buffer toIoBuffer(const std::string &text) {
  io_buffer buffer;
  buffer.data = new uint8_t[text.size()];
  buffer.length = text.size() + 1;
  mempcpy(buffer.data, &text[0], text.size());
  buffer.data[text.size()] = 0;
  return buffer;
}
#include <imguiText/TextEditor.h>
static void editResource(IResource *resource) {
  static TextEditor editor;
  static IResource *currentResource = nullptr;

  if (currentResource != resource) {
    editor.SetText((const char *)resource->read()->data);
    currentResource = resource;
    editor.SetReadOnly(resource->readOnly);
    editor.SetShowWhitespaces(false);
  }

  ImGui::Begin("ShaderEditor", nullptr,
               ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);

  ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
  if (ImGui::BeginMenuBar()) {
    if (ImGui::BeginMenu("File")) {
      if (ImGui::MenuItem("Save")) {
        io_buffer *buffer = resource->read();
        delete[] buffer->data;
        *buffer = toIoBuffer(editor.GetText());
        resource->needsUpdate = true;
      }
      if (ImGui::MenuItem("Quit", "Alt-F4"))
        return;
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Edit")) {
      bool ro = editor.IsReadOnly();
      if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
        editor.SetReadOnly(ro);
      ImGui::Separator();

      if (ImGui::MenuItem("Undo", "ALT-Backspace", nullptr,
                          !ro && editor.CanUndo()))
        editor.Undo();
      if (ImGui::MenuItem("Redo", "Ctrl-Y", nullptr, !ro && editor.CanRedo()))
        editor.Redo();

      ImGui::Separator();

      if (ImGui::MenuItem("Copy", "Ctrl-C", nullptr, editor.HasSelection()))
        editor.Copy();
      if (ImGui::MenuItem("Cut", "Ctrl-X", nullptr,
                          !ro && editor.HasSelection()))
        editor.Cut();
      if (ImGui::MenuItem("Delete", "Del", nullptr,
                          !ro && editor.HasSelection()))
        editor.Delete();
      if (ImGui::MenuItem("Paste", "Ctrl-V", nullptr,
                          !ro && ImGui::GetClipboardText() != nullptr))
        editor.Paste();

      ImGui::Separator();

      if (ImGui::MenuItem("Select all", nullptr, nullptr))
        editor.SetSelection(TextEditor::Coordinates(),
                            TextEditor::Coordinates(editor.GetTotalLines(), 0));

      ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("View")) {
      if (ImGui::MenuItem("Dark palette"))
        editor.SetPalette(TextEditor::GetDarkPalette());
      if (ImGui::MenuItem("Light palette"))
        editor.SetPalette(TextEditor::GetLightPalette());
      if (ImGui::MenuItem("Retro blue palette"))
        editor.SetPalette(TextEditor::GetRetroBluePalette());
      ImGui::EndMenu();
    }
    ImGui::EndMenuBar();
  }
  auto cpos = editor.GetCursorPosition();
  ImGui::Text("%6d/%-6d %6d lines  | %s | %s | %s | %s", cpos.mLine + 1,
              cpos.mColumn + 1, editor.GetTotalLines(),
              editor.IsOverwrite() ? "Ovr" : "Ins",
              editor.CanUndo() ? "*" : " ",
              editor.GetLanguageDefinition().mName.c_str(),
              resource->resourcename.c_str());

  editor.Render(resource->resourcename.c_str());
  ImGui::End();
}
struct ProgramWindow : public EditorWindow {

  Shader *selectedShader = nullptr;
  Program *selectedProgram = nullptr;
  ImGuiTreeNodeFlags base_flags = ImGuiTreeNodeFlags_SpanAvailWidth;
  ProgramWindow() { EditorWindow::name = "Programs"; }
  void render(int frame) override {
    int n = loader::programCount();
    int id = 0;
    for (int i = 0; i < n; i++) {
      Program *current = loader::getProgram(i);
      Shader *fs = current->shaders[FRAGMENT_SHADER];
      std::string name;
      if (fs != nullptr) {
        name = fs->file->resourcename;
      }
      std::string base_filename = name.substr(name.find_last_of("/\\") + 1);

      ImGuiTreeNodeFlags node_flags = base_flags;
      if (selectedProgram == current) {
        node_flags |= ImGuiTreeNodeFlags_Selected;
      }
      if (ImGui::TreeNodeEx((void *)intptr_t(id++), node_flags,
                            base_filename.c_str())) {
        for (int i = 0; i < SHADER_TYPE_COUNT; i++) {
          node_flags = base_flags;
          Shader *sh = current->shaders[i];
          if (sh != nullptr) {
            node_flags |= ImGuiTreeNodeFlags_Leaf;
            if (sh == selectedShader)
              node_flags |= ImGuiTreeNodeFlags_Selected;
            if (ImGui::TreeNodeEx((void *)intptr_t(id++), node_flags,
                                  sh->file->resourcename.c_str())) {

              if (ImGui::IsItemClicked() || ImGui::IsItemToggledOpen()) {
                selectedShader = sh;
                selectedProgram = current;
              }
              ImGui::TreePop();
            }
          }
        }
        ImGui::TreePop();
      }
    }

    if (selectedShader != nullptr) {
      editResource(selectedShader->file);
    }
  }
};

static EditorState editorState;
static EditorWindow *nodeWindow;
static EditorWindow *componentWindow;
static EditorWindow *programWindow;
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
  gui::toggleButton("Progs", &programWindow->enabled);
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
  programWindow->draw(frame);
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

static void setupTheme();
void editor::editorBeginContext() {

  static bool firstInit = true;
  if (firstInit) {
    setupTheme();
    firstInit = false;
  }
}
void editor::editorEndContext() {}
void editor::editorInit() {
  static NodeTreeWindow nodes = NodeTreeWindow{};
  static ComponentWindow components = ComponentWindow{};
  static ProgramWindow programs = ProgramWindow{};
  nodeWindow = &nodes;
  componentWindow = &components;
  programWindow = &programs;
  setupTheme();
}

static void setupTheme() {

  ImGuiStyle *style = &ImGui::GetStyle();
  ImVec4 *colors = style->Colors;
  /*
   colors[ImGuiCol_Text] = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
   colors[ImGuiCol_TextDisabled] = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
   colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
   colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
   colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
   colors[ImGuiCol_Border] = ImVec4(0.78f, 0.52f, 0.01f, 1.00f);
   colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
   colors[ImGuiCol_FrameBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
   colors[ImGuiCol_FrameBgHovered] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
   colors[ImGuiCol_FrameBgActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
   colors[ImGuiCol_TitleBg] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
   colors[ImGuiCol_TitleBgActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
   colors[ImGuiCol_MenuBarBg] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
   colors[ImGuiCol_ScrollbarBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
   colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
   colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
   colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
   colors[ImGuiCol_SliderGrab] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_SliderGrabActive] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_Button] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
   colors[ImGuiCol_ButtonHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_ButtonActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
   colors[ImGuiCol_Header] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
   colors[ImGuiCol_HeaderHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_HeaderActive] = ImVec4(0.93f, 0.65f, 0.14f, 1.00f);
   colors[ImGuiCol_Separator] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
   colors[ImGuiCol_SeparatorHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_SeparatorActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
   colors[ImGuiCol_ResizeGrip] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
   colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_ResizeGripActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
   colors[ImGuiCol_Tab] = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
   colors[ImGuiCol_TabHovered] = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
   colors[ImGuiCol_TabActive] = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
   colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
   colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
   colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
   colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
   colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
   colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
   colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
   colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
   colors[ImGuiCol_NavHighlight] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
   colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
   colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
   colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
  */
  style->FramePadding = ImVec2(4, 2);
  style->ItemSpacing = ImVec2(10, 2);
  style->IndentSpacing = 12;
  style->ScrollbarSize = 10;

  style->WindowRounding = 0;
  style->FrameRounding = 0;
  style->PopupRounding = 0;
  style->ScrollbarRounding = 0;
  style->GrabRounding = 0;
  style->TabRounding = 0;

  style->WindowTitleAlign = ImVec2(1.0f, 0.5f);
  style->WindowMenuButtonPosition = ImGuiDir_Right;

  style->DisplaySafeAreaPadding = ImVec2(4, 4);
  ImGuiIO &io = ImGui::GetIO();
  io.ConfigWindowsMoveFromTitleBarOnly = false;

  io.Fonts->AddFontFromFileTTF("internal_assets/fonts/editor.ttf", 15);
}
