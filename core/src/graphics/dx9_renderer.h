#pragma once
#include <d3d9.h>
#include <d3dx9.h>

#include "color.h"
#include "graphics_renderer.h"

typedef HRESULT(WINAPI *pFnD3DXCreateSprite)(LPDIRECT3DDEVICE9 pDevice,
                                             LPD3DXSPRITE *ppSprite);

namespace overlay {
namespace core {
namespace graphics {

class Dx9Renderer : public IGraphicsRenderer {
 public:
  Dx9Renderer(IDirect3DDevice9 *device);
  ~Dx9Renderer();

  virtual bool Init();
  virtual void RenderSprites(
      const std::vector<std::shared_ptr<Sprite>> &sprites);

  virtual void OnResize(uint32_t width, uint32_t height, bool fullscreen);

 private:
  HMODULE d3dx9_module_;

  IDirect3DDevice9 *device_;
  ID3DXSprite *sprite_drawer_;

  void DrawSprite(const std::shared_ptr<Sprite> &sprite);

  IDirect3DTexture9 *CreateTextureFromSolidColor(Rect rect, Color color);
  IDirect3DTexture9 *CreateTextureFromBuffer(Rect rect, std::string &buffer);
  bool CopyBufferToTexture(IDirect3DTexture9 *texture, Rect rect,
                           std::string &buffer) const;
};

}  // namespace graphics
}  // namespace core
}  // namespace overlay