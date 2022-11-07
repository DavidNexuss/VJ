#include "shambhala.hpp"

struct FinalBoss : public shambhala::Model, public shambhala::LogicComponent {

  FinalBoss();

  void step(shambhala::StepInfo info) override;
  void draw() override;

private:
  float globalStep = 0.0f;
  struct Path {
    glm::vec2 position;
    glm::vec2 velocity;
    float t;
    int index;

    glm::mat4 getTransform();

    static Path interpolate(Path start, Path end, float t);
  };

  struct FinalBossPart {
    float step;
    float health;
  };

  struct ST {
    glm::vec2 offset;
    glm::vec2 scale;
  };

  ST getST(Path path);
  Path getPath(float t);

  shambhala::Node *rootNode;
  shambhala::Texture *texture;
  shambhala::Program *program;
  shambhala::Mesh *mesh;
  simple_vector<FinalBossPart> parts;
};
