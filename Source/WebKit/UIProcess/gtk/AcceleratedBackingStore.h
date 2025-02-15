/*
 * Copyright (C) 2016 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE COMPUTER, INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE COMPUTER, INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "RendererBufferFormat.h"
#include <wtf/AbstractRefCounted.h>
#include <wtf/Noncopyable.h>
#include <wtf/TZoneMalloc.h>
#include <wtf/WeakPtr.h>

typedef struct _cairo cairo_t;

#if USE(GTK4)
typedef struct _GdkSnapshot GdkSnapshot;
typedef GdkSnapshot GtkSnapshot;
#endif

namespace WebCore {
class IntRect;
class NativeImage;
}

namespace WebKit {

class LayerTreeContext;
class WebPageProxy;

class AcceleratedBackingStore : public AbstractRefCounted {
    WTF_MAKE_TZONE_ALLOCATED(AcceleratedBackingStore);
    WTF_MAKE_NONCOPYABLE(AcceleratedBackingStore);
public:
    static bool checkRequirements();
    static RefPtr<AcceleratedBackingStore> create(WebPageProxy&);
    virtual ~AcceleratedBackingStore() = default;

    virtual void update(const LayerTreeContext&) { }
#if USE(GTK4)
    virtual bool snapshot(GtkSnapshot*) = 0;
#else
    virtual bool paint(cairo_t*, const WebCore::IntRect&) = 0;
#endif
    virtual void realize() { };
    virtual void unrealize() { };
    virtual int renderHostFileDescriptor() { return -1; }
    virtual RendererBufferFormat bufferFormat() const { return { }; }
    virtual RefPtr<WebCore::NativeImage> bufferAsNativeImageForTesting() const = 0;

protected:
    explicit AcceleratedBackingStore(WebPageProxy&);

    WeakPtr<WebPageProxy> m_webPage;
};

} // namespace WebKit
