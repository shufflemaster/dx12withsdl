#pragma once

#include <entt\entt.hpp>

namespace GAL
{
    using Registry = entt::registry<>;
    using EntityId = Registry::entity_type;
    using ResourceId = entt::hashed_string::hash_type;
}