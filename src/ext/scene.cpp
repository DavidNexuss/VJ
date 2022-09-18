#include "scene.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <ext/resource.hpp>
#include <shambhala.hpp>
#include <standard.hpp>
using namespace shambhala;
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

    // Sets glTexture unit!!!
    // TODO: Fix
    DynamicTexture dtex;
    dtex.sourceTexture = materialtextures[0];
    dtex.unit = i;
    minstance->set(configuration.textureNames[i], dtex);
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

Mesh *createMesh(const aiScene *scene, aiMesh *mesh,
                 const SceneLoaderConfiguration &configuration) {

  int vertexSize = _vertexSize(configuration.attributes);
  simple_vector<float> vertexBuffer(mesh->mNumVertices * vertexSize);
  std::cerr << "Loading model with vertex size: " << vertexSize << std::endl;
  for (int i = 0; i < mesh->mNumVertices; i++) {
    int offset = 0;
    for (int attr = 0; attr < configuration.attributes.size(); attr++) {
      switch (configuration.attributes[attr].index) {
      case Standard::aPosition:
        vertexBuffer[i * vertexSize + offset] = mesh->mVertices[i].x;
        vertexBuffer[i * vertexSize + offset + 1] = mesh->mVertices[i].y;
        vertexBuffer[i * vertexSize + offset + 2] = mesh->mVertices[i].z;
        break;
      case Standard::aNormal:
        vertexBuffer[i * vertexSize + offset] = mesh->mNormals[i].x;
        vertexBuffer[i * vertexSize + offset + 1] = mesh->mNormals[i].y;
        vertexBuffer[i * vertexSize + offset + 2] = mesh->mNormals[i].z;
        break;
      case Standard::aBiTangent:
        vertexBuffer[i * vertexSize + offset] = mesh->mBitangents[i].x;
        vertexBuffer[i * vertexSize + offset + 1] = mesh->mBitangents[i].y;
        vertexBuffer[i * vertexSize + offset + 2] = mesh->mBitangents[i].z;
        break;
      case Standard::aColor:
        vertexBuffer[i * vertexSize + offset] = mesh->mColors[0][i].r;
        vertexBuffer[i * vertexSize + offset + 1] = mesh->mColors[0][i].g;
        vertexBuffer[i * vertexSize + offset + 2] = mesh->mColors[0][i].b;
        break;
      case Standard::aTangent:
        vertexBuffer[i * vertexSize + offset] = mesh->mTangents[i].x;
        vertexBuffer[i * vertexSize + offset + 1] = mesh->mTangents[i].y;
        vertexBuffer[i * vertexSize + offset + 2] = mesh->mTangents[i].z;
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

  vector<Standard::meshIndex> indices;
  for (int i = 0; i < mesh->mNumFaces; i++) {
    const aiFace &face = mesh->mFaces[i];
    for (int j = 0; j < face.mNumIndices; j++) {
      indices.push(face.mIndices[j]);
    }
  }

  int indexcount = indices.size();
  Mesh *meshid = shambhala::createMesh();
  meshid->vbo = shambhala::createVertexBuffer();
  meshid->ebo = shambhala::createIndexBuffer();
  meshid->vbo->vertexBuffer = vertexBuffer.drop();
  meshid->vbo->attributes = configuration.attributes;
  meshid->ebo->indexBuffer = indices.drop();
  return meshid;
}

Mesh *LoadingContext::loadMesh(const aiScene *scene, int index,
                               const SceneLoaderConfiguration &configuration) {
  auto it = meshes.find(index);
  if (it != meshes.end())
    return it->second;
  return meshes[index] =
             createMesh(scene, scene->mMeshes[index], configuration);
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
    model->node = engineNode;
    result.models.add(model);
  }

  for (size_t i = 0; i < node->mNumChildren; i++) {
    engineNode->addChildNode(
        processNode(node->mChildren[i], scene, result, context, configuration));
  }

  return engineNode;
}

Scene::Scene(const SceneDefinition &def) {
  Assimp::Importer importer;
  const aiScene *scene =
      importer.ReadFile(def.scenePath, def.configuration.assimpFlags);

  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    return;
  }

  LoadingContext context;
  rootNode =
      processNode(scene->mRootNode, scene, *this, context, def.configuration);
  sceneOwner = shambhala::getWorkingModelList();
}
Scene::~Scene() {}
