///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
      using Anyness::Text;
      Text result;
      result += token;
      result += Text::Operator::OpenScope;
      #if not LANGULUS(PARANOID) and LANGULUS(DEBUG)
         // Feel like getting doxxed? Directly dump the memory address  
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
      result += Text::Operator::CloseScope;
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
   template<class T>
   Resolvable::Resolvable(const T* type) IF_UNSAFE(noexcept)
      : mClassType {MetaDataOf<T>()} {
      LANGULUS_ASSUME(DevAssumes, mClassType,
         "Bad resolvable type");
      LANGULUS_ASSUME(DevAssumes, mClassType->mOrigin,
         "Resolvable type is incomplete");

      // Precalculate offset, no need to do it at runtime               
      /*RTTI::Base base;
      UNUSED() bool found = type->template GetBase<Resolvable>(0, base);
      LANGULUS_ASSUME(DevAssumes, found, "Unrelated type provided to Resolvable");
      const_cast<Offset&>(mClassOffset) = base.mOffset;*/
      mClassPointer = type;
   }

   /// Get the class meta                                                     
   ///   @return the meta data for the class type                             
   LANGULUS(INLINED)
   DMeta Resolvable::GetType() const noexcept {
      return mClassType;
   }

   /// Check if context interprets as a static type                           
   ///   @tparam T - the type to check for                                    
   ///   @return true if this context can be dynamically interpreted as T     
   template<CT::Data T> LANGULUS(INLINED)
   bool Resolvable::CastsTo() const {
      return mClassType->template CastsTo<T>();
   }

   /// Check if context is an exact static type                               
   ///   @tparam T - the type to check for                                    
   ///   @return true if this context can be dynamically interpreted as T     
   template<CT::Data T> LANGULUS(INLINED)
   bool Resolvable::Is() const {
      return mClassType->template Is<T>();
   }

   /// Invoke a verb on the resolved type                                     
   ///   @tparam DISPATCH - whether to allow custom dispatchers               
   ///   @tparam DEFAULT - whether to allow default/stateless verbs on fail   
   ///   @param verb - the verb to execute in this resolved type              
   ///   @attention unlike other Run methods, this one outputs into the verb  
   template<bool DISPATCH, bool DEFAULT> LANGULUS(INLINED)
   auto& Resolvable::Run(CT::VerbBased auto&& verb) {
      static_assert(CT::Mutable<Deref<decltype(verb)>>);
      auto environment = GetBlock();
      DispatchFlat<false, DISPATCH, DEFAULT>(environment, verb);
      return verb;
   }

   /// Get Nth reflected member by trait definition                           
   ///   @param trait - the type of trait to search for (nullptr for any)     
   ///   @param offset - the index of the match to return                     
   ///   @return the static mutable memory block representing the member      
   LANGULUS(INLINED)
   Block<> Resolvable::GetMember(TMeta trait, CT::Index auto offset) noexcept {
      auto member = mClassType->GetMember(trait, {}, offset);
      if (member)
         return GetBlock().GetMember(*member, 0);
      return {};
   }

   LANGULUS(INLINED)
   Block<> Resolvable::GetMember(TMeta trait, CT::Index auto offset) const noexcept {
      auto member = mClassType->GetMember(trait, {}, offset);
      if (member)
         return GetBlock().GetMember(*member, 0);
      return {};
   }

#if LANGULUS_FEATURE(MANAGED_MEMORY)
   /// Get Nth reflected member by trait token                                
   ///   @param trait - the trait token                                       
   ///   @param offset - the index of the match to return                     
   ///   @return the static mutable memory block representing the member      
   LANGULUS(INLINED)
   Block<> Resolvable::GetMember(const Token& trait, CT::Index auto offset) noexcept {
      return GetMember(RTTI::GetMetaTrait(trait), offset);
   }

   LANGULUS(INLINED)
   Block<> Resolvable::GetMember(const Token& trait, CT::Index auto offset) const noexcept {
      return GetMember(RTTI::GetMetaTrait(trait), offset);
   }
#endif

   /// Get a statically typed trait member and cast it to the desired type    
   ///   @tparam T - the trait to search for                                  
   ///   @param data - [out] the data to set                                  
   ///   @return true if trait was found, and data was set                    
   template<CT::Trait T> LANGULUS(INLINED)
   bool Resolvable::GetTrait(CT::Data auto& data) const {
      using D = Deref<decltype(data)>;
      auto member = mClassType->GetMember(MetaOf<T>());
      if (member) {
         try {
            data = GetBlock().GetMember(*member, 0).template AsCast<D>();
            return true;
         }
         catch (...) {}
      }
      return false;
   }

   /// Get a member of a specific type                                        
   ///   @param data - [out] the data to set                                  
   ///   @return true if trait was found, and data was set                    
   LANGULUS(INLINED)
   bool Resolvable::GetValue(CT::Data auto& data) const {
      using D = Deref<decltype(data)>;
      auto member = mClassType->GetMember({}, MetaDataOf<D>());
      if (member) {
         data = GetBlock().GetMember(*member, 0).template As<D>();
         return true;
      }
      return false;
   }

   /// Set a statically typed trait by move                                   
   ///   @tparam T - the trait to search for                                  
   ///   @tparam DIRECT - if true, will directly set the trait, without       
   ///                    using a dynamically dispatched Verbs::Associate;    
   ///                    this will not notify the context of the change, but 
   ///                    is considerably faster (false by default)           
   ///   @param data - [out] the data to move                                 
   ///   @return true if trait was found and overwritten                      
   template<CT::Trait T, bool DIRECT> LANGULUS(INLINED)
   bool Resolvable::SetTrait(CT::Data auto&& data) {
      using D = Deref<decltype(data)>;
      if constexpr (DIRECT) {
         auto member = mClassType->GetMember(MetaOf<T>());
         if (member)
            return GetBlock().GetMember(*member, 0).Copy(MakeBlock(data)) > 0;
         else return false;
      }
      else {
         Verbs::Associate verb {T {Forward<D>(data)}};
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
   template<bool DIRECT> LANGULUS(INLINED)
   bool Resolvable::SetValue(CT::Data auto&& data) {
      using D = Deref<decltype(data)>;
      if constexpr (DIRECT) {
         auto member = mClassType->GetMember({}, MetaDataOf<D>());
         if (member)
            return GetBlock().GetMember(*member, 0).Copy(MakeBlock(data)) > 0;
         else return false;
      }
      else {
         Verbs::Associate verb {Forward<D>(data)};
         Run(verb);
         return verb.IsDone();
      }
   }

} // namespace Langulus::Flow