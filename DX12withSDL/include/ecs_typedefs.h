#pragma once

#include <entt\entt.hpp>

namespace GAL
{
    using Registry = entt::registry<>;
    using EntityId = Registry::entity_type;
    constexpr EntityId NullEntity = entt::null;
    using ResourceHandle = int; //A negative value means invalid handle
    using EventDispatcher = entt::dispatcher;
}