#include <shambhala.hpp>
#include <string>
namespace shambhala {
namespace editor {
void addEditorTab(RenderCamera *renderCamera, const std::string &name);
void enableEditor(bool pEnable);
void editorStep();
void editorRender(int frame);

void editorBeginContext();
void editorEndContext();
void editorInit();
} // namespace editor
} // namespace shambhala
