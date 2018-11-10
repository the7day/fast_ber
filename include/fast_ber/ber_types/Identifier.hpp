#pragma once

#include "fast_ber/ber_types/Class.hpp"
#include "fast_ber/ber_types/Construction.hpp"
#include "fast_ber/ber_types/Tag.hpp"

namespace fast_ber
{

// Class is always universal
struct ExplicitIdentifier
{
    static const Class class_ = Class::universal;
    UniversalTag       tag;
};

// Inner class is always universal
struct TaggedExplicitIdentifier
{
    Class              outer_class;
    Tag                outer_tag;
    static const Class inner_class = Class::universal;
    UniversalTag       inner_tag;
};

// Any class or tag is valid
struct ImplicitIdentifier
{
    Class class_;
    Tag   tag;
};

template <typename ID1, typename ID2>
struct SequenceId
{
    ID1 sequence_id;
    ID2 inner_id;
};

inline Tag reference_tag(const ExplicitIdentifier& id) { return val(id.tag); }
inline Tag reference_tag(const TaggedExplicitIdentifier& id) { return id.outer_tag; }
inline Tag reference_tag(const ImplicitIdentifier& id) { return id.tag; }
template <typename ID1, typename ID2>
Tag reference_tag(const SequenceId<ID1, ID2>& id)
{
    return reference_tag(id.sequence_id);
}

} // namespace fast_ber