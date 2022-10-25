///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Verb.hpp"
#include "verbs/Interpret.inl"

#define DELEGATION_VERBOSE(a) pcLogSelfVerbose << a

namespace Langulus::Flow
{

   /// Serialize charge as code                                               
   Charge::operator Code() const {
      Code code;
      if (mMass != Charge::DefaultMass) {
         code += Code::Mass;
         code += Text {mMass};
      }
      if (mFrequency != Charge::DefaultFrequency) {
         code += Code::Frequency;
         code += Text {mFrequency};
      }
      if (mTime != Charge::DefaultTime) {
         code += Code::Time;
         code += Text {mTime};
      }
      if (mPriority != Charge::DefaultPriority) {
         code += Code::Priority;
         code += Text {mPriority};
      }
      return code;
   }

   /// Serialize charge as debug (same as code)                               
   Charge::operator Debug() const {
      return Debug {Charge::operator Code()};
   }

   /// Scale the mass of a charge                                             
   constexpr Charge Charge::operator * (const Real& scalar) const noexcept {
      return {mMass * scalar, mFrequency, mTime, mPriority};
   }

   /// Scale the frequency of a charge                                        
   constexpr Charge Charge::operator ^ (const Real& scalar) const noexcept {
      return {mMass, mFrequency * scalar, mTime, mPriority};
   }

   /// Scale the mass of a charge                                             
   constexpr Charge& Charge::operator *= (const Real& scalar) noexcept {
      mMass *= scalar;
      return *this;
   }

   /// Scale the frequency of a charge                                        
   constexpr Charge& Charge::operator ^= (const Real& scalar) noexcept {
      mFrequency *= scalar;
      return *this;
   }

   /// Disown-construct a verb                                                
   ///   @param other - the verb to disown and copy                           
   Verb::Verb(Disowned<Verb>&& other) noexcept
      : Any {other.Forward<Any>()}
      , Charge {other.mValue}
      , mVerb {other.mValue.mVerb}
      , mState {other.mValue.mState}
      , mSource {Disown(other.mValue.mSource)}
      , mOutput {Disown(other.mValue.mOutput)} { }

   /// Abandon-construct a verb                                               
   ///   @param other - the verb to abandon and move                          
   Verb::Verb(Abandoned<Verb>&& other) noexcept
      : Any {other.Forward<Any>()}
      , Charge {other.mValue}
      , mVerb {other.mValue.mVerb}
      , mState {other.mValue.mState}
      , mSource {Abandon(other.mValue.mSource)}
      , mOutput {Abandon(other.mValue.mOutput)} { }

   /// Manual constructor with verb meta                                      
   ///   @param verb - the verb type                                          
   Verb::Verb(VMeta verb)
      : Any {}
      , mVerb {verb} { }

   /// Disown-assign a verb                                                   
   ///   @param other - the verb to disown and copy                           
   ///   @return a reference to this verb                                     
   Verb& Verb::operator = (Disowned<Verb>&& other) {
      Any::operator = (other.Forward<Any>());
      Charge::operator = (other.mValue);
      mVerb = other.mValue.mVerb;
      mState = other.mValue.mState;
      mSource = Disown(other.mValue.mSource);
      mOutput = Disown(other.mValue.mOutput);
      return *this;
   }

   /// Abandon-assign a verb                                                  
   ///   @param other - the verb to abandon and move                          
   ///   @return a reference to this verb                                     
   Verb& Verb::operator = (Abandoned<Verb>&& other) {
      Any::operator = (other.Forward<Any>());
      Charge::operator = (other.mValue);
      mVerb = other.mValue.mVerb;
      mState = other.mValue.mState;
      mSource = Abandon(other.mValue.mSource);
      mOutput = Abandon(other.mValue.mOutput);
      return *this;
   }

   /// Hash the verb                                                          
   ///   @return the hash of the content                                      
   Hash Verb::GetHash() const {
      return HashData(mVerb->mHash, mSource, GetArgument(), mOutput);
   }

   /// Multiply verb mass                                                     
   ///   @param rhs - the mass to multiply by                                 
   ///   @return a new verb, with the modified mass                           
   Verb Verb::operator * (const Real& rhs) const {
      Verb shallowCopy = *this;
      shallowCopy.mMass *= rhs;
      return shallowCopy;
   }

   /// Multiply verb frequency                                                
   ///   @param rhs - the frequency to multiply by                            
   ///   @return a new verb, with the modified frequency                      
   Verb Verb::operator ^ (const Real& rhs) const {
      Verb shallowCopy = *this;
      shallowCopy.mFrequency *= rhs;
      return shallowCopy;
   }

   /// Multiply verb mass (destructively)                                     
   ///   @param rhs - the mass to multiply by                                 
   ///   @return a reference to this verb                                     
   Verb& Verb::operator *= (const Real& rhs) noexcept {
      mMass *= rhs;
      return *this;
   }

   /// Multiply verb frequency (destructively)                                
   ///   @param rhs - the frequency to multiply by                            
   ///   @return a reference to this verb                                     
   Verb& Verb::operator ^= (const Real& rhs) noexcept {
      mFrequency *= rhs;
      return *this;
   }

   /// Partial copy, copies only charge, verb, and short-circuitness          
   ///   @param other - the verb to use as base                               
   ///   @return the partially copied verb                                    
   Verb Verb::PartialCopy() const noexcept {
      return {mVerb, Any{}, GetCharge(), mState};
   }

   /// Clone the verb                                                         
   ///   @return the cloned verb                                              
   Verb Verb::Clone() const {
      Verb clone {mVerb, Any::Clone(), GetCharge(), mState};
      clone.mSource = mSource.Clone();
      clone.mOutput = mOutput.Clone();
      clone.mSuccesses = mSuccesses;
      return clone;
   }

   /// Reset all verb members and energy                                      
   void Verb::Reset() {
      mVerb = {};
      Any::Reset();
      Charge::Reset();
      mSource.Reset();
      mOutput.Reset();
      mSuccesses = {};
   };

   /// Check if verb id matches                                               
   ///   @param id - the verb id to check                                     
   ///   @return true if verb id matches                                      
   bool Verb::VerbIs(VMeta id) const noexcept {
      return !mVerb ? !id : mVerb->Is(id);
   }

   /// Check if a verb is valid for the given priority                        
   ///   @param priority - the priority to check                              
   ///   @return true if this verb's priority matches the provided one        
   bool Verb::Validate(const Index& priority) const noexcept {
      return int(mPriority) == priority.mIndex || priority == IndexAll;
   }
   
   /// Change the verb's circuitry                                            
   ///   @param toggle - enable or disable short-circuit                      
   ///   @return a reference to this verb for chaining                        
   Verb& Verb::ShortCircuit(bool toggle) noexcept {
      if (toggle)
         mState -= VerbState::LongCircuited;
      else
         mState += VerbState::LongCircuited;
      return *this;
   }

   /// Change the verb's castness                                             
   ///   @param toggle - enable or disable multicast                          
   ///   @return a reference to this verb for chaining                        
   Verb& Verb::Multicast(bool toggle) noexcept {
      if (toggle)
         mState -= VerbState::Monocast;
      else
         mState += VerbState::Monocast;
      return *this;
   }

   /// Get the verb token                                                     
   ///   @return the token as a literal                                       
   Token Verb::GetToken() const {
      if (!mVerb)
         return MetaVerb::DefaultToken;

      return mMass < 0 ? mVerb->mTokenReverse : mVerb->mToken;
   }

   /// Serialize verb to code                                                 
   Verb::operator Code() const {
      return SerializeVerb<Code>();
   }

   /// Serialize verb for logger                                              
   Verb::operator Debug() const {
      return SerializeVerb<Debug>();
   }

} // namespace Langulus::Flow

