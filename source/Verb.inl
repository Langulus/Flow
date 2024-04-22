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
#include "Code.inl"
#include "verbs/Interpret.inl"


namespace Langulus::Flow
{

   /// Generic constructor                                                    
   ///   @param other - the verb/argument and semantic to construct with      
   template<CT::Data T1, CT::Data...TN>
   requires CT::VerbMakable<T1, TN...> LANGULUS(INLINED)
   Verb::Verb(T1&& t1, TN&&...tn) {
      if constexpr (sizeof...(TN) == 0 and not CT::Array<T1>) {
         using S = SemanticOf<decltype(t1)>;
         using T = TypeOf<S>;

         if constexpr (CT::VerbBased<T>) {
            // Make sure the VMeta is initialized                       
            (void) DesemCast(t1).GetVerb();
            new (this) A::Verb {S::Nest(t1).template Forward<A::Verb>()};
         }
         else Many::operator = (Forward<T1>(t1));
      }
      else Many::Insert(IndexBack, Forward<T1>(t1), Forward<TN>(tn)...);
   }

   /// Generic assignment                                                     
   ///   @param rhs - the verb/argument and semantic to assign by             
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (CT::VerbAssignable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::VerbBased<T>) {
         // Make sure the VMeta is initialized                          
         (void) DesemCast(rhs).GetVerb();
         A::Verb::operator = (S::Nest(rhs).template Forward<A::Verb>());
      }
      else Many::operator = (S::Nest(rhs));
      return *this;
   }

   /// Create a statically typed verb with charge and state                   
   ///   @tparam V - the verb type                                            
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   template<CT::Verb V> LANGULUS(INLINED)
   Verb Verb::From(const Charge& charge, VerbState state) {
      return FromMeta(MetaVerbOf<V>(), charge, state);
   }

   /// Create a statically typed verb with argument, charge and state         
   ///   @tparam V - the verb type                                            
   ///   @param a - argument to initialize with                               
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   template<CT::Verb V> LANGULUS(INLINED)
   Verb Verb::From(
      CT::UnfoldInsertable auto&& a,
      const Charge& charge, VerbState state
   ) {
      using S = SemanticOf<decltype(a)>;
      return FromMeta(MetaVerbOf<V>(), S::Nest(a), charge, state);
   }

   /// Create a dynamically typed verb with argument, charge and state        
   ///   @param verb - type of the verb                                       
   ///   @param a - argument to move in                                       
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   LANGULUS(INLINED)
   Verb Verb::FromMeta(
      VMeta verb, CT::UnfoldInsertable auto&& a,
      const Charge& charge, VerbState state
   ) {
      using S = SemanticOf<decltype(a)>;
      auto result = FromMeta(verb, charge, state);
      result.SetArgument(S::Nest(a));
      return result;
   }

   /// Create a dynamically typed verb with charge and state                  
   ///   @param verb - type of the verb                                       
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   LANGULUS(INLINED)
   Verb Verb::FromMeta(VMeta verb, const Charge& charge, VerbState state) {
      Verb result;
      result.SetVerb(verb);
      result.SetCharge(charge);
      result.SetVerbState(state);
      return result;
   }

   /// Hash the verb                                                          
   ///   @return the hash of the content                                      
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   Hash Verb::GetHash() const {
      if constexpr (CT::Verb<THIS>)
         (void) GetVerb<THIS>();
      return A::Verb::GetHash();
   }

   /// Multiply verb mass                                                     
   ///   @param rhs - the mass to multiply by                                 
   ///   @return a new verb, with the modified mass                           
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS Verb::operator * (Real rhs) const {
      auto shallowCopy = *reinterpret_cast<const THIS*>(this);
      shallowCopy.mMass *= rhs;
      return shallowCopy;
   }

   /// Multiply verb frequency                                                
   ///   @param rhs - the frequency to multiply by                            
   ///   @return a new verb, with the modified frequency                      
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS Verb::operator ^ (Real rhs) const {
      auto shallowCopy = *reinterpret_cast<const THIS*>(this);
      shallowCopy.mRate *= rhs;
      return shallowCopy;
   }

   /// Multiply verb mass (destructively)                                     
   ///   @param rhs - the mass to multiply by                                 
   ///   @return a reference to this verb                                     
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::operator *= (Real rhs) noexcept {
      mMass *= rhs;
      return *reinterpret_cast<const THIS*>(this);
   }

   /// Multiply verb frequency (destructively)                                
   ///   @param rhs - the frequency to multiply by                            
   ///   @return a reference to this verb                                     
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::operator ^= (Real rhs) noexcept {
      mRate *= rhs;
      return *reinterpret_cast<const THIS*>(this);
   }

   /// Partial copy, copies only charge, verb, and short-circuitness          
   ///   @param other - the verb to use as base                               
   ///   @return the partially copied verb                                    
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS Verb::Fork(auto&&...args) const noexcept {
      if constexpr (CT::Verb<THIS>) {
         return THIS::From(
            Many {Forward<Deref<decltype(args)>>(args)...},
            GetCharge(),
            mState
         );
      }
      else {
         return THIS::FromMeta(
            mVerb,
            Many {Forward<Deref<decltype(args)>>(args)...},
            GetCharge(),
            mState
         );
      }
   }

   /// Change the verb's circuitry                                            
   ///   @param toggle - enable or disable short-circuit                      
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::ShortCircuit(bool toggle) noexcept {
      if (toggle)
         mState -= VerbState::LongCircuited;
      else
         mState += VerbState::LongCircuited;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Change the verb's castness                                             
   ///   @param toggle - enable or disable multicast                          
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::Multicast(bool toggle) noexcept {
      if (toggle)
         mState -= VerbState::Monocast;
      else
         mState += VerbState::Monocast;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Check if verb is matches the provided V1                               
   ///   @tparam V1 - the verb to compare against                             
   ///   @return true if verb matches                                         
   template<CT::Verb V1, CT::VerbBased THIS> LANGULUS(INLINED)
   constexpr bool Verb::IsVerb() const noexcept {
      if constexpr (CT::Verb<THIS>)
         return CT::Same<V1, THIS>;
      else
         return mVerb == MetaVerbOf<V1>();
   }

   /// Get the verb id                                                        
   ///   @return verb ID                                                      
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   constexpr VMeta Verb::GetVerb() const noexcept {
      if constexpr (CT::Verb<THIS>)
         return (mVerb = MetaVerbOf<THIS>());
      else
         return mVerb;
   }
   
   /// Check if verb id matches                                               
   ///   @param id - the verb id to check                                     
   ///   @return true if verb id matches                                      
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   constexpr bool Verb::IsVerb(VMeta id) const noexcept {
      if constexpr (CT::Verb<THIS>)
         return GetVerb<THIS>()->Is(id);
      else
         return not mVerb ? not id : mVerb->Is(id);
   }

   /// Get the verb token                                                     
   ///   @return the token as a literal                                       
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   constexpr Token Verb::GetToken() const {
      if constexpr (CT::Verb<THIS>) {
         return mMass < 0
            ? GetVerb<THIS>()->mTokenReverse
            : GetVerb<THIS>()->mToken;
      }
      else {
         if (not mVerb)
            return RTTI::MetaVerb::DefaultToken;
         return mMass < 0 ? mVerb->mTokenReverse : mVerb->mToken;
      }
   }

   /// Set the verb state                                                     
   ///   @param state - the verb state                                        
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::SetVerbState(VerbState state) noexcept {
      mState = state;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Invert the verb (use the antonym)                                      
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::Invert() noexcept {
      mMass *= Real {-1};
      return *reinterpret_cast<const THIS*>(this);
   }

   /// Set the verb type                                                      
   ///   @tparam V - the verb to assign                                       
   ///   @return a reference to self                                          
   template<CT::Verb V> LANGULUS(INLINED)
   Verb& Verb::SetVerb() {
      mVerb = MetaVerbOf<V>();
      return *this;
   }

   /// Set the verb type at runtime                                           
   ///   @param verb - the verb to assign                                     
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetVerb(VMeta verb) noexcept {
      mVerb = verb;
      return *this;
   }

   /// Set the verb mass                                                      
   ///   @param mass - the mass to set                                        
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::SetMass(const Real mass) noexcept {
      mMass = mass;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Set the verb frequency                                                 
   ///   @param rate - the rate to set                                        
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::SetRate(const Real rate) noexcept {
      mRate = rate;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Set the verb time                                                      
   ///   @param time - the time to set                                        
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::SetTime(const Real time) noexcept {
      mTime = time;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Set the verb priority                                                  
   ///   @param priority - the priority to set                                
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::SetPriority(const Real priority) noexcept {
      mPriority = priority;
      return *reinterpret_cast<THIS*>(this);
   }

   /// Set the verb mass, frequency, time, and priority (a.k.a. charge)       
   ///   @param charge - the charge to set                                    
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::SetCharge(const Charge& charge) noexcept {
      Charge::operator = (charge);
      return *reinterpret_cast<THIS*>(this);
   }

   /// Set the verb's source                                                  
   ///   @param t1, tail...  - the values to assign                           
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS, CT::Data T1, CT::Data...TN>
   requires CT::UnfoldInsertable<T1, TN...> LANGULUS(INLINED)
   THIS& Verb::SetSource(T1&& t1, TN&&...tn) {
      mSource = Many {Forward<T1>(t1), Forward<TN>(tn)...};
      // We guarantee that source is exactly Any, so we unconstrain it  
      // in order to be safely able to overwrite it anytime             
      mSource.MakeTypeConstrained(false);
      return *reinterpret_cast<THIS*>(this);
   }
   
   /// Set the verb's argument                                                
   ///   @param t1, tail...  - the values to assign                           
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS, CT::Data T1, CT::Data...TN>
   requires CT::UnfoldInsertable<T1, TN...> LANGULUS(INLINED)
   THIS& Verb::SetArgument(T1&& t1, TN&&...tn) {
      Many::operator = (Many {Forward<T1>(t1), Forward<TN>(tn)...});
      // We guarantee that argument is exactly Any, so we unconstrain it
      // in order to be safely able to overwrite it anytime             
      MakeTypeConstrained(false);
      return *reinterpret_cast<THIS*>(this);
   }
   
   /// Set the verb's output                                                  
   ///   @param t1, tail...  - the values to assign                           
   ///   @return a reference to self                                          
   template<CT::VerbBased THIS, CT::Data T1, CT::Data...TN>
   requires CT::UnfoldInsertable<T1, TN...> LANGULUS(INLINED)
   THIS& Verb::SetOutput(T1&& t1, TN&&...tn) {
      mOutput = Many {Forward<T1>(t1), Forward<TN>(tn)...};
      // We guarantee that output is exactly Any, so we unconstrain it  
      // in order to be safely able to overwrite it anytime             
      mOutput.MakeTypeConstrained(false);
      return *reinterpret_cast<THIS*>(this);
   }

   /// Compare verbs                                                          
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   bool Verb::operator == (const CT::VerbBased auto& rhs) const {
      using OTHER = Deref<decltype(rhs)>;

      if constexpr (CT::Verb<THIS, OTHER>) {
         if constexpr (not CT::Similar<typename THIS::VerbType,
                                       typename OTHER::VerbType>)
            return false;
         else {
            return mSource == rhs.mSource
               and Many::operator == (rhs.GetArgument())
               and mOutput == rhs.mOutput
               and mState == rhs.mState;
         }
      }
      else return A::Verb::operator == (rhs);
   }

   /// Compare verb types for equality                                        
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   bool Verb::operator == (VMeta rhs) const noexcept {
      return GetVerb<THIS>() == rhs;
   }

   /// Push anything to end of the outputs, satisfying the verb               
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param rhs - the data (and semantic) to push                         
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::operator << (CT::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<T>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *reinterpret_cast<THIS*>(this);
         }

         if (mOutput.SmartPush(IndexBack, S::Nest(rhs)))
            Done();
      }

      return *reinterpret_cast<THIS*>(this);
   }

   /// Push anything to the front of the outputs, satisfying the verb         
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param rhs - the data (and semantic) to push                         
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::operator >> (CT::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<T>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *reinterpret_cast<THIS*>(this);
         }

         if (mOutput.SmartPush(IndexFront, S::Nest(rhs)))
            Done();
      }

      return *reinterpret_cast<THIS*>(this);
   }

   /// Merge anything to output's back by a semantic                          
   ///   @param rhs - the data (and semantic) to merge                        
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::operator <<= (CT::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<TypeOf<S>>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *reinterpret_cast<THIS*>(this);
         }

         auto ptr = PointerDecay(DesemCast(rhs));
         if (mOutput.Find(ptr))
            return *reinterpret_cast<THIS*>(this);

         if (mOutput.SmartPush(IndexBack, ptr))
            Done();
      }

      return *reinterpret_cast<THIS*>(this);
   }

   /// Merge anything to output's front by a semantic                         
   ///   @param rhs - the data (and semantic) to merge                        
   ///   @return a reference to this verb for chaining                        
   template<CT::VerbBased THIS> LANGULUS(INLINED)
   THIS& Verb::operator >>= (CT::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<TypeOf<S>>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *reinterpret_cast<THIS*>(this);
         }

         auto ptr = PointerDecay(DesemCast(rhs));
         if (mOutput.Find(ptr))
            return *reinterpret_cast<THIS*>(this);

         if (mOutput.SmartPush(IndexBack, ptr))
            Done();
      }

      return *reinterpret_cast<THIS*>(this);
   }

   /// Finalize a dispatch execution by setting satisfaction state and output 
   ///   @tparam OR - whether the dispatch happened in an OR context or not   
   ///   @param successes - number of successes                               
   ///   @param output - the output container                                 
   ///   @return the number of successes for the verb                         
   template<bool OR> LANGULUS(INLINED)
   Count Verb::CompleteDispatch(const Count successes, Abandoned<Many>&& output) {
      if (IsShortCircuited()) {
         // If reached, this will result in failure in OR-context, or   
         // success if AND, as long as the verb is short-circuited      
         mSuccesses = OR ? 0 : successes;
      }
      else {
         // If verb is not short-circuited, then a single success       
         // is always enough                                            
         mSuccesses = successes;
      }

      // Set output                                                     
      if (mSuccesses)
         mOutput = output.Forward<Many>();
      else
         mOutput.Reset();
      return mSuccesses;
   }

   /// Check if reflected abilities in T support this verb                    
   /// This is a slow runtime check, use statically optimized variants inside 
   /// specific verbs if you know them at compile time                        
   ///   @return true if the ability exists                                   
   template<CT::Dense T> LANGULUS(INLINED)
   bool Verb::GenericAvailableFor() const noexcept {
      const auto meta = MetaDataOf<Decay<T>>();
      return meta and meta->template
         GetAbility<CT::Mutable<Deref<T>>>(mVerb, GetType());
   }

   /// Execute a known/unknown verb in an known/unknown context               
   ///   @attention assumes that if context is deep, it contains one item     
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb was executed                                    
   LANGULUS(INLINED)
   bool Verb::GenericExecuteIn(CT::Dense auto& context, CT::VerbBased auto& verb) {
      using T = Deref<decltype(context)>;
      using V = Deref<decltype(verb)>;

      if constexpr (not CT::Deep<T> and CT::Verb<V>) {
         // Always prefer statically optimized routine when available   
         // Literally zero ability searching overhead!                  
         if constexpr (V::template AvailableFor<T>())
            return V::ExecuteIn(context, verb);
         return false;
      }
      else {
         // Search for the ability via RTTI                             
         const DMeta fromMeta = context.GetType();
         DMeta toMeta;

         if constexpr (CT::DerivedFrom<V, Verbs::Interpret>
         and requires { typename V::Type; }) {
            // Scan for a reflected converter as statically as possible 
            toMeta = MetaDataOf<typename V::Type>();
         }
         else if (verb.template IsVerb<Verbs::Interpret>()) {
            // Scan for a reflected converter by scanning argument      
            toMeta = verb.template As<DMeta>();
         }

         const auto foundConverter = fromMeta->GetConverter(toMeta);
         if (foundConverter) {
            // Converter was found, prioritize it                       
            // No escape from this scope                                
            auto result = Many::FromMeta(toMeta);
            result.template Reserve<true>(1);
            foundConverter(context.GetRaw(), result.GetRaw());
            verb << Abandon(result);
            return verb.IsDone();
         }

         // Scan for any other runtime ability                          
         const auto foundAbility = fromMeta->template
            GetAbility<CT::Mutable<T>>(verb.GetVerb(), verb.GetType());
         if (not foundAbility)
            return false;

         foundAbility(context.GetRaw(), verb);
      }

      return verb.IsDone();
   }

   /// Execute a known/unknown verb with its default behavior inside a        
   /// mutable context                                                        
   ///   @attention assumes that context contains exactly one item            
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if verb was executed                                    
   LANGULUS(INLINED)
   bool Verb::GenericExecuteDefault(Block& context, CT::VerbBased auto& verb) {
      using V = Deref<decltype(verb)>;

      if constexpr (CT::Verb<V>) {
         // Always prefer statically optimized routine when available   
         // Literally zero ability searching overhead!                  
         if constexpr (CT::DefaultableVerb<V>)
            return V::ExecuteDefault(context, verb);
         else
            return false;
      }
      else {
         // Execute appropriate default routine by RTTI                 
         const auto vmeta = verb.GetVerb();
         if (not vmeta)
            return false;

         if (vmeta->mDefaultInvocationMutable) {
            vmeta->mDefaultInvocationMutable(context, verb);
            return verb.IsDone();
         }
         else if (vmeta->mDefaultInvocationConstant) {
            vmeta->mDefaultInvocationConstant(context, verb);
            return verb.IsDone();
         }
         else return false;
      }
   }

   /// Execute a known/unknown verb with its default behavior inside a        
   /// constant context                                                       
   ///   @attention assumes that context contains exactly one item            
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if verb was executed                                    
   LANGULUS(INLINED)
   bool Verb::GenericExecuteDefault(const Block& context, CT::VerbBased auto& verb) {
      using V = Deref<decltype(verb)>;

      if constexpr (CT::Verb<V>) {
         // Always prefer statically optimized routine when available   
         // Literally zero ability searching overhead!                  
         if constexpr (CT::DefaultableVerbConstant<V>)
            return V::ExecuteDefault(context, verb);
         else
            return false;
      }
      else {
         // Execute appropriate default routine by RTTI                 
         const auto vmeta = verb.GetVerb();
         if (not vmeta)
            return false;

         if (vmeta->mDefaultInvocationConstant) {
            vmeta->mDefaultInvocationConstant(context, verb);
            return verb.IsDone();
         }
         else return false;
      }
   }

   /// Execute a known/unknown verb without context                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if verb was executed                                    
   LANGULUS(INLINED)
   bool Verb::GenericExecuteStateless(CT::VerbBased auto& verb) {
      using V = Deref<decltype(verb)>;

      if constexpr (CT::Verb<V>) {
         // Always prefer statically optimized routine when available   
         // Literally zero ability searching overhead!                  
         if constexpr (CT::StatelessVerb<V>)
            return V::ExecuteStateless(verb);
         else
            return false;
      }
      else {
         // Execute appropriate stateless routine by RTTI               
         const auto vmeta = verb.GetVerb();
         if (not vmeta)
            return false;

         if (vmeta->mStatelessInvocation) {
            vmeta->mStatelessInvocation(verb);
            return verb.IsDone();
         }
         else return false;
      }
   }

   /// Serialize verb to code                                                 
   LANGULUS(INLINED)
   Verb::operator Code() const {
      Code result;
      SerializeVerb(result);
      return result;
   }
   
   /// Execute a verb in the resulting output of this one                     
   ///   @param verb - the verb to execute                                    
   ///   @return a reference to the new verb                                  
   template<CT::VerbBased V>
   V& Verb::Then(V& verb) const {
      DispatchDeep(mOutput, verb);
      return verb;
   }

   /// Execute a verb in the resulting output of this one                     
   ///   @param verb - the verb to execute                                    
   ///   @return a reference to the new verb                                  
   template<CT::VerbBased V>
   V& Verb::Then(V& verb) {
      DispatchDeep(mOutput, verb);
      return verb;
   }

} // namespace Langulus::Flow
