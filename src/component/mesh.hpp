#include "buffer.hpp"
#include "component.hpp"
#include "shambhala.hpp"

namespace shambhala {
struct VertexAttribute {
  int index;
  int size;
  int attributeDivisor;
};

using VertexLayout = std::vector<VertexAttribute>;

struct DriverBuffer {
  Buffer *data = nullptr;
  SHenum mode;

  virtual SHuint use();

private:
  SHuint _gl = -1;
};

struct VertexBuffer : public DriverBuffer {
  VertexLayout *layout = nullptr;
  int vertexSize;
  SHuint use() override;
};

struct IndexBuffer : public DriverBuffer {};

struct Mesh {
  VertexBuffer *vertexBuffer = nullptr;
  IndexBuffer *indexBuffer = nullptr;

  int vertexCount();
  void use();
};
} // namespace shambhala
