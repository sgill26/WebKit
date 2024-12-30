/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"
#include "AppleVisualEffect.h"

#if HAVE(CORE_MATERIAL)

#include <wtf/text/TextStream.h>

namespace WebCore {

bool appleVisualEffectNeedsBackdrop(AppleVisualEffect effect)
{
    switch (effect) {
    case AppleVisualEffect::BlurUltraThinMaterial:
    case AppleVisualEffect::BlurThinMaterial:
    case AppleVisualEffect::BlurMaterial:
    case AppleVisualEffect::BlurThickMaterial:
    case AppleVisualEffect::BlurChromeMaterial:
        return true;
    case AppleVisualEffect::None:
    case AppleVisualEffect::VibrancyLabel:
    case AppleVisualEffect::VibrancySecondaryLabel:
    case AppleVisualEffect::VibrancyTertiaryLabel:
    case AppleVisualEffect::VibrancyQuaternaryLabel:
    case AppleVisualEffect::VibrancyFill:
    case AppleVisualEffect::VibrancySecondaryFill:
    case AppleVisualEffect::VibrancyTertiaryFill:
    case AppleVisualEffect::VibrancySeparator:
        return false;
    }

    ASSERT_NOT_REACHED();
    return false;
}

TextStream& operator<<(TextStream& ts, AppleVisualEffect effect)
{
    switch (effect) {
    case AppleVisualEffect::None:
        ts << "none";
        break;
    case AppleVisualEffect::BlurUltraThinMaterial:
        ts << "blur-material-ultra-thin";
        break;
    case AppleVisualEffect::BlurThinMaterial:
        ts << "blur-material-thin";
        break;
    case AppleVisualEffect::BlurMaterial:
        ts << "blur-material";
        break;
    case AppleVisualEffect::BlurThickMaterial:
        ts << "blur-material-thick";
        break;
    case AppleVisualEffect::BlurChromeMaterial:
        ts << "blur-material-chrome";
        break;
    case AppleVisualEffect::VibrancyLabel:
        ts << "vibrancy-label";
        break;
    case AppleVisualEffect::VibrancySecondaryLabel:
        ts << "vibrancy-secondary-label";
        break;
    case AppleVisualEffect::VibrancyTertiaryLabel:
        ts << "vibrancy-tertiary-label";
        break;
    case AppleVisualEffect::VibrancyQuaternaryLabel:
        ts << "vibrancy-quaternary-label";
        break;
    case AppleVisualEffect::VibrancyFill:
        ts << "vibrancy-fill";
        break;
    case AppleVisualEffect::VibrancySecondaryFill:
        ts << "vibrancy-secondary-fill";
        break;
    case AppleVisualEffect::VibrancyTertiaryFill:
        ts << "vibrancy-tertiary-fill";
        break;
    case AppleVisualEffect::VibrancySeparator:
        ts << "vibrancy-separator";
        break;
    }
    return ts;
}

} // namespace WebCore

#endif // HAVE(CORE_MATERIAL)
