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

   /// Construct from a type and shallow-copied arguments                     
   ///   @param type - the type of the content                                
   ///   @param arguments - the arguments to copy                             
   ///   @param charge - the charge for the construction                      
   template<CT::Data T>
   Construct::Construct(DMeta type, const T& arguments, const Charge& charge)
      : Any {arguments}
      , Charge {charge}
      , mType {type} { }

   template<CT::Data T>
   Construct::Construct(DMeta type, T& arguments, const Charge& charge)
      : Construct {type, const_cast<const T&>(arguments), charge} { }

   /// Construct from a type and moved arguments                              
   ///   @param type - the type of the content                                
   ///   @param arguments - the arguments to move                             
   ///   @param charge - the charge for the construction                      
   template<CT::Data T>
   Construct::Construct(DMeta type, T&& arguments, const Charge& charge)
      : Any {Forward<T>(arguments)}
      , Charge {charge}
      , mType {type} { }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Construct from a type token and shallow-copied arguments               
   ///   @param token - the type of the content                               
   ///   @param arguments - the arguments to copy                             
   ///   @param charge - the charge for the construction                      
   template<CT::Data T>
   Construct::Construct(const Token& token, const T& arguments, const Charge& charge)
      : Any {arguments}
      , Charge {charge}
      , mType {RTTI::Database.GetMetaData(token)} { }

   template<CT::Data T>
   Construct::Construct(const Token& token, T& arguments, const Charge& charge)
      : Construct {token, const_cast<const T&>(arguments), charge} { }

   /// Construct from a type token and moved arguments                        
   ///   @param token - the type of the content                               
   ///   @param arguments - the arguments to move                             
   ///   @param charge - the charge for the construction                      
   template<CT::Data T>
   Construct::Construct(const Token& token, T&& arguments, const Charge& charge)
      : Any {Forward<T>(arguments)}
      , Charge {charge}
      , mType {RTTI::Database.GetMetaData(token)} { }
#endif

   /// Create content descriptor from a static type and arguments by copy     
   ///   @tparam T - type of the construct                                    
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
   Construct Construct::From(const HEAD& head, const TAIL&... tail) {
      const auto meta = MetaData::Of<Decay<T>>();
      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, head};
      else
         return Construct {meta, Any {head, tail...}};
   }

   /// Create content descriptor from a static type and arguments by move     
   ///   @tparam T - type of the construct                                    
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
   Construct Construct::From(HEAD&& head, TAIL&&... tail) {
      const auto meta = MetaData::Of<Decay<T>>();
      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, Forward<HEAD>(head)};
      else
         return Construct {meta, Any {Forward<HEAD>(head), Forward<TAIL>(tail)...}};
   }

   /// Create content descriptor from a static type (without arguments)       
   ///   @tparam T - type of the construct                                    
   ///   @return the request                                                  
   template<CT::Data T>
   Construct Construct::From() {
      return Construct {MetaData::Of<T>()};
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Create content descriptor from a type token and arguments by copy      
   ///   @tparam HEAD, TAIL - types of the arguments (deducible)              
   ///   @param token - the type name for the construct                       
   ///   @param head, tail  - the constructor arguments                       
   ///   @return the request                                                  
   template<CT::Data HEAD, CT::Data... TAIL>
   Construct Construct::FromToken(const Token& token, const HEAD& head, const TAIL&... tail) {
      const auto meta = RTTI::Database.GetMetaData(token);
      if constexpr (sizeof...(tail) == 0)
         return Construct {meta, Forward<HEAD>(head)};
      else
         return Construct {meta, Any {head, tail...}};
   }

   /// Create content descriptor from a type token (without arguments)        
   ///   @param token - type of the construct                                 
   ///   @return the request                                                  
   inline Construct Construct::FromToken(const Token& token) {
      return Construct {RTTI::Database.GetMetaData(token)};
   }
#endif
   
   /// Check if construct type can be interpreted as a given static type      
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   bool Construct::CastsTo() const {
      return CastsTo(MetaData::Of<T>());
   }

   /// Check if construct type fully matches a given static type              
   ///   @tparam T - type of the construct to compare against                 
   template<CT::Data T>
   bool Construct::Is() const {
      return Is(MetaData::Of<T>());
   }

   /// Get the argument for the construct                                     
   ///   @return the constant arguments container                             
   inline const Any& Construct::GetArgument() const noexcept {
      return static_cast<const Any&>(*this);
   }

   /// Get the argument for the construct                                     
   ///   @return the arguments container                                      
   inline Any& Construct::GetArgument() noexcept {
      return static_cast<Any&>(*this);
   }

   /// Get construct's charge                                                 
   ///   @return the charge                                                   
   inline Charge& Construct::GetCharge() noexcept {
      return static_cast<Charge&>(*this);
   }

   /// Get construct's charge (const)                                         
   ///   @return the charge                                                   
   inline const Charge& Construct::GetCharge() const noexcept {
      return static_cast<const Charge&>(*this);
   }

   /// Get the type of the construct                                          
   ///   @return the type                                                     
   inline DMeta Construct::GetType() const noexcept {
      return mType;
   }

   /// Get the producer of the construct                                      
   ///   @return the type of the producer                                     
   inline DMeta Construct::GetProducer() const noexcept {
      return mType ? mType->mProducer : nullptr;
   }

   /// Clone the construct, substituting the construct type with another      
   ///   @tparam T - the new type of the construct                            
   ///   @return the new construct                                            
   template<CT::Data T>
   Construct Construct::CloneAs() const {
      return Clone(MetaData::Of<T>());
   }

   /// Push arguments to the back by copy                                     
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
   Construct& Construct::operator << (const T& whatever) {
      if (SmartPush<IndexBack, true, true>(whatever))
         mHash = {};
      return *this;
   }

   /// Push arguments to the back by move                                     
   ///   @param whatever - the thing you wish to move and push                
   template<CT::Data T>
   Construct& Construct::operator << (T&& whatever) {
      if (SmartPush<IndexBack, true, true>(Forward<T>(whatever)))
         mHash = {};
      return *this;
   }

   /// Push arguments to the front by copy                                    
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
   Construct& Construct::operator >> (const T& whatever) {
      if (SmartPush<IndexFront, true, true>(whatever))
         mHash = {};
      return *this;
   }

   /// Push arguments to the front by move                                    
   ///   @param whatever - the thing you wish to move and push                
   template<CT::Data T>
   Construct& Construct::operator >> (T&& whatever) {
      if (SmartPush<IndexFront, true, true>(Forward<T>(whatever)))
         mHash = {};
      return *this;
   }

   /// Merge items at the back of the arguments by copy                       
   ///   @param whatever - the thing you wish to copy and push                
   template<CT::Data T>
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
   const Trait* Construct::Get(const Offset& index) const {
      return Get(MetaTrait::Of<T>(), index);
   }

} // namespace Langulus::Flow
