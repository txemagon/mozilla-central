/* -*- Mode: C++; tab-width: 20; indent-tabs-mode: nil; c-basic-offset: 2 -*-
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "mozilla/layers/ImageContainerParent.h"
#include "ipc/AutoOpenSurface.h"
#include "ImageHost.h"

namespace mozilla {
namespace layers {

ImageHostTexture::ImageHostTexture(Compositor* aCompositor)
  : ImageHost(aCompositor)
  , mTextureHost(nullptr)
{
}

const SharedImage*
ImageHostTexture::UpdateImage(const TextureIdentifier& aTextureIdentifier,
                              const SharedImage& aImage)
{
  return mTextureHost->Update(aImage);
}

void
ImageHostTexture::Composite(EffectChain& aEffectChain,
                            float aOpacity,
                            const gfx::Matrix4x4& aTransform,
                            const gfx::Point& aOffset,
                            const gfx::Filter& aFilter,
                            const gfx::Rect& aClipRect,
                            const nsIntRegion* aVisibleRegion)
{
  if (Effect* effect = mTextureHost->Lock(aFilter)) {
    aEffectChain.mEffects[effect->mType] = effect;
  } else {
    return;
  }

  TileIterator* it = mTextureHost->GetAsTileIterator();
  NS_ASSERTION(it, "Texture host should be iterable as tiles");

  it->BeginTileIteration();
  do {
    nsIntRect tileRect = it->GetTileRect();
    gfx::Rect rect(tileRect.x, tileRect.y, tileRect.width, tileRect.height);
    gfx::Rect sourceRect(0, 0, tileRect.width, tileRect.height);
    mCompositor->DrawQuad(rect, &sourceRect, nullptr, &aClipRect, aEffectChain,
                          aOpacity, aTransform, aOffset);
  } while (it->NextTile());

  mTextureHost->Unlock();
}

void
ImageHostTexture::AddTextureHost(const TextureIdentifier& aTextureIdentifier, TextureHost* aTextureHost)
{
  NS_ASSERTION(aTextureIdentifier.mBufferType == BUFFER_TEXTURE &&
               aTextureIdentifier.mTextureType == TEXTURE_SHMEM,
               "BufferType mismatch.");
  mTextureHost = aTextureHost;
}

ImageHostShared::ImageHostShared(Compositor* aCompositor)
  : ImageHost(aCompositor)
  , mTextureHost(nullptr)
{
}

const SharedImage*
ImageHostShared::UpdateImage(const TextureIdentifier& aTextureIdentifier,
                             const SharedImage& aImage)
{
  return mTextureHost->Update(aImage);
}

void
ImageHostShared::Composite(EffectChain& aEffectChain,
                           float aOpacity,
                           const gfx::Matrix4x4& aTransform,
                           const gfx::Point& aOffset,
                           const gfx::Filter& aFilter,
                           const gfx::Rect& aClipRect,
                           const nsIntRegion* aVisibleRegion)
{
  if (Effect* effect = mTextureHost->Lock(aFilter)) {
    aEffectChain.mEffects[effect->mType] = effect;
  } else {
    return;
  }

  gfx::Rect rect(0, 0, mTextureHost->GetSize().width, mTextureHost->GetSize().height);
  mCompositor->DrawQuad(rect, nullptr, nullptr, &aClipRect, aEffectChain,
                        aOpacity, aTransform, aOffset);

  mTextureHost->Unlock();
}

void
ImageHostShared::AddTextureHost(const TextureIdentifier& aTextureIdentifier, TextureHost* aTextureHost)
{
  NS_ASSERTION(aTextureIdentifier.mBufferType == BUFFER_SHARED &&
               aTextureIdentifier.mTextureType == TEXTURE_SHARED,
               "BufferType mismatch.");
  mTextureHost = aTextureHost;
}


// aImage contains all three plains, we could also send them seperately and 
// update mTextures one at a time
const SharedImage*
YUVImageHost::UpdateImage(const TextureIdentifier& aTextureIdentifier,
                          const SharedImage& aImage)
{
  NS_ASSERTION(aTextureIdentifier.mBufferType == BUFFER_YUV, "BufferType mismatch.");
  
  if (aImage.type() == SharedImage::TYUVImage) {
    // update all channels at once
    const YUVImage& yuv = aImage.get_YUVImage();

    mTextures[0]->Update(SurfaceDescriptor(yuv.Ydata()));
    mTextures[1]->Update(SurfaceDescriptor(yuv.Udata()));
    mTextures[2]->Update(SurfaceDescriptor(yuv.Vdata()));

    return &aImage;
  }

  // update a single channel
  mTextures[aTextureIdentifier.mDescriptor]->Update(aImage);

  return &aImage;
}

void
YUVImageHost::Composite(EffectChain& aEffectChain,
                        float aOpacity,
                        const gfx::Matrix4x4& aTransform,
                        const gfx::Point& aOffset,
                        const gfx::Filter& aFilter,
                        const gfx::Rect& aClipRect,
                        const nsIntRegion* aVisibleRegion /* = nullptr */)
{
  mTextures[0]->Lock(aFilter);
  mTextures[1]->Lock(aFilter);
  mTextures[2]->Lock(aFilter);

  EffectYCbCr* effect = new EffectYCbCr(mTextures[0], mTextures[1], mTextures[2], aFilter);
  aEffectChain.mEffects[EFFECT_YCBCR] = effect;
  gfx::Rect rect(0, 0, mPictureRect.width, mPictureRect.height);
  gfx::Rect sourceRect(mPictureRect.x, mPictureRect.y, mPictureRect.width, mPictureRect.height);
  mCompositor->DrawQuad(rect, &sourceRect, nullptr, &aClipRect, aEffectChain, aOpacity, aTransform, aOffset);

  mTextures[0]->Unlock();
  mTextures[1]->Unlock();
  mTextures[2]->Unlock();
}

void
YUVImageHost::AddTextureHost(const TextureIdentifier& aTextureIdentifier, TextureHost* aTextureHost)
{
  NS_ASSERTION(aTextureIdentifier.mBufferType == BUFFER_YUV, "BufferType mismatch.");
  mTextures[aTextureIdentifier.mDescriptor] = aTextureHost;
}

const SharedImage*
ImageHostBridge::UpdateImage(const TextureIdentifier& aTextureIdentifier,
                             const SharedImage& aImage)
{
  // The image data will be queried at render time
  PRUint64 newID = aTextureIdentifier.mDescriptor;
  if (newID != mImageContainerID) {
    mImageContainerID = newID;
    mImageVersion = 0;
  }

  return &aImage;
}

void
ImageHostBridge::Composite(EffectChain& aEffectChain,
                           float aOpacity,
                           const gfx::Matrix4x4& aTransform,
                           const gfx::Point& aOffset,
                           const gfx::Filter& aFilter,
                           const gfx::Rect& aClipRect,
                           const nsIntRegion* aVisibleRegion /* = nullptr */)
{
  ImageContainerParent::SetCompositorIDForImage(mImageContainerID,
                                                mCompositor->GetCompositorID());
  PRUint32 imgVersion = ImageContainerParent::GetSharedImageVersion(mImageContainerID);
  if (imgVersion != mImageVersion) {
    SharedImage* img = ImageContainerParent::GetSharedImage(mImageContainerID);
    if (img &&
        img->type() == SharedImage::TYUVImage) {
      TextureIdentifier textureId;
      textureId.mBufferType = BUFFER_YUV;
      textureId.mTextureType = TEXTURE_SHMEM;
      textureId.mDescriptor = 3;
      YUVImageHost::UpdateImage(textureId, *img);
  
      mImageVersion = imgVersion;
    }
  }

  YUVImageHost::Composite(aEffectChain,
                          aOpacity,
                          aTransform,
                          aOffset,
                          aFilter,
                          aClipRect,
                          aVisibleRegion);
}

void
ImageHostBridge::AddTextureHost(const TextureIdentifier& aTextureIdentifier, TextureHost* aTextureHost)
{
  // ImageHostBridge does not have a texture host corresponding to its TextureClient,
  // so we ignore the args to this method. But we need to create the YUV texture hosts
  // for later.

  NS_ASSERTION(aTextureIdentifier.mBufferType == BUFFER_BRIDGE, "BufferType mismatch.");

  TextureIdentifier id;
  id.mBufferType = BUFFER_YUV;
  id.mTextureType = TEXTURE_SHMEM;
  mTextures[0] = mCompositor->CreateTextureHost(id, NoFlags);
  mTextures[1] = mCompositor->CreateTextureHost(id, NoFlags);
  mTextures[2] = mCompositor->CreateTextureHost(id, NoFlags);
}

}
}
