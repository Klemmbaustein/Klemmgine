#pragma once
#define MESHCOMPONENT_GENERATED(Category) MeshComponent() : WorldObject(ObjectDescription("MeshComponent", 3774279003)) {}\
static std::string GetCategory() { return Category; }\
static uint32_t GetID() { return 3774279003;}