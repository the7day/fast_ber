#pragma once

#include "fast_ber/ber_types/OctetString.hpp"
#include "fast_ber/ber_types/Tag.hpp"
#include "fast_ber/ber_types/TaggedType.hpp"

namespace fast_ber
{
template <typename Identifier = ExplicitIdentifier<UniversalTag::bmp_string>>
using BMPString = fast_ber::OctetString<Identifier>;
}
