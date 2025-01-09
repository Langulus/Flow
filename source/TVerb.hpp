///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Verb.hpp"


namespace Langulus::Flow
{

   ///                                                                        
   /// Statically typed verb, used as CRTP for all specific verbs             
   ///                                                                        
   template<class VERB>
   struct TVerb : Verb {
      LANGULUS_BASES(Verb);

      using VerbType = VERB;

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr TVerb() noexcept = default;
      TVerb(const TVerb&);
      TVerb(TVerb&&);

      template<CT::Data T1, CT::Data...TN> requires CT::VerbMakable<T1, TN...>
      TVerb(T1&&, TN&&...);

      static VERB From(const Charge& = {}, VerbState = {});
      static VERB From(CT::UnfoldInsertable auto&&, const Charge& = {}, VerbState = {});

      VERB Fork(auto&&...) const noexcept;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      using Verb::operator =;
      TVerb& operator = (const TVerb&);
      TVerb& operator = (TVerb&&);

      explicit operator Code() const;
      explicit operator Text() const;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      template<CT::Data T1, CT::Data...TN>
      requires CT::UnfoldInsertable<T1, TN...>
      VERB& SetSource(T1&&, TN&&...);
      
      template<CT::Data T1, CT::Data...TN>
      requires CT::UnfoldInsertable<T1, TN...>
      VERB& SetArgument(T1&&, TN&&...);
      
      template<CT::Data T1, CT::Data...TN>
      requires CT::UnfoldInsertable<T1, TN...>
      VERB& SetOutput(T1&&, TN&&...);

      ///                                                                     
      ///   Charge arithmetics                                                
      ///                                                                     
      VERB  operator *  (Real) const;
      VERB  operator ^  (Real) const;

      VERB& operator *= (Real) noexcept;
      VERB& operator ^= (Real) noexcept;

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      Hash  GetHash() const;

      VERB& ShortCircuit(bool) noexcept;
      VERB& Multicast(bool) noexcept;
      VERB& SetVerbState(VerbState) noexcept;
      VERB& Invert() noexcept;
      VERB& SetMass(Real) noexcept;
      VERB& SetRate(Real) noexcept;
      VERB& SetTime(Real) noexcept;
      VERB& SetPriority(Real) noexcept;
      VERB& SetCharge(const Charge&) noexcept;

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::Verb>
      constexpr bool  IsVerb() const noexcept;
      constexpr bool  IsVerb(VMeta) const noexcept;
      constexpr VMeta GetVerb() const noexcept;
      constexpr Token GetToken() const;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      bool operator == (const CT::VerbBased auto&) const;
      bool operator == (VMeta) const noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      VERB& operator <<  (CT::UnfoldInsertable auto&&);
      VERB& operator >>  (CT::UnfoldInsertable auto&&);
      
      VERB& operator <<= (CT::UnfoldInsertable auto&&);
      VERB& operator >>= (CT::UnfoldInsertable auto&&);

   private:
      // Functionality graveyard                                        
      using Verb::SetVerb;
      using Verb::FromMeta;
   };

} // namespace Langulus::Flow
