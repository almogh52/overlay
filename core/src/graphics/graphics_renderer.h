#pragma once
#include <cstdint>
#include <memory>
#include <mutex>
#include <vector>

#include "sprite.h"

namespace overlay {
namespace core {
namespace graphics {

class IGraphicsRenderer {
 public:
  inline virtual ~IGraphicsRenderer() {}

  virtual bool Init() = 0;
  virtual void RenderSprites(
      const std::vector<std::shared_ptr<Sprite>>& sprites) = 0;

  virtual void OnResize(uint32_t width, uint32_t height, bool fullscreen) = 0;

  inline void QueueTextureRelease(IUnknown* texture) {
    std::lock_guard lk(texture_release_queue_mutex_);
    texture_release_queue_.push_back(texture);
  }

  inline uint32_t get_width() const { return width_; }
  inline uint32_t get_height() const { return height_; }
  inline bool is_fullscreen() const { return fullscreen_; }

 protected:
  inline void ReleaseTextures() {
    std::lock_guard lk(texture_release_queue_mutex_);

    // Release the textures
    for (auto& texture : texture_release_queue_) {
      texture->Release();
    }

    // Clear the vector
    texture_release_queue_.clear();
  }

  inline Rect TargetFillRect() const {
    Rect rect;

    rect.x = 0;
    rect.y = 0;
    rect.width = get_width();
    rect.height = get_height();

    return rect;
  }

  inline void set_width(uint32_t width) { width_ = width; }
  inline void set_height(uint32_t height) { height_ = height; }
  inline void set_fullscreen(bool fullscreen) { fullscreen_ = fullscreen; }

 private:
  uint32_t width_, height_;
  bool fullscreen_;

  std::vector<IUnknown*> texture_release_queue_;
  std::mutex texture_release_queue_mutex_;
};

}  // namespace graphics
}  // namespace core
}  // namespace overlay