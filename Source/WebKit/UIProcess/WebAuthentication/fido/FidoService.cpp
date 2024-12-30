/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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
#include "FidoService.h"

#if ENABLE(WEB_AUTHN)

#include "CtapAuthenticator.h"
#include "CtapDriver.h"
#include "Logging.h"
#include "U2fAuthenticator.h"
#include <WebCore/DeviceRequestConverter.h>
#include <WebCore/DeviceResponseConverter.h>
#include <WebCore/FidoConstants.h>
#include <WebCore/FidoHidMessage.h>
#include <wtf/RunLoop.h>
#include <wtf/TZoneMallocInlines.h>
#include <wtf/text/Base64.h>

#define CTAP_RELEASE_LOG(fmt, ...) RELEASE_LOG(WebAuthn, "%p - FidoService::" fmt, this, ##__VA_ARGS__)

namespace WebKit {
using namespace fido;

WTF_MAKE_TZONE_ALLOCATED_IMPL(FidoService);

FidoService::FidoService(AuthenticatorTransportServiceObserver& observer)
    : AuthenticatorTransportService(observer)
{
}

void FidoService::getInfo(Ref<CtapDriver>&& driver)
{
    // Get authenticator info from the device.
    driver->transact(encodeEmptyAuthenticatorRequest(CtapRequestCommand::kAuthenticatorGetInfo), [weakThis = WeakPtr { *this }, weakDriver = WeakPtr { driver.get() }] (Vector<uint8_t>&& response) mutable {
        ASSERT(RunLoop::isMain());
        if (!weakThis)
            return;
        weakThis->continueAfterGetInfo(WTFMove(weakDriver), WTFMove(response));
    });
    auto addResult = m_drivers.add(WTFMove(driver));
    ASSERT_UNUSED(addResult, addResult.isNewEntry);
}

void FidoService::continueAfterGetInfo(WeakPtr<CtapDriver>&& weakDriver, Vector<uint8_t>&& response)
{
    if (!weakDriver)
        return;

    RefPtr driver = m_drivers.take(weakDriver.get());
    if (!driver || !observer() || response.isEmpty())
        return;

    CTAP_RELEASE_LOG("Got response from getInfo: %s", base64EncodeToString(response).utf8().data());

    auto info = readCTAPGetInfoResponse(response);
    if (info && info->versions().find(ProtocolVersion::kCtap2) != info->versions().end()) {
        driver->setMaxMsgSize(info->maxMsgSize());
        observer()->authenticatorAdded(CtapAuthenticator::create(driver.releaseNonNull(), WTFMove(*info)));
        return;
    }
    driver->setProtocol(ProtocolVersion::kU2f);
    observer()->authenticatorAdded(U2fAuthenticator::create(driver.releaseNonNull()));
}

} // namespace WebKit

#endif // ENABLE(WEB_AUTHN)
