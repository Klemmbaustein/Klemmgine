#pragma once
#define COMPONENT_GENERATED(Category) Component() : WorldObject(ObjectDescription("Component", 718440320)) {}\
static std::string GetCategory() { return Category; }\
static uint32_t GetID() { return 718440320;}