// Copyright 2017 Dolphin Emulator Project
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#ifdef USE_UPNP

#include "discimagekit/types.h"

namespace Common::UPnP
{
void TryPortmapping(u16 port);
void StopPortmapping();
}  // namespace Common::UPnP

#endif
