///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"

namespace Langulus::Flow
{

   ///                                                                        
   ///   Abstract context                                                     
   ///                                                                        
   /// Holds a reflected class type and context state                         
   ///                                                                        
   struct Resolvable {
      LANGULUS(ABSTRACT) true;
      LANGULUS(UNINSERTABLE) true;
      LANGULUS_CONVERSIONS(Debug);
   protected:
      DMeta mClassType;
      Offset mClassOffset;

   public:
      Resolvable() = delete;
      Resolvable(const Resolvable&) noexcept = default;
      Resolvable(Resolvable&&) noexcept = default;
      Resolvable(DMeta) noexcept;

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

      template<bool DISPATCH = true, bool DEFAULT = true, CT::Verb V>
      NOD() bool Run(V& verb);

      template<CT::Index INDEX>
      NOD() Block GetMember(TMeta, const INDEX&) noexcept;
      template<CT::Index INDEX>
      NOD() Block GetMember(TMeta, const INDEX&) const noexcept;

      NOD() explicit operator Debug() const;
   };
   
} // namespace Langulus::Flow

namespace Langulus
{

   template<class T>
   NOD() LANGULUS(ALWAYSINLINE) Anyness::Text IdentityOf(const T&);

   template<class T>
   NOD() LANGULUS(ALWAYSINLINE) Anyness::Text IdentityOf(RTTI::DMeta, const T&);

} //namespace Langulus

#include "Resolvable.inl"