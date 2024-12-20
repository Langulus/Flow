///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Create.hpp"
#include "../TVerb.inl"
#include "Associate.inl"
#include "Select.inl"
#include "Interpret.inl"
#include <Anyness/TMap.hpp>

#if 0
   #define VERBOSE_CREATION(...) Logger::Verbose(__VA_ARGS__)
   #define ERROR_CREATION(...)   Logger::Error(__VA_ARGS__)
#else
   #define VERBOSE_CREATION(...) LANGULUS(NOOP)
   #define ERROR_CREATION(...)   LANGULUS(NOOP)
#endif


namespace Langulus::Verbs
{

   /// Check if the verb is available in a type, and with given arguments     
   ///   @return true if verb is available in T with arguments A...           
   template<CT::Dense T, CT::Data...A>
   constexpr bool Create::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Create(v); };
      else
         return requires (T& t, Verb& v, A...a) { t.Create(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Create::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Create(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Create(verb, args...);
         };
      }
   }

   /// Execute creation verb in a specific context                            
   ///   @param context - the producer                                        
   ///   @param verb - the creation/destruction verb                          
   ///   @return true if verb was satisfied                                   
   template<CT::Dense T>
   bool Create::ExecuteIn(T& context, Verb& verb) {
      static_assert(Create::AvailableFor<T>(),
         "Verb is not available for this context, "
         "this shouldn't be reached by flow");
      context.Create(verb);
      return verb.IsDone();
   }

   /// Default creation/destruction in a context                              
   ///   @param context - the producer                                        
   ///   @param verb - the creation/destruction verb                          
   ///   @return true if verb was satisfied                                   
   inline bool Create::ExecuteDefault(Many&, Verb& verb) {
      // Attempt creating/destroying constructs                         
      verb.ForEachDeep([&](const Construct& construct) {
         if (construct.GetProducer()) {
            // Creation of customly produced type hit default creation, 
            // and that should not be allowed - you probably forgot to  
            // add and reflect the Verbs::Create in the producer        
            return;
         }
         else if (construct->IsMissingDeep()) {
            // Creation of missing stuff is not allowed                 
            return;
         }

         if (construct.GetCharge().mMass * verb.GetMass() < 0) {
            TODO(); //destroy
         }
         else {
            // Create                                                   
            // First allocate and default-initialize the results        
            auto created = Many::FromMeta(construct.GetType());
            created.New(Count(construct.GetCharge().mMass));
            auto& arguments = construct.GetDescriptor();

            // Then forward the constructors to each element            
            if (arguments) {
               for (Count i = 0; i < created.GetCount(); ++i) {
                  Many element = created.GetElement(i);

                  // First attempt delegating                           
                  VERBOSE_CREATION(Logger::Yellow,
                     "Delegating: ", arguments, " to ", element);
                  Verbs::Create creator {arguments};

                  if (ExecuteVerb(element, creator)) {
                     VERBOSE_CREATION(Logger::Yellow <<
                        "Sideproduct: " << creator.GetOutput());
                     created.MergeBlock(IndexBack, Abandon(creator.GetOutput()));
                     continue;
                  }

                  VERBOSE_CREATION(Logger::Yellow,
                     "Couldn't delegate ", arguments, " inside: ", element);

                  // If reached, let's attempt to set reflected members 
                  SetMembers(element, arguments);
               }
            }

            // Commit                                                   
            verb << Abandon(created);
         }
      });

      return verb.IsDone();
   }

   /// Stateless creation of any type without a producer                      
   ///   @param verb - the creation verb                                      
   ///   @return true if verb was satisfied                                   
   inline bool Create::ExecuteStateless(Verb& verb) {
      if (not verb or verb.GetMass() <= 0)
         return false;

      const auto createInner = [&](const Construct& descriptor) {
         if (descriptor.GetType()->mProducerRetriever
         or  descriptor->IsMissingDeep()) {
            // Creation of missing/runtime stuff is not allowed         
            return;
         }

         // Charged creation of a type                                  
         const auto type = descriptor.GetType();
         const auto count = static_cast<Count>(descriptor.GetCharge().mMass * verb.GetMass());
         auto result = Many::FromMeta(type);

         if (type->mDescriptorConstructor and descriptor.GetDescriptor()) {
            for (Offset i = 0; i < count; ++i) {
               if (count != 1) {
                  VERBOSE_CREATION(Logger::Yellow,
                     "Charged creation (descriptor-constructed) - creating ",
                     i + 1, " of ", count
                  );
               }

               try {
                  result.Emplace(IndexBack, Describe {descriptor.GetDescriptor()});
               }
               catch (...) {
                  ERROR_CREATION("Can't statelessly produce ", descriptor);
                  return;
               }
            }
         }
         else if (type->mDefaultConstructor and not descriptor.GetDescriptor()) {
            for (Offset i = 0; i < count; ++i) {
               if (count != 1) {
                  VERBOSE_CREATION(Logger::Yellow,
                     "Charged creation (default-constructed) - creating ",
                     i + 1, " of ", count
                  );
               }

               try {
                  result.Emplace(IndexBack);
               }
               catch (...) {
                  ERROR_CREATION("Can't statelessly produce ", descriptor);
                  return;
               }
            }
         }
         else {
            ERROR_CREATION("Can't statelessly produce ", descriptor);
            return;
         }

         verb << Abandon(result);
      };

      // Scan the request                                               
      verb.ForEachDeep([&](const Many& group) {
         if (group.IsMissing()) {
            // Creation of missing stuff is not allowed                 
            return;
         }

         group.ForEach(
            [&](const Construct& construct) {
               if (construct.GetType() and construct.GetCharge().mMass > 0) {
                  VERBOSE_CREATION("Creating: ", Logger::Yellow, construct);
                  createInner(construct);
               }
            },
            [&](const DMeta& type) {
               if (type) {
                  VERBOSE_CREATION("Creating: ", Logger::Yellow, type);
                  createInner(Construct {type});
               }
            }
         );
      });

      return verb.IsDone();
   }

   /// Set members in all elements inside context to the provided data        
   ///   @param context - the contexts to analyze                             
   ///   @param data - the data to set to                                     
   ///   @return true if at least one member in one element was set           
   inline void Create::SetMembers(Many& context, const Many& data) {
      TUnorderedMap<TMeta, Count> satisfiedTraits;
      TUnorderedMap<DMeta, Count> satisfiedData;

      data.ForEachDeep([&](const Many& group) {
         VERBOSE_CREATION("Manually initializing ", context, " with ", Logger::Cyan, group);

         // Search for similar data in the current context              
         // in an attempt to overwrite member variables and such        
         for (Count i = 0; i < group.GetCount(); ++i) {
            Many element = group.GetElementResolved(i);
            if (element.Is<Trait>()) {
               // Search for the trait                                  
               const auto meta = element.Get<Trait>().GetTrait();
               const auto sati = satisfiedTraits.Find(meta);
               const auto index = sati
                  ? satisfiedTraits.GetValue(sati)
                  : IndexFirst;

               VERBOSE_CREATION("Searching trait ", meta, "... ", " (", index, ")");

               Verbs::Select selector {meta, index};
               Verb::GenericExecuteIn(context, selector);
               if (selector.GetOutput()) {
                  VERBOSE_CREATION("Initializing trait ", selector.GetOutput(),
                     " with ", Logger::Cyan, element, " (", index, ")");

                  Verbs::Associate associator {element};
                  if (Verb::GenericExecuteIn(selector.GetOutput(), associator)) {
                     // Trait was found and overwritten                 
                     if (sati)
                        ++satisfiedTraits.GetValue(sati);
                     else
                        satisfiedTraits.Insert(meta, Count {1});

                     VERBOSE_CREATION(Logger::Yellow, "Initialized ", 
                        selector.GetOutput(), " (", index, ")");
                     continue;
                  }
               }
            }

            // Search for the data block                                
            // This is only reached if the trait attempt fails          
            // Failing this is considered critical - context should be  
            // later discarded - it's cosidered ill-formed              
            auto meta = element.Is<Trait>()
               ? element.Get<Trait>().GetType()
               : element.GetType();

            if (meta->CastsTo<A::Number>(1)) {
               // If number, keep it abstract                           
               //TODO this seems sketchy and i don't like it!
               meta = MetaOf<A::Number>();
            }

            const auto sati = satisfiedData.Find(meta);
            const auto index = sati
               ? satisfiedData.GetValue(sati)
               : IndexFirst;

            VERBOSE_CREATION("Searching for data ", meta, "... ", " (", index, ")");

            Verbs::Select selector {meta, index};
            Verb::GenericExecuteIn(context, selector);
            if (selector.GetOutput()) {
               VERBOSE_CREATION("Initializing data ", selector.GetOutput(),
                  " with ", Logger::Cyan, element, " (", index, ")");

               Verbs::Associate associator {Langulus::Move(element)};
               if (Verb::GenericExecuteIn(selector.GetOutput(), associator)) {
                  // Data was found and was overwritten                 
                  if (sati)
                     ++satisfiedData.GetValue(sati);
                  else
                     satisfiedData.Insert(meta, Count {1});

                  VERBOSE_CREATION(Logger::Yellow,
                     "Initialized ", selector.GetOutput(), " (", index, ")");
               }
               else {
                  // Can't set the member                               
                  LANGULUS_THROW(Construct, "Couldn't initialize member ");
               }
            }
            else {
               // Failure occurs for the given argument                 
               // It may be because of excess arguments, so check if    
               // the constructed type is fully satisfied at this point 
               if (not sati or satisfiedData.GetValue(sati) != context.GetType()->GetMemberCount(nullptr, meta)) {
                  // The context wasn't satisfied                       
                  LANGULUS_THROW(Construct, "Excess, or insufficient arguments");
               }
            }
         }
      });
   }

} // namespace Langulus::Verbs

#undef VERBOSE_CREATION
#undef ERROR_CREATION