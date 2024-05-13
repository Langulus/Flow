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
   /// Represents the frequency at which data is recomputed                   
   /// Many of these rates map onto shader stages                             
   ///                                                                        
   struct RefreshRate {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;
      LANGULUS(NAME) "Rate";
      LANGULUS(INFO) "Refresh rate";

      using Type = uint8_t;

      enum Enum : Type {
         Auto = 0,         // Automatically determined refresh rate,    
                           // based on traits and context               

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

      Type mMode = Auto;

      LANGULUS_NAMED_VALUES(
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
      );

      // Rates that are considered shader stages, mapped to ShaderStage 
      static constexpr Offset StagesBegin = Enum::Vertex;
      static constexpr Offset StagesEnd = Enum::Counter;
      static constexpr Count  StagesCount = StagesEnd - StagesBegin;

      // Rates that are considered uniforms                             
      static constexpr Offset UniformBegin = Enum::Tick;
      static constexpr Offset UniformEnd = StagesBegin;
      static constexpr Count  UniformCount = UniformEnd - UniformBegin;

      // Rates that are considered inputs                               
      static constexpr Offset InputBegin = UniformBegin;
      static constexpr Offset InputEnd = StagesEnd;
      static constexpr Count  InputCount = InputEnd - InputBegin;

      // Rates that are considered static                               
      static constexpr Offset StaticUniformBegin = UniformBegin;
      static constexpr Offset StaticUniformEnd = Enum::Camera;
      static constexpr Count  StaticUniformCount = StaticUniformEnd - StaticUniformBegin;

      // Rates that are considered dynamic                              
      static constexpr Offset DynamicUniformBegin = StaticUniformEnd;
      static constexpr Offset DynamicUniformEnd = UniformEnd;
      static constexpr Count  DynamicUniformCount = DynamicUniformEnd - DynamicUniformBegin;

   public:
      constexpr RefreshRate() noexcept = default;
      constexpr RefreshRate(const CT::Number auto&) noexcept;
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

} // namespace Langulus::Flow