///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Resolvable.hpp"
#include "verbs/Do.inl"
#include "verbs/Associate.inl"
#include "verbs/Interpret.inl"


namespace Langulus
{

   /// Get a string representing an instance in memory                        
   /// Used all across framework to stringify short instance IDs              
   ///   @param type - the resolved type, from which token is taken           
   ///   @param instance - the instance to stringify                          
   ///   @return text containing the generated identity                       
   LANGULUS(INLINED)
   Anyness::Text IdentityOf(const Token& token, const auto& instance) {
      using Flow::Code;
      Code result;
      result += token;
      result += Code::OpenScope;
      #if not LANGULUS(PARANOID) and LANGULUS(DEBUG)
         // Feel like getting doxxed - directly dump the memory address 
         result += Anyness::Text {
            fmt::format("{:X}",
               reinterpret_cast<intptr_t>(&DenseCast(instance))
            )
         };
      #else
         // Obfuscate the pointer, by hashing it                        
         result += Anyness::Text {
            fmt::format("{:X}", HashOf(&DenseCast(instance)).mHash)
         };
      #endif
      result += Code::CloseScope;
      return Abandon(result);
   }

   /// Get a string representing an instance in memory                        
   /// Used all across framework to stringify short instance IDs              
   ///   @param instance - the instance to stringify                          
   ///   @return text containing the generated identity                       
   LANGULUS(INLINED)
   Anyness::Text IdentityOf(const auto& instance) {
      return IdentityOf(NameOf<Decay<decltype(instance)>>(), instance);
   }

} // namespace Langulus


namespace Langulus::Flow
{

   /// Constructor                                                            
   ///   @attention type is assumed valid and complete                        
   ///   @attention type is assumed derived from Resolvable                   
   ///   @param type - type of the resolvable                                 
   LANGULUS(INLINED)
   Resolvable::Resolvable(DMeta type) IF_UNSAFE(noexcept)
      : mClassType {type}
      , mClassOffset {0} {
      LANGULUS_ASSUME(DevAssumes, type,
         "Bad resolvable type");
      LANGULUS_ASSUME(DevAssumes, type->mOrigin,
         "Resolvable type is incomplete");

      // Precalculate offset, no need to do it at runtime               
      RTTI::Base base;
      UNUSED() bool found = type->template GetBase<Resolvable>(0, base);
      LANGULUS_ASSUME(DevAssumes, found, "Unrelated type provided to Resolvable");
      const_cast<Offset&>(mClassOffset) = base.mOffset;
   }

   /// Get the class meta                                                     
   ///   @return the meta data for the class type                             
   LANGULUS(INLINED)
   constexpr DMeta Resolvable::GetType() const noexcept {
      return mClassType;
   }

   /// Get the class name token                                               
   ///   @return the token                                                    
   LANGULUS(INLINED)
   Token Resolvable::GetToken() const noexcept {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         return mClassType->GetShortestUnambiguousToken();
      #else
         return mClassType->mToken;
      #endif
   }

   /// Check if context interprets as a type                                  
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   LANGULUS(INLINED)
   bool Resolvable::CastsTo(DMeta type) const noexcept {
      return mClassType->CastsTo(type);
   }

   /// Check if context interprets as a static type                           
   ///   @tparam T - the type to check for                                    
   ///   @return true if this context can be dynamically interpreted as T     
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Resolvable::CastsTo() const {
      return mClassType->template CastsTo<T>();
   }

   /// Check if context is an exact type                                      
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   LANGULUS(INLINED)
   bool Resolvable::Is(DMeta type) const noexcept {
      return mClassType->Is(type);
   }

   /// Check if context is an exact static type                               
   ///   @tparam T - the type to check for                                    
   ///   @return true if this context can be dynamically interpreted as T     
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Resolvable::Is() const {
      return mClassType->template Is<T>();
   }

   /// Stringify the context (shows class type and an identifier)             
   LANGULUS(INLINED)
   Resolvable::operator Debug() const {
      return IdentityOf(mClassType->mToken, this);
   }

   /// Convenience function that logs this's identity and suffixes with ": "  
   /// Useful when used like: Logger::Verbose() << Self() << "etc..."         
   LANGULUS(INLINED)
   Debug Resolvable::Self() const {
      auto temp = operator Debug();
      temp += ": ";
      return Abandon(temp);
   }

   /// Wrap this context instance in a static memory block                    
   /// The availability of this function is reflected via CT::Resolvable      
   /// You can invoke this function via Block::GetElementResolved()           
   ///   @return the static memory block representing this instance           
   LANGULUS(INLINED)
   Block Resolvable::GetBlock() const noexcept {
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
   ///   @param verb - the verb to execute in this resolved type              
   ///   @return a reference to the verb's output                             
   template<bool DISPATCH, bool DEFAULT>
   LANGULUS(INLINED)
   Any Resolvable::Run(CT::VerbBased auto& verb) {
      auto environment = GetBlock();
      DispatchFlat<false, DISPATCH, DEFAULT>(environment, verb);
      return verb.GetOutput();
   }

   /// Invoke a verb on the resolved type                                     
   ///   @tparam DISPATCH - whether to allow custom dispatchers               
   ///   @tparam DEFAULT - whether to allow default/stateless verbs on fail   
   ///   @param verb - the verb to execute in this resolved type              
   ///   @return a reference to the verb's output                             
   template<bool DISPATCH, bool DEFAULT>
   LANGULUS(INLINED)
   Any Resolvable::Run(CT::VerbBased auto&& verb) {
      auto environment = GetBlock();
      auto moved = Move(verb);
      DispatchFlat<false, DISPATCH, DEFAULT>(environment, moved);
      return Abandon(moved.GetOutput());
   }
   
   /// Parse and execute a scope in the resolved context                      
   ///   @param code - the code to parse and execute                          
   ///   @return the results of the execution                                 
   LANGULUS(INLINED)
   Any Resolvable::Run(const Code& code) {
      if (not code)
         return {};
      return Run(code.Parse());
   }

   /// Execute a scope in the resolved context                                
   ///   @param scope - the scope to execute                                  
   ///   @return the results of the execution                                 
   LANGULUS(INLINED)
   Any Resolvable::Run(const Any& scope) {
      Any context {GetBlock()};
      Any output;
      if (not Execute(scope, context, output)) {
         Logger::Error("Can't execute scope: ", scope);
         return {};
      }

      return output;
   }
   
   /// Execute a temporal in the resolved context                             
   ///   @param scope - the scope to execute                                  
   ///   @return the results of the execution                                 
   LANGULUS(INLINED)
   Any Resolvable::Run(const Temporal&) {
      TODO();
      return {};
   }

   /// Get the first member matching a runtime trait definition               
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   LANGULUS(INLINED)
   Block Resolvable::GetMember(TMeta trait) noexcept {
      return GetBlock().GetMember(trait);
   }

   /// Get the first member matching a runtime trait definition (const)       
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   LANGULUS(INLINED)
   Block Resolvable::GetMember(TMeta trait) const noexcept {
      return GetBlock().GetMember(trait);
   }

   /// Get Nth reflected member by trait definition                           
   ///   @param trait - the type of trait to search for (nullptr for any)     
   ///   @param offset - the index of the match to return                     
   ///   @return the static mutable memory block representing the member      
   LANGULUS(INLINED)
   Block Resolvable::GetMember(TMeta trait, const CT::Index auto& offset) noexcept {
      return GetBlock().GetMember(trait, offset);
   }

   /// Get Nth reflected member by trait definition (const)                   
   ///   @param trait - the type of trait to search for (nullptr for any)     
   ///   @param offset - the index of the match to return                     
   ///   @return the static constant memory block representing the member     
   LANGULUS(INLINED)
   Block Resolvable::GetMember(TMeta trait, const CT::Index auto& offset) const noexcept {
      return GetBlock().GetMember(trait, offset);
   }

#if LANGULUS_FEATURE(MANAGED_MEMORY)
   /// Get the first member matching a runtime trait token                    
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   LANGULUS(INLINED)
   Block Resolvable::GetMember(const Token& trait) noexcept {
      return GetBlock().GetMember(RTTI::GetMetaTrait(trait));
   }

   /// Get the first member matching a runtime trait token (const)            
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   LANGULUS(INLINED)
   Block Resolvable::GetMember(const Token& trait) const noexcept {
      return GetBlock().GetMember(RTTI::GetMetaTrait(trait));
   }

   /// Get Nth reflected member by trait token                                
   ///   @param trait - the trait token                                       
   ///   @param offset - the index of the match to return                     
   ///   @return the static mutable memory block representing the member      
   LANGULUS(INLINED)
   Block Resolvable::GetMember(const Token& trait, const CT::Index auto& offset) noexcept {
      return GetBlock().GetMember(RTTI::GetMetaTrait(trait), offset);
   }

   /// Get Nth reflected member by trait token (const)                        
   ///   @param trait - the trait token                                       
   ///   @param offset - the index of the match to return                     
   ///   @return the static constant memory block representing the member     
   LANGULUS(INLINED)
   Block Resolvable::GetMember(const Token& trait, const CT::Index auto& offset) const noexcept {
      return GetBlock().GetMember(RTTI::GetMetaTrait(trait), offset);
   }
#endif

   /// Get a statically typed trait member and cast it to the desired type    
   ///   @tparam T - the trait to search for                                  
   ///   @param data - [out] the data to set                                  
   ///   @return true if trait was found, and data was set                    
   template<CT::Trait T>
   LANGULUS(INLINED)
   bool Resolvable::GetTrait(CT::Data auto& data) const {
      auto found = GetBlock().GetMember(MetaOf<T>());
      try {
         data = found.template AsCast<Deref<decltype(data)>>();
         return true;
      }
      catch (...) {}
      return false;
   }

   /// Get a member of a specific type                                        
   ///   @param data - [out] the data to set                                  
   ///   @return true if trait was found, and data was set                    
   LANGULUS(INLINED)
   bool Resolvable::GetValue(CT::Data auto& data) const {
      using D = Deref<decltype(data)>;
      auto found = GetBlock().GetMember<D>();
      data = found.template As<D>();
      return true;
   }

   /// Set a statically typed trait by shallow copy                           
   ///   @tparam T - the trait to search for                                  
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @param data - [out] the data to copy                                 
   ///   @return true if trait was found and overwritten                      
   template<CT::Trait T, bool DIRECT>
   LANGULUS(INLINED)
   bool Resolvable::SetTrait(const CT::Data auto& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember<T>();
         if (not found)
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {T {data}};
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
   ///   @param data - [out] the data to move                                 
   ///   @return true if trait was found and overwritten                      
   template<CT::Trait T, bool DIRECT>
   LANGULUS(INLINED)
   bool Resolvable::SetTrait(CT::Data auto&& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember<T>();
         if (not found)
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {T {Move(data)}};
         Run(verb);
         return verb.IsDone();
      }
   }

   /// Set a statically typed data by shallow copy                            
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @param data - [out] the data to copy                                 
   ///   @return true if data was found and overwritten                       
   template<bool DIRECT>
   LANGULUS(INLINED)
   bool Resolvable::SetValue(const CT::Data auto& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember<Deref<decltype(data)>>();
         if (not found)
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {data};
         Run(verb);
         return verb.IsDone();
      }
   }

   /// Set a statically typed data by move                                    
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @param data - [out] the data to move                                 
   ///   @return true if data was found and overwritten                       
   template<bool DIRECT>
   LANGULUS(INLINED)
   bool Resolvable::SetValue(CT::Data auto&& data) {
      if constexpr (DIRECT) {
         auto found = GetBlock().GetMember<Deref<decltype(data)>>();
         if (not found)
            return false;
         return found.Copy(Block::From(data)) > 0;
      }
      else {
         Verbs::Associate verb {data};
         Run(verb);
         return verb.IsDone();
      }
   }

} // namespace Langulus::Flow