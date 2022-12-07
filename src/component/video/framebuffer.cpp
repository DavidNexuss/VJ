#include "framebuffer.hpp"
#include "adapters/video.hpp"
#include "adapters/viewport.hpp"

FrameBuffer::FrameBuffer() { clearColor = glm::vec4(0.0, 0.0, 0.0, 1.0); }
GLuint FrameBufferOutput::gl() {
  return framebuffer->getOutputAttachment(attachmentIndex);
}
FrameBufferOutput *FrameBuffer::getOutputTexture(int index) {
  auto *out = new FrameBufferOutput;
  out->framebuffer = this;
  out->attachmentIndex = index;
  return out;
}
GLuint FrameBuffer::getOutputAttachment(int index) {
  if (index == Standard::attachmentDepthBuffer)
    return gl_depthBuffer;
  return colorAttachments[index];
}

GLuint FrameBuffer::createDepthStencilBuffer() {
  return video::createRenderbuffer(
      {GL_DEPTH24_STENCIL8, bufferWidth, bufferHeight});
}

void FrameBuffer::initialize() {

  video::TextureUploadDesc text;
  text.width = bufferWidth;
  text.height = bufferHeight;
  text.textureID = 0;
  text.target = GL_TEXTURE_2D;

  if (gl_framebuffer == -1) {

    colorAttachments.resize(attachmentsDefinition.size());
    for (int i = 0; i < attachmentsDefinition.size(); i++) {

      video::TextureDesc textureDesc = attachmentsDefinition[i].desc;
      colorAttachments[i] = video::createTexture(textureDesc);
      text.textureID = colorAttachments[i];
      text.format = attachmentsDefinition[i].format;
      video::uploadTexture(text);
    }

    video::FrameBufferDesc fbodef;
    if (combinedDepthStencil()) {
      if (configuration & USE_RENDER_BUFFER) {
        gl_stencilDepthBuffer = createDepthStencilBuffer();
        fbodef.renderBufferAttachment = gl_stencilDepthBuffer;
      } else {
        gl_stencilDepthBuffer = video::createTexture({});
        text.textureID = gl_stencilDepthBuffer;
        video::uploadTexture(text);
        fbodef.depthStencilAttachment = gl_stencilDepthBuffer;
      }
    }

    else {
      if (configuration & USE_DEPTH) {
        gl_depthBuffer = video::createTexture(video::descDepthTexture());
        video::uploadTexture(
            video::descDepthUpload(bufferWidth, bufferHeight, gl_depthBuffer));
        fbodef.depthAttachment = gl_depthBuffer;
      }

      if (configuration & USE_STENCIL) {
        gl_stencilBuffer = video::createTexture(video::descStencilTexture());
        video::uploadTexture(video::descStencilUpload(bufferWidth, bufferHeight,
                                                      gl_stencilBuffer));
        fbodef.stencilAttachment = gl_stencilBuffer;
      }
    }

    fbodef.attachmentCount = colorAttachments.size();
    fbodef.attachments = &colorAttachments[0];
    gl_framebuffer = video::createFramebuffer(fbodef);
  } else {
    for (int i = 0; i < attachmentsDefinition.size(); i++) {
      text.textureID = colorAttachments[i];
      text.format = attachmentsDefinition[i].format;
      video::uploadTexture(text);
    }
  }
}

void FrameBuffer::dispose() {}

void FrameBuffer::resize(int screenWidth, int screenHeight) {
  if (bufferWidth == screenWidth && bufferHeight == screenHeight)
    return;
  if (gl_framebuffer != -1)
    dispose();

  bufferWidth = screenWidth;
  bufferHeight = screenHeight;

  initialize();
}

void FrameBuffer::begin() { begin(desiredWidth, desiredHeight); }

void FrameBuffer::begin(int screenWidth, int screenHeight) {

  if (screenWidth < 0)
    screenWidth = viewport::getScreenWidth() / -desiredWidth;
  if (screenHeight < 0)
    screenHeight = viewport::getScreenHeight() / -desiredHeight;

  resize(screenWidth, screenHeight);
  video::bindFrameBuffer(gl_framebuffer);
  video::set(video::SH_CLEAR_COLOR, clearColor);
  video::clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  viewport::pushViewport(screenWidth, screenHeight);
  video::setViewport(viewport::getWidth(), viewport::getHeight());
}

void FrameBuffer::end() {
  video::bindFrameBuffer(0);
  viewport::popViewport();
  video::setViewport(viewport::getWidth(), viewport::getHeight());
}

void FrameBuffer::addOutput(video::TextureFormat format) {
  FrameBufferAttachmentDefinition def;
  def.format = format;
  attachmentsDefinition.push_back(def);
}
void FrameBuffer::addOutputAttachment(FrameBufferAttachmentDefinition def) {
  attachmentsDefinition.push_back(def);
}

void FrameBuffer::setConfiguration(FrameBufferDescriptorFlags flags) {
  configuration = flags;
}

int FrameBuffer::getWidth() { return bufferWidth; }
int FrameBuffer::getHeight() { return bufferHeight; }
