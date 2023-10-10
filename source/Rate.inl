///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Rate.hpp"


namespace Langulus::Flow
{

   constexpr RefreshRate::RefreshRate(const CT::DenseNumber auto& value) noexcept
      : mMode {static_cast<Type>(value)} {}

   constexpr RefreshRate::RefreshRate(const Enum& value) noexcept
      : mMode {value} {}

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsUniform() const noexcept {
      return mMode >= UniformBegin and mMode < UniformEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsStaticUniform() const noexcept {
      return mMode >= StaticUniformBegin and mMode < StaticUniformEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsDynamicUniform() const noexcept {
      return mMode >= DynamicUniformBegin and mMode < DynamicUniformEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsAttribute() const noexcept {
      return mMode == Enum::Vertex;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsInput() const noexcept {
      return mMode >= InputBegin and mMode < InputEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsShaderStage() const noexcept {
      return mMode >= StagesBegin and mMode < StagesEnd;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetInputIndex() const {
      if (not IsInput())
         LANGULUS_THROW(Convert, "Not an input");
      return mMode - InputBegin;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetStaticUniformIndex() const {
      if (not IsStaticUniform())
         LANGULUS_THROW(Convert, "Not a static uniform");
      return mMode - StaticUniformBegin;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetDynamicUniformIndex() const {
      if (not IsDynamicUniform())
         LANGULUS_THROW(Convert, "Not a dynamic uniform");
      return mMode - DynamicUniformBegin;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetStageIndex() const {
      if (not IsShaderStage())
         LANGULUS_THROW(Convert, "Not a shader stage");
      return mMode - StagesBegin;
   }

   /// Updated once per time step                                             
   constexpr Rate PerTick = Rate::Tick;
   /// Updated once per a render pass                                         
   constexpr Rate PerPass = Rate::Pass;
   /// Updated for each camera                                                
   constexpr Rate PerCamera = Rate::Camera;
   /// Updated for each level                                                 
   constexpr Rate PerLevel = Rate::Level;
   /// Updated for each renderable                                            
   constexpr Rate PerRenderable = Rate::Renderable;
   /// Updated for each instance                                              
   constexpr Rate PerInstance = Rate::Instance;
   /// Updated in vertex shader                                               
   constexpr Rate PerVertex = Rate::Vertex;
   /// Updated in geometry shader                                             
   constexpr Rate PerPrimitive = Rate::Primitive;
   /// Updated in tesselation control shader                                  
   constexpr Rate PerTessCtrl = Rate::TessCtrl;
   /// Updated in tesselation evaluation shader                               
   constexpr Rate PerTessEval = Rate::TessEval;
   /// Updated in pixel shader                                                
   constexpr Rate PerPixel = Rate::Pixel;

} // namespace Langulus::Flow
