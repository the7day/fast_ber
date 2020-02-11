﻿#pragma once

#include "fast_ber/ber_types/Identifier.hpp"
#include "fast_ber/util/Definitions.hpp"

#include "absl/types/span.h"

#include <cstddef> // for uint8_t
#include <cstring> // for std::memmove

namespace fast_ber
{

struct EncodeResult
{
    bool   success;
    size_t length;
};

template <Class T1, Tag T2>
EncodeResult encode_header(absl::Span<uint8_t> buffer, size_t content_length, Id<T1, T2> id)
{
    size_t header_length = encode_header(buffer, Construction::constructed, id.class_(), id.tag(), content_length);
    if (header_length == 0)
    {
        return EncodeResult{false, 0};
    }
    return EncodeResult{true, header_length + content_length};
}

template <typename OuterId, typename InnerId>
EncodeResult encode_header(absl::Span<uint8_t> buffer, size_t content_length, DoubleId<OuterId, InnerId> id)
{
    size_t inner_header_length = encoded_header_length(content_length, id.inner_id());
    size_t outer_header_length = encoded_header_length(inner_header_length + content_length, id.outer_id());

    if (buffer.size() < outer_header_length + inner_header_length + content_length)
    {
        return EncodeResult{false, 0};
    }

    auto inner = buffer;
    inner.remove_prefix(outer_header_length);

    encode_header(inner, content_length, id.inner_id());
    return encode_header(buffer, inner_header_length + content_length, id.outer_id());
}

template <Class T1, Tag T2>
constexpr size_t wrap_with_ber_header_length(size_t content_length, Id<T1, T2> id)
{
    return encoded_header_length(content_length, id) + content_length;
}
template <typename OuterId, typename InnerId>
constexpr size_t wrap_with_ber_header_length(size_t content_length, DoubleId<OuterId, InnerId> id)
{
    return wrap_with_ber_header_length(wrap_with_ber_header_length(content_length, id.inner_id()), id.outer_id()) +
           content_length;
}

// Creates a BER header with provided ID around the data currently in the buffer
// The data of interest should be located in the buffer, with length "content_length"

// An offset if the data in the buffer can specified by "content_offset"
// Building the data at a specfic content_offset will remove the need for moving memory after the header
// has been added.

template <typename Identifier>
EncodeResult wrap_with_ber_header(absl::Span<uint8_t> buffer, size_t content_length, Identifier id,
                                  size_t content_offset = 0)
{
    size_t header_length = encoded_header_length(Construction::primitive, id.class_(), id.tag(), content_length);
    if (header_length + content_length > buffer.length())
    {
        return EncodeResult{false, 0};
    }

    assert(buffer.length() >= content_length + content_offset);
    if (content_offset != header_length)
    {
        std::memmove(buffer.data() + header_length, buffer.data() + content_offset, content_length);
    }
    return encode_header(buffer, content_length, id);
}

template <typename T, Class T2, Tag T3>
EncodeResult encode_impl(absl::Span<uint8_t> output, const T& object, Id<T2, T3> id)
{
    size_t id_length = encode_identifier(output, Construction::primitive, id.class_(), id.tag());
    if (id_length == 0)
    {
        return EncodeResult{false, 0};
    }
    assert(id_length <= output.size());

    output.remove_prefix(id_length);

    EncodeResult encode_res = object.encode_content_and_length(output);
    encode_res.length += id_length;
    return encode_res;
}

template <typename T, typename OuterId, typename InnerId>
EncodeResult encode_impl(absl::Span<uint8_t> output, const T& object, DoubleId<OuterId, InnerId> id)
{
    const auto header_length_guess = 2;
    auto       inner_buffer        = output;
    if (output.length() < header_length_guess)
    {
        return EncodeResult{false, 0};
    }
    inner_buffer.remove_prefix(header_length_guess);
    EncodeResult inner_encoding = encode_impl(inner_buffer, object, id.inner_id());
    if (!inner_encoding.success)
    {
        return EncodeResult{false, 0};
    }

    return wrap_with_ber_header(output, inner_encoding.length, id.outer_id(), header_length_guess);
}

} // namespace fast_ber
