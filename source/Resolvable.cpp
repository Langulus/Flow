///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Resolvable.inl"


namespace Langulus::Flow
{

   /// Get the class name token                                               
   ///   @return the token                                                    
   Token Resolvable::GetToken() const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mClassType, "Bad resolvable type");
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         return mClassType->GetShortestUnambiguousToken();
      #else
         return mClassType->mToken;
      #endif
   }

   /// Check if context interprets as a type                                  
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   bool Resolvable::CastsTo(DMeta type) const IF_UNSAFE(noexcept) {
      LANGULUS_ASSUME(DevAssumes, mClassType,
         "Bad resolvable type");
      return mClassType->CastsTo(type);
   }

   /// Check if context is an exact type                                      
   ///   @param type - the type to check for                                  
   ///   @return true if this context can be dynamically interpreted to type  
   bool Resolvable::Is(DMeta type) const noexcept {
      return mClassType | type;
   }

   /// Stringify the context (shows class type and an identifier)             
   Resolvable::operator Text() const {
      return IdentityOf(GetToken(), this);
   }

   /// Convenience function that logs this's identity and suffixes with ": "  
   /// Useful when used like: Logger::Verbose() << Self() << "etc..."         
   Text Resolvable::Self() const {
      return Text {operator Text(), ": "};
   }

   /// Wrap this context instance in a static memory block                    
   /// The availability of this function is reflected via CT::Resolvable      
   /// You can invoke this function via Block::GetElementResolved()           
   ///   @return the static memory block representing this instance           
   Block<> Resolvable::GetBlock() const noexcept {
      return {
         DataState::Default, mClassType, 1,
         const_cast<void*>(mClassPointer)
      };
   }

   /// Parse and execute a scope in the resolved context                      
   ///   @param code - the code to parse and execute                          
   ///   @return the results of the execution                                 
   Many Resolvable::Run(const Code& code) {
      if (not code)
         return {};
      return Run(code.Parse());
   }

   /// Execute a scope in the resolved context                                
   ///   @param scope - the scope to execute                                  
   ///   @return the results of the execution                                 
   Many Resolvable::Run(const Many& scope) {
      Many context {GetBlock()};
      Many output;
      if (not Execute(scope, context, output, false)) {
         Logger::Error("Can't execute scope: ", scope);
         return {};
      }

      return output;
   }
   
   /// Execute a temporal in the resolved context                             
   ///   @param scope - the scope to execute                                  
   ///   @return the results of the execution                                 
   Many Resolvable::Run(const Temporal&) {
      TODO();
      return {};
   }

   /// Get the first member matching a runtime trait definition               
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   Block<> Resolvable::GetMember(TMeta trait) noexcept {
      auto member = mClassType->GetMember(trait);
      if (member)
         return GetBlock().GetMember(*member, 0);
      return {};
   }

   /// Get the first member matching a runtime trait definition (const)       
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   Block<> Resolvable::GetMember(TMeta trait) const noexcept {
      auto r = const_cast<Resolvable*>(this)->GetMember(trait);
      r.MakeConst();
      return r;
   }

#if LANGULUS_FEATURE(MANAGED_MEMORY)
   /// Get the first member matching a runtime trait token                    
   ///   @param trait - the trait to search for                               
   ///   @return the static mutable memory block representing the member      
   Block<> Resolvable::GetMember(const Token& trait) noexcept {
      return GetMember(RTTI::GetMetaTrait(trait));
   }

   /// Get the first member matching a runtime trait token (const)            
   ///   @param trait - the trait to search for                               
   ///   @return the static constant memory block representing the member     
   Block<> Resolvable::GetMember(const Token& trait) const noexcept {
      return GetMember(RTTI::GetMetaTrait(trait));
   }
#endif

} // namespace Langulus::Flow