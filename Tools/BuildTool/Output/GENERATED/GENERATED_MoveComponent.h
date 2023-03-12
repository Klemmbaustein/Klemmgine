#pragma once
#define MOVECOMPONENT_GENERATED(Category) MoveComponent() : WorldObject(ObjectDescription("MoveComponent", 602091155)) {}\
static std::string GetCategory() { return Category; }\
static uint32_t GetID() { return 602091155;}