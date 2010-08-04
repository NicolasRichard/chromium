/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#if USE(ACCELERATED_COMPOSITING)

#include "LayerChromium.h"

#include "LayerRendererChromium.h"
#if PLATFORM(SKIA)
#include "NativeImageSkia.h"
#include "PlatformContextSkia.h"
#endif
#include "RenderLayerBacking.h"
#include "skia/ext/platform_canvas.h"

#include <GLES2/gl2.h>

namespace WebCore {

using namespace std;

unsigned LayerChromium::m_shaderProgramId = 0;

PassRefPtr<LayerChromium> LayerChromium::create(GraphicsLayerChromium* owner)
{
    return adoptRef(new LayerChromium(owner));
}

LayerChromium::LayerChromium(GraphicsLayerChromium* owner)
    : m_owner(owner)
    , m_contentsDirty(false)
    , m_superlayer(0)
    , m_anchorPoint(0.5, 0.5)
    , m_backgroundColor(0, 0, 0, 0)
    , m_borderColor(0, 0, 0, 0)
    , m_layerRenderer(0)
    , m_edgeAntialiasingMask(0)
    , m_opacity(1.0)
    , m_zPosition(0.0)
    , m_anchorPointZ(0)
    , m_borderWidth(0)
    , m_allocatedTextureId(0)
    , m_clearsContext(false)
    , m_doubleSided(true)
    , m_hidden(false)
    , m_masksToBounds(false)
    , m_opaque(true)
    , m_geometryFlipped(false)
    , m_needsDisplayOnBoundsChange(false)
{
}

LayerChromium::~LayerChromium()
{
    // Our superlayer should be holding a reference to us so there should be no
    // way for us to be destroyed while we still have a superlayer.
    ASSERT(!superlayer());

    // Remove the superlayer reference from all sublayers.
    removeAllSublayers();

    // Notify the renderer to clean up the texture associated with the layer.
    if (m_layerRenderer)
        m_layerRenderer->freeLayerTexture(this);
}

void LayerChromium::setLayerRenderer(LayerRendererChromium* renderer)
{
    // It's not expected that layers will ever switch renderers.
    ASSERT(!renderer || !m_layerRenderer || renderer == m_layerRenderer);

    m_layerRenderer = renderer;
}

void LayerChromium::updateTextureContents(unsigned textureId)
{
    RenderLayerBacking* backing = static_cast<RenderLayerBacking*>(m_owner->client());
    if (!backing || backing->paintingGoesToWindow())
        return;

    ASSERT(drawsContent());

    void* pixels = 0;
    IntRect dirtyRect(m_dirtyRect);
    IntSize requiredTextureSize;
    IntSize bitmapSize;

#if PLATFORM(SKIA)
    const SkBitmap* skiaBitmap = 0;
    OwnPtr<skia::PlatformCanvas> canvas;
    OwnPtr<PlatformContextSkia> skiaContext;
    OwnPtr<GraphicsContext> graphicsContext;

    requiredTextureSize = m_bounds;
    IntRect boundsRect(IntPoint(0, 0), m_bounds);

    // If the texture needs to be reallocated then we must redraw the entire
    // contents of the layer.
    if (requiredTextureSize != m_allocatedTextureSize)
        dirtyRect = boundsRect;
    else {
        // Clip the dirtyRect to the size of the layer to avoid drawing outside
        // the bounds of the backing texture.
        dirtyRect.intersect(boundsRect);
    }

    canvas.set(new skia::PlatformCanvas(dirtyRect.width(), dirtyRect.height(), false));
    skiaContext.set(new PlatformContextSkia(canvas.get()));

#if OS(WINDOWS)
    // This is needed to get text to show up correctly. Without it,
    // GDI renders with zero alpha and the text becomes invisible.
    // Unfortunately, setting this to true disables cleartype.
    // FIXME: Does this take us down a very slow text rendering path?
    // FIXME: why is this is a windows-only call ?
    skiaContext->setDrawingToImageBuffer(true);
#endif

    graphicsContext.set(new GraphicsContext(reinterpret_cast<PlatformGraphicsContext*>(skiaContext.get())));

    // Bring the canvas into the coordinate system of the paint rect.
    canvas->translate(static_cast<SkScalar>(-dirtyRect.x()), static_cast<SkScalar>(-dirtyRect.y()));

    m_owner->paintGraphicsLayerContents(*graphicsContext, dirtyRect);
    const SkBitmap& bitmap = canvas->getDevice()->accessBitmap(false);
    skiaBitmap = &bitmap;
    ASSERT(skiaBitmap);

    SkAutoLockPixels lock(*skiaBitmap);
    SkBitmap::Config skiaConfig = skiaBitmap->config();
    // FIXME: do we need to support more image configurations?
    if (skiaConfig == SkBitmap::kARGB_8888_Config) {
        pixels = skiaBitmap->getPixels();
        bitmapSize = IntSize(skiaBitmap->width(), skiaBitmap->height());
    }
#elif PLATFORM(CG)
    requiredTextureSize = m_bounds;
    IntRect boundsRect(IntPoint(0, 0), m_bounds);

    // If the texture needs to be reallocated then we must redraw the entire
    // contents of the layer.
    if (requiredTextureSize != m_allocatedTextureSize)
        dirtyRect = boundsRect;
    else {
        // Clip the dirtyRect to the size of the layer to avoid drawing outside
        // the bounds of the backing texture.
        dirtyRect.intersect(boundsRect);
    }

    Vector<uint8_t> tempVector;
    int rowBytes = 4 * dirtyRect.width();
    tempVector.resize(rowBytes * dirtyRect.height());
    memset(tempVector.data(), 0, tempVector.size());
    // FIXME: unsure whether this is the best color space choice.
    RetainPtr<CGColorSpaceRef> colorSpace(AdoptCF, CGColorSpaceCreateWithName(kCGColorSpaceGenericRGBLinear));
    RetainPtr<CGContextRef> contextCG(AdoptCF, CGBitmapContextCreate(tempVector.data(),
                                                                     dirtyRect.width(), dirtyRect.height(), 8, rowBytes,
                                                                     colorSpace.get(),
                                                                     kCGImageAlphaPremultipliedLast));

    GraphicsContext graphicsContext(contextCG.get());

    // Translate the graphics contxt into the coordinate system of the dirty rect.
    graphicsContext.translate(-dirtyRect.x(), -dirtyRect.y());

    m_owner->paintGraphicsLayerContents(graphicsContext, dirtyRect);

    pixels = tempVector.data();
    bitmapSize = dirtyRect.size();
#else
#error "Need to implement for your platform."
#endif

    if (pixels)
        updateTextureRect(pixels, bitmapSize, requiredTextureSize,  dirtyRect, textureId);
}

void LayerChromium::updateTextureRect(void* pixels, const IntSize& bitmapSize, const IntSize& requiredTextureSize, const IntRect& updateRect, unsigned textureId)
{
    if (!pixels)
        return;

    glBindTexture(GL_TEXTURE_2D, textureId);
    // If the texture id or size changed since last time then we need to tell GL
    // to re-allocate a texture.
    if (m_allocatedTextureId != textureId || requiredTextureSize != m_allocatedTextureSize) {
        ASSERT(bitmapSize == requiredTextureSize);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, requiredTextureSize.width(), requiredTextureSize.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

        m_allocatedTextureId = textureId;
        m_allocatedTextureSize = requiredTextureSize;
    } else {
        ASSERT(updateRect.width() <= m_allocatedTextureSize.width() && updateRect.height() <= m_allocatedTextureSize.height());
        ASSERT(updateRect.width() == bitmapSize.width() && updateRect.height() == bitmapSize.height());
#if PLATFORM(CG)
        // The origin is at the lower left in Core Graphics' coordinate system. We need to correct for this here.
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        updateRect.x(), m_allocatedTextureSize.height() - updateRect.height() - updateRect.y(),
                        updateRect.width(), updateRect.height(),
                        GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#elif PLATFORM(SKIA)
        glTexSubImage2D(GL_TEXTURE_2D, 0, updateRect.x(), updateRect.y(), updateRect.width(), updateRect.height(), GL_RGBA, GL_UNSIGNED_BYTE, pixels);
#else
#error "Need to implement for your platform."
#endif
    }

    m_dirtyRect.setSize(FloatSize());
    m_contentsDirty = false;
}

void LayerChromium::setNeedsCommit()
{
    // Call notifySyncRequired(), which in this implementation plumbs through to
    // call setRootLayerNeedsDisplay() on the WebView, which will cause LayerRendererChromium
    // to render a frame.
    if (m_owner)
        m_owner->notifySyncRequired();
}

void LayerChromium::addSublayer(PassRefPtr<LayerChromium> sublayer)
{
    insertSublayer(sublayer, numSublayers());
}

void LayerChromium::insertSublayer(PassRefPtr<LayerChromium> sublayer, size_t index)
{
    index = min(index, m_sublayers.size());
    sublayer->removeFromSuperlayer();
    sublayer->setSuperlayer(this);
    m_sublayers.insert(index, sublayer);
    setNeedsCommit();
}

void LayerChromium::removeFromSuperlayer()
{
    if (m_superlayer)
        m_superlayer->removeSublayer(this);
}

void LayerChromium::removeSublayer(LayerChromium* sublayer)
{
    int foundIndex = indexOfSublayer(sublayer);
    if (foundIndex == -1)
        return;

    sublayer->setSuperlayer(0);
    m_sublayers.remove(foundIndex);
    setNeedsCommit();
}

void LayerChromium::replaceSublayer(LayerChromium* reference, PassRefPtr<LayerChromium> newLayer)
{
    ASSERT_ARG(reference, reference);
    ASSERT_ARG(reference, reference->superlayer() == this);

    if (reference == newLayer)
        return;

    int referenceIndex = indexOfSublayer(reference);
    if (referenceIndex == -1) {
        ASSERT_NOT_REACHED();
        return;
    }

    reference->removeFromSuperlayer();

    if (newLayer) {
        newLayer->removeFromSuperlayer();
        insertSublayer(newLayer, referenceIndex);
    }
}

int LayerChromium::indexOfSublayer(const LayerChromium* reference)
{
    for (size_t i = 0; i < m_sublayers.size(); i++) {
        if (m_sublayers[i] == reference)
            return i;
    }
    return -1;
}

void LayerChromium::setBounds(const IntSize& size)
{
    if (m_bounds == size)
        return;

    m_bounds = size;
    m_backingStoreSize = size;

    setNeedsCommit();
}

void LayerChromium::setFrame(const FloatRect& rect)
{
    if (rect == m_frame)
      return;

    m_frame = rect;
    setNeedsCommit();
}

const LayerChromium* LayerChromium::rootLayer() const
{
    const LayerChromium* layer = this;
    for (LayerChromium* superlayer = layer->superlayer(); superlayer; layer = superlayer, superlayer = superlayer->superlayer()) { }
    return layer;
}

void LayerChromium::removeAllSublayers()
{
    while (m_sublayers.size()) {
        LayerChromium* layer = m_sublayers[0].get();
        ASSERT(layer->superlayer());
        layer->removeFromSuperlayer();
    }
}

void LayerChromium::setSublayers(const Vector<RefPtr<LayerChromium> >& sublayers)
{
    if (sublayers == m_sublayers)
        return;

    removeAllSublayers();
    size_t listSize = sublayers.size();
    for (size_t i = 0; i < listSize; i++)
        addSublayer(sublayers[i]);
}

LayerChromium* LayerChromium::superlayer() const
{
    return m_superlayer;
}

void LayerChromium::setNeedsDisplay(const FloatRect& dirtyRect)
{
    // Simply mark the contents as dirty. The actual redraw will
    // happen when it's time to do the compositing.
    m_contentsDirty = true;

    m_dirtyRect.unite(dirtyRect);

    setNeedsCommit();
}

void LayerChromium::setNeedsDisplay()
{
    m_dirtyRect.setSize(m_bounds);
    m_contentsDirty = true;
}

}
#endif // USE(ACCELERATED_COMPOSITING)
