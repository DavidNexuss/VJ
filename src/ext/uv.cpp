#include "uv.hpp"
#include <shambhala.hpp>
#include <xatlas/xatlas.hpp>

using namespace shambhala;

xatlas::MeshDecl getMeshDeclaration(Mesh *mesh) {
  xatlas::MeshDecl meshDecl;

  meshDecl.vertexCount =
      mesh->vbo->vertexBuffer.size() / mesh->vbo->vertexSize();

  {
    VertexAttribute positionAttribute = mesh->getAttribute(Standard::aPosition);
    meshDecl.vertexPositionData =
        &mesh->vbo->vertexBuffer[0] + positionAttribute.size * sizeof(float);
    meshDecl.vertexPositionStride = positionAttribute.stride * sizeof(float);
  }
  {
    VertexAttribute uvAttribute = mesh->getAttribute(Standard::aUV);
    meshDecl.vertexUvData =
        &mesh->vbo->vertexBuffer[0] + uvAttribute.size * sizeof(float);
    meshDecl.vertexUvStride = uvAttribute.stride * sizeof(float);
  }
  {
    VertexAttribute normalAttribute = mesh->getAttribute(Standard::aNormal);
    meshDecl.vertexNormalData =
        &mesh->vbo->vertexBuffer[0] + normalAttribute.size * sizeof(float);
    meshDecl.vertexNormalStride = normalAttribute.stride * sizeof(float);
  }

  if (mesh->ebo != nullptr) {
    meshDecl.indexData = &mesh->ebo->indexBuffer[0];
    meshDecl.indexCount = mesh->vertexCount();

    // TODO SHOULD BE PARAMETRIZED UINT32
    meshDecl.indexFormat = xatlas::IndexFormat::UInt32;
  }
  return meshDecl;
}

std::vector<float> generateFromAtlas(xatlas::Atlas *atlas) {

  int count = 0;
  for (int i = 0; i < atlas->meshCount; i++) {
    const xatlas::Mesh &mesh = atlas->meshes[i];
    count += mesh.vertexCount;
  }

  std::vector<float> uvbuffer(count * 2);
  for (int i = 0; i < atlas->meshCount; i++) {
    const xatlas::Mesh &mesh = atlas->meshes[i];
    int offset = 0;
    for (int j = 0; j < mesh.vertexCount; j++) {
      uvbuffer[offset + j * 2] = mesh.vertexArray[j].uv[0];
      uvbuffer[offset + j * 2 + 1] = mesh.vertexArray[j].uv[1];
    }
    offset += mesh.vertexCount * 2;
  }
  return uvbuffer;
}
std::vector<float> shambhala::ui::generateUVMap(Mesh **meshes, int count) {

  xatlas::Atlas *atlas = xatlas::Create();
  for (int i = 0; i < count; i++) {
    xatlas::AddMesh(atlas, getMeshDeclaration(meshes[i]));
  }
  xatlas::Generate(atlas);
  return generateFromAtlas(atlas);
}
