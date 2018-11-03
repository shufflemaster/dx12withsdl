#pragma once

#include <entt\entt.hpp>

namespace GAL
{
    using Registry = entt::registry<>;
    using EntityId = Registry::entity_type;
    using ResourceHandle = int; //A negative value means invalid handle
    using EventDispatcher = entt::dispatcher;
}