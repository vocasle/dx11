//
// Created by vocasle on 8/4/2022.
//

#include "Actor.h"

Actor::Actor()
    : m_isVisible(true),
      m_world(MathMat4X4Identity()){};

Actor::Actor(std::vector<Mesh> meshes)
    : m_meshes(std::move(meshes)),
      m_isVisible(true),
      m_world(MathMat4X4Identity()) {
}
const std::vector<Mesh> &
Actor::GetMeshes() const {
    return m_meshes;
}
void
Actor::SetVisible(bool isVisible) {
    m_isVisible = isVisible;
}
void
Actor::SetWorld(const Mat4X4 &world) {
    m_world = world;
}

bool
Actor::IsVisible() const {
    return m_isVisible;
}

Mat4X4
Actor::GetWorld() const {
    return m_world;
}