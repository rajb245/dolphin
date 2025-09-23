#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "Common/CommonTypes.h"
#include "DiscIO/Enums.h"
#include "Core/IOS/IOSC.h"

namespace IOS
{
namespace ES
{
enum class SignatureType : u32
{
  RSA4096 = 0x00010000,
  RSA2048 = 0x00010001,
};

enum class PublicKeyType : u32
{
  RSA2048 = 1,
};

#pragma pack(push, 4)
struct SignatureRSA2048
{
  SignatureType type;
  u8 sig[0x100];
  u8 fill[0x3c];
  char issuer[0x40];
};
static_assert(sizeof(SignatureRSA2048) == 0x180, "Wrong size for SignatureRSA2048");

struct TMDHeader
{
  SignatureRSA2048 signature;
  u8 tmd_version;
  u8 ca_crl_version;
  u8 signer_crl_version;
  u8 is_vwii;
  u64 ios_id;
  u64 title_id;
  u32 title_flags;
  u16 group_id;
  u16 zero;
  u16 region;
  u8 ratings[16];
  u8 reserved[12];
  u8 ipc_mask[12];
  u8 reserved2[18];
  u32 access_rights;
  u16 title_version;
  u16 num_contents;
  u16 boot_index;
  u16 fill2;
};
static_assert(sizeof(TMDHeader) == 0x1e4, "TMDHeader has the wrong size");
static_assert(offsetof(TMDHeader, ios_id) == 0x184);

struct Content
{
  bool IsShared() const;
  bool IsOptional() const;
  u32 id;
  u16 index;
  u16 type;
  u64 size;
  std::array<u8, 20> sha1;
};
static_assert(sizeof(Content) == 36, "Content has the wrong size");

struct TimeLimit
{
  u32 enabled;
  u32 seconds;
};

struct TicketView
{
  u8 version;
  u64 ticket_id;
  u32 device_id;
  u64 title_id;
  u16 access_mask;
  u32 permitted_title_id;
  u32 permitted_title_mask;
  u8 title_export_allowed;
  u8 common_key_index;
  u8 unknown2[0x30];
  u8 content_access_permissions[0x40];
  TimeLimit time_limits[8];
};
static_assert(sizeof(TicketView) == 0xd8, "TicketView has the wrong size");

struct Ticket
{
  SignatureRSA2048 signature;
  u8 server_public_key[0x3c];
  u8 version;
  u8 ca_crl_version;
  u8 signer_crl_version;
  u8 title_key[0x10];
  u64 ticket_id;
  u32 device_id;
  u64 title_id;
  u16 access_mask;
  u16 ticket_version;
  u32 permitted_title_id;
  u32 permitted_title_mask;
  u8 title_export_allowed;
  u8 common_key_index;
  u8 unknown2[0x30];
  u8 content_access_permissions[0x40];
  TimeLimit time_limits[8];
};
static_assert(sizeof(Ticket) == 0x2A4, "Ticket has the wrong size");
#pragma pack(pop)

constexpr u32 MAX_TMD_SIZE = 0x49e4;

class SignedBlobReader
{
public:
  SignedBlobReader() = default;
  explicit SignedBlobReader(std::vector<u8> bytes);

  const std::vector<u8>& GetBytes() const { return m_bytes; }
  void SetBytes(std::vector<u8> bytes);

  SignatureType GetSignatureType() const;
  size_t GetSignatureSize() const;
  std::string GetIssuer() const;
  bool IsSignatureValid() const;
  std::vector<u8> GetSignatureData() const;

protected:
  std::vector<u8> m_bytes;
};

bool IsValidTMDSize(size_t size);

class TMDReader final : public SignedBlobReader
{
public:
  TMDReader() = default;
  explicit TMDReader(std::vector<u8> bytes);

  bool IsValid() const;

  std::vector<u8> GetRawView() const;

  u16 GetBootIndex() const;
  u64 GetIOSId() const;
  u64 GetTitleId() const;
  u32 GetTitleFlags() const;
  u16 GetTitleVersion() const;
  u16 GetGroupId() const;
  DiscIO::Region GetRegion() const;
  bool IsvWii() const;
  std::string GetGameID() const;
  std::string GetGameTDBID() const;

  u16 GetNumContents() const;
  bool GetContent(u16 index, Content* content) const;
  std::vector<Content> GetContents() const;
  bool FindContentById(u32 id, Content* content) const;
};

class TicketReader final : public SignedBlobReader
{
public:
  TicketReader() = default;
  explicit TicketReader(std::vector<u8> bytes);

  bool IsValid() const;
  bool IsV1Ticket() const;

  size_t GetNumberOfTickets() const;
  u32 GetTicketSize() const;

  std::vector<u8> GetRawTicket(u64 ticket_id_to_find) const;
  std::vector<u8> GetRawTicketView(u32 ticket_num) const;

  u8 GetVersion() const;
  u32 GetDeviceId() const;
  u64 GetTitleId() const;
  u8 GetCommonKeyIndex() const;

  std::array<u8, 16> GetTitleKey() const;
  HLE::IOSC::ConsoleType GetConsoleType() const;

  void DeleteTicket(u64 ticket_id_to_delete);
  void OverwriteCommonKeyIndex(u8 index);

private:
  std::array<u8, 16> DecryptTitleKey(HLE::IOSC::ConsoleType console_type, u8 key_index) const;
};

bool IsTitleType(u64 title_id, u32 title_type);
bool IsChannel(u64 title_id);

}  // namespace ES
}  // namespace IOS
