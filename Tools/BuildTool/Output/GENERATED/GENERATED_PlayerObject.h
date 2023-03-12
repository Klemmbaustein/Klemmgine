#pragma once
#define PLAYEROBJECT_GENERATED(Category) PlayerObject() : WorldObject(ObjectDescription("PlayerObject", 4230145047)) {}\
static std::string GetCategory() { return Category; }\
static uint32_t GetID() { return 4230145047;}