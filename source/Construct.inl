///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Construct.hpp"

namespace Langulus::Flow
{

   /// Shallow-copy constructor                                               
   ///   @param other - construct to shallow-copy                             
   LANGULUS(INLINED)
   Construct::Construct(const Construct& other) noexcept
      : Construct {Copy(other)} {}

   /// Move constructor                                                       
   ///   @param other - construct to move                                     
   LANGULUS(INLINED)
   Construct::Construct(Construct&& other) noexcept
      : Construct {Move(other)} {}

   /// Semantic constructor                                                   
   ///   @tparam S - semantic and type (deducible)                            
   ///   @param other - the construct and semantic                            
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct::Construct(S&& other) requires (CT::Exact<TypeOf<S>, Construct>)
      : Any {other.template Forward<Any>()}
      , Charge {*other}
      , mType {other->mType}
      , mHash {other->mHash} {
      if constexpr (S::Move && S::Keep) {
         other->ResetCharge();
         other->mType = nullptr;
         other->mHash = {};
      }
   }

   /// Construct from a type                                                  
   ///   @param type - the type of the content                                
   LANGULUS(INLINED)
   Construct::Construct(DMeta type)
      : mType {type ? type->mOrigin : nullptr} {}

   /// Shallow-copying manual construct constructor                           
   ///   @tparam T - the type of arguments (deducible)                        
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, const T& arguments, const Charge& charge)
      : Construct {type, Copy(arguments), charge} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, T& arguments, const Charge& charge)
      : Construct {type, Copy(arguments), charge} {}

   /// Moving manual construct constructor                                    
   ///   @tparam T - the type of arguments (deducible)                        
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, T&& arguments, const Charge& charge)
      : Construct {type, Move(arguments), charge} {}
   
   /// Semantic manual construct constructor                                  
   ///   @tparam S - the semantic and type of arguments (deducible)           
   ///   @param type - the type of the construct                              
   ///   @param arguments - the arguments for construction                    
   ///   @param charge - the charge for the construction                      
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct::Construct(DMeta type, S&& arguments, const Charge& charge)
      : Any {arguments.Forward()}
      , Charge {charge}
      , mType {type ? type->mOrigin : nullptr} { }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Construct from a type token                                            
   ///   @param type - the type of the content                                
   LANGULUS(INLINED)
   Construct::Construct(const Token& token)
      : mType {RTTI::Database.GetMetaData(token)->mOrigin} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, const T& arguments, const Charge& charge)
      : Construct {token, Copy(arguments), charge} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, T& arguments, const Charge& charge)
      : Construct {token, Copy(arguments), charge} {}

   template<CT::NotSemantic T>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, T&& arguments, const Charge& charge)
      : Construct {token, Move(arguments), charge} {}

   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct::Construct(const Token& token, S&& arguments, const Charge& charge)
      : Construct {RTTI::Database.GetMetaData(token), arguments.Forward(), charge} {}
#endif

   /// Copy-assignment                                                        
   ///   @param rhs - the construct to shallow-copy                           
   ///   @return a reference to this construct                                
   LANGULUS(INLINED)
   Construct& Construct::operator = (const Construct& rhs) noexcept {
      return operator = (Copy(rhs));
   }

   /// Move-assignment                                                        
   ///   @param rhs - the construct to move                                   
   ///   @return a reference to this construct                                
   LANGULUS(INLINED)
   Construct& Construct::operator = (Construct&& rhs) noexcept {
      return operator = (Move(rhs));
   }

   /// Semantic-assignment                                                    
   ///   @tparam S - semantic to use (deducible)                              
   ///   @param rhs - the right hand side                                     
   ///   @return a reference to this construct                                
   template<CT::Semantic S>
   LANGULUS(INLINED)
   Construct& Construct::operator = (S&& rhs) requires (CT::Exact<TypeOf<S>, Construct>) {
      Any::Free();
      new (this) Construct {rhs.Forward()};
      return *this;
   }

   /// Create content descriptor from a static type and arguments by move     
   ///   @tparam T - type of the construct                                    
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   Construct Construct::From(HEAD&& head, TAIL&&... tail) {
      static_assert(CT::Decayed<T>, "T must be fully decayed");
      const auto meta = RTTI::MetaData::Of<T>();
      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, Forward<HEAD>(head)};
      else
         return Construct {meta, Any {Forward<HEAD>(head), Forward<TAIL>(tail)...}};
   }

   /// Create content descriptor from a static type (without arguments)       
   ///   @tparam T - type of the construct                                    
   ///   @return the request                                                  
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct Construct::From() {
      static_assert(CT::Decayed<T>, "T must be fully decayed");
      return Construct {RTTI::MetaData::Of<T>()};
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Create content descriptor from a type token and arguments by copy      
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param token - the type name for the construct                       
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data HEAD, CT::Data... TAIL>
   LANGULUS(INLINED)
   Construct Construct::FromToken(const Token& token, HEAD&& head, TAIL&&... tail) {
      const auto meta = dynamic_cast<DMeta>(
         RTTI::Database.DisambiguateMeta(token)
      );

      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, Forward<HEAD>(head)};
      else
         return Construct {meta, Any {Forward<HEAD>(head), Forward<TAIL>(tail)...}};
   }

   /// Create content descriptor from a type token (without arguments)        
   ///   @param token - type of the construct                                 
   ///   @return the request                                                  
   LANGULUS(INLINED)
   Construct Construct::FromToken(const Token& token) {
      const auto meta = dynamic_cast<DMeta>(
         RTTI::Database.DisambiguateMeta(token)
      );
      return Construct {meta};
   }
#endif
   
   /// Check if construct type can be interpreted as a given static type      
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Construct::CastsTo() const {
      return CastsTo(RTTI::MetaData::Of<T>());
   }

   /// Check if construct type fully matches a given static type              
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   LANGULUS(INLINED)
   bool Construct::Is() const {
      return Is(RTTI::MetaData::Of<T>());
   }

   /// Get the argument for the construct                                     
   ///   @return the constant arguments container                             
   LANGULUS(INLINED)
   const Any& Construct::GetArgument() const noexcept {
      return static_cast<const Any&>(*this);
   }

   /// Get the argument for the construct                                     
   ///   @return the arguments container                                      
   LANGULUS(INLINED)
   Any& Construct::GetArgument() noexcept {
      return static_cast<Any&>(*this);
   }

   /// Get construct's charge                                                 
   ///   @return the charge                                                   
   LANGULUS(INLINED)
   Charge& Construct::GetCharge() noexcept {
      return static_cast<Charge&>(*this);
   }

   /// Get construct's charge (const)                                         
   ///   @return the charge                                                   
   LANGULUS(INLINED)
   const Charge& Construct::GetCharge() const noexcept {
      return static_cast<const Charge&>(*this);
   }

   /// Get the type of the construct                                          
   ///   @return the type                                                     
   LANGULUS(INLINED)
   DMeta Construct::GetType() const noexcept {
      return mType;
   }
   
   /// Get the token of the construct's type                                  
   ///   @return the token, if type is set, or default token if not           
   LANGULUS(INLINED)
   Token Construct::GetToken() const noexcept {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         return mType ? mType->GetShortestUnambiguousToken() : MetaData::DefaultToken;
      #else
         return mType ? mType->mToken : MetaData::DefaultToken;
      #endif
   }

   /// Get the producer of the construct                                      
   ///   @return the type of the producer                                     
   LANGULUS(INLINED)
   DMeta Construct::GetProducer() const noexcept {
      return mType ? mType->mProducer : nullptr;
   }

   /// Push arguments to the back by copy                                     
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator << (const T& whatever) {
      if (SmartPush<IndexBack, true, true>(whatever))
         mHash = {};
      return *this;
   }

   /// Push arguments to the back by move                                     
   ///   @param whatever - the thing you wish to move and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator << (T&& whatever) {
      if (SmartPush<IndexBack, true, true>(Forward<T>(whatever)))
         mHash = {};
      return *this;
   }

   /// Push arguments to the front by copy                                    
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator >> (const T& whatever) {
      if (SmartPush<IndexFront, true, true>(whatever))
         mHash = {};
      return *this;
   }

   /// Push arguments to the front by move                                    
   ///   @param whatever - the thing you wish to move and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator >> (T&& whatever) {
      if (SmartPush<IndexFront, true, true>(Forward<T>(whatever)))
         mHash = {};
      return *this;
   }

   /// Merge items at the back of the arguments by copy                       
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator <<= (const T& whatever) {
      if constexpr (CT::Same<T, Trait>)
         return Set(DenseCast(whatever));
      else {
         if (!FindDeep(whatever) && SmartPush<IndexBack, true, true>(whatever))
            mHash = {};
         return *this;
      }
   }

   /// Merge items at the back of the arguments by move                       
   ///   @param whatever - the thing you wish to move and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator <<= (T&& whatever) {
      if constexpr (CT::Same<T, Trait>)
         return Set(DenseCast(whatever));
      else {
         if (!FindDeep(whatever) && SmartPush<IndexBack, true, true>(Forward<T>(whatever)))
            mHash = {};
         return *this;
      }
   }

   /// Merge items at the back of the arguments by copy                       
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator >>= (const T& whatever) {
      if constexpr (CT::Same<T, Trait>)
         Set(DenseCast(whatever));
      else {
         if (!FindDeep(whatever) && SmartPush<IndexFront, true, true>(whatever))
            mHash = {};
      }

      return *this;
   }

   /// Merge items at the back of the arguments by move                       
   ///   @param whatever - the thing you wish to move and push                
   template<CT::Data T>
   LANGULUS(INLINED)
   Construct& Construct::operator >>= (T&& whatever) {
      if constexpr (CT::Same<T, Trait>)
         Set(DenseCast(whatever));
      else {
         if (!FindDeep(whatever) && SmartPush<IndexFront, true, true>(Forward<T>(whatever)))
            mHash = {};
      }

      return *this;
   }

   /// Get traits from constructor                                            
   ///   @return pointer to found traits or nullptr if none found             
   template<CT::Trait T>
   LANGULUS(INLINED)
   const Trait* Construct::Get(const Offset& index) const {
      return Get(T::GetTrait(), index);
   }

} // namespace Langulus::Flow
