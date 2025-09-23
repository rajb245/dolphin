#include "Core/IOS/ES/Formats.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <ranges>

#include <fmt/format.h>

#include "Common/Crypto/AES.h"
#include "Common/Crypto/SHA1.h"
#include "Common/StringUtil.h"
#include "Common/Swap.h"
#include "DiscIO/DiscUtils.h"
#include "discimagekit/log.h"

namespace IOS::ES
{
namespace
{
constexpr size_t CONTENT_VIEW_SIZE = 0x10;

constexpr std::array<u8, 16> RETAIL_COMMON_KEY = {
    0xeb, 0xe4, 0x2a, 0x22, 0x5e, 0x85, 0x93, 0xe4,
    0x48, 0xd9, 0xc5, 0x45, 0x73, 0x81, 0xaa, 0xf7};
constexpr std::array<u8, 16> DEV_COMMON_KEY = {
    0xa1, 0x60, 0x4a, 0x6a, 0x71, 0x23, 0xb5, 0x29,
    0xae, 0x8b, 0xec, 0x32, 0xc8, 0x16, 0xfc, 0xaa};
constexpr std::array<u8, 16> NEW_COMMON_KEY = {
    0x63, 0xb8, 0x2b, 0xb4, 0xf4, 0x61, 0x4e, 0x2e,
    0x13, 0xf2, 0xfe, 0xfb, 0xba, 0x4c, 0x9b, 0x7e};

constexpr size_t TICKET_SIZE = sizeof(Ticket);

const Ticket* GetTicketPtr(const std::vector<u8>& bytes, size_t index)
{
  const size_t offset = index * TICKET_SIZE;
  return offset + TICKET_SIZE <= bytes.size() ?
             reinterpret_cast<const Ticket*>(bytes.data() + offset) : nullptr;
}

void CopyContent(const u8* src, Content* content)
{
  content->id = Common::swap32(src + offsetof(Content, id));
  content->index = Common::swap16(src + offsetof(Content, index));
  content->type = Common::swap16(src + offsetof(Content, type));
  content->size = Common::swap64(src + offsetof(Content, size));
  std::copy_n(src + offsetof(Content, sha1), content->sha1.size(), content->sha1.begin());
}

}  // namespace

bool Content::IsShared() const
{
  return (type & 0x8000) != 0;
}

bool Content::IsOptional() const
{
  return (type & 0x4000) != 0;
}

SignedBlobReader::SignedBlobReader(std::vector<u8> bytes) : m_bytes(std::move(bytes))
{
}

void SignedBlobReader::SetBytes(std::vector<u8> bytes)
{
  m_bytes = std::move(bytes);
}

SignatureType SignedBlobReader::GetSignatureType() const
{
  if (m_bytes.size() < sizeof(SignatureType))
    return SignatureType::RSA2048;
  return static_cast<SignatureType>(Common::swap32(m_bytes.data()));
}

size_t SignedBlobReader::GetSignatureSize() const
{
  switch (GetSignatureType())
  {
  case SignatureType::RSA4096:
    return 0x280;
  case SignatureType::RSA2048:
  default:
    return 0x180;
  }
}

std::string SignedBlobReader::GetIssuer() const
{
  const size_t signature_size = GetSignatureSize();
  if (m_bytes.size() < signature_size)
    return {};
  const char* issuer = reinterpret_cast<const char*>(m_bytes.data() + signature_size - 0x40);
  return std::string(issuer, strnlen(issuer, 0x40));
}

bool SignedBlobReader::IsSignatureValid() const
{
  return m_bytes.size() >= GetSignatureSize();
}

std::vector<u8> SignedBlobReader::GetSignatureData() const
{
  const size_t signature_size = GetSignatureSize();
  if (m_bytes.size() < signature_size)
    return {};
  return std::vector<u8>(m_bytes.begin(), m_bytes.begin() + signature_size);
}

bool IsValidTMDSize(size_t size)
{
  return size <= MAX_TMD_SIZE;
}

TMDReader::TMDReader(std::vector<u8> bytes) : SignedBlobReader(std::move(bytes))
{
}

bool TMDReader::IsValid() const
{
  if (!IsSignatureValid())
    return false;

  if (m_bytes.size() < sizeof(TMDHeader))
    return false;

  const size_t required_size = sizeof(TMDHeader) + GetNumContents() * sizeof(Content);
  return m_bytes.size() >= required_size;
}

std::vector<u8> TMDReader::GetRawView() const
{
  std::vector<u8> view;
  view.reserve(sizeof(TMDHeader) - offsetof(TMDHeader, tmd_version) +
               GetNumContents() * CONTENT_VIEW_SIZE);

  const auto* begin = m_bytes.data() + offsetof(TMDHeader, tmd_version);
  const auto* end = m_bytes.data() + offsetof(TMDHeader, access_rights);
  view.insert(view.end(), begin, end);

  const auto* version = m_bytes.data() + offsetof(TMDHeader, title_version);
  view.insert(view.end(), version, version + sizeof(TMDHeader::title_version));

  const auto* num_contents = m_bytes.data() + offsetof(TMDHeader, num_contents);
  view.insert(view.end(), num_contents, num_contents + sizeof(TMDHeader::num_contents));

  for (size_t i = 0; i < GetNumContents(); ++i)
  {
    const u8* content_base = m_bytes.data() + sizeof(TMDHeader) + i * sizeof(Content);
    view.insert(view.end(), content_base, content_base + CONTENT_VIEW_SIZE);
  }

  return view;
}

u16 TMDReader::GetBootIndex() const
{
  return Common::swap16(m_bytes.data() + offsetof(TMDHeader, boot_index));
}

u64 TMDReader::GetIOSId() const
{
  return Common::swap64(m_bytes.data() + offsetof(TMDHeader, ios_id));
}

u64 TMDReader::GetTitleId() const
{
  return Common::swap64(m_bytes.data() + offsetof(TMDHeader, title_id));
}

u32 TMDReader::GetTitleFlags() const
{
  return Common::swap32(m_bytes.data() + offsetof(TMDHeader, title_flags));
}

u16 TMDReader::GetTitleVersion() const
{
  return Common::swap16(m_bytes.data() + offsetof(TMDHeader, title_version));
}

u16 TMDReader::GetGroupId() const
{
  return Common::swap16(m_bytes.data() + offsetof(TMDHeader, group_id));
}

DiscIO::Region TMDReader::GetRegion() const
{
  if (!IsChannel(GetTitleId()))
    return DiscIO::Region::Unknown;

  if (GetTitleId() == 0x0000000100000002ULL)
    return DiscIO::GetSysMenuRegion(GetTitleVersion());

  const DiscIO::Region region =
      static_cast<DiscIO::Region>(Common::swap16(m_bytes.data() + offsetof(TMDHeader, region)));
  return region <= DiscIO::Region::NTSC_K ? region : DiscIO::Region::Unknown;
}

bool TMDReader::IsvWii() const
{
  return *(m_bytes.data() + offsetof(TMDHeader, is_vwii)) != 0;
}

std::string TMDReader::GetGameID() const
{
  char game_id[6];
  std::memcpy(game_id, m_bytes.data() + offsetof(TMDHeader, title_id) + 4, 4);
  std::memcpy(game_id + 4, m_bytes.data() + offsetof(TMDHeader, group_id), 2);

  if (std::ranges::all_of(game_id, Common::IsPrintableCharacter))
    return std::string(game_id, sizeof(game_id));

  return fmt::format("{:016x}", GetTitleId());
}

std::string TMDReader::GetGameTDBID() const
{
  const u8* begin = m_bytes.data() + offsetof(TMDHeader, title_id) + 4;
  const u8* end = begin + 4;
  if (std::all_of(begin, end, Common::IsPrintableCharacter))
    return std::string(reinterpret_cast<const char*>(begin), reinterpret_cast<const char*>(end));
  return fmt::format("{:016x}", GetTitleId());
}

u16 TMDReader::GetNumContents() const
{
  return Common::swap16(m_bytes.data() + offsetof(TMDHeader, num_contents));
}

bool TMDReader::GetContent(u16 index, Content* content) const
{
  if (!content)
    return false;
  const u8* base = m_bytes.data() + sizeof(TMDHeader) + index * sizeof(Content);
  if (base + sizeof(Content) > m_bytes.data() + m_bytes.size())
    return false;
  CopyContent(base, content);
  return true;
}

std::vector<Content> TMDReader::GetContents() const
{
  std::vector<Content> contents(GetNumContents());
  for (u16 i = 0; i < contents.size(); ++i)
    GetContent(i, &contents[i]);
  return contents;
}

bool TMDReader::FindContentById(u32 id, Content* content) const
{
  for (u16 index = 0; index < GetNumContents(); ++index)
  {
    if (!GetContent(index, content))
      return false;
    if (content->id == id)
      return true;
  }
  return false;
}

TicketReader::TicketReader(std::vector<u8> bytes) : SignedBlobReader(std::move(bytes))
{
}

bool TicketReader::IsValid() const
{
  return IsSignatureValid() && m_bytes.size() >= TICKET_SIZE;
}

bool TicketReader::IsV1Ticket() const
{
  return !m_bytes.empty() && m_bytes[offsetof(Ticket, version)] >= 2;
}

size_t TicketReader::GetNumberOfTickets() const
{
  return m_bytes.size() / TICKET_SIZE;
}

u32 TicketReader::GetTicketSize() const
{
  return static_cast<u32>(TICKET_SIZE);
}

std::vector<u8> TicketReader::GetRawTicket(u64 ticket_id_to_find) const
{
  for (size_t i = 0; i < GetNumberOfTickets(); ++i)
  {
    const Ticket* ticket = GetTicketPtr(m_bytes, i);
    if (!ticket)
      continue;
    const u8* ticket_bytes = reinterpret_cast<const u8*>(ticket);
    if (Common::swap64(ticket_bytes + offsetof(Ticket, ticket_id)) == ticket_id_to_find)
      return {ticket_bytes, ticket_bytes + TICKET_SIZE};
  }
  return {};
}

std::vector<u8> TicketReader::GetRawTicketView(u32 ticket_num) const
{
  const Ticket* ticket = GetTicketPtr(m_bytes, ticket_num);
  if (!ticket)
    return {};

  std::vector<u8> view(sizeof(TicketView));
  view[0] = GetVersion();
  const u8* source = reinterpret_cast<const u8*>(ticket) + offsetof(Ticket, ticket_id);
  std::copy_n(source, view.size() - 1, view.begin() + 1);
  return view;
}

u8 TicketReader::GetVersion() const
{
  return m_bytes[offsetof(Ticket, version)];
}

u32 TicketReader::GetDeviceId() const
{
  return Common::swap32(m_bytes.data() + offsetof(Ticket, device_id));
}

u64 TicketReader::GetTitleId() const
{
  return Common::swap64(m_bytes.data() + offsetof(Ticket, title_id));
}

u8 TicketReader::GetCommonKeyIndex() const
{
  return m_bytes[offsetof(Ticket, common_key_index)];
}

std::array<u8, 16> TicketReader::DecryptTitleKey(HLE::IOSC::ConsoleType console_type,
                                                 u8 key_index) const
{
  std::array<u8, 16> key{};
  const std::array<u8, 16>* common_key = nullptr;
  switch (key_index)
  {
  case 0:
    common_key = console_type == HLE::IOSC::ConsoleType::RVT ? &DEV_COMMON_KEY : &RETAIL_COMMON_KEY;
    break;
  case 1:
    common_key = &NEW_COMMON_KEY;
    break;
  default:
    dik::log_warn("Unsupported common key index {} for title {:016x}; falling back to index 0",
                  static_cast<int>(key_index), GetTitleId());
    common_key = console_type == HLE::IOSC::ConsoleType::RVT ? &DEV_COMMON_KEY : &RETAIL_COMMON_KEY;
    break;
  }

  auto aes = Common::AES::CreateContextDecrypt(common_key->data());
  std::array<u8, 16> iv{};
  std::copy_n(m_bytes.data() + offsetof(Ticket, title_id), sizeof(Ticket::title_id), iv.begin());

  std::array<u8, 16> encrypted{};
  std::copy_n(m_bytes.data() + offsetof(Ticket, title_key), encrypted.size(), encrypted.begin());
  aes->Crypt(iv.data(), encrypted.data(), key.data(), encrypted.size());
  return key;
}

std::array<u8, 16> TicketReader::GetTitleKey() const
{
  return DecryptTitleKey(GetConsoleType(), GetCommonKeyIndex());
}

HLE::IOSC::ConsoleType TicketReader::GetConsoleType() const
{
  return GetIssuer() == "Root-CA00000002-XS00000006" ? HLE::IOSC::ConsoleType::RVT :
                                                       HLE::IOSC::ConsoleType::Retail;
}

void TicketReader::DeleteTicket(u64 ticket_id_to_delete)
{
  std::vector<u8> new_ticket;
  new_ticket.reserve(m_bytes.size());
  for (size_t i = 0; i < GetNumberOfTickets(); ++i)
  {
    const Ticket* ticket = GetTicketPtr(m_bytes, i);
    if (!ticket)
      continue;
    const u8* ticket_bytes = reinterpret_cast<const u8*>(ticket);
    if (Common::swap64(ticket_bytes + offsetof(Ticket, ticket_id)) == ticket_id_to_delete)
      continue;
    new_ticket.insert(new_ticket.end(), ticket_bytes, ticket_bytes + TICKET_SIZE);
  }
  m_bytes = std::move(new_ticket);
}

void TicketReader::OverwriteCommonKeyIndex(u8 index)
{
  if (GetNumberOfTickets() == 0)
    return;
  m_bytes[offsetof(Ticket, common_key_index)] = index;
}

bool IsTitleType(u64 title_id, u32 title_type)
{
  return static_cast<u32>(title_id >> 32) == title_type;
}

bool IsChannel(u64 title_id)
{
  constexpr u32 CHANNEL = 0x00010001;
  constexpr u32 SYSTEM_CHANNEL = 0x00010002;
  constexpr u32 GAME_WITH_CHANNEL = 0x00010004;
  constexpr u32 HIDDEN_CHANNEL = 0x00010008;

  if (title_id == 0x0000000100000002ULL)
    return true;

  const u32 type = static_cast<u32>(title_id >> 32);
  return type == CHANNEL || type == SYSTEM_CHANNEL || type == GAME_WITH_CHANNEL || type == HIDDEN_CHANNEL;
}

}  // namespace ES
