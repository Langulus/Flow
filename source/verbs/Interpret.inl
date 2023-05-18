///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Scope.hpp"
#include "../Serial.hpp"
#include "../Time.hpp"
#include "../Construct.hpp"
#include "Do.inl"

#include <Anyness/View/Any.hpp>
#include <Anyness/View/Map.hpp>
#include <Anyness/View/Set.hpp>
#include <Anyness/Pair.hpp>
#include <Anyness/Own.hpp>
#include <Anyness/Ref.hpp>
#include <fmt/chrono.h>
#include <Flow/Verbs/Interpret.hpp>
#include "../Verb.inl"

#define VERBOSE_CONVERSION(...) // Logger::Verbose(__VA_ARGS__)

namespace Langulus::Verbs
{
   
   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data... A>
   constexpr bool Interpret::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Interpret(v); };
      else
         return requires (T& t, Verb& v, A... a) { t.Interpret(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data... A>
   constexpr auto Interpret::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Interpret(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A... args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Interpret(verb, args...);
         };
      }
   }

   /// Execute the interpretation verb in a specific context                  
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool Interpret::ExecuteIn(T& context, Verb& verb) {
      static_assert(Interpret::AvailableFor<T>(),
         "Verb is not available for this context"
         "(this shouldn't be reached by flow)");
      context.Interpret(verb);
      return verb.IsDone();
   }

   /// Execute the default verb in an immutable context                       
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Interpret::ExecuteDefault(const Block& context, Verb& verb) {
      verb.ForEach([&](DMeta to) {
         if (to->CastsTo<A::Text>())
            return !InterpretTo<Text>::ExecuteDefault(context, verb);
         //TODO check reflected morphisms?
         return true;
      });

      return verb.IsDone();
   }

   /// Execute the default verb in an immutable context                       
   /// Statically optimized to avoid passing an argument                      
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   template<class TO>
   bool InterpretTo<TO>::ExecuteDefault(const Block& context, Verb& verb) {
      if constexpr (CT::Text<TO>) {
         const auto from = context.GetType();

         // Stringify context, if it matches any of its named values    
         for (auto& named : from->mNamedValues) {
            if (from->mComparer(named->mPtrToValue, context.GetRaw())) {
               verb << Text {named};
               return true;
            }
         }
         return false;
      }
      else return false;
   }

   /// Statically optimized interpret verb                                    
   ///   @param from - the element to convert                                 
   ///   @return the converted element                                        
   template<class TO, class FROM>
   TO Interpret::To(const FROM& from) {
      if constexpr (CT::Same<TO, FROM>) {
         // Types are the same                                          
         return from;
      }
      else if constexpr (CT::Same<TO, Any>) {
         // Always interpreted as deserialization                       
         #if LANGULUS_FEATURE(MANAGED_REFLECTION)
            if constexpr (CT::SameAsOneOf<FROM, Code, Bytes>)
               return Flow::Deserialize(from);
            else
               LANGULUS_ERROR("No deserializer exists between these types");
         #else
            LANGULUS_ERROR(
               "No deserializer exists between these types"
               " (managed reflection is disabled)");
         #endif
      }
      else if constexpr (CT::Convertible<FROM, TO>) {
         // Directly convert if static conversion exists                
         if constexpr (requires { TO {from}; }) {
            return TO {from};
         }
         else if constexpr (requires { TO {from.operator TO()}; }) {
            return TO {from.operator TO()};
         }
         else if constexpr (requires { static_cast<Debug>(from); }) {
            return static_cast<TO>(from);
         }
         else LANGULUS_ERROR("Unhandled conversion route");
      }
      else if constexpr (CT::SameAsOneOf<TO, Code, Text, Debug, Bytes>) {
         // No constructor/conversion operator exists, that would do    
         // the conversion, but we can rely on the serializer,          
         // if TO is supported                                          
         return Flow::Serialize<TO>(from);
      }
      else LANGULUS_ERROR(
         "No static conversion routine, or dynamic serializer "
         "exists between these types");
   }

} // namespace Langulus::Verbs

namespace Langulus
{
   namespace CT
   {
      namespace Inner
      {
         template<class T>
         concept Debuggable =
            requires (T& a) { a.operator Anyness::Debug(); } ||
            requires (const T& a) { a.operator Anyness::Debug(); };
      }

      /// A debuggable type is one that has either an implicit or explicit    
      /// cast operator to Debug type. Reverse conversion through             
      /// constructors is avoided to mitigate ambiguity problems.             
      template<class... T>
      concept Debuggable = (Inner::Debuggable<T> && ...);
   }

   namespace Flow
   {
      
      /// Serialize verb to any form of text                                  
      ///   @tparam T - the type of text to serialize to                      
      ///   @return the serialized verb                                       
      template<CT::Text T>
      LANGULUS(INLINED)
      T Verb::SerializeVerb() const {
         Code result;

         if (mSuccesses) {
            // If verb has been executed, just dump the output          
            result += Verbs::Interpret::To<T>(mOutput);
            return result;
         }

         // If reached, then verb hasn't been executed yet              
         // Let's check if there's a source in which verb is executed   
         if (mSource.IsValid())
            result += Verbs::Interpret::To<T>(mSource);

         // After the source, we decide whether to write verb token or  
         // verb operator, depending on the verb definition, state and  
         // charge                                                      
         bool enscope = true;
         if (!mVerb) {
            // An invalid verb is always written as token               
            result += MetaVerb::DefaultToken;
         }
         else {
            // A valid verb is written either as token, or as operator  
            if (mMass < 0) {
               if (!mVerb->mOperatorReverse.empty() && (GetCharge() * -1).IsDefault() && mState.IsDefault()) {
                  // Write as operator                                  
                  result += mVerb->mOperatorReverse;
                  enscope = GetCount() > 1 || (!IsEmpty() && CastsTo<Verb>());
               }
               else {
                  // Write as token                                     
                  if (mSource.IsValid())
                     result += Text {' '};
                  result += mVerb->mTokenReverse;
                  result += Verbs::Interpret::To<T>(GetCharge() * -1);
               }
            }
            else {
               if (!mVerb->mOperator.empty() && GetCharge().IsDefault() && mState.IsDefault()) {
                  // Write as operator                                  
                  result += mVerb->mOperator;
                  enscope = GetCount() > 1 || (!IsEmpty() && CastsTo<Verb>());
               }
               else {
                  // Write as token                                     
                  if (mSource.IsValid())
                     result += Text {' '};
                  result += mVerb->mToken;
                  result += Verbs::Interpret::To<T>(GetCharge());
               }
            }
         }

         if (IsLongCircuited())
            result += " long ";
         if (IsMonocast())
            result += " mono ";

         if (enscope)
            result += Code::OpenScope;

         if (IsValid())
            result += Verbs::Interpret::To<T>(GetArgument());

         if (enscope)
            result += Code::CloseScope;

         return result;
      }
   }

} // namespace Langulus::CT

namespace fmt
{
   using namespace ::Langulus;

   ///                                                                        
   /// Extend FMT to be capable of logging any meta definition                
   ///                                                                        
   template<CT::Meta T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& meta, CONTEXT& ctx) {
         #if LANGULUS_FEATURE(MANAGED_REFLECTION)
            auto asView = meta.GetShortestUnambiguousToken();
         #else
            auto asView = meta.mToken;
         #endif

         return fmt::format_to(ctx.out(), "{}", asView);
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging Flow::Time                         
   ///                                                                        
   template<>
   struct formatter<Flow::Time> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(Flow::Time const& element, CONTEXT& ctx) {
         return fmt::format_to(ctx.out(), "{}", 
            static_cast<const Flow::Time::Base&>(element));
      }
   };
   
   ///                                                                        
   /// Extend FMT to be capable of logging anything that is statically        
   /// convertible to a Debug string by an explicit or implicit conversion    
   /// operator.                                                              
   ///                                                                        
   template<CT::Debuggable T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         auto asText = element.operator Anyness::Debug();
         return fmt::format_to(ctx.out(), "{}",
            static_cast<Logger::TextView>(asText));
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging anything CT::Deep                  
   ///                                                                        
   template<CT::Deep T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         const auto asText = Verbs::Interpret::To<Flow::Debug>(element);
         return fmt::format_to(ctx.out(), "{}",
            static_cast<Logger::TextView>(asText));
      }
   };
      
   ///                                                                        
   /// Extend FMT to be capable of logging Descriptor (same as CT::Deep)      
   ///                                                                        
   template<>
   struct formatter<Anyness::Descriptor> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(Anyness::Descriptor const& element, CONTEXT& ctx) {
         const auto asText = Verbs::Interpret::To<Flow::Debug>(
            static_cast<const Anyness::Any&>(element));
         return fmt::format_to(ctx.out(), "{}",
            static_cast<Logger::TextView>(asText));
      }
   };
      
   ///                                                                        
   /// Extend FMT to be capable of logging any trait                          
   ///                                                                        
   template<CT::Trait T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         const auto type = element.GetTrait();
         return fmt::format_to(ctx.out(), "{}({})", 
            (type ? type->mToken : RTTI::MetaTrait::DefaultToken),
            static_cast<const Anyness::Any&>(element)
         );
      }
   };
      
   ///                                                                        
   /// Extend FMT to be capable of logging any pair                           
   ///                                                                        
   template<CT::Pair T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         return fmt::vformat_to(ctx.out(), "Pair({}, {})", 
            DenseCast(element.mKey), 
            DenseCast(element.mValue)
         );
      }
   };
   
   ///                                                                        
   /// Extend FMT to be capable of logging any map                            
   ///                                                                        
   template<CT::Map T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         fmt::format_to(ctx.out(), "Map(");
         bool first = true;
         for (auto pair : element) {
            if (!first)
               fmt::format_to(ctx.out(), ", ");
            first = false;

            fmt::format_to(ctx.out(), "({}, {})",
               DenseCast(pair.mKey),
               DenseCast(pair.mValue)
            );
         }

         return fmt::format_to(ctx.out(), ")");
      }
   };
   
   ///                                                                        
   /// Extend FMT to be capable of logging any set                            
   ///                                                                        
   template<CT::Set T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         fmt::format_to(ctx.out(), "Set(");
         bool first = true;
         for (auto key : element) {
            if (!first)
               fmt::format_to(ctx.out(), ", ");
            first = false;

            fmt::format_to(ctx.out(), "{}", DenseCast(key));
         }

         return fmt::format_to(ctx.out(), ")");
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging any owned values                   
   ///                                                                        
   template<CT::Owned T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         if constexpr (CT::Sparse<TypeOf<T>>) {
            if (element == nullptr) {
               const auto type = element.GetType();
               if (type)
                  return fmt::format_to(ctx.out(), "{}(null)", *type);
               else
                  return fmt::format_to(ctx.out(), "null");
            }
            else return fmt::format_to(ctx.out(), "{}", *element.Get());
         }
         else {
            static_assert(CT::Dense<decltype(element.Get())>,
               "T not dense, but not sparse either????");
            return fmt::format_to(ctx.out(), "{}", element.Get());
         }
      }
   };
   
   ///                                                                        
   /// Extend FMT to be capable of logging any shared pointers                
   ///                                                                        
   template<CT::Pointer T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         if (element == nullptr) {
            const auto type = element.GetType();
            if (type)
               return fmt::format_to(ctx.out(), "{}(null)", *type);
            else
               return fmt::format_to(ctx.out(), "null");
         }
         else return fmt::format_to(ctx.out(), "{}", *element.Get());
      }
   };

} // namespace fmt

#undef VERBOSE_CONVERSION
