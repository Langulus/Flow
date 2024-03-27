///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"
#include "Temporal.hpp"


namespace Langulus::Flow
{

   ///                                                                        
   ///   Abstract context                                                     
   ///                                                                        
   /// Holds a reflected class type and context state                         
   ///                                                                        
   struct LANGULUS_API(FLOW) Resolvable : virtual Referenced {
      LANGULUS(ABSTRACT) true;
      LANGULUS_CONVERTS_TO(Text);

   private:
      // Concrete type of the resolvable                                
      DMeta mClassType;
      // Byte offset from an instance of Resolvable, to the derived     
      const void* mClassPointer;

   public:
      Resolvable() = delete;
      Resolvable(const Resolvable&) noexcept = default;
      Resolvable(Resolvable&&) noexcept = default;
      template<class T>
      Resolvable(const T*) IF_UNSAFE(noexcept);
      virtual ~Resolvable() = default;

      Resolvable& operator = (const Resolvable&) noexcept = default;
      Resolvable& operator = (Resolvable&&) noexcept = default;

      template<CT::Data>
      NOD() bool CastsTo() const;
      NOD() bool CastsTo(DMeta) const IF_UNSAFE(noexcept);

      template<CT::Data>
      NOD() bool Is() const;
      NOD() bool Is(DMeta) const noexcept;

      NOD() Token GetToken() const IF_UNSAFE(noexcept);
      NOD() DMeta GetType()  const noexcept;
      NOD() Block GetBlock() const noexcept;

      template<bool DISPATCH = true, bool DEFAULT = true>
      auto& Run(CT::VerbBased auto&&);

      Any Run(const Code&);
      Any Run(const Any&);
      Any Run(const Temporal&);

      NOD() Block GetMember(TMeta) noexcept;
      NOD() Block GetMember(TMeta) const noexcept;

      NOD() Block GetMember(TMeta, CT::Index auto) noexcept;
      NOD() Block GetMember(TMeta, CT::Index auto) const noexcept;

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         NOD() Block GetMember(const Token&) noexcept;
         NOD() Block GetMember(const Token&) const noexcept;

         NOD() Block GetMember(const Token&, CT::Index auto) noexcept;
         NOD() Block GetMember(const Token&, CT::Index auto) const noexcept;
      #endif

      template<CT::Trait>
      bool GetTrait(CT::Data auto&) const;
      bool GetValue(CT::Data auto&) const;

      template<CT::Trait, bool DIRECT = false>
      bool SetTrait(CT::Data auto&&);
      template<bool DIRECT = false>
      bool SetValue(CT::Data auto&&);

      // All inheritances of Resolvable will become convertible to Text 
      // and will share the reflected conversions list, but with one    
      // condition: the conversion operator must remain implicit.       
      NOD() operator Text() const;

      Text Self() const;
   };
   
} // namespace Langulus::Flow

namespace Langulus
{

   NOD() Anyness::Text IdentityOf(const auto&);
   NOD() Anyness::Text IdentityOf(const Token&, const auto&);

} // namespace Langulus
