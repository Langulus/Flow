///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"

namespace Langulus::Flow
{

   ///                                                                        
   ///   Refresh rates                                                        
   ///                                                                        
   /// Represents the frequency at which data is computed                     
   /// Many of these states map onto shader stages                            
   ///                                                                        
   struct RefreshRate {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;
      LANGULUS(INFO) "Refresh rate";

      using Type = uint8_t;

      enum Enum : Type {
         Auto = 0,         // Use the default refresh rate of the trait 
         None,             // A constant, essentially                   

         Tick,             // Updated once per time step                
         Pass,             // Updated once per a render pass            
         Camera,           // Updated for each camera                   
         Level,            // Updated for each level                    
         Renderable,       // Updated for each renderable               
         Instance,         // Updated for each instance                 

         // The following are mapped to ShaderStage::Enum               
         Vertex,           // Updated in vertex shader                  
         Primitive,        // Updated in geometry shader                
         TessCtrl,         // Updated in tesselation control shader     
         TessEval,         // Updated in tesselation evaluation shader  
         Pixel,            // Updated in pixel shader                   

         Counter,
      };

      Type mMode {Enum::Auto};

      LANGULUS_NAMED_VALUES(Enum) {
         {
            "PerAuto",
            Enum::Auto,
            "Automatically determined refresh rate, based on traits and context"
         },
         {
            "PerNone",
            Enum::None,
            "No refresh rate (a constant, never refreshes)"
         },

         {
            "PerTick",
            Enum::Tick,
            "Refresh once per tick (when flow moves forward in time)"
         },
         {
            "PerPass",
            Enum::Pass,
            "Refresh once per render pass"
         },
         {
            "PerCamera",
            Enum::Camera,
            "Refresh once per camera"
         },
         {
            "PerLevel",
            Enum::Level,
            "Refresh once per level"
         },
         {
            "PerRenderable",
            Enum::Renderable,
            "Refresh once per renderable"
         },
         {
            "PerInstance",
            Enum::Instance,
            "Refresh once per instance"
         },

         {
            "PerVertex",
            Enum::Vertex,
            "Refresh once per vertex (inside vertex shader)"
         },
         {
            "PerPrimitive",
            Enum::Primitive,
            "Refresh once per geometric primitive (inside geometry shader)"
         },
         {
            "PerTessCtrl",
            Enum::TessCtrl,
            "Refresh once per tesselation control unit (inside tesselation control shader)"
         },
         {
            "PerTessEval",
            Enum::TessEval,
            "Refresh once per tesselation evaluation unit (inside tesselation evaluation shader)"
         },
         {
            "PerPixel",
            Enum::Pixel,
            "Refresh once per pixel (inside fragment shader)"
         },
      };

      // Rates that are considered shader stages, mapped to ShaderStage 
      static constexpr Offset StagesBegin = Enum::Vertex;
      static constexpr Offset StagesEnd = Enum::Counter;
      static constexpr Count StagesCount = StagesEnd - StagesBegin;

      // Rates that are considered uniforms                             
      static constexpr Offset UniformBegin = Enum::Tick;
      static constexpr Offset UniformEnd = StagesBegin;
      static constexpr Count UniformCount = UniformEnd - UniformBegin;

      // Rates that are considered inputs                               
      static constexpr Offset InputBegin = UniformBegin;
      static constexpr Offset InputEnd = StagesEnd;
      static constexpr Count InputCount = InputEnd - InputBegin;

      // Rates that are considered static                               
      static constexpr Offset StaticUniformBegin = UniformBegin;
      static constexpr Offset StaticUniformEnd = Enum::Camera;
      static constexpr Count StaticUniformCount = StaticUniformEnd - StaticUniformBegin;

      // Rates that are considered dynamic                              
      static constexpr Offset DynamicUniformBegin = StaticUniformEnd;
      static constexpr Offset DynamicUniformEnd = UniformEnd;
      static constexpr Count DynamicUniformCount = DynamicUniformEnd - DynamicUniformBegin;

   public:
      constexpr RefreshRate() noexcept = default;
      constexpr RefreshRate(const CT::DenseNumber auto&) noexcept;
      constexpr RefreshRate(const Enum&) noexcept;

      NOD() constexpr bool IsUniform() const noexcept;
      NOD() constexpr bool IsStaticUniform() const noexcept;
      NOD() constexpr bool IsDynamicUniform() const noexcept;
      NOD() constexpr bool IsAttribute() const noexcept;
      NOD() constexpr bool IsInput() const noexcept;
      NOD() constexpr bool IsShaderStage() const noexcept;
      NOD() constexpr Offset GetInputIndex() const;
      NOD() constexpr Offset GetStaticUniformIndex() const;
      NOD() constexpr Offset GetDynamicUniformIndex() const;
      NOD() constexpr Offset GetStageIndex() const;

      constexpr operator Enum () const noexcept {
         return static_cast<Enum>(mMode);
      }
   };

   using Rate = RefreshRate;

} // namespace Langulus::Flow

#include "Rate.inl"

namespace Langulus::Flow
{

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
