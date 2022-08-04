//
// Created by vocasle on 8/4/2022.
//

#ifndef DX11_JUNIOR_ROADMAP_ACTOR_H
#define DX11_JUNIOR_ROADMAP_ACTOR_H

#include <vector>

#include "ModelLoader.h"
#include "NE_Math.h"

class Actor {
public:
    Actor();
    explicit Actor(std::vector<Mesh> meshes);

    [[nodiscard]] bool IsVisible() const;
    [[nodiscard]] const std::vector<Mesh> &GetMeshes() const;
    [[nodiscard]] Mat4X4 GetWorld() const;
    [[nodiscard]] float GetRoughness() const;

    void SetVisible(bool isVisible);
    void SetWorld(const Mat4X4 &world);
    void SetRoughness(float roughness);

private:
    std::vector<Mesh> m_meshes;
    bool m_isVisible;
    Mat4X4 m_world;
    float m_roughness;
};
#endif  // DX11_JUNIOR_ROADMAP_ACTOR_H
