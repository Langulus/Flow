///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "TVerb.hpp"
#include "Verb.inl"

#define TEMPLATE()   template<class VERB>
#define TME()        TVerb<VERB>


namespace Langulus::Flow
{

   /// Refer-constructor                                                      
   TEMPLATE() LANGULUS(INLINED)
   TME()::TVerb(const TVerb& other)
      : Verb {Refer(other)} {}

   /// Move-constructor                                                       
   TEMPLATE() LANGULUS(INLINED)
   TME()::TVerb(TVerb&& other)
      : Verb {Move(other)} {}

   /// Generic constructor                                                    
   ///   @param other - the verb/argument and intent to construct with        
   TEMPLATE() template<CT::Data T1, CT::Data...TN>
   requires CT::VerbMakable<T1, TN...> LANGULUS(INLINED)
   TME()::TVerb(T1&& t1, TN&&...tn)
      : Verb {Forward<T1>(t1), Forward<TN>(tn)...} {
      (void) GetVerb();
   }

   /// Create a verb from charge and state                                    
   TEMPLATE() LANGULUS(INLINED)
   VERB TME()::From(const Charge& charge, VerbState state) {
      return Verb::From<VERB>(charge, state);
   }

   /// Create a verb from contents, charge and state                          
   TEMPLATE() LANGULUS(INLINED)
   VERB TME()::From(CT::UnfoldInsertable auto&& contents, const Charge& charge, VerbState state) {
      using T = Deref<decltype(contents)>;
      return Verb::From<VERB>(Forward<T>(contents), charge, state);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB TME()::Fork(auto&&...a) const noexcept {
      return Verb::Fork<VERB>(Forward<Deref<decltype(a)>>(a)...);
   }

   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (const TVerb& rhs) {
      Verb::operator = (Refer(rhs));
      return *this;
   }

   TEMPLATE() LANGULUS(INLINED)
   TME()& TME()::operator = (TVerb&& rhs) {
      Verb::operator = (Move(rhs));
      return *this;
   }

   /// Serialize verb to code                                                 
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator Code() const {
      mVerb = MetaVerbOf<VERB>();
      return Verb::operator Code();
   }

   /// Serialize verb for logger                                              
   TEMPLATE() LANGULUS(INLINED)
   TME()::operator Text() const {
      mVerb = MetaVerbOf<VERB>();
      return Verb::operator Text();
   }

   TEMPLATE() template<CT::Data T1, CT::Data...TN>
   requires CT::UnfoldInsertable<T1, TN...> LANGULUS(INLINED)
   VERB& TME()::SetSource(T1&& t1, TN&&...tn) {
      return Verb::SetSource<VERB>(Forward<T1>(t1), Forward<TN>(tn)...);
   }
      
   TEMPLATE() template<CT::Data T1, CT::Data...TN>
   requires CT::UnfoldInsertable<T1, TN...> LANGULUS(INLINED)
   VERB& TME()::SetArgument(T1&& t1, TN&&...tn) {
      return Verb::SetArgument<VERB>(Forward<T1>(t1), Forward<TN>(tn)...);
   }
      
   TEMPLATE() template<CT::Data T1, CT::Data...TN>
   requires CT::UnfoldInsertable<T1, TN...> LANGULUS(INLINED)
   VERB& TME()::SetOutput(T1&& t1, TN&&...tn) {
      return Verb::SetOutput<VERB>(Forward<T1>(t1), Forward<TN>(tn)...);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB TME()::operator * (Real rhs) const {
      return Verb::operator * <VERB> (rhs);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB TME()::operator ^ (Real rhs) const {
      return Verb::operator ^ <VERB> (rhs);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::operator *= (Real rhs) noexcept {
      return Verb::operator *= <VERB> (rhs);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::operator ^= (Real rhs) noexcept {
      return Verb::operator ^= <VERB> (rhs);
   }

   TEMPLATE() LANGULUS(INLINED)
   Hash TME()::GetHash() const {
      return Verb::GetHash<VERB>();
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::ShortCircuit(bool state) noexcept {
      return Verb::ShortCircuit<VERB>(state);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::Multicast(bool state) noexcept {
      return Verb::Multicast<VERB>(state);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::SetVerbState(VerbState state) noexcept {
      return Verb::SetVerbState<VERB>(state);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::Invert() noexcept {
      return Verb::Invert<VERB>();
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::SetMass(Real value) noexcept {
      return Verb::SetMass<VERB>(value);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::SetRate(Real value) noexcept {
      return Verb::SetRate<VERB>(value);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::SetTime(Real value) noexcept {
      return Verb::SetTime<VERB>(value);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::SetPriority(Real value) noexcept {
      return Verb::SetPriority<VERB>(value);
   }

   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::SetCharge(const Charge& charge) noexcept {
      return Verb::SetCharge<VERB>(charge);
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr Token TME()::GetToken() const {
      return Verb::GetToken<VERB>();
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr bool TME()::IsVerb(VMeta meta) const noexcept {
      return Verb::IsVerb<VERB>(meta);
   }

   TEMPLATE() template<CT::Verb V1> LANGULUS(INLINED)
   constexpr bool TME()::IsVerb() const noexcept {
      return Verb::IsVerb<V1, VERB>();
   }

   TEMPLATE() LANGULUS(INLINED)
   constexpr VMeta TME()::GetVerb() const noexcept {
      return Verb::GetVerb<VERB>();
   }
   
   /// Compare verbs                                                          
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   TEMPLATE() LANGULUS(INLINED)
   bool TME()::operator == (const CT::VerbBased auto& rhs) const {
      return Verb::operator == <VERB> (rhs);
   }

   /// Compare verb types for equality                                        
   ///   @param rhs - the verb to compare against                             
   ///   @return true if verbs match                                          
   TEMPLATE() LANGULUS(INLINED)
   bool TME()::operator == (VMeta rhs) const noexcept {
      return Verb::operator == <VERB> (rhs);
   }

   /// Push anything to end of the outputs, satisfying the verb               
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param rhs - the data and intent to push                             
   ///   @return a reference to this verb for chaining                        
   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::operator << (CT::UnfoldInsertable auto&& rhs) {
      using T = Deref<decltype(rhs)>;
      return Verb::operator << <VERB>(Forward<T>(rhs));
   }

   /// Push anything to the front of the outputs, satisfying the verb         
   ///   @attention nullptrs are never pushed and don't satisfy verb          
   ///   @param rhs - the data and intent to push                             
   ///   @return a reference to this verb for chaining                        
   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::operator >> (CT::UnfoldInsertable auto&& rhs) {
      using T = Deref<decltype(rhs)>;
      return Verb::operator >> <VERB>(Forward<T>(rhs));
   }

   /// Merge anything to output's back, with or without an intent             
   ///   @param rhs - the data and intent to merge                            
   ///   @return a reference to this verb for chaining                        
   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::operator <<= (CT::UnfoldInsertable auto&& rhs) {
      using T = Deref<decltype(rhs)>;
      return Verb::operator <<= <VERB>(Forward<T>(rhs));
   }

   /// Merge anything to output's front, with or without an intent            
   ///   @param rhs - the data and intent to merge                            
   ///   @return a reference to this verb for chaining                        
   TEMPLATE() LANGULUS(INLINED)
   VERB& TME()::operator >>= (CT::UnfoldInsertable auto&& rhs) {
      using T = Deref<decltype(rhs)>;
      return Verb::operator >>= <VERB>(Forward<T>(rhs));
   }

} // namespace Langulus::Flow

#undef TME
#undef TEMPLATE
