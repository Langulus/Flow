///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Construct.hpp"
#include "Serial.hpp"
#include "verbs/Interpret.inl"
#include "verbs/Do.inl"
#include "verbs/Create.inl"
#include "verbs/Select.inl"

#define VERBOSE_CONSTRUCT(a) //Logger::Verbose() << a

namespace Langulus::Flow
{

   /// Rehash the construct                                                   
   /// The hash is cached, so this is a cheap function                        
   ///   @return the hash of the content                                      
   Hash Construct::GetHash() const {
      if (mHash.mHash)
         return mHash;

      if (mType)
         mHash = HashData(mType->mHash, Any::GetHash());
      else
         mHash = Any::GetHash();

      return mHash;
   }

   /// Clears arguments and charge                                            
   void Construct::Clear() {
      Charge::Reset();
      Any::Reset();
      mHash = {};
   }

   /// Reset charge                                                           
   void Construct::ResetCharge() noexcept {
      Charge::Reset();
   }

   /// Compare constructs                                                     
   ///   @param rhs - descriptor to compare with                              
   ///   @return true if both constructs are the same                         
   bool Construct::operator == (const Construct& rhs) const {
      return GetHash() == rhs.GetHash()
         && (mType == rhs.mType || (mType && mType->IsExact(rhs.mType)))
         && Any::operator == (rhs.GetArgument());
   }

   /// Attempt to create construct statically if possible                     
   /// If not possible, simply propagate the construct                        
   ///   @param output - [out] results go here                                
   bool Construct::StaticCreation(Any& output) const {
      if (mType->mProducer)
         return false;

      // If reached, data doesn't rely on a producer                    
      // Make sure we're creating something concrete                    
      Verbs::Create creator {this};
      if (Verbs::Create::ExecuteStateless(creator)) {
         VERBOSE_CONSTRUCT("Constructed from initializer-list: "
            << Logger::Cyan << creator.GetOutput());
         output << Move(creator.GetOutput());
         return true;
      }

      return false;
   }

   /// Check if construct type can be interpreted as another type             
   ///   @param type - the type check if current type interprets to           
   ///   @return true if able to interpret current header to 'type'           
   bool Construct::CastsTo(DMeta type) const {
      return !type || (mType == type || mType->CastsTo(type));
   }

   /// Check if constructor header is exactly another type                    
   ///   @param type - the type to check for (must be a dense type)           
   ///   @return true if current header is 'type'                             
   bool Construct::Is(DMeta type) const {
      return !type || (mType && mType->Is(type));
   }

   /// Set a tagged argument inside constructor                               
   ///   @param trait - trait to set                                          
   ///   @param index - the index we're interested with if repeated           
   ///   @return a reference to this construct for chaining                   
   Construct& Construct::Set(const Trait& trait, const Offset& index) {
      bool done = false;
      Count counter = 0;
      ForEachDeep([&](Trait& t) {
         if (t.GetTrait() != trait.GetTrait())
            return true;

         if (counter == index) {
            t = trait;
            mHash = {};
            done = true;
            return false;
         }
            
         ++counter;
         return true;
      });

      if (!done)
         *this << Any {trait};
      return *this;
   }

   /// Get a tagged argument inside constructor                               
   ///   @param meta - trait to search for                                    
   ///   @param index - the index we're interested in, if repeated            
   ///   @return selected trait or nullptr if none was found                  
   const Trait* Construct::Get(TMeta meta, const Offset& index) const {
      const Trait* found = nullptr;
      Count counter = 0;
      ForEachDeep([&](const Trait& t) {
         if (t.GetTrait() != meta)
            return true;

         if (counter == index) {
            found = &t;
            return false;
         }

         ++counter;
         return true;
      });

      return found;
   }

   /// Serialize a construct to code                                          
   Construct::operator Code() const {
      Code result;
      result += GetToken();
      if (!Charge::IsDefault() || !IsEmpty()) {
         result += Verbs::Interpret::To<Code>(GetCharge());
         result += Code::OpenScope;
         result += Verbs::Interpret::To<Code>(GetArgument());
         result += Code::CloseScope;
      }

      return result;
   }

   /// Stringify a construct for logging                                      
   Construct::operator Debug() const {
      Code result;
      result += GetToken();
      if (!Charge::IsDefault() || !IsEmpty()) {
         result += Verbs::Interpret::To<Debug>(GetCharge());
         result += Code::OpenScope;
         result += Verbs::Interpret::To<Debug>(GetArgument());
         result += Code::CloseScope;
      }

      return Debug {result};
   }

} // namespace Langulus::Flow
