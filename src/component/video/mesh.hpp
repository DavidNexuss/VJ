#include "buffer.hpp"
#include <adapters/video.hpp>

struct VertexAttribute {
  int index;
  int size;
  int attributeDivisor;
  VertexAttribute() {}
  VertexAttribute(int _index, int _size) : index(_index), size(_size) {}
};

using VertexLayout = std::vector<VertexAttribute>;

struct DriverBuffer {
  virtual GLuint use();
  static DriverBuffer *create(Buffer *data);

protected:
  Buffer *data = nullptr;
  GLenum mode;
  GLuint _gl = -1;
};

struct VertexBuffer : public DriverBuffer {
  GLuint use() override;

  VertexBuffer();
  static VertexBuffer *create(Buffer *data, VertexLayout *layout);

private:
  VertexLayout *layout = nullptr;
  int vertexSize;
};

struct IndexBuffer : public DriverBuffer {
  IndexBuffer();
  static IndexBuffer *create(Buffer *data);
};

struct Mesh {

  int vertexCount();
  int indexCount();

  void use();

  static Mesh *create(VertexBuffer *vbo, IndexBuffer *ebo);

  bool invertedFaces = false;

private:
  VertexBuffer *vertexBuffer = nullptr;
  IndexBuffer *indexBuffer = nullptr;
};
