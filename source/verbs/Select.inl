///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Select.hpp"
#include "../TVerb.inl"

#if 0
   #define VERBOSE_SELECT(...)      Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_SELECT_TAB(...)  const auto tab = Logger::VerboseTab(__VA_ARGS__)
#else
   #define VERBOSE_SELECT(...)      LANGULUS(NOOP)
   #define VERBOSE_SELECT_TAB(...)  LANGULUS(NOOP)
#endif


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool Select::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Select(v); };
      else
         return requires (T& t, Verb& v, A...a) { t.Select(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Select::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Select(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Select(verb, args...);
         };
      }
   }

   /// Execute the selection verb in a specific context                       
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   template<CT::Dense T>
   bool Select::ExecuteIn(T& context, Verb& verb) {
      static_assert(Select::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Select(verb);
      return verb.IsDone();
   }

   /// Stateless selection, for selecting some global entities, like the      
   /// logger, for example                                                    
   ///   @param verb - selection verb                                         
   ///   @return true if verb has been satisfied                              
   inline bool Select::ExecuteStateless(Verb& verb) {
      verb.ForEachDeep([&](TMeta t) {
         if (t->Is<Traits::Logger>())
            verb << &Logger::Instance;
      });
      return verb.IsDone();
   }

   /// Execute the default verb in an immutable context                       
   /// Returns immutable results                                              
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Select::ExecuteDefault(const Many& context, Verb& verb) {
      return DefaultSelect<false>(const_cast<Many&>(context), verb);
   }

   /// Execute the default verb in a mutable context                          
   /// Returns mutable results                                                
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   inline bool Select::ExecuteDefault(Many& context, Verb& verb) {
      return DefaultSelect<true>(context, verb);
   }

   /// Default selection logic for members and abilities                      
   ///   @tparam MUTABLE - whether or not selection is constant               
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb instance to execute                           
   ///   @return true if execution was a success                              
   template<bool MUTABLE>
   bool Select::DefaultSelect(Many& context, Verb& verb) {
      VERBOSE_SELECT_TAB("Default select: ", verb);
      if (verb.IsMissing() or context.IsMissing()) {
         VERBOSE_SELECT("Can't select using missing argument/context");
         return false;
      }
      else if (not context) {
         VERBOSE_SELECT("Can't select in empty context");
         return false;
      }

      TMany<Index> indices;
      indices.GatherFrom(verb.GetArgument());
      bool containsOnlyIndices = not indices.IsEmpty();

      TMany<Trait> selectedTraits;
      TMany<const RTTI::Ability*> selectedAbilities;

      // Scan verb argument for anything but indices                    
      verb.ForEachDeep([&](const Many& group) {
         // Skip indices - they were gathered before the loop           
         if (group.Is<Index>())
            return;

         group.ForEach(
            [&](const Construct& construct) {
               VERBOSE_SELECT("Selecting construct: ", construct.GetDescriptor());
               containsOnlyIndices = false;
               auto nested = verb.Fork(construct.GetDescriptor());
               ExecuteDefault(context, nested);
               verb << Abandon(nested.GetOutput());
            },
            [&](const Trait& trait) {
               VERBOSE_SELECT("Selecting trait: ", trait);
               containsOnlyIndices = false;
               auto tmeta = trait.GetTrait();
               if (tmeta)
                  PerIndex<MUTABLE>(context, selectedTraits, tmeta, tmeta, indices);
               else
                  PerIndex<MUTABLE>(context, selectedTraits, tmeta, trait.GetType(), indices);
            },
            [&](TMeta tmeta) {
               VERBOSE_SELECT("Selecting trait: ", tmeta);
               containsOnlyIndices = false;
               PerIndex<MUTABLE>(context, selectedTraits, tmeta, tmeta, indices);
            },
            [&](DMeta dmeta) {
               VERBOSE_SELECT("Selecting data: ", dmeta);
               containsOnlyIndices = false;
               SelectByMeta<MUTABLE>(indices, dmeta, context, selectedTraits, selectedAbilities);
            }
         );
      });

      if (containsOnlyIndices) {
         // Try selecting via indices only                              
         // This is allowed only if no metas were found in the argument 
         VERBOSE_SELECT("Selecting via indices only: ", indices);
         PerIndex<MUTABLE>(context, selectedTraits, TMeta {}, DMeta {}, indices);
      }

      // Output results if any, satisfying the verb                     
      for (auto& trait : selectedTraits)
         verb << static_cast<Many&>(trait);

      verb << selectedAbilities;

      if (verb.IsDone())
         VERBOSE_SELECT(Logger::Green, "Selected: ", verb.GetOutput());
      else
         VERBOSE_SELECT(Logger::Red, "Nothing was selected");
      return verb.IsDone();
   }

   /// Select members by providing either meta data or meta trait             
   ///   @tparam MUTABLE - whether or not selection will be mutable           
   ///   @param context - the thing we're searching in                        
   ///   @param selectedTraits - [out] found traits go here                   
   ///   @param resultingTrait - the type of trait to push to selectedTraits  
   ///   @param meta - the filter we'll be using                              
   ///   @param indices - optional indices (i.e. Nth trait of a kind          
   ///   @return true if at least one trait has been pushed to selectedTraits 
   template<bool MUTABLE>
   bool Select::PerIndex(
      Many& context,
      TMany<Trait>& selectedTraits,
      TMeta resultingTrait,
      CT::Meta auto meta,
      const TMany<Index>& indices
   ) {
      using META = decltype(meta);
      bool done = false;
      static const Index fallbacki = 0;
      const Index* i = indices ? indices.GetRaw() : &fallbacki;

      do {
         // Search for each meta-index pair                             
         const RTTI::Member* member {};
         if constexpr (CT::Similar<META, DMeta>)
            member = context.GetType()->GetMember(nullptr, meta, i->GetOffset());
         else if constexpr (CT::Similar<META, TMeta>)
            member = context.GetType()->GetMember(meta, nullptr, i->GetOffset());
         else
            static_assert(false, "Meta not supported for selecting members");

         if (member) {
            VERBOSE_SELECT(
               "Found member of type ", member->mTypeRetriever(),
               " at index ", i->GetOffset(),
               " while searching in type ", context.GetType()
            );

            Many variable = context.GetMember(*member, 0);
            if constexpr (not MUTABLE)
               variable.MakeConst();

            if (variable.IsAllocated()) {
               selectedTraits << Trait::From(resultingTrait, variable);
               done = true;
            }
         }

         if (indices)
            ++i;
      }
      while (i != indices.GetRawEnd() and i != &fallbacki);

      return done;
   }

   /// Select ability or member by a meta                                     
   ///   @param indices - optional index for trait/verb (Nth trait/verb)      
   ///   @param id - type of data we'll be selecting                          
   ///   @param context - the thing we're searching in                        
   ///   @param selectedTraits - [out] found traits go here                   
   ///   @param selectedVerbs - [out] found verb go here                      
   ///   @return if at least trait/verb has been pushed to outputs            
   template<bool MUTABLE>
   inline bool Select::SelectByMeta(
      const TMany<Index>& indices,
      DMeta id,
      Many& context,
      TMany<Trait>& selectedTraits,
      TMany<const RTTI::Ability*>& selectedVerbs
   ) {
      const auto type = context.GetType();
      if (id->Is<VMeta>()) {
         if (not indices or indices == IndexAll) {
            // Retrieve each ability corresponding to verbs in rhs      
            for (auto& ability : type->mAbilities)
               selectedVerbs << &ability.second;
         }
         else for (auto& idx : indices) {
            // Retrieve specified abilities by index                    
            Count counter = 0;
            for (auto& ability : type->mAbilities) {
               if (counter == idx.GetOffset()) {
                  selectedVerbs << &ability.second;
                  break;
               }
               ++counter;
            }
         }

         return true;
      }

      // Select data IDs                                                
      return PerIndex<MUTABLE>(context, selectedTraits, nullptr, id, indices);
   };

} // namespace Langulus::Verbs

#undef VERBOSE_SELECT