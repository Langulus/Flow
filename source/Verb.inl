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

   /// Manual construction                                                    
   ///   @param state - the state                                             
   LANGULUS(INLINED)
   constexpr VerbState::VerbState(const Type& state) noexcept
      : mState {state} {}

   /// Explicit convertion to bool                                            
   ///   @return true if state is not default                                 
   LANGULUS(INLINED)
   constexpr VerbState::operator bool() const noexcept {
      return not IsDefault();
   }
   
   /// Combine two states                                                     
   ///   @param rhs - the other state                                         
   ///   @return a new combined state                                         
   LANGULUS(INLINED)
   constexpr VerbState VerbState::operator + (const VerbState& rhs) const noexcept {
      return mState | rhs.mState;
   }
   
   /// Remove rhs state from this state                                       
   ///   @param rhs - the other state                                         
   ///   @return a new leftover state                                         
   LANGULUS(INLINED)
   constexpr VerbState VerbState::operator - (const VerbState& rhs) const noexcept {
      return mState & (~rhs.mState);
   }
   
   /// Destructively add state                                                
   ///   @param rhs - the other state                                         
   ///   @return a reference to this state                                    
   LANGULUS(INLINED)
   constexpr VerbState& VerbState::operator += (const VerbState& rhs) noexcept {
      mState |= rhs.mState;
      return *this;
   }
   
   /// Destructively remove state                                             
   ///   @param rhs - the other state                                         
   ///   @return a reference to this state                                    
   LANGULUS(INLINED)
   constexpr VerbState& VerbState::operator -= (const VerbState& rhs) noexcept {
      mState &= ~rhs.mState;
      return *this;
   }
   
   LANGULUS(INLINED)
   constexpr bool VerbState::operator & (const VerbState& rhs) const noexcept {
      return (mState & rhs.mState) == rhs.mState;
   }
   
   LANGULUS(INLINED)
   constexpr bool VerbState::operator % (const VerbState& rhs) const noexcept {
      return (mState & rhs.mState) == 0;
   }
   
   /// Check if default data state                                            
   /// Default state is inclusive, mutable, nonpolar, nonvacuum, nonstatic,   
   /// nonencrypted, noncompressed, untyped, and dense                        
   LANGULUS(INLINED)
   constexpr bool VerbState::IsDefault() const noexcept {
      return mState == VerbState::Default;
   }
   
   /// Check if state is multicast                                            
   LANGULUS(INLINED)
   constexpr bool VerbState::IsMulticast() const noexcept {
      return (mState & VerbState::Monocast) == 0;
   }
   
   /// Check if state is monocast                                             
   LANGULUS(INLINED)
   constexpr bool VerbState::IsMonocast() const noexcept {
      return mState & VerbState::Monocast;
   }
   
   /// Check if state is long-circuited                                       
   LANGULUS(INLINED)
   constexpr bool VerbState::IsLongCircuited() const noexcept {
      return mState & VerbState::LongCircuited;
   }
   
   /// Check if state is short-circuited                                      
   LANGULUS(INLINED)
   constexpr bool VerbState::IsShortCircuited() const noexcept {
      return (mState & VerbState::LongCircuited) == 0;
   }

   /// Verb shallow-copy constructor                                          
   ///   @param other - the verb to shallow-copy                              
   LANGULUS(INLINED)
   Verb::Verb(const Verb& other)
      : Verb {Copy(other)} {}

   /// Verb move constructor                                                  
   ///   @param other - the verb to move                                      
   LANGULUS(INLINED)
   Verb::Verb(Verb&& other)
      : Verb {Move(other)} {}

   /// Verb argument copy-constructor                                         
   ///   @param other - the argument to shallow-copy                          
   LANGULUS(INLINED)
   Verb::Verb(const CT::NotSemantic auto& other)
      : Verb {Copy(other)} {}

   /// Verb argument copy-constructor                                         
   ///   @param other - the argument to shallow-copy                          
   LANGULUS(INLINED)
   Verb::Verb(CT::NotSemantic auto& other)
      : Verb {Copy(other)} {}

   /// Verb argument move-constructor                                         
   ///   @param other - the argument to move                                  
   LANGULUS(INLINED)
   Verb::Verb(CT::NotSemantic auto&& other)
      : Verb {Move(other)} {}

   /// Verb semantic-constructor                                              
   ///   @param other - the verb/argument and semantic to construct with      
   LANGULUS(INLINED)
   Verb::Verb(CT::Semantic auto&& other)
      : Any {CT::VerbBased<TypeOf<Decay<decltype(other)>>>
         ? Any {other.template Forward<Any>()}
         : Any {other.Forward()}
      } {
      using S = Decay<decltype(other)>;
      if constexpr (CT::VerbBased<TypeOf<S>>) {
         Charge::operator = (*other);
         mVerb = other->mVerb;
         mState = other->mState;
         mSource = S::Nest(other->mSource);
         mOutput = S::Nest(other->mOutput);
      }
   }

   /// Verb list-of-arguments constructor                                     
   ///   @param head, tail... - arguments                                     
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   LANGULUS(INLINED)
   Verb::Verb(T1&& t1, T2&& t2, TAIL&&... tail)
      : Any {Forward<T1>(t1), Forward<T2>(t2), Forward<TAIL>(tail)...} {}

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

   /// Verb semantic-assignment                                               
   ///   @param rhs - the verb/argument and semantic to assign by             
   ///   @return a reference to this verb                                     
   LANGULUS(INLINED)
   Verb& Verb::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      using T = TypeOf<S>;

      if constexpr (CT::VerbBased<T>) {
         Any::operator = (rhs.template Forward<Any>());
         Charge::operator = (*rhs);
         mVerb = rhs->mVerb;
         mState = rhs->mState;
         mSource = S::Nest(rhs->mSource);
         mOutput = S::Nest(rhs->mOutput);
      }
      else LANGULUS_ERROR("Bad verb assignment");
      return *this;
   }

   /// Create a statically typed verb with charge and state                   
   ///   @tparam VERB - the verb type                                         
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   template<CT::Data VERB>
   LANGULUS(INLINED)
   Verb Verb::From(const Charge& charge, const VerbState& state) {
      Verb result;
      result.template SetVerb<VERB>();
      result.SetCharge(charge);
      result.SetVerbState(state);
      return result;
   }

   /// Create a statically typed verb with argument, charge and state         
   ///   @tparam VERB - the verb type                                         
   ///   @tparam DATA - the argument type (deducible)                         
   ///   @param argument - argument to move in                                
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   template<CT::Data VERB, CT::Data DATA>
   LANGULUS(INLINED)
   Verb Verb::From(DATA&& argument, const Charge& charge, const VerbState& state) {
      Verb result;
      result.template SetVerb<VERB>();
      result.SetArgument(Forward<DATA>(argument));
      result.SetCharge(charge);
      result.SetVerbState(state);
      return result;
   }

   /// Create a dynamically typed verb with argument, charge and state        
   ///   @tparam DATA - the argument type (deducible)                         
   ///   @param verb - type of the verb                                       
   ///   @param argument - argument to move in                                
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   template<CT::Data DATA>
   LANGULUS(INLINED)
   Verb Verb::FromMeta(VMeta verb, DATA&& argument, const Charge& charge, const VerbState& state) {
      Verb result;
      result.SetVerb(verb);
      result.SetArgument(Forward<DATA>(argument));
      result.SetCharge(charge);
      result.SetVerbState(state);
      return result;
   }

   /// Create a dynamically typed verb with charge and state                  
   ///   @param verb - type of the verb                                       
   ///   @param charge - verb charge                                          
   ///   @param state - verb state                                            
   ///   @return the new Verb instance                                        
   LANGULUS(INLINED)
   Verb Verb::FromMeta(VMeta verb, const Charge& charge, const VerbState& state) {
      Verb result;
      result.SetVerb(verb);
      result.SetCharge(charge);
      result.SetVerbState(state);
      return result;
   }

   /// Check if verb is of a specific type                                    
   ///   @tparam T - the verb to compare against                              
   ///   @return true if verbs match                                          
   template<CT::Data... T>
   LANGULUS(INLINED)
   bool Verb::VerbIs() const noexcept {
      static_assert(CT::Verb<T...>, "Provided types must be verb definitions");
      return (VerbIs(T::GetVerb()) or ...);
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

   /// Set the verb ID                                                        
   ///   @param verb - the verb to assign                                     
   ///   @return a reference to self                                          
   template<CT::Data VERB>
   LANGULUS(INLINED)
   Verb& Verb::SetVerb() {
      static_assert(CT::Verb<VERB>, "VERB must be a verb type");
      mVerb = VERB::GetVerb();
      return *this;
   }

   /// Set the verb ID                                                        
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

   /// Set the verb's source by shallow-copy                                  
   ///   @param value  - the value to shallow-copy                            
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetSource(const CT::NotSemantic auto& value) {
      return SetSource(Copy(value));
   }

   LANGULUS(INLINED)
   Verb& Verb::SetSource(CT::NotSemantic auto& value) {
      return SetSource(Copy(value));
   }

   /// Set the verb's source by move                                          
   ///   @param value  - the value to move                                    
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetSource(CT::NotSemantic auto&& value) {
      return SetSource(Move(value));
   }
   
   /// Set the verb's source by a semantic                                    
   ///   @param value  - the value and semantic to use                        
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetSource(CT::Semantic auto&& value) {
      mSource = value.Forward();
      // We guarantee that source is exactly Any, so we unconstrain it  
      // in order to be safely able to overwrite it anytime             
      mSource.MakeTypeConstrained(false);
      return *this;
   }
   
   /// Set the verb's argument by shallow-copy                                
   ///   @param value  - the value to shallow-copy                            
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetArgument(const CT::NotSemantic auto& value) {
      return SetArgument(Copy(value));
   }

   LANGULUS(INLINED)
   Verb& Verb::SetArgument(CT::NotSemantic auto& value) {
      return SetArgument(Copy(value));
   }

   /// Set the verb's argument by move                                        
   ///   @param value  - the value to move                                    
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetArgument(CT::NotSemantic auto&& value) {
      return SetArgument(Move(value));
   }
   
   /// Set the verb's argument by a semantic                                  
   ///   @param value  - the value and semantic to use                        
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetArgument(CT::Semantic auto&& value) {
      Any::operator = (value.Forward());
      // We guarantee that argument is exactly Any, so we unconstrain it
      // in order to be safely able to overwrite it anytime             
      MakeTypeConstrained(false);
      return *this;
   }
   
   /// Set the verb's output by shallow-copy                                  
   ///   @param value  - the value to shallow-copy                            
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetOutput(const CT::NotSemantic auto& value) {
      return SetOutput(Copy(value));
   }

   LANGULUS(INLINED)
   Verb& Verb::SetOutput(CT::NotSemantic auto& value) {
      return SetOutput(Copy(value));
   }

   /// Set the verb's output by move                                          
   ///   @param value  - the value to move                                    
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetOutput(CT::NotSemantic auto&& value) {
      return SetOutput(Move(value));
   }
   
   /// Set the verb's output by a semantic                                    
   ///   @param value  - the value and semantic to use                        
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetOutput(CT::Semantic auto&& value) {
      mOutput = value.Forward();
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
      return (mVerb == rhs.mVerb or (mVerb and mVerb->Is(rhs.mVerb)))
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
      return VerbIs(rhs); 
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

   /// Push anything to end of outputs by shallow-copy, satisfying the verb   
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data to push (deducible)                           
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator << (const CT::NotSemantic auto& data) {
      return operator << (Copy(data));
   }
   
   /// Push anything to end of outputs by shallow-copy, satisfying the verb   
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data to push (deducible)                           
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator << (CT::NotSemantic auto& data) {
      return operator << (Copy(data));
   }
   
   /// Push anything to end of outputs by move, satisfying the verb           
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data to push (deducible)                           
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator << (CT::NotSemantic auto&& data) {
      return operator << (Move(data));
   }
   
   /// Push anything to end of the outputs via semantic, satisfying the verb  
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data and semantic to push (deducible)              
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator << (CT::Semantic auto&& data) {
      using S = Decay<decltype(data)>;
      using T = TypeOf<S>;

      if constexpr (CT::Nullptr<T>) {
         // Can't push a nullptr_t                                      
         return *this;
      }
      else if constexpr (CT::PointerRelated<TypeOf<S>>) {
         // Push a pointer, but check if valid first                    
         if (not *data)
            return *this;
         if (mOutput.SmartPush(PointerDecay(*data)))
            Done();
         return *this;
      }
      else {
         // Push anything dense                                         
         if (mOutput.SmartPush(data.Forward()))
            Done();
         return *this;
      }
   }
   
   /// Push anything to start of outputs by shallow-copy, satisfying the verb 
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data to push (deducible)                           
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >> (const CT::NotSemantic auto& data) {
      return operator >> (Copy(data));
   }
   
   /// Push anything to start of outputs by shallow-copy, satisfying the verb 
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data to push (deducible)                           
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >> (CT::NotSemantic auto& data) {
      return operator >> (Copy(data));
   }
   
   /// Push anything to start of outputs by move, satisfying the verb         
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data to push (deducible)                           
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >> (CT::NotSemantic auto&& data) {
      return operator >> (Move(data));
   }
   
   /// Push anything to start of outputs via semantic, satisfying the verb    
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param data - the data and semantic to push (deducible)              
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >> (CT::Semantic auto&& data) {
      using S = Decay<decltype(data)>;
      using T = TypeOf<S>;

      if constexpr (CT::Nullptr<T>)
         // Can't push a nullptr_t                                      
         return *this;
      else if constexpr (CT::PointerRelated<TypeOf<S>>) {
         // Push a pointer, but check if valid first                    
         if (not *data)
            return *this;
         if (mOutput.SmartPush<IndexFront>(PointerDecay(*data)))
            Done();
         return *this;
      }
      else {
         // Push anything dense                                         
         if (mOutput.SmartPush<IndexFront>(data.Forward()))
            Done();
         return *this;
      }
   }

   /// Merge anything to output's back                                        
   ///   @param data - the data to merge (deducible)                          
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator <<= (const CT::NotSemantic auto& data) {
      return operator <<= (Copy(data));
   }
   
   /// Merge anything to output's back                                        
   ///   @param data - the data to merge (deducible)                          
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator <<= (CT::NotSemantic auto& data) {
      return operator <<= (Copy(data));
   }
   
   /// Merge anything to output's back                                        
   ///   @param data - the data to merge (deducible)                          
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator <<= (CT::NotSemantic auto&& data) {
      return operator <<= (Move(data));
   }
   
   /// Merge anything to output's back by a semantic                          
   ///   @param data - the data and semantic to merge (deducible)             
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator <<= (CT::Semantic auto&& data) {
      using S = Decay<decltype(data)>;
      using T = TypeOf<S>;

      if constexpr (CT::Nullptr<T>)
         // Can't push a nullptr_t                                      
         return *this;
      else if constexpr (CT::PointerRelated<TypeOf<S>>) {
         // Push a pointer, but check if valid first                    
         if (not *data)
            return *this;

         auto ptr = PointerDecay(*data);
         if (mOutput.Find(ptr))
            return *this;

         if (mOutput.SmartPush(ptr))
            Done();
      }
      else {
         // Push anything dense                                         
         if (mOutput.Find(*data))
            return *this;

         if (mOutput.SmartPush(*data))
            Done();
      }

      return *this;
   }
   
   /// Merge anything to output's front                                       
   ///   @param data - the data to merge (deducible)                          
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >>= (const CT::NotSemantic auto& data) {
      return operator >>= (Copy(data));
   }
   
   /// Merge anything to output's front                                       
   ///   @param data - the data to merge (deducible)                          
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >>= (CT::NotSemantic auto& data) {
      return operator >>= (Copy(data));
   }
   
   /// Merge anything to output's front                                       
   ///   @param data - the data to merge (deducible)                          
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >>= (CT::NotSemantic auto&& data) {
      return operator >>= (Move(data));
   }
   
   /// Merge anything to output's front by a semantic                         
   ///   @param data - the data and semantic to merge (deducible)             
   ///   @return a reference to this verb for chaining                        
   LANGULUS(INLINED)
   Verb& Verb::operator >>= (CT::Semantic auto&& data) {
      using S = Decay<decltype(data)>;
      using T = TypeOf<S>;

      if constexpr (CT::Nullptr<T>)
         // Can't push a nullptr_t                                      
         return *this;
      else if constexpr (CT::PointerRelated<TypeOf<S>>) {
         // Push a pointer, but check if valid first                    
         if (not *data)
            return *this;

         auto ptr = PointerDecay(*data);
         if (mOutput.Find(ptr)) //TODO: find deep instead?
            return *this;

         if (mOutput.SmartPush<IndexFront>(ptr))
            Done();
      }
      else {
         // Push anything dense                                         
         if (mOutput.Find(*data))
            return *this;

         if (mOutput.SmartPush<IndexFront>(*data))
            Done();
      }

      return *this;
   }

   /// Finalize a dispatch execution by setting satisfaction state and output 
   ///   @tparam OR - whether the dispatch happened in an OR context or not   
   ///   @param successes - number of successes                               
   ///   @param output - the output container                                 
   ///   @return the number of successes for the verb                         
   template<bool OR>
   LANGULUS(INLINED)
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
   template<CT::Dense T>
   LANGULUS(INLINED)
   bool Verb::GenericAvailableFor() const noexcept {
      const auto meta = MetaOf<Decay<T>>();
      return meta and meta->template GetAbility<CT::Mutable<T>>(mVerb, GetType());
   }

   /// Execute a known/unknown verb in an known/unknown context               
   ///   @attention assumes that if T is deep, it contains exactly one item   
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb was executed                                    
   template<CT::Dense T, CT::Data V>
   LANGULUS(INLINED)
   bool Verb::GenericExecuteIn(T& context, V& verb) {
      static_assert(CT::VerbBased<V>, "V must be VerbBased");

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
         if constexpr (CT::DerivedFrom<V, Verbs::Interpret> and requires { typename V::Type; }) {
            // Scan for a reflected converter as statically as possible 
            using TO = typename V::Type;
            const auto found = meta->template GetConverter<TO>();
            if (found) {
               // Converter was found, prioritize it                    
               // No escape from this scope                             
               auto result = Block::From<TO>();
               result.AllocateFresh(result.RequestSize(1));
               result.mCount = 1;
               found(context.GetRaw(), result.GetRaw());
               verb << Abandon(result);
               result.Free();
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
               Block result {to};
               result.AllocateFresh(result.RequestSize(1));
               result.mCount = 1;
               found(context.GetRaw(), result.GetRaw());
               verb << Abandon(result);
               result.Free();
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
   ///   @tparam V - the verb type (deducible)                                
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if verb was executed                                    
   template<CT::Data V>
   LANGULUS(INLINED)
   bool Verb::GenericExecuteDefault(Block& context, V& verb) {
      static_assert(CT::VerbBased<V>, "V must be VerbBased");

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
   ///   @tparam V - the verb type (deducible)                                
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if verb was executed                                    
   template<CT::Data V>
   LANGULUS(INLINED)
   bool Verb::GenericExecuteDefault(const Block& context, V& verb) {
      static_assert(CT::VerbBased<V>, "V must be VerbBased");

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
   ///   @tparam V - the verb type (deducible)                                
   ///   @param verb - the verb instance to execute                           
   ///   @return true if verb was executed                                    
   template<CT::Data V>
   LANGULUS(INLINED)
   bool Verb::GenericExecuteStateless(V& verb) {
      static_assert(CT::VerbBased<V>, "V must be VerbBased");

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
   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb() {
      SetVerb<VERB>();
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(const StaticVerb& other)
      : Verb {Copy(other)} {}

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(StaticVerb&& other)
      : Verb {Move(other)} {}

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(const CT::NotSemantic auto& other)
      : Verb {Copy(other)} {
      SetVerb<VERB>();
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(CT::NotSemantic auto& other)
      : Verb {Copy(other)} {
      SetVerb<VERB>();
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(CT::NotSemantic auto&& other)
      : Verb {Move(other)} {
      SetVerb<VERB>();
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(CT::Semantic auto&& other)
      : Verb {other.Forward()} {
      SetVerb<VERB>();
   }

   template<class VERB>
   template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(T1&& t1, T2&& t2, TAIL&&... tail)
      : Verb {Forward<T1>(t1), Forward<T2>(t2), Forward<TAIL>(tail)...} {
      SetVerb<VERB>();
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (const StaticVerb& rhs) {
      return operator = (Copy(rhs));
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (StaticVerb&& rhs) {
      return operator = (Move(rhs));
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (CT::Semantic auto&& rhs) {
      using S = Decay<decltype(rhs)>;
      if constexpr (CT::VerbBased<TypeOf<S>>) {
         Any::operator = (rhs.template Forward<Any>());
         mSuccesses = rhs->mSuccesses;
         mState = rhs->mState;
         mSource = S::Nest(rhs->mSource);
         mOutput = S::Nest(rhs->mOutput);
      }
      else LANGULUS_ERROR("Bad verb assignment");
      return *this;
   }

   template<class VERB>
   LANGULUS(INLINED)
   VMeta StaticVerb<VERB>::GetVerb() {
      return RTTI::MetaVerb::Of<VERB>();
   }

} // namespace Langulus::Flow
