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
#include "Code.hpp"
#include <Anyness/Verb.hpp>


namespace Langulus::Flow
{

   ///                                                                        
   ///   A type-erased verb                                                   
   ///                                                                        
   /// It's practically a single call to the framework, or a single statement 
   /// in a code flow. Langulus is based around natural language processing   
   /// theory found on verbs, so this is the natural name for such thing      
   ///                                                                        
   struct Verb : A::Verb {
      LANGULUS(NAME) "Verb";
      LANGULUS_CONVERTS_TO(Code, Text);
      LANGULUS_BASES(A::Verb);

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      using A::Verb::Verb;

      template<CT::Data T1, CT::Data...TN> requires CT::VerbMakable<T1, TN...>
      Verb(T1&&, TN&&...);

      template<CT::Verb>
      NOD() static Verb From(const Charge& = {}, VerbState = {});
      template<CT::Verb>
      NOD() static Verb From(CT::Inner::UnfoldInsertable auto&&, const Charge& = {}, VerbState = {});
      NOD() static Verb FromMeta(VMeta, CT::Inner::UnfoldInsertable auto&&, const Charge& = {}, VerbState = {});
      NOD() static Verb FromMeta(VMeta, const Charge& = {}, VerbState = {});

      template<CT::VerbBased THIS = Verb>
      NOD() THIS PartialCopy() const noexcept;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      using A::Verb::operator =;

      Verb& operator = (CT::VerbAssignable auto&&);

      template<CT::VerbBased THIS = Verb, CT::Data T1, CT::Data...TN>
      requires CT::Inner::UnfoldInsertable<T1, TN...>
      THIS& SetSource(T1&&, TN&&...);
      
      template<CT::VerbBased THIS = Verb, CT::Data T1, CT::Data...TN>
      requires CT::Inner::UnfoldInsertable<T1, TN...>
      THIS& SetArgument(T1&&, TN&&...);
      
      template<CT::VerbBased THIS = Verb, CT::Data T1, CT::Data...TN>
      requires CT::Inner::UnfoldInsertable<T1, TN...>
      THIS& SetOutput(T1&&, TN&&...);

      ///                                                                     
      ///   Charge arithmetics                                                
      ///                                                                     
      template<CT::VerbBased THIS = Verb>
      THIS operator * (Real) const;
      template<CT::VerbBased THIS = Verb>
      THIS operator ^ (Real) const;

      template<CT::VerbBased THIS = Verb>
      THIS& operator *= (Real) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& operator ^= (Real) noexcept;

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      template<CT::VerbBased THIS = Verb>
      NOD() Hash GetHash() const;

      template<CT::VerbBased THIS = Verb>
      THIS& ShortCircuit(bool) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& Multicast(bool) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& SetVerbState(VerbState) noexcept;

      template<CT::VerbBased THIS = Verb>
      THIS& Invert() noexcept;

      template<CT::VerbBased THIS = Verb>
      THIS& SetMass(Real) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& SetRate(Real) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& SetTime(Real) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& SetPriority(Real) noexcept;
      template<CT::VerbBased THIS = Verb>
      THIS& SetCharge(const Charge&) noexcept;

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      template<CT::VerbBased = Verb>
      NOD() constexpr Token GetToken() const;

      template<CT::VerbBased = Verb>
      NOD() constexpr bool IsVerb(VMeta) const noexcept;
      template<CT::VerbBased, CT::Verb, CT::Verb...>
      NOD() constexpr bool IsVerb() const noexcept;

      template<CT::VerbBased = Verb>
      NOD() constexpr VMeta GetVerb() const noexcept;

      template<CT::Verb>
      Verb& SetVerb();
      LANGULUS_API(FLOW)
      Verb& SetVerb(VMeta) noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      template<CT::VerbBased = Verb>
      NOD() bool operator == (const CT::VerbBased auto&) const;
      template<CT::VerbBased = Verb>
      NOD() bool operator == (VMeta) const noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      template<CT::VerbBased THIS = Verb>
      THIS& operator <<  (CT::Inner::UnfoldInsertable auto&&);
      template<CT::VerbBased THIS = Verb>
      THIS& operator >>  (CT::Inner::UnfoldInsertable auto&&);
      
      template<CT::VerbBased THIS = Verb>
      THIS& operator <<= (CT::Inner::UnfoldInsertable auto&&);
      template<CT::VerbBased THIS = Verb>
      THIS& operator >>= (CT::Inner::UnfoldInsertable auto&&);

      template<CT::Dense>
      bool GenericAvailableFor() const noexcept;
      static bool GenericExecuteIn(CT::Dense auto&, CT::VerbBased auto&);
      static bool GenericExecuteDefault(const Block&, CT::VerbBased auto&);
      static bool GenericExecuteDefault(      Block&, CT::VerbBased auto&);
      static bool GenericExecuteStateless(CT::VerbBased auto&);

      NOD() explicit operator Code() const;

      template<bool OR>
      Count CompleteDispatch(Count, Abandoned<Any>&&);

   private:
      // Functionality graveyard                                        
      using Any::Serialize;
   };

   /// A handy container for verbs                                            
   using Script = TAny<Verb>;

} // namespace Langulus::Flow
