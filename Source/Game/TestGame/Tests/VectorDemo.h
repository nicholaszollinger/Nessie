// VectorTester.h
#pragma once

#include "../Demo.h"
#include "Math/Vector2.h"

class VectorDemo final : public Demo
{
    enum class VectorTest : uint8_t
    {
        Addition = 0,
        DotProduct,
        //CrossProduct, // [TODO] Do when I can render 3D. (hopefully soon)
        Num,
    };
    
    inline static std::string s_testTypeNames[3]
    {
        "Addition",
        "Dot Product",
    };
    
    nes::Vector2 m_vectorA{}; 
    nes::Vector2 m_vectorB{}; 
    nes::Vector2 m_resultVector{}; 
    float m_rotateAngleRadians = 45.f;
    float m_resultScalar = 0.f;
    float m_vectorDrawScale = 10.f;
    VectorTest m_testType = VectorTest::Addition;
    
public:
    virtual void Reset() override;
    virtual void Render(const nes::Renderer& renderer, const nes::Matrix3x3& viewMatrix) override;
    virtual void RenderImGui() override;
    virtual const char* GetName() const override { return "Vectors"; }

private:
    void DrawArrow(const nes::Renderer& renderer, const nes::Vector2& from, const nes::Vector2& offset, const nes::LinearColor& color, float thickness = 1.f) const;
};
