///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
#if LANGULUS_COMPILER(MSVC)
   #pragma warning(suppress: 4275)
   struct LANGULUS_API(FLOW) Resolvable {
#else
   struct LANGULUS_API(FLOW) Resolvable {
#endif
      LANGULUS(ABSTRACT) true;
      LANGULUS_CONVERTS_TO(Text);

   private:
      // Concrete type of the resolvable                                
      #if LANGULUS_COMPILER(MSVC)
         #pragma warning(suppress: 4251)
         DMeta mClassType;
      #else
         DMeta mClassType;
      #endif

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
      NOD() Block<> GetBlock() const noexcept;

      template<bool DISPATCH = true, bool DEFAULT = true>
      auto& Run(CT::VerbBased auto&&);

      Many Run(const Code&);
      Many Run(const Many&);
      Many Run(const Temporal&);

      NOD() Block<> GetMember(TMeta) noexcept;
      NOD() Block<> GetMember(TMeta) const noexcept;

      NOD() Block<> GetMember(TMeta, CT::Index auto) noexcept;
      NOD() Block<> GetMember(TMeta, CT::Index auto) const noexcept;

      #if LANGULUS_FEATURE(MANAGED_MEMORY)
         NOD() Block<> GetMember(const Token&) noexcept;
         NOD() Block<> GetMember(const Token&) const noexcept;

         NOD() Block<> GetMember(const Token&, CT::Index auto) noexcept;
         NOD() Block<> GetMember(const Token&, CT::Index auto) const noexcept;
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
