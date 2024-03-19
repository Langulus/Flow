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

   /// Manual rate constructor from a dense number                            
   ///   @param value - the number to interpret as symbolical rate            
   constexpr RefreshRate::RefreshRate(const CT::DenseNumber auto& value) noexcept
      : mMode {static_cast<Type>(value)} {}

   /// Manual rate constructor from internal enumerator                       
   ///   @param value - the rate                                              
   constexpr RefreshRate::RefreshRate(const Enum& value) noexcept
      : mMode {value} {}

   /// Does this rate map onto uniform variables (as in shader uniforms)      
   /// Uniforms are called like this, because they rarely change              
   ///   @return true if this rate qualifies as a shader uniform              
   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsUniform() const noexcept {
      return mMode >= UniformBegin and mMode < UniformEnd;
   }

   /// Shader uniforms can be either static, or dynamic, depending on rate    
   /// Static uniforms tend to change less often than dynamic ones            
   ///   @return true if this rate qualifies as a static uniform rate         
   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsStaticUniform() const noexcept {
      return mMode >= StaticUniformBegin and mMode < StaticUniformEnd;
   }

   /// Shader uniforms can be either static, or dynamic, depending on rate    
   /// Static uniforms tend to change less often than dynamic ones            
   ///   @return true if this rate qualifies as a static uniform rate         
   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsDynamicUniform() const noexcept {
      return mMode >= DynamicUniformBegin and mMode < DynamicUniformEnd;
   }

   /// Check if rate is considered per-vertex (as in vertex attribute)        
   /// This rate acts as the gateway to shader attributes (vertex stage is    
   /// the entry stage of a shader)                                           
   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsAttribute() const noexcept {
      return mMode == Enum::Vertex;
   }

   /// An input rate is any rate, that isn't None/Auto                        
   /// In other words - anything that needs to be recomputed (not constant or 
   /// unknown) at runtime is considered an input rate                        
   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsInput() const noexcept {
      return mMode >= InputBegin and mMode < InputEnd;
   }

   /// Check if rate maps onto shader stages                                  
   LANGULUS(INLINED)
   constexpr bool RefreshRate::IsShaderStage() const noexcept {
      return mMode >= StagesBegin and mMode < StagesEnd;
   }

   /// Get the relative input index                                           
   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetInputIndex() const {
      if (not IsInput())
         LANGULUS_THROW(Convert, "Not an input");
      return mMode - InputBegin;
   }

   /// Get the relative static uniform index                                  
   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetStaticUniformIndex() const {
      if (not IsStaticUniform())
         LANGULUS_THROW(Convert, "Not a static uniform");
      return mMode - StaticUniformBegin;
   }

   /// Get the relative dynamic uniform index                                 
   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetDynamicUniformIndex() const {
      if (not IsDynamicUniform())
         LANGULUS_THROW(Convert, "Not a dynamic uniform");
      return mMode - DynamicUniformBegin;
   }

   /// Get the relative shader stage index                                    
   LANGULUS(INLINED)
   constexpr Offset RefreshRate::GetStageIndex() const {
      if (not IsShaderStage())
         LANGULUS_THROW(Convert, "Not a shader stage");
      return mMode - StagesBegin;
   }

   namespace Rate
   {

      constexpr RefreshRate Auto       {RefreshRate::Auto      };
      constexpr RefreshRate None       {RefreshRate::None      };
      constexpr RefreshRate Tick       {RefreshRate::Tick      };

      constexpr RefreshRate Pass       {RefreshRate::Pass      };
      constexpr RefreshRate Camera     {RefreshRate::Camera    };
      constexpr RefreshRate Level      {RefreshRate::Level     };
      constexpr RefreshRate Renderable {RefreshRate::Renderable};
      constexpr RefreshRate Instance   {RefreshRate::Instance  };

      constexpr RefreshRate Vertex     {RefreshRate::Vertex    };
      constexpr RefreshRate Primitive  {RefreshRate::Primitive };
      constexpr RefreshRate TessCtrl   {RefreshRate::TessCtrl  };
      constexpr RefreshRate TessEval   {RefreshRate::TessEval  };
      constexpr RefreshRate Pixel      {RefreshRate::Pixel     };

   } // namespace Langulus::Flow::Rate

} // namespace Langulus::Flow
