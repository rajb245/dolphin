#pragma once

#include <array>

#include "discimagekit/types.h"

namespace Common::ec
{
using Signature = std::array<u8, 60>;
using PublicKey = std::array<u8, 60>;

Signature Sign(const u8* key, const u8* hash);
bool VerifySignature(const u8* public_key, const u8* signature, const u8* hash);
PublicKey PrivToPub(const u8* key);
std::array<u8, 60> ComputeSharedSecret(const u8* private_key, const u8* public_key);
}

