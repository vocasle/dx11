# Topics to cover

- [X] Initialize D3D11 context

- [X] Process input and camera control

- [X] Load simple model without animation (teapot, statue, car, plane, tank etc.)

- [X] Load mip-mapped textures (diffuse, normal, specular, gloss)

- [X] Calculate Phong lighting from direct and point lights in the pixel shader

- [X] Particle System (10000+ particles)

- [X] Make soft shadows (SM+PCF, VSM or ESM)

- [X] Make dynamic environment reflections via render to cube map

- [X] Add fog effect (pixel shader or post-process)

- [X] Add bloom post-process effect

---

Rendering equation terms explained:

Lo(X, Wo) = Le(X, Wo) + Integral over S^2 { Li(X, Wi) * Fx(Wi,Wo) * |Wi, n| * dWi }

X - a point in the scene
Wo - outgoing direction
Wi - incoming direction
n - surface normal
Integral over S^2 - all incoming directions

Lo(X, Wo) - outgoing light. This defines what light is seen from point X if I stay in the direction that Wo is pointing to

Le(X, Wo) - emitted light. This defines what light is seen from point X if I stay in the direction that Wo is pointing to

Li(X, Wi) - incoming light. This defines what light is seen from point X if I stay in the direction that Wo is pointing to

Fx(Wi,Wo) - material. Given an incoming direction and outgoing direction it determines which light is going to outgoing direction

|Wi,n| - lambert. A dot product between incoming direction and surface normal. 
