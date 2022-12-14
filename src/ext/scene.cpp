/*
#include "scene.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <ext/resource.hpp>
#include <shambhala.hpp>
#include <standard.hpp>
using namespace shambhala;

struct Scene {
  Node *rootNode;

  Scene();
  Scene(const SceneDefinition &definition);
  ~Scene();

  Scene createInstance();
};

glm::mat4 getTransformMatrix(const aiMatrix4x4 &from) {
  glm::mat4 to;

  to[0][0] = (GLfloat)from.a1;
  to[0][1] = (GLfloat)from.b1;
  to[0][2] = (GLfloat)from.c1;
  to[0][3] = (GLfloat)from.d1;
  to[1][0] = (GLfloat)from.a2;
  to[1][1] = (GLfloat)from.b2;
  to[1][2] = (GLfloat)from.c2;
  to[1][3] = (GLfloat)from.d2;
  to[2][0] = (GLfloat)from.a3;
  to[2][1] = (GLfloat)from.b3;
  to[2][2] = (GLfloat)from.c3;
  to[2][3] = (GLfloat)from.d3;
  to[3][0] = (GLfloat)from.a4;
  to[3][1] = (GLfloat)from.b4;
  to[3][2] = (GLfloat)from.c4;
  to[3][3] = (GLfloat)from.d4;

  return to;
}

glm::vec3 getColor(const aiColor3D &col) {
  return glm::vec3(col.r, col.g, col.b);
}

struct LoadingContext {
  std::unordered_map<int, Mesh *> meshes;
  std::unordered_map<int, Material *> materials;

  Mesh *loadMesh(const aiScene *scene, int index,
                 const SceneLoaderConfiguration &configuration);

  Material *loadMaterial(const aiScene *scene, int index,
                         const SceneLoaderConfiguration &configuration);
};

vector<Texture *> loadMaterialTextures(aiMaterial *mat, aiTextureType type) {

  vector<Texture *> result;
  for (int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    Texture *text = shambhala::createTexture();
    text->addTextureResource(
        shambhala::resource::stbiTextureFile(str.C_Str(), 3));
    result.push(text);
  }
  return result;
}
Material *
createMaterialInstance(const aiScene *scene, aiMaterial *material,
                       const SceneLoaderConfiguration &configuration) {

  Material *minstance = shambhala::createMaterial();
  for (int i = 0; i < configuration.textureTypes.size(); i++) {
    vector<Texture *> materialtextures =
        loadMaterialTextures(material, configuration.textureTypes[i]);

    minstance->set(configuration.textureNames[i], materialtextures[0]);
  }
  if (configuration.materialInstanceAmbientCoef) {
    aiColor3D ka;
    material->Get(AI_MATKEY_COLOR_AMBIENT, ka);
    minstance->set("ka", getColor(ka));
  }
  if (configuration.materialInstanceDiffuseCoef) {
    aiColor3D kd;
    material->Get(AI_MATKEY_COLOR_DIFFUSE, kd);
    minstance->set("kd", getColor(kd));
  }
  if (configuration.materialInstanceSpecularCoef) {
    aiColor3D ks;
    material->Get(AI_MATKEY_COLOR_SPECULAR, ks);
    minstance->set("ks", getColor(ks));
  }
  if (configuration.materialInstanceEmissiveCoef) {
    aiColor3D ke;
    material->Get(AI_MATKEY_COLOR_SPECULAR, ke);
    minstance->set("ke", getColor(ke));
  }
  if (configuration.materialInstanceShinnessCoef) {
    float sh;
    material->Get(AI_MATKEY_SHININESS, sh);
    minstance->set("sh", sh);
  }

  return minstance;
}
static int _vertexSize(const vector<VertexAttribute> &attrs) {
  int size = 0;
  for (int i = 0; i < attrs.size(); i++) {
    size += attrs[i].size;
  }
  return size;
}

void bufferVertices(float *vertexBuffer, aiMesh *mesh,
                    const SceneLoaderConfiguration &configuration,
                    const glm::mat4 *transform = nullptr) {
  glm::mat3 normalMat;
  bool useTransform = transform != nullptr && *transform != glm::mat4(1.0f);
  if (useTransform) {
    normalMat = glm::mat3(glm::transpose(glm::inverse(*transform)));
  }
  int vertexSize = _vertexSize(configuration.attributes);
  for (int i = 0; i < mesh->mNumVertices; i++) {
    int offset = 0;
    for (int attr = 0; attr < configuration.attributes.size(); attr++) {
      switch (configuration.attributes[attr].index) {
      case Standard::aPosition:
        if (useTransform) {
          glm::vec4 position =
              glm::vec4(mesh->mVertices[i].x, mesh->mVertices[i].y,
                        mesh->mVertices[i].z, 1.0);
          position = *transform * position;
          vertexBuffer[i * vertexSize + offset] = position.x;
          vertexBuffer[i * vertexSize + offset + 1] = position.y;
          vertexBuffer[i * vertexSize + offset + 2] = position.z;
        } else {
          vertexBuffer[i * vertexSize + offset] = mesh->mVertices[i].x;
          vertexBuffer[i * vertexSize + offset + 1] = mesh->mVertices[i].y;
          vertexBuffer[i * vertexSize + offset + 2] = mesh->mVertices[i].z;
        }
        break;
      case Standard::aNormal:
        if (useTransform) {
          glm::vec3 normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y,
                                       mesh->mNormals[i].z);
          normal = normalMat * normal;
          vertexBuffer[i * vertexSize + offset] = normal.x;
          vertexBuffer[i * vertexSize + offset + 1] = normal.y;
          vertexBuffer[i * vertexSize + offset + 2] = normal.z;
        } else {
          vertexBuffer[i * vertexSize + offset] = mesh->mNormals[i].x;
          vertexBuffer[i * vertexSize + offset + 1] = mesh->mNormals[i].y;
          vertexBuffer[i * vertexSize + offset + 2] = mesh->mNormals[i].z;
        }
        break;
      case Standard::aBiTangent:

        if (useTransform) {
          glm::vec3 normal =
              glm::vec3(mesh->mBitangents[i].x, mesh->mBitangents[i].y,
                        mesh->mBitangents[i].z);
          normal = normalMat * normal;
          vertexBuffer[i * vertexSize + offset] = normal.x;
          vertexBuffer[i * vertexSize + offset + 1] = normal.y;
          vertexBuffer[i * vertexSize + offset + 2] = normal.z;
        } else {
          vertexBuffer[i * vertexSize + offset] = mesh->mBitangents[i].x;
          vertexBuffer[i * vertexSize + offset + 1] = mesh->mBitangents[i].y;
          vertexBuffer[i * vertexSize + offset + 2] = mesh->mBitangents[i].z;
        }
        break;
      case Standard::aColor:
        vertexBuffer[i * vertexSize + offset] = mesh->mColors[0][i].r;
        vertexBuffer[i * vertexSize + offset + 1] = mesh->mColors[0][i].g;
        vertexBuffer[i * vertexSize + offset + 2] = mesh->mColors[0][i].b;
        break;
      case Standard::aTangent:

        if (useTransform) {
          glm::vec3 normal = glm::vec3(
              mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z);
          normal = normalMat * normal;
          vertexBuffer[i * vertexSize + offset] = normal.x;
          vertexBuffer[i * vertexSize + offset + 1] = normal.y;
          vertexBuffer[i * vertexSize + offset + 2] = normal.z;
        } else {

          vertexBuffer[i * vertexSize + offset] = mesh->mTangents[i].x;
          vertexBuffer[i * vertexSize + offset + 1] = mesh->mTangents[i].y;
          vertexBuffer[i * vertexSize + offset + 2] = mesh->mTangents[i].z;
        }
        break;
      case Standard::aUV:
        vertexBuffer[i * vertexSize + offset] = mesh->mTextureCoords[0][i].x;
        vertexBuffer[i * vertexSize + offset + 1] =
            mesh->mTextureCoords[0][i].y;
        break;
      }
      offset += configuration.attributes[attr].size;
    }
  }
}

void bufferIndices(simple_vector<Standard::meshIndex> &indices, aiMesh *mesh,
                   int start) {

  for (int i = 0; i < mesh->mNumFaces; i++) {
    const aiFace &face = mesh->mFaces[i];
    for (int j = 0; j < face.mNumIndices; j++) {
      indices.push(face.mIndices[j] + start);
    }
  }
}
Mesh *createMesh(aiMesh *mesh, const SceneLoaderConfiguration &configuration) {

  int vertexSize = _vertexSize(configuration.attributes);
  simple_vector<float> vertexBuffer(mesh->mNumVertices * vertexSize);
  vector<Standard::meshIndex> indices;

  bufferVertices(&vertexBuffer[0], mesh, configuration);
  bufferIndices(indices, mesh, 0);

  int indexcount = indices.size();
  Mesh *meshid = shambhala::createMesh();
  meshid->vbo = shambhala::createVertexBuffer();
  meshid->ebo = shambhala::createIndexBuffer();
  meshid->vbo->vertexBuffer = vertexBuffer.drop();
  meshid->vbo->attributes = configuration.attributes;
  meshid->ebo->indexBuffer = indices.drop();
  return meshid;
}

Mesh *createMeshCombined(const simple_vector<aiMesh *> meshes,
                         const simple_vector<glm::mat4> &transforms,
                         const SceneLoaderConfiguration &configuration) {
  Mesh *result = shambhala::createMesh();

  int vertexSize = _vertexSize(configuration.attributes);
  int numVertices = 0;
  int count = meshes.size();
  for (int i = 0; i < count; i++)
    numVertices += meshes[i]->mNumVertices;

  simple_vector<float> vertexBuffer(numVertices * vertexSize);
  simple_vector<Standard::meshIndex> indices;

  float *vertexBufferPtr = &vertexBuffer[0];

  for (int i = 0; i < count; i++) {
    bufferVertices(vertexBufferPtr, meshes[i], configuration, &transforms[i]);
    vertexBufferPtr += meshes[i]->mNumVertices * vertexSize;
  }

  int start = 0;
  for (int i = 0; i < count; i++) {
    bufferIndices(indices, meshes[i], start);
    start += meshes[i]->mNumVertices;
  }

  result->vbo = shambhala::createVertexBuffer();
  result->ebo = shambhala::createIndexBuffer();
  result->vbo->vertexBuffer = vertexBuffer.drop();
  result->ebo->indexBuffer = indices.drop();
  result->vbo->attributes = configuration.attributes;
  return result;
}

Mesh *LoadingContext::loadMesh(const aiScene *scene, int index,
                               const SceneLoaderConfiguration &configuration) {
  auto it = meshes.find(index);
  if (it != meshes.end())
    return it->second;
  return meshes[index] = createMesh(scene->mMeshes[index], configuration);
}

Material *
LoadingContext::loadMaterial(const aiScene *scene, int index,
                             const SceneLoaderConfiguration &configuration) {
  auto it = materials.find(index);
  if (it != materials.end())
    return it->second;
  return materials[index] = createMaterialInstance(
             scene, scene->mMaterials[index], configuration);
}

Node *processNode(aiNode *node, const aiScene *scene, Scene &result,
                  LoadingContext &context,
                  const SceneLoaderConfiguration &configuration) {

  Node *engineNode = shambhala::createNode();
  engineNode->setTransformMatrix(getTransformMatrix(node->mTransformation));

  for (size_t i = 0; i < node->mNumMeshes; i++) {
    Model *model = shambhala::createModel();
    model->mesh = context.loadMesh(scene, node->mMeshes[i], configuration);
    model->material = context.loadMaterial(
        scene, scene->mMeshes[node->mMeshes[i]]->mMaterialIndex, configuration);
    model->setNode(engineNode);
    shambhala::getWorkingModelList()->add(model);
  }

  for (size_t i = 0; i < node->mNumChildren; i++) {
    engineNode->addChildNode(
        processNode(node->mChildren[i], scene, result, context, configuration));
  }

  return engineNode;
}

void dfs_nodes(const aiScene *scene, aiNode *node,
               simple_vector<aiMesh *> &meshes,
               simple_vector<glm::mat4> &transforms, glm::mat4 transform) {

  for (size_t i = 0; i < node->mNumMeshes; i++) {
    meshes.push(scene->mMeshes[node->mMeshes[i]]);
    transforms.push(transform);
  }

  for (size_t i = 0; i < node->mNumChildren; i++) {
    glm::mat4 childTransform =
        transform * getTransformMatrix(node->mTransformation);
    dfs_nodes(scene, node->mChildren[i], meshes, transforms, childTransform);
  }
}
Node *processNodeCombine(aiNode *node, const aiScene *scene, Scene &result,
                         const SceneLoaderConfiguration &configuration) {
  simple_vector<aiMesh *> meshes;
  simple_vector<glm::mat4> transforms;
  Node *engineNode = shambhala::createNode();
  engineNode->setTransformMatrix(getTransformMatrix(node->mTransformation));

  dfs_nodes(scene, node, meshes, transforms, glm::mat4(1.0f));

  Model *model = shambhala::createModel();
  model->mesh = createMeshCombined(meshes, transforms, configuration);
  model->setNode(engineNode);
  shambhala::getWorkingModelList()->add(model);
  return engineNode;
}

Scene::Scene() {}
Scene::Scene(const SceneDefinition &def) {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(def.scenePath, def.configuration.assimpFlags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    return;
  }

  LoadingContext context;
  if (def.configuration.combineMeshes) {
    rootNode =
        processNodeCombine(scene->mRootNode, scene, *this, def.configuration);
  } else {
    rootNode =
        processNode(scene->mRootNode, scene, *this, context, def.configuration);
  }
}
Scene::~Scene() {}

struct SceneContainer : public loader::LoaderMap<Node, SceneContainer> {

  static Node *create(const char *path) {

    SceneLoaderConfiguration configuration;
    configuration.combineMeshes = true;
    configuration.assimpFlags =
        aiProcess_FlipUVs | aiProcess_GenNormals | aiProcess_Triangulate |
        aiProcess_CalcTangentSpace | aiProcess_OptimizeGraph |
        aiProcess_OptimizeMeshes | aiProcess_PreTransformVertices;

    configuration.attributes = {{Standard::aPosition, 3},
                                {Standard::aNormal, 3},
                                {Standard::aUV, 2},
                                {Standard::aTangent, 3}};

    SceneDefinition def;
    def.scenePath = path;
    def.configuration = configuration;
    return (new Scene(def))->rootNode;
  }

  static loader::Key computeKey(const char *path) {
    return loader::computeKey(path);
  }
};

static SceneContainer sceneContainer;
Node *loader::loadScene(const char *path) { return sceneContainer.get(path); }
void loader::unloadScene(Node *scene) { sceneContainer.unload(scene); }*/
