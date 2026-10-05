///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
//#include "Executor.hpp"
#include <Langulus/Text.hpp>
#include <Langulus/Many.hpp>
#include <Langulus/TVerb.hpp>
#include <Langulus/CT/Charged.hpp>
#include "Langulus/RTTI/Definition.hpp"
#include "Flow/Export.hpp"


namespace Langulus::Flow
{
   struct Code;
}

namespace Langulus::CT
{
   namespace Inner
   {
      /// Workaround, because of MSVC ICEs introduced in 19.40.33811.0        
      /// Hopefully it will be resolved by them one day                       
      template<class T>
      consteval bool CodifiableByOperator_AvoidMSVC_ICE() {
         return std::is_object_v<T> and requires (const T& a) {
            a.operator ::Langulus::Flow::Code();
         };
      }

      /// Do types have an explicit or implicit cast operator to Code         
      template<class...T>
      concept CodifiableByOperator = 
         (CodifiableByOperator_AvoidMSVC_ICE<T>() and ...);

      /// Does Code has an explicit/implicit constructor that accepts T       
      template<class...T>
      concept CodifiableByConstructor = requires (const T&...a) {
         ((::Langulus::Flow::Code {a}), ...); };
   }

   /// A codifiable type is one that has either an implicit or explicit       
   /// cast operator to Code type, or can be used to explicitly initialize a  
   /// Code container                                                         
   template<class...T>
   concept Codifiable = ((Inner::CodifiableByOperator<T>
        or Inner::CodifiableByConstructor<T>) and ...);
}

namespace Langulus::Flow
{
   ///                                                                        
   ///   Langulus code container, parser, serializer and deserializer         
   ///                                                                        
   struct Code : Text {
      using CTTI_ReflectAs = Code;
      using CTTI_Files     = Yes<"flow">;
      using CTTI_Bases     = Text;
      using Operator       = Serial::Operator;

      using Text::Text;

      /// Construction from any kind of text that is an Annies container     
      template<CT::Text T> requires CT::Container<T>
      constexpr Code(T&& text) {
         this->Absorb(LglsFwd(text));
      }

      //explicit Code(CT::Number auto const&);

      LANGULUS_API(FLOW) Many Parse(bool optimize = true) const;
      //LANGULUS_API(FLOW) Code RightOf(size_t) const assumptious;
      //LANGULUS_API(FLOW) Code LeftOf(size_t) const assumptious;
      /*LANGULUS_API(FLOW) bool StartsWithSpecial() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithSkippable() const noexcept;
      LANGULUS_API(FLOW) bool EndsWithSkippable() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithLetter() const noexcept;
      LANGULUS_API(FLOW) bool EndsWithLetter() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithDigit() const noexcept;
      LANGULUS_API(FLOW) bool EndsWithDigit() const noexcept;
      LANGULUS_API(FLOW) bool StartsWithOperator(size_t) const noexcept;*/

      ///                                                                     
      ///   Concatenation                                                     
      ///                                                                     
      /*template<class T> requires CT::Codifiable<Deint<T>>
      Code operator + (T&&) const;

      template<class T> requires CT::Codifiable<Deint<T>>
      Code& operator += (T&&);

      template<class T>
      Code& TypeSuffix();*/

      LANGULUS_API(FLOW) static bool IsReserved(Text const&);
      LANGULUS_API(FLOW) static bool IsValidKeyword(Text const&);
   };
}

namespace Langulus::CTTI
{
   /// The presence of this structure makes Code a CT::Serializer             
   template<>
   struct Serializer<Flow::Code> : Serializer<Annies::Text> {
      // Code serializer can't be lossy - it's isomorphic               
      static constexpr bool CriticalFailure = true;
      static constexpr bool SkipElements = false;
   };
}

namespace Langulus
{
   /// Make a code literal                                                    
   auto operator ""_code(const char* text, size_t size) -> Flow::Code {
      return Flow::Code(Annies::Text::FromText(text, size));
   }

   /// Make a code literal and parse it                                       
   auto operator ""_parse(const char* text, size_t size) -> Many {
      return Flow::Code(Annies::Text::FromText(text, size)).Parse();
   }
}