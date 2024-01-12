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
#include "VerbState.inl"
#include "verbs/Interpret.inl"


namespace Langulus::Flow
{

   /// Shallow-copy constructor                                               
   ///   @param other - the verb to shallow-copy                              
   LANGULUS(INLINED)
   Verb::Verb(const Verb& other)
      : Verb {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - the verb to move                                      
   LANGULUS(INLINED)
   Verb::Verb(Verb&& other)
      : Verb {Move(other)} {}

   /// Generic constructor                                                    
   ///   @param other - the verb/argument and semantic to construct with      
   template<CT::Data T1, CT::Data...TAIL>
   requires CT::VerbMakable<T1, TAIL...> LANGULUS(INLINED)
   Verb::Verb(T1&& t1, TAIL&&...tail) {
      static_assert(sizeof(Verb) == sizeof(A::Verb));
      if constexpr (sizeof...(TAIL) == 0 and not CT::Array<T1>) {
         using S = SemanticOf<T1>;
         using T = TypeOf<S>;

         if constexpr (CT::VerbBased<T>) {
            decltype(auto) verb = DesemCast(t1);
            Any::operator = (S::Nest(t1).template Forward<Any>());
            Charge::operator = (verb);
            mVerb = verb.mVerb;
            mState = verb.mState;
            mSource = S::Nest(verb.mSource);
            mOutput = S::Nest(verb.mOutput);
         }
         else Any::operator = (Forward<T1>(t1));
      }
      else Any::Insert(0, Forward<T1>(t1), Forward<TAIL>(tail)...);
   }

   /// Verb shallow-copy assignment                                           
   ///   @param rhs - the verb to shallow-copy assign                         
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (const Verb& rhs) {
      return operator = (Copy(rhs));
   }

   /// Verb move-assignment                                                   
   ///   @param rhs - the verb to move-assign                                 
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (Verb&& rhs) {
      return operator = (Move(rhs));
   }

   /// Generic assignment                                                     
   ///   @param rhs - the verb/argument and semantic to assign by             
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (CT::VerbAssignable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::VerbBased<T>) {
         decltype(auto) verb = DesemCast(rhs);
         Any::operator = (S::Nest(rhs).template Forward<Any>());
         Charge::operator = (verb);
         mVerb = verb.mVerb;
         mState = verb.mState;
         mSource = S::Nest(verb.mSource);
         mOutput = S::Nest(verb.mOutput);
      }
      else Any::operator = (S::Nest(rhs));
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
   Verb Verb::From(CT::Inner::UnfoldInsertable auto&& a, const Charge& charge, VerbState state) {
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
   Verb Verb::FromMeta(VMeta verb, CT::Inner::UnfoldInsertable auto&& a, const Charge& charge, VerbState state) {
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

   /// Check if verb is matches one of the provided verb types                
   ///   @tparam V1, VN... - the verbs to compare against                     
   ///   @return true if at least one verb matches                            
   template<CT::Verb V1, CT::Verb...VN> LANGULUS(INLINED)
   bool Verb::VerbIs() const noexcept {
      return mVerb == MetaVerbOf<V1>() or ((mVerb == MetaVerbOf<VN>()) or ...);
   }

   /// Check if verb has been satisfied at least once                         
   ///   @return true if verb has been satisfied at least once                
   LANGULUS(INLINED)
   bool Verb::IsDone() const noexcept {
      return mSuccesses > 0;
   }

   /// Check if verb is multicast                                             
   ///   @return true if verb is multicast                                    
   LANGULUS(INLINED)
   constexpr bool Verb::IsMulticast() const noexcept {
      return mState.IsMulticast();
   }

   /// Check if verb is monocast                                              
   ///   @return true if verb is monocast                                     
   LANGULUS(INLINED)
   constexpr bool Verb::IsMonocast() const noexcept {
      return mState.IsMonocast();
   }

   /// Check if verb is short-circuited                                       
   ///   @return true if verb is short-circuited                              
   LANGULUS(INLINED)
   constexpr bool Verb::IsShortCircuited() const noexcept {
      return mState.IsShortCircuited();
   }

   /// Check if verb is long-circuited                                        
   ///   @return true if verb is long-circuited                               
   LANGULUS(INLINED)
   constexpr bool Verb::IsLongCircuited() const noexcept {
      return mState.IsLongCircuited();
   }

   /// Get the verb state                                                     
   ///   @return the verb state                                               
   LANGULUS(INLINED)
   const VerbState& Verb::GetVerbState() const noexcept {
      return mState;
   }

   /// Set the verb state                                                     
   ///   @param state - the verb state                                        
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::SetVerbState(const VerbState& state) noexcept {
      mState = state;
      return *this;
   }

   /// Get the number of successful execution of the verb                     
   ///   @return the number of successful executions                          
   LANGULUS(INLINED)
   Count Verb::GetSuccesses() const noexcept {
      return mSuccesses;
   }

   /// Check if anything inside the verb is missing on the surface level      
   ///   @return true if anything is missing                                  
   LANGULUS(INLINED)
   bool Verb::IsMissing() const noexcept {
      return mSource.IsMissing()
          or Any::IsMissing()
          or mOutput.IsMissing();
   }

   /// Check if anything inside the verb is missing deeply                    
   ///   @return true if anything is missing                                  
   LANGULUS(INLINED)
   bool Verb::IsMissingDeep() const noexcept {
      return mSource.IsMissingDeep()
          or Any::IsMissingDeep()
          or mOutput.IsMissingDeep();
   }

   /// Satisfy verb a number of times                                         
   LANGULUS(INLINED)
   void Verb::Done(Count c) noexcept {
      mSuccesses = c;
   }

   /// Satisfy verb once                                                      
   LANGULUS(INLINED)
   void Verb::Done() noexcept {
      ++mSuccesses;
   }

   /// Reset verb satisfaction, clear output                                  
   LANGULUS(INLINED)
   void Verb::Undo() noexcept {
      mSuccesses = 0;
      mOutput.Reset();
   }

   /// Invert the verb (use the antonym)                                      
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::Invert() noexcept {
      mMass *= Real {-1};
      return *this;
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
   LANGULUS(INLINED)
   Verb& Verb::SetMass(const Real mass) noexcept {
      mMass = mass;
      return *this;
   }

   /// Set the verb frequency                                                 
   ///   @param rate - the rate to set                                        
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetRate(const Real rate) noexcept {
      mRate = rate;
      return *this;
   }

   /// Set the verb time                                                      
   ///   @param time - the time to set                                        
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetTime(const Real time) noexcept {
      mTime = time;
      return *this;
   }

   /// Set the verb priority                                                  
   ///   @param priority - the priority to set                                
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetPriority(const Real priority) noexcept {
      mPriority = priority;
      return *this;
   }

   /// Set the verb mass, frequency, time, and priority (a.k.a. charge)       
   ///   @param charge - the charge to set                                    
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetCharge(const Charge& charge) noexcept {
      Charge::operator = (charge);
      return *this;
   }

   /// Set the verb's source                                                  
   ///   @param t1, tail...  - the values to assign                           
   ///   @return a reference to self                                          
   template<CT::Data T1, CT::Data... TAIL>
   requires CT::Inner::UnfoldInsertable<T1, TAIL...> LANGULUS(INLINED)
   Verb& Verb::SetSource(T1&& t1, TAIL&&...tail) {
      mSource = Any {Forward<T1>(t1), Forward<TAIL>(tail)...};
      // We guarantee that source is exactly Any, so we unconstrain it  
      // in order to be safely able to overwrite it anytime             
      mSource.MakeTypeConstrained(false);
      return *this;
   }
   
   /// Set the verb's argument                                                
   ///   @param t1, tail...  - the values to assign                           
   ///   @return a reference to self                                          
   template<CT::Data T1, CT::Data... TAIL>
   requires CT::Inner::UnfoldInsertable<T1, TAIL...> LANGULUS(INLINED)
   Verb& Verb::SetArgument(T1&& t1, TAIL&&...tail) {
      Any::operator = (Any {Forward<T1>(t1), Forward<TAIL>(tail)...});
      // We guarantee that argument is exactly Any, so we unconstrain it
      // in order to be safely able to overwrite it anytime             
      MakeTypeConstrained(false);
      return *this;
   }
   
   /// Set the verb's output                                                  
   ///   @param t1, tail...  - the values to assign                           
   ///   @return a reference to self                                          
   template<CT::Data T1, CT::Data... TAIL>
   requires CT::Inner::UnfoldInsertable<T1, TAIL...> LANGULUS(INLINED)
   Verb& Verb::SetOutput(T1&& t1, TAIL&&...tail) {
      mOutput = Any {Forward<T1>(t1), Forward<TAIL>(tail)...};
      // We guarantee that output is exactly Any, so we unconstrain it  
      // in order to be safely able to overwrite it anytime             
      mOutput.MakeTypeConstrained(false);
      return *this;
   }

   /// Compare verbs                                                          
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   LANGULUS(INLINED)
   bool Verb::operator == (const Verb& rhs) const {
      return mVerb == rhs.mVerb
         and mSource == rhs.mSource
         and Any::operator == (rhs.GetArgument())
         and mOutput == rhs.mOutput
         and mState == rhs.mState;
   }

   /// Compare verb types for equality                                        
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   LANGULUS(INLINED)
   bool Verb::operator == (VMeta rhs) const noexcept {
      return mVerb == rhs;
   }

   /// Check if verb is satisfied at least once                               
   ///   @param rhs - flag to compare against                                 
   ///   @return true if verb satisfaction matches rhs                        
   LANGULUS(INLINED)
   bool Verb::operator == (const bool rhs) const noexcept {
      return IsDone() == rhs; 
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has larger or equal priority                     
   LANGULUS(INLINED)
   bool Verb::operator < (const Verb& ext) const noexcept {
      return mPriority < ext.mPriority;
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has smaller or equal priority                    
   LANGULUS(INLINED)
   bool Verb::operator > (const Verb& ext) const noexcept {
      return mPriority > ext.mPriority;
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has smaller priority                             
   LANGULUS(INLINED)
   bool Verb::operator >= (const Verb& ext) const noexcept {
      return mPriority >= ext.mPriority;
   }

   /// Compare verb priorities                                                
   ///   @param rhs - the verb to compare against                             
   ///   @return true if rhs has larger priority                              
   LANGULUS(INLINED)
   bool Verb::operator <= (const Verb& rhs) const noexcept {
      return mPriority <= rhs.mPriority;
   }

   /// Get the verb id                                                        
   ///   @return verb ID                                                      
   LANGULUS(INLINED)
   VMeta Verb::GetVerb() const noexcept {
      return mVerb;
   }

   /// Get the verb id and charge                                             
   ///   @return verb charge                                                  
   LANGULUS(INLINED)
   const Charge& Verb::GetCharge() const noexcept {
      return static_cast<const Charge&>(*this);
   }

   /// Get the verb mass (a.k.a. magnitude)                                   
   ///   @return the current mass                                             
   LANGULUS(INLINED)
   Real Verb::GetMass() const noexcept {
      return mMass;
   }

   /// Get the verb frequency                                                 
   ///   @return the current frequency                                        
   LANGULUS(INLINED)
   Real Verb::GetRate() const noexcept {
      return mRate;
   }

   /// Get the verb time                                                      
   ///   @return the current time                                             
   LANGULUS(INLINED)
   Real Verb::GetTime() const noexcept {
      return mTime;
   }

   /// Get the verb priority                                                  
   ///   @return the current priority                                         
   LANGULUS(INLINED)
   Real Verb::GetPriority() const noexcept {
      return mPriority;
   }

   /// Get verb source                                                        
   ///   @return the verb source                                              
   LANGULUS(INLINED)
   Any& Verb::GetSource() noexcept {
      return mSource;
   }

   /// Get verb source (constant)                                             
   ///   @return the verb source                                              
   LANGULUS(INLINED)
   const Any& Verb::GetSource() const noexcept {
      return mSource;
   }

   /// Get verb argument                                                      
   ///   @return the verb argument                                            
   LANGULUS(INLINED)
   Any& Verb::GetArgument() noexcept {
      return static_cast<Any&>(*this);
   }

   /// Get verb argument (constant)                                           
   ///   @return the verb argument                                            
   LANGULUS(INLINED)
   const Any& Verb::GetArgument() const noexcept {
      return static_cast<const Any&>(*this);
   }

   /// Get verb output                                                        
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   Any& Verb::GetOutput() noexcept {
      return mOutput;
   }

   /// Get verb output (constant)                                             
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   const Any& Verb::GetOutput() const noexcept {
      return mOutput;
   }

   /// Convenience operator for accessing the output container inside verb    
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   const Any* Verb::operator -> () const noexcept {
      return &mOutput;
   }

   /// Convenience operator for accessing the output container inside verb    
   ///   @return the verb output                                              
   LANGULUS(INLINED)
   Any* Verb::operator -> () noexcept {
      return &mOutput;
   }

   /// Push anything to end of the outputs, satisfying the verb               
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param rhs - the data (and semantic) to push                         
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator <<  (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<T>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *this;
         }

         if (mOutput.SmartPush(IndexBack, S::Nest(rhs)))
            Done();
      }

      return *this;
   }

   /// Push anything to the front of the outputs, satisfying the verb         
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param rhs - the data (and semantic) to push                         
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >>  (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<T>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *this;
         }

         if (mOutput.SmartPush(IndexFront, S::Nest(rhs)))
            Done();
      }

      return *this;
   }

   /// Merge anything to output's back by a semantic                          
   ///   @param rhs - the data (and semantic) to merge                        
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator <<= (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<TypeOf<S>>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *this;
         }

         auto ptr = PointerDecay(DesemCast(rhs));
         if (mOutput.Find(ptr))
            return *this;

         if (mOutput.SmartPush(IndexBack, ptr))
            Done();
      }

      return *this;
   }

   /// Merge anything to output's front by a semantic                         
   ///   @param rhs - the data (and semantic) to merge                        
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >>= (CT::Inner::UnfoldInsertable auto&& rhs) {
      using S = SemanticOf<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (not CT::Nullptr<T>) {
         if constexpr (CT::PointerRelated<TypeOf<S>>) {
            // Push a pointer, but check if valid first                 
            if (not DesemCast(rhs))
               return *this;
         }

         auto ptr = PointerDecay(DesemCast(rhs));
         if (mOutput.Find(ptr))
            return *this;

         if (mOutput.SmartPush(IndexBack, ptr))
            Done();
      }

      return *this;
   }

   /// Finalize a dispatch execution by setting satisfaction state and output 
   ///   @tparam OR - whether the dispatch happened in an OR context or not   
   ///   @param successes - number of successes                               
   ///   @param output - the output container                                 
   ///   @return the number of successes for the verb                         
   template<bool OR> LANGULUS(INLINED)
   Count Verb::CompleteDispatch(const Count successes, Abandoned<Any>&& output) {
      if (IsShortCircuited()) {
         // If reached, this will result in failure in OR-context, or   
         // success if AND, as long as the verb is short-circuited      
         if constexpr (OR)
            mSuccesses = 0;
         else
            mSuccesses = successes;
      }
      else {
         // If verb is not short-circuited, then a single success       
         // is always enough                                            
         mSuccesses = successes;
      }

      // Set output                                                     
      if (mSuccesses)
         mOutput = output.Forward<Any>();
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
         const auto meta = context.GetType();
         if constexpr (CT::DerivedFrom<V, Verbs::Interpret>
         and requires { typename V::Type; }) {
            // Scan for a reflected converter as statically as possible 
            using TO = typename V::Type;
            const auto found = meta->template GetConverter<TO>();
            if (found) {
               // Converter was found, prioritize it                    
               // No escape from this scope                             
               auto result = Any::From<TO>();
               result.Reserve<true>(1);
               found(context.GetRaw(), result.GetRaw());
               verb << Abandon(result);
               return verb.IsDone();
            }
         }
         else if (verb.template VerbIs<Verbs::Interpret>()) {
            // Scan for a reflected converter by scanning argument      
            const auto to = verb.template As<DMeta>();
            const auto found = meta->GetConverter(to);
            if (found) {
               // Converter was found, prioritize it                    
               // No escape from this scope                             
               auto result = Any::FromMeta(to);
               result.Reserve<true>(1);
               found(context.GetRaw(), result.GetRaw());
               verb << Abandon(result);
               return verb.IsDone();
            }
         }

         // Scan for any other runtime ability                          
         const auto found = meta->template
            GetAbility<CT::Mutable<T>>(verb.mVerb, verb.GetType());
         if (not found)
            return false;

         found(context.GetRaw(), verb);
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
         if (verb.mVerb->mDefaultInvocationMutable) {
            verb.mVerb->mDefaultInvocationMutable(context, verb);
            return verb.IsDone();
         }
         else if (verb.mVerb->mDefaultInvocationConstant) {
            verb.mVerb->mDefaultInvocationConstant(context, verb);
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
         if (verb.mVerb->mDefaultInvocationConstant) {
            verb.mVerb->mDefaultInvocationConstant(context, verb);
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
         if (verb.mVerb->mStatelessInvocation) {
            verb.mVerb->mStatelessInvocation(verb);
            return verb.IsDone();
         }
         else return false;
      }
   }

   
   ///                                                                        
   ///   Static verb implementation                                           
   ///                                                                        
   template<class VERB> LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(const StaticVerb& other)
      : Verb {Copy(other)} {}

   template<class VERB> LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(StaticVerb&& other)
      : Verb {Move(other)} {}

   template<class VERB> LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (const StaticVerb& rhs) {
      Verb::operator = (Copy(rhs));
      return *this;
   }

   template<class VERB> LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (StaticVerb&& rhs) {
      Verb::operator = (Move(rhs));
      return *this;
   }

   template<class VERB> LANGULUS(INLINED)
   VMeta StaticVerb<VERB>::GetVerb() const noexcept {
      static_assert(sizeof(StaticVerb) == sizeof(A::Verb));
      return (mVerb = MetaVerbOf<VERB>());
   }

} // namespace Langulus::Flow
