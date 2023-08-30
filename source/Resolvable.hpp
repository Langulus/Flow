///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"
#include <Anyness/Referenced.hpp>

namespace Langulus::Flow
{

   ///                                                                        
   ///   Abstract context                                                     
   ///                                                                        
   /// Holds a reflected class type and context state                         
   ///                                                                        
   struct Resolvable : Referenced {
      LANGULUS(ABSTRACT) true;
      LANGULUS(UNINSERTABLE) true;
      LANGULUS_CONVERSIONS(Debug);

   private:
      // Concrete type of the resolvable                                
      DMeta mClassType;
      // Byte offset from an instance of Resolvable, to the derived     
      Offset mClassOffset;

   public:
      Resolvable() = delete;
      Resolvable(const Resolvable&) noexcept = default;
      Resolvable(Resolvable&&) noexcept = default;
      Resolvable(DMeta) SAFETY_NOEXCEPT();

      Resolvable& operator = (const Resolvable&) noexcept = default;
      Resolvable& operator = (Resolvable&&) noexcept = default;

      NOD() bool CastsTo(DMeta) const noexcept;
      template<CT::Data T>
      NOD() bool CastsTo() const;

      NOD() bool Is(DMeta) const noexcept;
      template<CT::Data T>
      NOD() bool Is() const;

      NOD() Token GetToken() const noexcept;
      NOD() constexpr DMeta GetType() const noexcept;
      NOD() Block GetBlock() const noexcept;

      template<bool DISPATCH = true, bool DEFAULT = true, CT::VerbBased V>
      Any Run(const V&);
      template<bool DISPATCH = true, bool DEFAULT = true, CT::VerbBased V>
      Any Run(V&);
      template<bool DISPATCH = true, bool DEFAULT = true, CT::VerbBased V>
      Any Run(V&&) requires (CT::Mutable<V>);

      Any Run(const Code&);
      Any Run(const Any&);
      Any Run(const Temporal&);

      NOD() Block GetMember(TMeta) noexcept;
      NOD() Block GetMember(TMeta) const noexcept;

      template<CT::Index INDEX>
      NOD() Block GetMember(TMeta, const INDEX&) noexcept;
      template<CT::Index INDEX>
      NOD() Block GetMember(TMeta, const INDEX&) const noexcept;

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         NOD() Block GetMember(const Token&) noexcept;
         NOD() Block GetMember(const Token&) const noexcept;

         template<CT::Index INDEX>
         NOD() Block GetMember(const Token&, const INDEX&) noexcept;
         template<CT::Index INDEX>
         NOD() Block GetMember(const Token&, const INDEX&) const noexcept;
      #endif

      template<CT::Trait, CT::Data D>
      bool GetTrait(D&) const;
      template<CT::Data D>
      bool GetValue(D&) const;

      template<CT::Trait, bool DIRECT = false, CT::Data D>
      bool SetTrait(const D&);
      template<bool DIRECT = false, CT::Data D>
      bool SetValue(const D&);
      template<CT::Trait, bool DIRECT = false, CT::Data D>
      bool SetTrait(D&&);
      template<bool DIRECT = false, CT::Data D>
      bool SetValue(D&&);

      // All inheritances of Resolvable will become convertible to Debug
      // and will share the reflected conversions list, but with one    
      // condition: the conversion operator must remain implicit.       
      NOD() operator Debug() const;

      Debug Self() const;
   };
   
} // namespace Langulus::Flow

namespace Langulus
{

   template<class T>
   NOD() Anyness::Text IdentityOf(const T&);

   template<class T>
   NOD() Anyness::Text IdentityOf(RTTI::DMeta, const T&);

} // namespace Langulus
