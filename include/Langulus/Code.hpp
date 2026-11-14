///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/SerializeText.hpp>
#include <Langulus/Rate.hpp>
#include <Langulus/Many.hpp>
#include <Langulus/TVerb.hpp>
#include <Langulus/CT/Charged.hpp>
#include "Langulus/RTTI/Definition.hpp"
#include "Flow/Export.hpp"


namespace Langulus::Flow
{
   ///                                                                        
   ///   Langulus code container, parser, serializer and deserializer         
   struct Code : Text {
      using CTTI_ReflectAs = Code;
      using CTTI_Files     = Yes<"flow">;
      using CTTI_Bases     = Text;
      using Text::Text;

      /// Construction from any kind of text that is an Annies container      
      template<CT::Text T> requires CT::Container<T>
      constexpr Code(T&& text) {
         this->Absorb(LglsFwd(text));
      }

      LANGULUS_API(FLOW) Many Parse(bool optimize = true) const;
      LANGULUS_API(FLOW) static bool IsReserved(Text const&);
      LANGULUS_API(FLOW) static bool IsValidKeyword(Text const&);
   };
}

/// Convert std::string_view -> Flow::Code                                    
LANGULUS_MORPHISM(std::string_view, Flow::Code);

/// Convert Text -> Flow::Code                                                
LANGULUS_MORPHISM(Annies::Text, Flow::Code);

/// Convert RRate -> Flow::Code                                               
LANGULUS_MORPHISM_CUSTOM(RRate, {
      return Annies::Text::Named(from);
   },
   Flow::Code
);

/// Convert Literal -> Flow::Code                                             
LANGULUS_MORPHISM_CONCEPT(CT::Literal, Flow::Code);

namespace Langulus::CTTI
{
   /// The presence of this structure makes Code a CT::Serializer             
   template<>
   struct Serializer<Flow::Code> : Serializer<Annies::Text> {
      // Code serializer can't be lossy - it's isomorphic               
      static constexpr bool CriticalFailure = true;
      static constexpr bool SkipElements = false;
   };

   /// Rule for serializing Code to Text. Wraps it in {} symbols.             
   template<>
   struct SerializationRule<Annies::Text, Flow::Code> {
      using S = Serializer<Annies::Text>;
      using Context = typename S::Context;

      static void Serialize(Flow::Code const& item, Annies::Text& out, Context*) {
         out += Serial::OpenCode.Token;
         out += item;
         out += Serial::CloseCode.Token;
      }
   };
}

namespace Langulus
{
   /// Make a code literal                                                    
   constexpr auto operator ""_code(const char* text, size_t size) -> Flow::Code {
      return Flow::Code(Annies::Text::FromText(text, size));
   }

   /// Make a code literal and parse it                                       
   constexpr auto operator ""_parse(const char* text, size_t size) -> Many {
      return Flow::Code(Annies::Text::FromText(text, size)).Parse();
   }
}