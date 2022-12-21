///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Resolvable.hpp"
#include "verbs/Do.inl"

namespace Langulus
{

   /// Get a string representing an instance in memory                        
   /// Used all across framework to stringify short instance IDs              
   ///   @tparam T - type of the instance (deducible)                         
   ///   @param type - the resolved type, from which token is taken           
   ///   @param instance - the instance to stringify                          
   ///   @return text containing the generated identity                       
   template<class T>
   Anyness::Text IdentityOf(RTTI::DMeta type, const T& instance) {
      Flow::Code result;
      result += type->mToken;
      result += Flow::Code::OpenScope;
      #if !LANGULUS(PARANOID) && LANGULUS(DEBUG)
         // We're not paranoid, so directly dump the memory address     
         result += Anyness::Text {
            fmt::format("{:X}",
            reinterpret_cast<intptr_t>(SparseCast(instance)))
         };
      #else
         // Obfuscate the pointer, by hashing it                        
         result += Anyness::Text {
            fmt::format("{:X}",
            HashNumber(reinterpret_cast<intptr_t>(SparseCast(instance))).mHash)
         };
      #endif
      result += Flow::Code::CloseScope;
      return Abandon(static_cast<Anyness::Text&>(result));
   }

   /// Get a string representing an instance in memory                        
   /// Used all across framework to stringify short instance IDs              
   ///   @tparam T - type of the instance (deducible)                         
   ///   @param instance - the instance to stringify                          
   ///   @return text containing the generated identity                       
   template<class T>
   Anyness::Text IdentityOf(const T& instance) {
      return IdentityOf(MetaOf<Decay<T>>(), instance);
   }

} // namespace Langulus


namespace Langulus::Flow
{

   /// Constructor                                                            
   ///   @attention type is assumed derived from Resolvable                   
   ///   @param type - type of the resolvable                                 
   inline Resolvable::Resolvable(DMeta type) SAFETY_NOEXCEPT()
      : mClassType {type}
      , mClassOffset {0} {
      // Precalculate offset, no need to do it at runtime               
      RTTI::Base base;
      UNUSED() bool found = mClassType->template GetBase<Resolvable>(0, base);
      SAFETY(if (!found)
         LANGULUS_THROW(Construct, "Unrelated type provided to Resolvable"));
      const_cast<Offset&>(mClassOffset) = base.mOffset;
   }

   /// Get the class meta                                                     
   ///   @return the meta data for the class type                             
   constexpr DMeta Resolvable::GetType() const noexcept {
      return mClassType;
   }

   /// Get the class name token                                               
   ///   @return the token                                                    
   inline Token Resolvable::GetToken() const noexcept {
      return mClassType->mToken;
   }

   /// Check if context interprets as a type                                  
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   inline bool Resolvable::CastsTo(DMeta type) const noexcept {
      return mClassType->CastsTo(type);
   }

   /// Check if context interprets as a static type                           
   ///   @tparam T - the type to check for                                    
   ///   @return true if this context can be dynamically interpreted as T     
   template<CT::Data T>
   bool Resolvable::CastsTo() const {
      return mClassType->template CastsTo<T>();
   }

   /// Check if context is an exact type                                      
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   inline bool Resolvable::Is(DMeta type) const noexcept {
      return mClassType->Is(type);
   }

   /// Check if context is an exact static type                               
   ///   @tparam T - the type to check for                                    
   ///   @return true if this context can be dynamically interpreted as T     
   template<CT::Data T>
   inline bool Resolvable::Is() const {
      return mClassType->template Is<T>();
   }

   /// Stringify the context (shows class type and an identifier)             
   inline Resolvable::operator Debug() const {
      return IdentityOf(mClassType, this);
   }

   /// Convenience function that logs this's identity and suffixes with ": "  
   /// Useful when used like: Logger::Verbose() << Self() << "etc..."         
   inline Debug Resolvable::Self() const {
      auto temp = operator Debug();
      temp += ": ";
      return Abandon(temp);
   }

   /// Wrap this context instance in a static memory block                    
   /// The availability of this function is reflected via CT::Resolvable      
   /// You can invoke this function via Block::GetElementResolved()           
   ///   @return the static memory block representing this instance           
   inline Block Resolvable::GetBlock() const noexcept {
      // 'this' pointer points to Resolvable object, so we need to      
      // compensate this, by offsetting 'this' by the relative class    
      // type offset. I like to live dangerously <3                     
      // But seriously, this is well tested                             
      auto thisint = reinterpret_cast<Offset>(this);
      auto offsetd = reinterpret_cast<void*>(thisint - mClassOffset);
      return Block {DataState::Static, mClassType, 1, offsetd};
   }

   /// Invoke a verb on the resolved type                                     
   ///   @tparam DISPATCH - whether to allow custom dispatchers               
   ///   @tparam DEFAULT - whether to allow default/stateless verbs on fail   
   ///   @tparam V - type of verb to run (deducible)                          
   ///   @param verb - the verb to execute in this resolved type              
   ///   @return a reference to the verb's output                             
   template<bool DISPATCH, bool DEFAULT, CT::Verb V>
   const Any& Resolvable::Run(const V& verb) {
      return Run<DISPATCH, DEFAULT, V>(const_cast<V&>(verb));
   }

   /// Invoke a verb on the resolved type                                     
   ///   @tparam DISPATCH - whether to allow custom dispatchers               
   ///   @tparam DEFAULT - whether to allow default/stateless verbs on fail   
   ///   @tparam V - type of verb to run (deducible)                          
   ///   @param verb - the verb to execute in this resolved type              
   ///   @return a reference to the verb's output                             
   template<bool DISPATCH, bool DEFAULT, CT::Verb V>
   Any& Resolvable::Run(V& verb) {
      auto environment = GetBlock();
      DispatchFlat<false, DISPATCH, DEFAULT>(environment, verb);
      return verb.GetOutput();
   }

   /// Invoke a verb on the resolved type                                     
   ///   @tparam DISPATCH - whether to allow custom dispatchers               
   ///   @tparam DEFAULT - whether to allow default/stateless verbs on fail   
   ///   @tparam V - type of verb to run (deducible)                          
   ///   @param verb - the verb to execute in this resolved type              
   ///   @return a reference to the verb's output                             
   template<bool DISPATCH, bool DEFAULT, CT::Verb V>
   Any& Resolvable::Run(V&& verb) requires (CT::Mutable<V>) {
      return Run<DISPATCH, DEFAULT, V>(verb);
   }

   /// Get the first member matching a runtime trait definition               
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   inline Block Resolvable::GetMember(TMeta trait) noexcept {
      return GetBlock().GetMember(trait);
   }

   /// Get the first member matching a runtime trait definition (const)       
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   inline Block Resolvable::GetMember(TMeta trait) const noexcept {
      return GetBlock().GetMember(trait);
   }

   /// Get the first member matching a runtime trait token                    
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   inline Block Resolvable::GetMember(const Token& trait) noexcept {
      return GetBlock().GetMember(RTTI::Database.GetMetaTrait(trait));
   }

   /// Get the first member matching a runtime trait token (const)            
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   inline Block Resolvable::GetMember(const Token& trait) const noexcept {
      return GetBlock().GetMember(RTTI::Database.GetMetaTrait(trait));
   }

   /// Get Nth reflected member by trait definition                           
   ///   @tparam INDEX - type of indexing used                                
   ///   @param trait - the type of trait to search for (nullptr for any)     
   ///   @param offset - the index of the match to return                     
   ///   @return the static mutable memory block representing the member      
   template<CT::Index INDEX>
   Block Resolvable::GetMember(TMeta trait, const INDEX& offset) noexcept {
      return GetBlock().GetMember(trait, offset);
   }

   /// Get Nth reflected member by trait definition (const)                   
   ///   @tparam INDEX - type of indexing used                                
   ///   @param trait - the type of trait to search for (nullptr for any)     
   ///   @param offset - the index of the match to return                     
   ///   @return the static constant memory block representing the member     
   template<CT::Index INDEX>
   Block Resolvable::GetMember(TMeta trait, const INDEX& offset) const noexcept {
      return GetBlock().GetMember(trait, offset);
   }

   /// Get Nth reflected member by trait token                                
   ///   @tparam INDEX - type of indexing used                                
   ///   @param trait - the trait token                                       
   ///   @param offset - the index of the match to return                     
   ///   @return the static mutable memory block representing the member      
   template<CT::Index INDEX>
   Block Resolvable::GetMember(const Token& trait, const INDEX& offset) noexcept {
      return GetBlock().GetMember(RTTI::Database.GetMetaTrait(trait), offset);
   }

   /// Get Nth reflected member by trait token (const)                        
   ///   @tparam INDEX - type of indexing used                                
   ///   @param trait - the trait token                                       
   ///   @param offset - the index of the match to return                     
   ///   @return the static constant memory block representing the member     
   template<CT::Index INDEX>
   Block Resolvable::GetMember(const Token& trait, const INDEX& offset) const noexcept {
      return GetBlock().GetMember(RTTI::Database.GetMetaTrait(trait), offset);
   }

   /// Get a statically typed trait member and cast it to the desired type    
   ///   @tparam T - the trait to search for                                  
   ///   @tparam D - the data to interpret it as (deducible)                  
   ///   @param data - [out] the data to set                                  
   ///   @return true if trait was found, and data was set                    
   template<CT::Trait T, CT::Data D>
   bool Resolvable::GetTrait(D& data) const {
      auto found = GetBlock().GetMember(RTTI::MetaTrait::Of<T>());
      try {
         data = found.template AsCast<D>();
         return true;
      }
      catch (...) {}
      return false;
   }

   /// Get a member of a specific type                                        
   ///   @tparam D - the data to interpret it as (deducible)                  
   ///   @param data - [out] the data to set                                  
   ///   @return true if trait was found, and data was set                    
   template<CT::Data D>
   bool Resolvable::GetValue(D& data) const {
      auto found = GetBlock().GetMember(RTTI::MetaData::Of<D>());
      data = found.As<D>();
      return true;
   }

   /// Set a statically typed trait by shallow copy                           
   ///   @tparam T - the trait to search for                                  
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @tparam D - the data to set (deducible)                              
   ///   @param data - [out] the data to copy                                 
   ///   @return true if trait was found and overwritten                      
   template<CT::Trait T, bool DIRECT, CT::Data D>
   bool Resolvable::SetTrait(const D& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember(RTTI::MetaTrait::Of<T>());
         if (found.IsEmpty())
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {T{data}};
         Run(verb);
         return verb.IsDone();
      }
   }

   /// Set a statically typed data by shallow copy                            
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @tparam D - the data to set (deducible)                              
   ///   @param data - [out] the data to copy                                 
   ///   @return true if data was found and overwritten                       
   template<bool DIRECT, CT::Data D>
   bool Resolvable::SetValue(const D& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember(RTTI::MetaData::Of<D>());
         if (found.IsEmpty())
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {data};
         Run(verb);
         return verb.IsDone();
      }
   }

   /// Set a statically typed trait by move                                   
   ///   @tparam T - the trait to search for                                  
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @tparam D - the data to set (deducible)                              
   ///   @param data - [out] the data to move                                 
   ///   @return true if trait was found and overwritten                      
   template<CT::Trait T, bool DIRECT, CT::Data D>
   bool Resolvable::SetTrait(D&& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember(RTTI::MetaTrait::Of<T>());
         if (found.IsEmpty())
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {T{Forward<D>(data)}};
         Run(verb);
         return verb.IsDone();
      }
   }

   /// Set a statically typed data by move                                    
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @tparam D - the data to set (deducible)                              
   ///   @param data - [out] the data to move                                 
   ///   @return true if data was found and overwritten                       
   template<bool DIRECT, CT::Data D>
   bool Resolvable::SetValue(D&&) {

   }

} // namespace Langulus::Flow