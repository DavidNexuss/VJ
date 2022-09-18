#include "shambhala.hpp"
#include <assimp/scene.h>
#include <string>

namespace shambhala {
struct SceneLoaderConfiguration {
  int assimpFlags;
  vector<VertexAttribute> attributes;
  vector<aiTextureType> textureTypes;
  vector<std::string> textureNames;

  bool materialInstanceDiffuseCoef;
  bool materialInstanceAmbientCoef;
  bool materialInstanceSpecularCoef;
  bool materialInstanceShinnessCoef;
  bool materialInstanceEmissiveCoef;
};

struct SceneDefinition {
  std::string scenePath;
  SceneLoaderConfiguration configuration;

  inline operator uint32_t() const { return hash(scenePath); }
};

struct Scene {
  ModelList *sceneOwner;
  Node *rootNode;
  ModelList models;

  Scene(const SceneDefinition &definition);
  ~Scene();
};
} // namespace shambhala
