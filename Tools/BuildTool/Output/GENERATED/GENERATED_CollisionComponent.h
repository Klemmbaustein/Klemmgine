#pragma once
#define COLLISIONCOMPONENT_GENERATED(Category) CollisionComponent() : WorldObject(ObjectDescription("CollisionComponent", 2701016970)) {}\
static std::string GetCategory() { return Category; }\
static uint32_t GetID() { return 2701016970;}