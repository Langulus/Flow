///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Verb.hpp"
#include "Serial.inl"

#define DELEGATION_VERBOSE(a) pcLogSelfVerbose << a

LANGULUS_RTTI_BOUNDARY("MAIN")

namespace Langulus::Flow
{
     
   /// Verb destructor                                                        
   Verb::~Verb() { }

   /// Hash the verb                                                          
   ///   @return the hash of the content                                      
   Hash Verb::GetHash() const {
      return HashOf(mVerb->mHash, mSource, GetArgument(), mOutput);
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
      shallowCopy.mRate *= rhs;
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
      mRate *= rhs;
      return *this;
   }

   /// Partial copy, copies only charge, verb, and short-circuitness          
   ///   @param other - the verb to use as base                               
   ///   @return the partially copied verb                                    
   Verb Verb::PartialCopy() const noexcept {
      return Verb::FromMeta(mVerb, GetCharge(), mState);
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
      return not mVerb ? not id : mVerb->Is(id);
   }

   /// Check if a verb is valid for the given priority                        
   ///   @param priority - the priority to check                              
   ///   @return true if this verb's priority matches the provided one        
   bool Verb::Validate(const Index& priority) const noexcept {
      return int(mPriority) == priority.mIndex or priority == IndexAll;
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
      if (not mVerb)
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

