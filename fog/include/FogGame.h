#pragma once

#include "Game.h"

class FogGame : public Game
{
public:
    FogGame();
    ~FogGame() override;
    void Tick() override;
    void Initialize(HWND hWnd, uint32_t width, uint32_t height) override;
    void OnWindowSizeChanged(int width, int height) override;
private:
    void Update() override;
    void Render() override;
    void CreateWindowSizeDependentResources() override;
    void UpdateImgui() override;
};
