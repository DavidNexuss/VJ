#include "mesh.hpp"
#include <shambhala.hpp>

using namespace shambhala::video;

static void uploadBuffer(Buffer *data, GLuint _gl) {
  if (data->pendingUpdate.size() != 0) {
    data->mergeUpdates();
    BufferUploadDesc upload;

    upload.buffer = data->memory;
    upload.id = _gl;
    for (int i = 0; i < data->pendingUpdate.size(); i++) {
      upload.start = data->pendingUpdate[i].start;
      upload.size = data->pendingUpdate[i].size;

      shambhala::vid()->uploadBuffer(upload);
    }

    data->pendingUpdate.clear();
  }
}
SHuint DriverBuffer::use() {
  BufferDesc desc;
  desc.type = mode;
  if (_gl == -1)
    _gl = shambhala::vid()->createBuffer(desc);

  uploadBuffer(data, _gl);
  shambhala::vid()->bindBuffer(_gl);
  return _gl;
}

SHuint VertexBuffer::use() {
  SHuint vbo = DriverBuffer::use();
  auto &attributes = *this->layout;
  int offset = 0;
  int stride = vertexSize;
  for (int i = 0; i < attributes.size(); i++) {
    int index = attributes[i].index;
    int divisor = attributes[i].attributeDivisor;
    int size = attributes[i].size;

    AttributeDesc attr;
    attr.buffer = vbo;
    attr.index = index;
    attr.size = size;
    attr.stride = stride;
    attr.offset = offset * sizeof(float);
    attr.divisor = divisor;
    shambhala::vid()->bindAttribute(attr);
    offset += attributes[i].size;
  }
  return vbo;
}
void Mesh::use() {
  if (vertexBuffer)
    vertexBuffer->use();
  if (indexBuffer)
    indexBuffer->use();
}
