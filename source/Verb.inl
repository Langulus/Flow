///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Verb.hpp"
#include "Code.hpp"

namespace Langulus::Flow
{

   /// Charge construction                                                    
   ///   @param mass - the mass charge                                        
   ///   @param freq - the frequency charge                                   
   ///   @param time - the time charge                                        
   ///   @param prio - the priority charge                                    
   LANGULUS(INLINED)
   constexpr Charge::Charge(Real mass, Real freq, Real time, Real prio) noexcept
      : mMass {mass}
      , mFrequency {freq}
      , mTime {time}
      , mPriority {prio} {}

   /// Compare charges                                                        
   ///   @param rhs - the charge to compare against                           
   ///   @return true if both charges match exactly                           
   LANGULUS(INLINED)
   constexpr bool Charge::operator == (const Charge& rhs) const noexcept {
      return mMass == rhs.mMass
         && mFrequency == rhs.mFrequency
         && mTime == rhs.mTime
         && mPriority == rhs.mPriority;
   }

   /// Check if charge is default                                             
   ///   @return true if charge is default                                    
   LANGULUS(INLINED)
   constexpr bool Charge::IsDefault() const noexcept {
      return *this == Charge {};
   }

   /// Check if charge is default                                             
   ///   @return true if charge is default                                    
   LANGULUS(INLINED)
   constexpr bool Charge::IsFlowDependent() const noexcept {
      return mFrequency != DefaultFrequency
         || mTime != DefaultTime
         || mPriority != DefaultPriority;
   }

   /// Get the hash of the charge                                             
   ///   @return the hash of the charge                                       
   LANGULUS(INLINED)
   Hash Charge::GetHash() const noexcept {
      return HashOf(mMass, mFrequency, mTime, mPriority);
   }

   /// Reset the charge to the default                                        
   LANGULUS(INLINED)
   void Charge::Reset() noexcept {
      mMass = DefaultMass;
      mFrequency = DefaultFrequency;
      mTime = DefaultTime;
      mPriority = DefaultPriority;
   }

   /// Manual construction                                                    
   ///   @param state - the state                                             
   LANGULUS(INLINED)
   constexpr VerbState::VerbState(const Type& state) noexcept
      : mState {state} {}

   /// Explicit convertion to bool                                            
   ///   @return true if state is not default                                 
   LANGULUS(INLINED)
   constexpr VerbState::operator bool() const noexcept {
      return !IsDefault();
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

   LANGULUS(INLINED)
   Verb::Verb(const Verb& other)
      : Verb {Langulus::Copy(other)} {}

   LANGULUS(INLINED)
   Verb::Verb(Verb&& other)
      : Verb {Langulus::Move(other)} {}

   LANGULUS(INLINED)
   Verb::Verb(const CT::NotSemantic auto& other)
      : Verb {Langulus::Copy(other)} {}

   LANGULUS(INLINED)
   Verb::Verb(CT::NotSemantic auto& other)
      : Verb {Langulus::Copy(other)} {}

   LANGULUS(INLINED)
   Verb::Verb(CT::NotSemantic auto&& other)
      : Verb {Langulus::Copy(other)} {}

   template<CT::Semantic S>
   LANGULUS(INLINED)
   Verb::Verb(S&& other)
      : Any {CT::Verb<TypeOf<S>>
         ? Any {other.template Forward<Any>()}
         : Any {other.Forward()}} {
      if constexpr (CT::Verb<TypeOf<S>>) {
         Charge::operator = (other.mValue);
         mVerb = other.mValue.mVerb;
         mState = other.mValue.mState;
         mSource = S::Nest(other.mValue.mSource);
         mOutput = S::Nest(other.mValue.mOutput);
      }
   }

   template<CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   Verb::Verb(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Any {Forward<HEAD>(head), Forward<TAIL>(tail)...} {}

   LANGULUS(INLINED)
   Verb& Verb::operator = (const Verb& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   LANGULUS(INLINED)
   Verb& Verb::operator = (Verb&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   template<CT::Semantic S>
   LANGULUS(INLINED)
   Verb& Verb::operator = (S&& rhs) {
      if constexpr (CT::Verb<TypeOf<S>>) {
         Any::operator = (rhs.template Forward<Any>());
         Charge::operator = (rhs.mValue);
         mVerb = rhs.mValue.mVerb;
         mState = rhs.mValue.mState;
         mSource = S::Nest(rhs.mValue.mSource);
         mOutput = S::Nest(rhs.mValue.mOutput);
      }
      else LANGULUS_ERROR("Bad verb assignment");
      return *this;
   }

   template<CT::Data VERB>
   LANGULUS(INLINED)
   Verb Verb::From(const Charge& charge, const VerbState& state) {
      Verb result;
      result.template SetVerb<VERB>();
      result.SetCharge(charge);
      result.SetVerbState(state);
      return result;
   }

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
      return (VerbIs(MetaVerb::Of<T>()) || ...);
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
         || Any::IsMissing()
         || mOutput.IsMissing();
   }

   /// Check if anything inside the verb is missing deeply                    
   ///   @return true if anything is missing                                  
   LANGULUS(INLINED)
   bool Verb::IsMissingDeep() const noexcept {
      return mSource.IsMissingDeep()
         || Any::IsMissingDeep()
         || mOutput.IsMissingDeep();
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
      mVerb = MetaVerb::Of<Decay<VERB>>();
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
   ///   @param frequency - the frequency to set                              
   ///   @return a reference to self                                          
   LANGULUS(INLINED)
   Verb& Verb::SetFrequency(const Real frequency) noexcept {
      mFrequency = frequency;
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
   ///   @param value  - the value to set                                     
   ///   @return a reference to self                                          
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetSource(const T& value) {
      mSource = value;
      return *this;
   }

   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetSource(T& value) {
      mSource = value;
      return *this;
   }

   /// Set the verb's source by move                                          
   ///   @param value  - the value to set                                     
   ///   @return a reference to self                                          
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetSource(T&& value) requires CT::Mutable<T> {
      mSource = Forward<T>(value);
      return *this;
   }

   /// Set the verb's argument by shallow-copy                                
   ///   @param value  - the value to set                                     
   ///   @return a reference to self                                          
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetArgument(const T& value) {
      static_cast<Any&>(*this).operator = (value);
      return *this;
   }

   /// Set the verb's argument by move                                        
   ///   @param value  - the value to set                                     
   ///   @return a reference to self                                          
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetArgument(T&& value) requires CT::Mutable<T> {
      static_cast<Any&>(*this).operator = (Forward<T>(value));
      return *this;
   }

   /// Set the verb's output by shallow-copy                                  
   ///   @param value  - the value to set                                     
   ///   @return a reference to self                                          
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetOutput(const T& value) {
      mOutput = value;
      return *this;
   }

   /// Set the verb's output by move                                          
   ///   @param value  - the value to set                                     
   ///   @return a reference to self                                          
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::SetOutput(T&& value) requires CT::Mutable<T> {
      mOutput = Forward<T>(value);
      return *this;
   }

   /// Compare verbs                                                          
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   LANGULUS(INLINED)
   bool Verb::operator == (const Verb& rhs) const {
      return (mVerb == rhs.mVerb || (mVerb && mVerb->Is(rhs.mVerb)))
         && mSource == rhs.mSource
         && Any::operator == (rhs.GetArgument())
         && mOutput == rhs.mOutput
         && mState == rhs.mState;
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
   Real Verb::GetFrequency() const noexcept {
      return mFrequency; 
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

   /// Push anything to output via shallow copy, satisfying the verb once     
   ///   @tparam T - the type of the data to push (deducible)                 
   ///   @param data - the data to push                                       
   ///   @return a reference to this verb for chaining                        
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::operator << (const T& data) {
      if (mOutput.SmartPush<IndexBack, true, true>(data))
         Done();
      return *this;
   }

   /// Output anything to the back by a move                                  
   ///   @tparam T - the type of the data to move (deducible)                 
   ///   @param data - the data to move to output                             
   ///   @return a reference to this verb for chaining                        
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::operator << (T&& data) {
      if (mOutput.SmartPush<IndexBack, true, true>(Forward<T>(data)))
         Done();
      return *this;
   }

   /// Output anything to the front by a shallow copy                         
   ///   @tparam T - the type of the data to push (deducible)                 
   ///   @param data - the data to push to output                             
   ///   @return a reference to this verb for chaining                        
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::operator >> (const T& data) {
      if (mOutput.SmartPush<IndexFront, true, true>(data))
         Done();
      return *this;
   }

   /// Output anything to the front by a move                                 
   ///   @tparam T - the type of the data to move (deducible)                 
   ///   @param data - the data to push                                       
   ///   @return a reference to this verb for chaining                        
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::operator >> (T&& data) {
      if (mOutput.SmartPush<IndexFront, true, true>(Forward<T>(data)))
         Done();
      return *this;
   }

   /// Merge anything to output's back                                        
   ///   @tparam T - the type of the data to merge                            
   ///   @param data - the data to merge                                      
   ///   @return a reference to this verb for chaining                        
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::operator <<= (const T&) {
      TODO();
      return *this;
   }

   /// Merge anything to output's front                                       
   ///   @tparam T - the type of the data to merge                            
   ///   @param data - the data to merge                                      
   ///   @return a reference to this verb for chaining                        
   template<CT::Data T>
   LANGULUS(INLINED)
   Verb& Verb::operator >>= (const T&) {
      TODO();
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
      const auto meta = MetaData::Of<Decay<T>>();
      return meta && meta->template GetAbility<CT::Mutable<T>>(mVerb, GetType());
   }

   /// Execute a known/unknown verb in an known/unknown context               
   ///   @attention assumes that if T is deep, it contains exactly one item   
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb was executed                                    
   template<CT::Dense T, CT::Data V>
   LANGULUS(INLINED)
   bool Verb::GenericExecuteIn(T& context, V& verb) {
      static_assert(CT::Verb<V>, "V must be a verb");

      if constexpr (!CT::Deep<T> && !CT::Same<V, Verb>) {
         // Always prefer statically optimized routine when available   
         // Literally zero ability searching overhead!                  
         if constexpr (V::template AvailableFor<T>())
            return V::ExecuteIn(context, verb);
         return false;
      }
      else {
         // Search for the ability via RTTI                             
         const auto meta = context.GetType();
         if constexpr (CT::DerivedFrom<V, Verbs::Interpret> && requires { typename V::Type; }) {
            // Scan for a reflected converter as statically as possible 
            using TO = typename V::Type;
            const auto found = meta->template GetConverter<TO>();
            if (!found)
               return false;

            auto result = Block::From<TO>();
            result.AllocateFresh(result.RequestSize(1));
            result.mCount = 1;
            found(context.GetRaw(), result.GetRaw());
            verb << Abandon(result);
            result.Free();
         }
         else {
            // Find ability at runtime                                  
            if (verb.template VerbIs<Verbs::Interpret>()) {
               // Scan for a reflected converter by scanning argument   
               const auto to = verb.template As<DMeta>();
               const auto found = meta->GetConverter(to);
               if (!found)
                  return false;

               Block result {to};
               result.AllocateFresh(result.RequestSize(1));
               result.mCount = 1;
               found(context.GetRaw(), result.GetRaw());
               verb << Abandon(result);
               result.Free();
            }
            else {
               // Scan for any other ability                            
               const auto found = meta->template 
                  GetAbility<CT::Mutable<T>>(verb.mVerb, verb.GetType());
               if (!found)
                  return false;

               found(context.GetRaw(), verb);
            }
         }
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
      static_assert(CT::Verb<V>, "V must be a verb");

      if constexpr (!CT::Same<V, Verb>) {
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
      static_assert(CT::Verb<V>, "V must be a verb");

      if constexpr (!CT::Same<V, Verb>) {
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
      static_assert(CT::Verb<V>, "V must be a verb");

      if constexpr (!CT::Same<V, Verb>) {
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
      : Verb {Langulus::Copy(other)} {}

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(StaticVerb&& other)
      : Verb {Langulus::Move(other)} {}

   template<class VERB>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(const T& other)
      : Verb {Langulus::Copy(other)} {
      SetVerb<VERB>();
   }

   template<class VERB>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(T& other)
      : Verb {Langulus::Copy(other)} {
      SetVerb<VERB>();
   }

   template<class VERB>
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(T&& other)
      : Verb {Langulus::Move(other)} {
      SetVerb<VERB>();
   }

   template<class VERB>
   template<CT::Semantic S>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(S&& other)
      : Verb {other.Forward()} {
      SetVerb<VERB>();
   }

   template<class VERB>
   template<CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   StaticVerb<VERB>::StaticVerb(HEAD&& head, TAIL&&... tail) requires (sizeof...(TAIL) >= 1)
      : Verb {Forward<HEAD>(head), Forward<TAIL>(tail)...} {
      SetVerb<VERB>();
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (const StaticVerb& rhs) {
      return operator = (Langulus::Copy(rhs));
   }

   template<class VERB>
   LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (StaticVerb&& rhs) {
      return operator = (Langulus::Move(rhs));
   }

   template<class VERB>
   template<CT::Semantic S>
   LANGULUS(INLINED)
   StaticVerb<VERB>& StaticVerb<VERB>::operator = (S&& rhs) {
      if constexpr (CT::Verb<TypeOf<S>>) {
         Any::operator = (rhs.template Forward<Any>());
         mSuccesses = rhs.mValue.mSuccesses;
         mState = rhs.mValue.mState;
         mSource = S::Nest(rhs.mValue.mSource);
         mOutput = S::Nest(rhs.mValue.mOutput);
      }
      else LANGULUS_ERROR("Bad verb assignment");
      return *this;
   }

} // namespace Langulus::Flow
