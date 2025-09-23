#include "Common/Crypto/AES.h"
#include "Common/Crypto/SHA1.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <memory>

#include <mbedtls/aes.h>
#include <mbedtls/sha1.h>

namespace
{
class MbedAesContext final : public Common::AES::Context
{
public:
  explicit MbedAesContext(Common::AES::Mode mode, const u8* key) : m_mode(mode)
  {
    mbedtls_aes_init(&m_ctx);
    if (m_mode == Common::AES::Mode::Encrypt)
      mbedtls_aes_setkey_enc(&m_ctx, key, KEY_SIZE * 8);
    else
      mbedtls_aes_setkey_dec(&m_ctx, key, KEY_SIZE * 8);
  }

  ~MbedAesContext() override { mbedtls_aes_free(&m_ctx); }

  bool Crypt(const u8* iv, u8* iv_out, const u8* buf_in, u8* buf_out, size_t len) const override
  {
    std::array<u8, BLOCK_SIZE> iv_tmp{};
    if (iv)
      std::memcpy(iv_tmp.data(), iv, BLOCK_SIZE);

    if (len % BLOCK_SIZE != 0)
      return false;

    if (mbedtls_aes_crypt_cbc(const_cast<mbedtls_aes_context*>(&m_ctx),
                               m_mode == Common::AES::Mode::Encrypt ? MBEDTLS_AES_ENCRYPT :
                                                                       MBEDTLS_AES_DECRYPT,
                               len, iv ? iv_tmp.data() : nullptr, buf_in, buf_out) != 0)
    {
      return false;
    }

    if (iv_out)
      std::memcpy(iv_out, iv_tmp.data(), BLOCK_SIZE);

    return true;
  }

private:
  Common::AES::Mode m_mode;
  mutable mbedtls_aes_context m_ctx;
};

class MbedSha1Context final : public Common::SHA1::Context
{
public:
  MbedSha1Context() { mbedtls_sha1_init(&m_ctx); mbedtls_sha1_starts_ret(&m_ctx); }
  ~MbedSha1Context() override { mbedtls_sha1_free(&m_ctx); }

  void Update(const u8* msg, size_t len) override { mbedtls_sha1_update_ret(&m_ctx, msg, len); }

  Common::SHA1::Digest Finish() override
  {
    Common::SHA1::Digest digest;
    mbedtls_sha1_finish_ret(&m_ctx, digest.data());
    return digest;
  }

  bool HwAccelerated() const override { return false; }

private:
  mbedtls_sha1_context m_ctx;
};
}

namespace Common::AES
{
std::unique_ptr<Context> CreateContextEncrypt(const u8* key)
{
  return std::make_unique<MbedAesContext>(Mode::Encrypt, key);
}

std::unique_ptr<Context> CreateContextDecrypt(const u8* key)
{
  return std::make_unique<MbedAesContext>(Mode::Decrypt, key);
}

void CryptOFB(const u8* key, const u8* iv, u8* iv_out, const u8* buf_in, u8* buf_out, size_t size)
{
  mbedtls_aes_context ctx;
  mbedtls_aes_init(&ctx);
  mbedtls_aes_setkey_enc(&ctx, key, Context::KEY_SIZE * 8);

  std::array<u8, Context::BLOCK_SIZE> feedback{};
  std::memcpy(feedback.data(), iv, feedback.size());

  size_t offset = 0;
  while (offset < size)
  {
    std::array<u8, Context::BLOCK_SIZE> stream{};
    mbedtls_aes_crypt_ecb(&ctx, MBEDTLS_AES_ENCRYPT, feedback.data(), stream.data());

    const size_t chunk = std::min(Context::BLOCK_SIZE, size - offset);
    for (size_t i = 0; i < chunk; ++i)
      buf_out[offset + i] = buf_in[offset + i] ^ stream[i];

    std::memcpy(feedback.data(), stream.data(), Context::BLOCK_SIZE);
    offset += chunk;
  }

  if (iv_out)
    std::memcpy(iv_out, feedback.data(), feedback.size());

  mbedtls_aes_free(&ctx);
}
}  // namespace Common::AES

namespace Common::SHA1
{
std::unique_ptr<Context> CreateContext()
{
  return std::make_unique<MbedSha1Context>();
}

Digest CalculateDigest(const u8* msg, size_t len)
{
  auto ctx = CreateContext();
  ctx->Update(msg, len);
  return ctx->Finish();
}

std::string DigestToString(const Digest& digest)
{
  static constexpr char hex[] = "0123456789abcdef";
  std::string result;
  result.reserve(DIGEST_LEN * 2);
  for (u8 byte : digest)
  {
    result.push_back(hex[byte >> 4]);
    result.push_back(hex[byte & 0xF]);
  }
  return result;
}
}  // namespace Common::SHA1
