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
#include "VerbState.hpp"


namespace Langulus::A
{

   ///                                                                        
   /// Abstract verb, dictating canonical verb size, used in various concepts 
   ///                                                                        
   struct Verb : Anyness::Any, Flow::Charge {
      LANGULUS(POD) false;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS(DEEP) false;
      LANGULUS_BASES(Any, Charge);

   protected:
      // Verb meta, mass, rate, time and priority                       
      mutable VMeta mVerb {};
      // The number of successful executions                            
      Count mSuccesses {};
      // Verb short-circuiting                                          
      Flow::VerbState mState {};
      // Verb context                                                   
      Any mSource;
      // The container where output goes after execution                
      Any mOutput;
   };

} // namespace Langulus::A

namespace Langulus::CT
{
   
   /// A VerbBased type is any type that inherits A::Verb                     
   template<class...T>
   concept VerbBased = (DerivedFrom<T, A::Verb> and ...);

   /// A reflected verb type is any type that inherits A::Verb, is binary     
   /// compatible to it, and is reflected as a verb                           
   template<class...T>
   concept Verb = VerbBased<T...> and ((
         sizeof(T) == sizeof(A::Verb)
         and (requires { {T::CTTI_Verb} -> Exact<Token>;} or requires {
            {T::CTTI_PositiveVerb} -> Exact<Token>;
            {T::CTTI_NegativeVerb} -> Exact<Token>;
         })
      ) and ...);

   /// Concept for recognizing arguments, with which a verb can be constructed
   template<class...A>
   concept VerbMakable = Inner::UnfoldInsertable<A...>
        or (sizeof...(A) == 1 and VerbBased<Desem<FirstOf<A...>>>);

   /// Concept for recognizing argument, with which a verb can be assigned    
   template<class A>
   concept VerbAssignable = VerbMakable<A>;

} // namespace Langulus::CT

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
      LANGULUS_CONVERSIONS(Code, Debug);
      LANGULUS_BASES(A::Verb);

      ///                                                                     
      ///   Construction                                                      
      ///                                                                     
      constexpr Verb() noexcept = default;
      LANGULUS_API(FLOW) Verb(const Verb&);
      LANGULUS_API(FLOW) Verb(Verb&&);

      template<CT::Data T1, CT::Data...TAIL>
      requires CT::VerbMakable<T1, TAIL...>
      Verb(T1&&, TAIL&&...);

      LANGULUS_API(FLOW) ~Verb();

      template<CT::Verb>
      NOD() static Verb From(const Charge& = {}, VerbState = {});
      template<CT::Verb>
      NOD() static Verb From(CT::Inner::UnfoldInsertable auto&&, const Charge& = {}, VerbState = {});
      NOD() static Verb FromMeta(VMeta, CT::Inner::UnfoldInsertable auto&&, const Charge& = {}, VerbState = {});

      NOD() LANGULUS_API(FLOW)
      static Verb FromMeta(VMeta, const Charge& = {}, VerbState = {});

      NOD() LANGULUS_API(FLOW)
      Verb PartialCopy() const noexcept;

      ///                                                                     
      ///   Assignment                                                        
      ///                                                                     
      LANGULUS_API(FLOW) Verb& operator = (const Verb&);
      LANGULUS_API(FLOW) Verb& operator = (Verb&&);
      Verb& operator = (CT::VerbAssignable auto&&);

      template<CT::Data T1, CT::Data... TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Verb& SetSource(T1&&, TAIL&&...);
      
      template<CT::Data T1, CT::Data... TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Verb& SetArgument(T1&&, TAIL&&...);
      
      template<CT::Data T1, CT::Data... TAIL>
      requires CT::Inner::UnfoldInsertable<T1, TAIL...>
      Verb& SetOutput(T1&&, TAIL&&...);

      LANGULUS_API(FLOW) Verb  operator *  (const Real&) const;
      LANGULUS_API(FLOW) Verb  operator ^  (const Real&) const;

      LANGULUS_API(FLOW) Verb& operator *= (const Real&) noexcept;
      LANGULUS_API(FLOW) Verb& operator ^= (const Real&) noexcept;

      ///                                                                     
      ///   Capsulation                                                       
      ///                                                                     
      NOD() LANGULUS_API(FLOW) Hash GetHash() const;

      NOD() LANGULUS_API(FLOW) const Charge& GetCharge() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetMass() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetRate() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetTime() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetPriority() const noexcept;

      NOD() LANGULUS_API(FLOW)       Any& GetSource() noexcept;
      NOD() LANGULUS_API(FLOW) const Any& GetSource() const noexcept;

      NOD() LANGULUS_API(FLOW)       Any& GetArgument() noexcept;
      NOD() LANGULUS_API(FLOW) const Any& GetArgument() const noexcept;

      NOD() LANGULUS_API(FLOW)       Any& GetOutput() noexcept;
      NOD() LANGULUS_API(FLOW) const Any& GetOutput() const noexcept;

      NOD() const Any* operator -> () const noexcept;
      NOD()       Any* operator -> () noexcept;
      
      NOD() LANGULUS_API(FLOW) Token GetToken() const;
      NOD() LANGULUS_API(FLOW) Count GetSuccesses() const noexcept;
      NOD() LANGULUS_API(FLOW) const VerbState& GetVerbState() const noexcept;
      NOD() LANGULUS_API(FLOW) bool IsDone() const noexcept;

      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;

      NOD() LANGULUS_API(FLOW) bool IsMissing() const noexcept;
      NOD() LANGULUS_API(FLOW) bool IsMissingDeep() const noexcept;
      NOD() LANGULUS_API(FLOW) bool Validate(const Index&) const noexcept;

      LANGULUS_API(FLOW) Verb& ShortCircuit(bool) noexcept;
      LANGULUS_API(FLOW) Verb& Multicast(bool) noexcept;
      LANGULUS_API(FLOW) Verb& SetVerbState(const VerbState&) noexcept;

      LANGULUS_API(FLOW) void Done(Count) noexcept;
      LANGULUS_API(FLOW) void Done() noexcept;
      LANGULUS_API(FLOW) void Undo() noexcept;
      LANGULUS_API(FLOW) Verb& Invert() noexcept;

      LANGULUS_API(FLOW) Verb& SetMass(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetRate(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetTime(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetPriority(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetCharge(const Charge&) noexcept;

      ///                                                                     
      ///   RTTI                                                              
      ///                                                                     
      NOD() LANGULUS_API(FLOW)
      bool VerbIs(VMeta) const noexcept;

      template<CT::Verb, CT::Verb...>
      NOD() bool VerbIs() const noexcept;

      template<CT::Verb>
      Verb& SetVerb();

            LANGULUS_API(FLOW) Verb& SetVerb(VMeta) noexcept;
      NOD() LANGULUS_API(FLOW) VMeta GetVerb() const noexcept;

      ///                                                                     
      ///   Comparison                                                        
      ///                                                                     
      NOD() LANGULUS_API(FLOW) bool operator == (const Verb&) const;
      NOD() LANGULUS_API(FLOW) bool operator == (VMeta) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator == (bool) const noexcept;

      NOD() LANGULUS_API(FLOW) bool operator <  (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator >  (const Verb&) const noexcept;

      NOD() LANGULUS_API(FLOW) bool operator >= (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator <= (const Verb&) const noexcept;

      ///                                                                     
      ///   Insertion                                                         
      ///                                                                     
      Verb& operator <<  (CT::Inner::UnfoldInsertable auto&&);
      Verb& operator >>  (CT::Inner::UnfoldInsertable auto&&);
      
      Verb& operator <<= (CT::Inner::UnfoldInsertable auto&&);
      Verb& operator >>= (CT::Inner::UnfoldInsertable auto&&);

      NOD() LANGULUS_API(FLOW) explicit operator Code() const;
      NOD() LANGULUS_API(FLOW) explicit operator Debug() const;

      template<CT::Dense T>
      bool GenericAvailableFor() const noexcept;
      static bool GenericExecuteIn(CT::Dense auto&, CT::VerbBased auto&);
      static bool GenericExecuteDefault(const Block&, CT::VerbBased auto&);
      static bool GenericExecuteDefault(      Block&, CT::VerbBased auto&);
      static bool GenericExecuteStateless(CT::VerbBased auto&);

      ///                                                                     
      ///   Removal                                                           
      ///                                                                     
      LANGULUS_API(FLOW) void Reset();

      template<bool OR>
      Count CompleteDispatch(const Count, Abandoned<Any>&&);

   protected:
      template<CT::Text T>
      T SerializeVerb() const;
   };

   /// A handy container for verbs                                            
   using Script = TAny<Verb>;


   ///                                                                        
   /// Statically typed verb, used as CRTP for all specific verbs             
   ///                                                                        
   template<class VERB>
   struct StaticVerb : Verb {
      LANGULUS_BASES(Verb);

      using Verb::Verb;
      StaticVerb(const StaticVerb&);
      StaticVerb(StaticVerb&&);

      using Verb::operator =;
      StaticVerb& operator = (const StaticVerb&);
      StaticVerb& operator = (StaticVerb&&);

      NOD() VMeta GetVerb() const noexcept;
   };

} // namespace Langulus::Flow