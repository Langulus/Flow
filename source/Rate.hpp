///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
      static constexpr size_t StagesBegin = Enum::Vertex;
      static constexpr size_t StagesEnd = Enum::Counter;
      static constexpr size_t  StagesCount = StagesEnd - StagesBegin;

      // Rates that are considered uniforms                             
      static constexpr size_t UniformBegin = Enum::Tick;
      static constexpr size_t UniformEnd = StagesBegin;
      static constexpr size_t  UniformCount = UniformEnd - UniformBegin;

      // Rates that are considered inputs                               
      static constexpr size_t InputBegin = UniformBegin;
      static constexpr size_t InputEnd = StagesEnd;
      static constexpr size_t  InputCount = InputEnd - InputBegin;

      // Rates that are considered static                               
      static constexpr size_t StaticUniformBegin = UniformBegin;
      static constexpr size_t StaticUniformEnd = Enum::Camera;
      static constexpr size_t  StaticUniformCount = StaticUniformEnd - StaticUniformBegin;

      // Rates that are considered dynamic                              
      static constexpr size_t DynamicUniformBegin = StaticUniformEnd;
      static constexpr size_t DynamicUniformEnd = UniformEnd;
      static constexpr size_t  DynamicUniformCount = DynamicUniformEnd - DynamicUniformBegin;

   public:
      constexpr RefreshRate() noexcept = default;
      constexpr RefreshRate(const CT::Number auto&) noexcept;
      constexpr RefreshRate(const Enum&) noexcept;

      constexpr bool IsUniform() const noexcept;
      constexpr bool IsStaticUniform() const noexcept;
      constexpr bool IsDynamicUniform() const noexcept;
      constexpr bool IsAttribute() const noexcept;
      constexpr bool IsInput() const noexcept;
      constexpr bool IsShaderStage() const noexcept;
      constexpr auto GetInputIndex() const -> size_t;
      constexpr auto GetStaticUniformIndex() const -> size_t;
      constexpr auto GetDynamicUniformIndex() const -> size_t;
      constexpr auto GetStageIndex() const -> size_t;

      constexpr operator Enum () const noexcept {
         return static_cast<Enum>(mMode);
      }
   };

} // namespace Langulus::Flow