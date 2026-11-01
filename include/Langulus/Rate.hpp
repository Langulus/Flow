///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Utils/Literal.hpp>
#include <Langulus/Utils/Values.hpp>
#include <Langulus/CT/Number.hpp>
#include <Langulus/Assume.hpp>


namespace Langulus
{
   ///                                                                        
   ///   Refresh rates                                                        
   ///                                                                        
   /// Represents the frequency at which data is recomputed.                  
   /// Many of these rates map onto shader stages.                            
   struct RefreshRate {
      using CTTI_POD       = Yup;
      using CTTI_Nullable  = Yup;
      using CTTI_Named     = Yes<"Rate">;
      using CTTI_Info      = Yes<"Refresh rate">;

      enum Enum : uint8_t {
         Auto = 0,         // Automatically determined refresh rate,    
                           // based on tags and context                 

         None,             // No refresh rate (a constant, that never   
                           // refreshes)                                

         Tick,             // Refresh once per tick (when temporal flow 
                           // moves forward in time)                    

         Pass,             // Updated once per a render pass            
         Camera,           // Updated for each camera                   
         Level,            // Updated for each level                    
         Renderable,       // Updated for each renderable               
         Instance,         // Updated for each instance                 

         // The following are mapped to ShaderStage::Enum               
         Vertex,           // Refresh once per vertex (by vertex shader)

         Primitive,        // Refresh once per geometric primitive      
                           // (by geometry shader)                      

         TessCtrl,         // Refresh once per tesselation control unit 
                           // (by tesselation control shader)           

         TessEval,         // Refresh once per tesselation evaluation   
                           // unit (by tesselation evaluation shader)   

         Pixel,            // Refresh once per pixel (by fragment       
                           // shader)                                   
         Counter,
      };

      using Type = ::std::underlying_type_t<Enum>;
      Type mMode = Auto;

      using CTTI_Values = Values<
         Auto,
         None,
         Tick,

         Pass,
         Camera,
         Level,
         Renderable,
         Instance,

         Vertex,
         Primitive,
         TessCtrl,
         TessEval,
         Pixel
      >;

      // Rates that are considered shader stages, mapped to ShaderStage 
      static constexpr size_t StagesBegin = Enum::Vertex;
      static constexpr size_t StagesEnd = Enum::Counter;
      static constexpr size_t StagesCount = StagesEnd - StagesBegin;

      // Rates that are considered uniforms                             
      static constexpr size_t UniformBegin = Enum::Tick;
      static constexpr size_t UniformEnd = StagesBegin;
      static constexpr size_t UniformCount = UniformEnd - UniformBegin;

      // Rates that are considered inputs                               
      static constexpr size_t InputBegin = UniformBegin;
      static constexpr size_t InputEnd = StagesEnd;
      static constexpr size_t InputCount = InputEnd - InputBegin;

      // Rates that are considered static                               
      static constexpr size_t StaticUniformBegin = UniformBegin;
      static constexpr size_t StaticUniformEnd = Enum::Camera;
      static constexpr size_t StaticUniformCount = StaticUniformEnd - StaticUniformBegin;

      // Rates that are considered dynamic                              
      static constexpr size_t DynamicUniformBegin = StaticUniformEnd;
      static constexpr size_t DynamicUniformEnd = UniformEnd;
      static constexpr size_t DynamicUniformCount = DynamicUniformEnd - DynamicUniformBegin;

   public:
      constexpr RefreshRate() noexcept = default;

      /// Manual rate constructor from a dense number                         
      ///   @param value the number to interpret as symbolical rate           
      constexpr RefreshRate(CT::Number auto const& value) noexcept
         : mMode {static_cast<Type>(value)} {}

      /// Manual rate constructor from internal enumerator                    
      ///   @param value the rate                                             
      constexpr RefreshRate(Enum const& value) noexcept
         : mMode {value} {}

      /// Does this rate map onto uniform variables (as in shader uniforms)?  
      /// Uniforms are called like this, because they rarely change.          
      ///   @return true if this rate qualifies as a shader uniform           
      constexpr bool IsUniform() const noexcept {
         return mMode >= UniformBegin and mMode < UniformEnd;
      }

      /// Shader uniforms can be either static, or dynamic, depending on rate.
      /// Static uniforms tend to change less often than dynamic ones.        
      ///   @return true if this rate qualifies as a static uniform rate      
      constexpr bool IsStaticUniform() const noexcept {
         return mMode >= StaticUniformBegin and mMode < StaticUniformEnd;
      }

      /// Shader uniforms can be either static, or dynamic, depending on rate.
      /// Dynamic uniforms tend to change more often than static ones.        
      ///   @return true if this rate qualifies as a dynamic uniform rate     
      constexpr bool IsDynamicUniform() const noexcept {
         return mMode >= DynamicUniformBegin and mMode < DynamicUniformEnd;
      }

      /// Check if rate is considered per-vertex (as in vertex attribute).    
      /// This rate acts as the gateway to shader attributes (vertex stage is 
      /// the entry stage of a shader)                                        
      constexpr bool IsAttribute() const noexcept {
         return mMode == Enum::Vertex;
      }

      /// An input rate is any rate, that isn't None/Auto.                    
      /// In other words: anything that needs to be recomputed (not constant  
      /// or unknown) at runtime is considered an input rate.                 
      constexpr bool IsInput() const noexcept {
         return mMode >= InputBegin and mMode < InputEnd;
      }

      /// Check if rate maps onto shader stages                               
      constexpr bool IsShaderStage() const noexcept {
         return mMode >= StagesBegin and mMode < StagesEnd;
      }

      /// Get the relative input index                                        
      constexpr auto GetInputIndex() const -> size_t {
         LglsAssert(IsInput(), "Not an input");
         return mMode - InputBegin;
      }

      /// Get the relative static uniform index                               
      constexpr auto GetStaticUniformIndex() const -> size_t {
         LglsAssert(IsStaticUniform(), "Not a static uniform");
         return mMode - StaticUniformBegin;
      }

      /// Get the relative dynamic uniform index                              
      constexpr auto GetDynamicUniformIndex() const -> size_t {
         LglsAssert(IsDynamicUniform(), "Not a dynamic uniform");
         return mMode - DynamicUniformBegin;
      }
   
      /// Get the relative shader stage index                                 
      constexpr auto GetStageIndex() const -> size_t {
         LglsAssert(IsShaderStage(), "Not a shader stage");
         return mMode - StagesBegin;
      }

      constexpr operator Enum () const noexcept {
         return static_cast<Enum>(mMode);
      }
   };

   using RRate = RefreshRate;
}

namespace Langulus::Rate
{
   constexpr RRate Auto       {RRate::Auto      };
   constexpr RRate None       {RRate::None      };
   constexpr RRate Tick       {RRate::Tick      };

   constexpr RRate Pass       {RRate::Pass      };
   constexpr RRate Camera     {RRate::Camera    };
   constexpr RRate Level      {RRate::Level     };
   constexpr RRate Renderable {RRate::Renderable};
   constexpr RRate Instance   {RRate::Instance  };

   constexpr RRate Vertex     {RRate::Vertex    };
   constexpr RRate Primitive  {RRate::Primitive };
   constexpr RRate TessCtrl   {RRate::TessCtrl  };
   constexpr RRate TessEval   {RRate::TessEval  };
   constexpr RRate Pixel      {RRate::Pixel     };
}