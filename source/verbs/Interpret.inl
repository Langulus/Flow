///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Interpret.hpp"
#include "Create.hpp"
#include "Do.hpp"
#include "../Serial.hpp"

#include <Anyness/View/Any.hpp>
#include <Anyness/View/Map.hpp>
#include <Anyness/View/Set.hpp>
#include <Anyness/Pair.hpp>
#include <Anyness/Own.hpp>
#include <Anyness/Ref.hpp>
#include <Anyness/Neat.hpp>

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
            return not InterpretAs<Text>::ExecuteDefault(context, verb);

         //TODO check reflected morphisms?
         return true;
      });

      return verb.IsDone();
   }

   /// Specialized interpret verb default construction adds the TO type as    
   /// an argument automatically                                              
   template<CT::Data TO>
   InterpretAs<TO>::InterpretAs() {
      SetArgument(MetaOf<TO>());
   }

   /// Execute the default verb in an immutable context                       
   /// Statically optimized to avoid passing an argument                      
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   template<CT::Data TO>
   bool InterpretAs<TO>::ExecuteDefault(const Block& context, Verb& verb) {
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
      else if constexpr (CT::Convertible<FROM, TO> and not CT::Deep<FROM>) {
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
      else if constexpr (CT::Text<TO> or CT::Bytes<TO>) {
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

namespace Langulus::Flow
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
      if (not mVerb) {
         // An invalid verb is always written as token                  
         result += MetaVerb::DefaultToken;
      }
      else {
         // A valid verb is written either as token, or as operator     
         if (mMass < 0) {
            if (not mVerb->mOperatorReverse.empty() and (GetCharge() * -1).IsDefault() and mState.IsDefault()) {
               // Write as operator                                     
               result += mVerb->mOperatorReverse;
               enscope = GetCount() > 1 or (not IsEmpty() and CastsTo<Verb>());
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
            if (not mVerb->mOperator.empty() and GetCharge().IsDefault() and mState.IsDefault()) {
               // Write as operator                                     
               result += mVerb->mOperator;
               enscope = GetCount() > 1 or (not IsEmpty() and CastsTo<Verb>());
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

} // namespace Langulus::CT

namespace fmt
{

   ///                                                                        
   /// Extend FMT to be capable of logging anything CT::Deep                  
   ///                                                                        
   template<Langulus::CT::Deep T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         const auto asText = Verbs::Interpret::To<Flow::Debug>(element);
         return fmt::format_to(ctx.out(), "{}",
            static_cast<Logger::TextView>(asText));
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging any pair                           
   ///                                                                        
   template<Langulus::CT::Pair T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         return fmt::vformat_to(ctx.out(), "Pair({}, {})",
            DenseCast(element.mKey),
            DenseCast(element.mValue)
         );
      }
   };
   
   ///                                                                        
   /// Extend FMT to be capable of logging Neat                               
   ///                                                                        
   template<>
   struct formatter<Langulus::Anyness::Neat> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(Langulus::Anyness::Neat const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         const auto asText = Verbs::Interpret::To<Flow::Debug>(element);
         return fmt::format_to(ctx.out(), "{}",
            static_cast<Logger::TextView>(asText));
      }
   };
    
   ///                                                                        
   /// Extend FMT to be capable of logging Construct                          
   ///                                                                        
   template<>
   struct formatter<Langulus::Anyness::Construct> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(Langulus::Anyness::Construct const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         const auto asText = Verbs::Interpret::To<Flow::Debug>(element);
         return fmt::format_to(ctx.out(), "{}",
            static_cast<Logger::TextView>(asText));
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging any trait                          
   ///                                                                        
   template<Langulus::CT::TraitBased T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         const auto type = element.GetTrait();
         return fmt::format_to(ctx.out(), "{}({})",
            (type ? type->mToken : RTTI::MetaTrait::DefaultToken),
            static_cast<const Anyness::Any&>(element)
         );
      }
   };

   ///                                                                        
   /// Extend FMT to be capable of logging any map                            
   ///                                                                        
   template<Langulus::CT::Map T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         fmt::format_to(ctx.out(), "Map(");
         bool first = true;
         for (auto pair : element) {
            if (not first)
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
   template<Langulus::CT::Set T>
   struct formatter<T> {
      template<class CONTEXT>
      constexpr auto parse(CONTEXT& ctx) {
         return ctx.begin();
      }

      template<class CONTEXT>
      LANGULUS(INLINED)
      auto format(T const& element, CONTEXT& ctx) {
         using namespace ::Langulus;

         fmt::format_to(ctx.out(), "Set(");
         bool first = true;
         for (auto key : element) {
            if (not first)
               fmt::format_to(ctx.out(), ", ");

            first = false;
            fmt::format_to(ctx.out(), "{}", DenseCast(key));
         }

         return fmt::format_to(ctx.out(), ")");
      }
   };

} // namespace fmt


namespace Langulus::Anyness
{

   /// Define the otherwise undefined Langulus::Anyness::Block::AsCast        
   /// to use the interpret verb pipeline for runtime conversion              
   ///   @tparam T - the type to convert to                                   
   ///   @tparam FATAL_FAILURE - true to throw on failure, otherwise          
   ///                           return a default-initialized T on fail       
   ///   @return the first element, converted to T                            
   template<CT::Data T, bool FATAL_FAILURE, CT::Index IDX>
   T Block::AsCast(const IDX& index) const {
      if (IsEmpty()) {
         if constexpr (FATAL_FAILURE)
            LANGULUS_THROW(Convert, "Unable to AsCast, container is empty");
         else if constexpr (CT::Defaultable<T>)
            return {};
         else {
            LANGULUS_ERROR(
               "Unable to AsCast to non-default-constructible type, "
               "when lack of FATAL_FAILURE demands it");
         }
      }

      // Simplify the index as early as possible                        
      const auto idx = mType->Is<T>()
         ? SimplifyIndex<T>(index)
         : SimplifyIndex<void>(index);

      // Attempt pointer arithmetic conversion first                    
      try { return As<T>(idx); }
      catch (...) {}

      // If reached, then pointer arithmetic conversion failed, and we  
      // need more advanced conversions                                 
      if constexpr (CT::Inner::DescriptorMakable<T>) {
         // If this is reached, we attempt runtime conversion by        
         // invoking descriptor constructor of T, with the desired      
         // element, as descriptor                                      
         try { return T {Describe(GetElement(idx))}; }
         catch (...) {}
      }

      // Alternatively, we attempt runtime conversion by                
      // dispatching Verbs::Interpret to the indicated element          
      const auto meta = MetaOf<T>();
      Verbs::Interpret interpreter {meta};
      auto context = GetElementResolved(idx);
      if (Flow::DispatchDeep<false>(context, interpreter)) {
         // Success                                                     
         return interpreter->As<T>();
      }

      // Failure if reached                                             
      if constexpr (FATAL_FAILURE)
         LANGULUS_THROW(Convert, "Unable to AsCast");
      else if constexpr (CT::Defaultable<T>)
         return {};
      else {
         LANGULUS_ERROR(
            "Unable to AsCast to non-default-constructible type, "
            "when lack of FATAL_FAILURE demands it");
      }
   }
   
   /// Define the otherwise undefined Langulus::Anyness::Neat::ExtractDataAs  
   /// to use the interpret verb pipeline for runtime conversion              
   /// Extract any data, convertible to D                                     
   ///   @param value - [out] where to save the value, if found               
   ///   @return the number of extracted values (always 1 if not an array)    
   inline Count Neat::ExtractDataAs(CT::Data auto& value) const {
      using D = Deref<decltype(value)>;

      if constexpr (CT::Array<D>) {
         // Fill a bounded array                                        
         Count scanned = 0;
         for (auto pair : mAnythingElse) {
            for (auto& group : pair.mValue) {
               const auto toscan = ::std::min(ExtentOf<D> - scanned, group.GetCount());
               for (Offset i = 0; i < toscan; ++i) {
                  //TODO can be optimized-out for POD
                  try {
                     value[scanned] = group.template AsCast<Deext<D>>(i);
                     ++scanned;
                  }
                  catch (...) {}
               }

               if (scanned >= ExtentOf<D>)
                  return ExtentOf<D>;
            }
         }

         return scanned;
      }
      else {
         // Fill a single value                                         
         for (auto pair : mAnythingElse) {
            for (auto& group : pair.mValue) {
               try {
                  value = group.template AsCast<D>();
                  return 1;
               }
               catch (...) {}
            }
         }
      }

      return 0;
   }

} // namespace Langulus::Anyness

#undef VERBOSE_CONVERSION
