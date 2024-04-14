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


namespace Langulus::Flow
{

   ///                                                                        
   /// Statically typed verb, used as CRTP for all specific verbs             
   ///                                                                        
   template<class VERB>
   struct TVerb : Verb {
      using VerbType = VERB;
      LANGULUS_BASES(Verb);

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      using Verb::Verb;
      TVerb(const TVerb&);
      TVerb(TVerb&&);

      NOD() static VERB From(const Charge& = {}, VerbState = {});
      NOD() static VERB From(CT::UnfoldInsertable auto&&, const Charge& = {}, VerbState = {});

      NOD() VERB Fork(auto&&...) const noexcept;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      using Verb::operator =;
      TVerb& operator = (const TVerb&);
      TVerb& operator = (TVerb&&);

      NOD() explicit operator Code() const;
      NOD() explicit operator Text() const;

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
      VERB operator * (Real) const;
      VERB operator ^ (Real) const;

      VERB& operator *= (Real) noexcept;
      VERB& operator ^= (Real) noexcept;

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() Hash GetHash() const;

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
      NOD() constexpr bool  IsVerb() const noexcept;
      NOD() constexpr bool  IsVerb(VMeta) const noexcept;
      NOD() constexpr VMeta GetVerb() const noexcept;
      NOD() constexpr Token GetToken() const;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      NOD() bool operator == (const CT::VerbBased auto&) const;
      NOD() bool operator == (VMeta) const noexcept;

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
