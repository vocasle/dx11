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

    bool IsVisible() const;
    const std::vector<Mesh> &GetMeshes() const;
    [[nodiscard]] Mat4X4 GetWorld() const;

    void SetVisible(bool isVisible);
    void SetWorld(const Mat4X4 &world);

private:
    std::vector<Mesh> m_meshes;
    bool m_isVisible;
    Mat4X4 m_world;
};
#endif  // DX11_JUNIOR_ROADMAP_ACTOR_H
