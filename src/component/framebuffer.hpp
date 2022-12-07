
#include "shambhala.hpp"
enum FrameBufferDescriptorFlags {
  FRAME_BUFFER_NULL = 0,
  USE_RENDER_BUFFER = 1 << 0,
  USE_DEPTH = 1 << 1,
  USE_STENCIL = 1 << 2,
  SEPARATE_DEPTH_STENCIL = 1 << 3,
  ONLY_WRITE_DEPTH = 1 << 4,
  ONLY_READ_DEPTH = 1 << 5,
};

struct FrameBufferAttachmentDefinition {
  video::TextureDesc desc;
  video::TextureFormat format;
};

ENUM_OPERATORS(FrameBufferDescriptorFlags)

struct FrameBuffer;
struct FrameBufferOutput : public ITexture {
  GLuint gl() override;

private:
  FrameBuffer *framebuffer;
  int attachmentIndex;
  friend FrameBuffer;
};

class FrameBuffer {
  FrameBufferDescriptorFlags configuration = FRAME_BUFFER_NULL;
  int bufferWidth = -1, bufferHeight = -1;

  int desiredWidth = -1;
  int desiredHeight = -1;

  void initialize();
  void dispose();
  void resize(int screenWidth, int screenHeight);

  GLuint createDepthStencilBuffer();

  inline bool combinedDepthStencil() const {
    return configuration & USE_DEPTH && configuration & USE_STENCIL &&
           !(configuration & SEPARATE_DEPTH_STENCIL);
  }
  simple_vector<GLuint> colorAttachments;
  simple_vector<FrameBufferAttachmentDefinition> attachmentsDefinition;

  GLuint gl_framebuffer = -1;
  GLuint gl_stencilDepthBuffer = -1;
  GLuint gl_depthBuffer = -1;
  GLuint gl_stencilBuffer = -1;

public:
  FrameBuffer();
  FrameBufferOutput *getOutputTexture(int index);
  GLuint getOutputAttachment(int index);
  void addOutputAttachment(FrameBufferAttachmentDefinition def);
  void addOutput(video::TextureFormat format);

  void begin(int width, int height);
  void begin();
  void end();

  void setConfiguration(FrameBufferDescriptorFlags flags);

  inline void setWidth(int width) { desiredWidth = width; }
  inline void setHeight(int height) { desiredHeight = height; }
  int getWidth();
  int getHeight();

  glm::vec4 clearColor;
};
