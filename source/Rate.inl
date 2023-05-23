///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
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
      return mMode >= UniformBegin && mMode < UniformEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsStaticUniform() const noexcept {
      return mMode >= StaticUniformBegin && mMode < StaticUniformEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsDynamicUniform() const noexcept {
      return mMode >= DynamicUniformBegin && mMode < DynamicUniformEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsAttribute() const noexcept {
      return mMode == Enum::Vertex;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsInput() const noexcept {
      return mMode >= InputBegin && mMode < InputEnd;
   }

   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsShaderStage() const noexcept {
      return mMode >= StagesBegin && mMode < StagesEnd;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetInputIndex() const {
      if (!IsInput())
         LANGULUS_THROW(Convert, "Not an input");
      return mMode - InputBegin;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetStaticUniformIndex() const {
      if (!IsStaticUniform())
         LANGULUS_THROW(Convert, "Not a static uniform");
      return mMode - StaticUniformBegin;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetDynamicUniformIndex() const {
      if (!IsDynamicUniform())
         LANGULUS_THROW(Convert, "Not a dynamic uniform");
      return mMode - DynamicUniformBegin;
   }

   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetStageIndex() const {
      if (!IsShaderStage())
         LANGULUS_THROW(Convert, "Not a shader stage");
      return mMode - StagesBegin;
   }

} // namespace Langulus::Flow
