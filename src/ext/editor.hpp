#include <string>
namespace editor {
void addEditorTab(RenderCamera *renderCamera, RenderShot shot,
                  const std::string &name);
void enableEditor(bool pEnable);
void editorStep();
void editorRender(int frame);

void editorBeginContext();
void editorEndContext();
void editorInit();
} // namespace editor

namespace gui {
void renderCamera(RenderCamera *renderCamera);
void texture(Texture *texture, glm::vec2 uv, glm::vec2 uv2, glm::vec2 size);
void texture(GLuint, glm::vec2 uv, glm::vec2 uv2, glm::vec2 size);
void textEditor(ResourceHandler handler, const char *windowName);
void materialEditor(Material *material);
int selectableList(simple_vector<std::string> &list, int last_selected);
} // namespace gui
