///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Resolvable.inl"


namespace Langulus::Flow
{

   /// Constructor                                                            
   ///   @attention type is assumed valid and complete                        
   ///   @attention type is assumed derived from Resolvable                   
   ///   @param type - type of the resolvable                                 
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

   /// Get the class name token                                               
   ///   @return the token                                                    
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
   bool Resolvable::CastsTo(DMeta type) const noexcept {
      return mClassType->CastsTo(type);
   }

   /// Check if context is an exact type                                      
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   bool Resolvable::Is(DMeta type) const noexcept {
      return mClassType->Is(type);
   }

   /// Stringify the context (shows class type and an identifier)             
   Resolvable::operator Debug() const {
      return IdentityOf(mClassType->mToken, this);
   }

   /// Convenience function that logs this's identity and suffixes with ": "  
   /// Useful when used like: Logger::Verbose() << Self() << "etc..."         
   Debug Resolvable::Self() const {
      auto temp = operator Debug();
      temp += ": ";
      return Abandon(temp);
   }

   /// Wrap this context instance in a static memory block                    
   /// The availability of this function is reflected via CT::Resolvable      
   /// You can invoke this function via Block::GetElementResolved()           
   ///   @return the static memory block representing this instance           
   Block Resolvable::GetBlock() const noexcept {
      // 'this' pointer points to Resolvable object, so we need to      
      // compensate this, by offsetting 'this' by the relative class    
      // type offset. I like to live dangerously <3                     
      // But seriously, this is well tested                             
      auto thisint = reinterpret_cast<Offset>(this);
      auto offsetd = reinterpret_cast<void*>(thisint - mClassOffset);
      return Block {DataState::Static, mClassType, 1, offsetd};
   }

   /// Parse and execute a scope in the resolved context                      
   ///   @param code - the code to parse and execute                          
   ///   @return the results of the execution                                 
   Any Resolvable::Run(const Code& code) {
      if (not code)
         return {};
      return Run(code.Parse());
   }

   /// Execute a scope in the resolved context                                
   ///   @param scope - the scope to execute                                  
   ///   @return the results of the execution                                 
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
   Any Resolvable::Run(const Temporal&) {
      TODO();
      return {};
   }

   /// Get the first member matching a runtime trait definition               
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   Block Resolvable::GetMember(TMeta trait) noexcept {
      auto member = mClassType->GetMember(trait);
      if (member)
         return GetBlock().GetMember(*member, 0);
      return {};
   }

   /// Get the first member matching a runtime trait definition (const)       
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   Block Resolvable::GetMember(TMeta trait) const noexcept {
      auto r = const_cast<Resolvable*>(this)->GetMember(trait);
      r.MakeConst();
      return r;
   }

#if LANGULUS_FEATURE(MANAGED_MEMORY)
   /// Get the first member matching a runtime trait token                    
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   Block Resolvable::GetMember(const Token& trait) noexcept {
      return GetMember(RTTI::GetMetaTrait(trait));
   }

   /// Get the first member matching a runtime trait token (const)            
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   Block Resolvable::GetMember(const Token& trait) const noexcept {
      return GetMember(RTTI::GetMetaTrait(trait));
   }
#endif

} // namespace Langulus::Flow