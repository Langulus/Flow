///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Do.hpp"
#include "../TVerb.inl"


namespace Langulus::Verbs
{

   /// Compile-time check if a verb is implemented in the provided type       
   ///   @return true if verb is available                                    
   template<CT::Dense T, CT::Data...A>
   constexpr bool Do::AvailableFor() noexcept {
      if constexpr (sizeof...(A) == 0)
         return requires (T& t, Verb& v) { t.Do(v); };
      else
         return requires (T& t, Verb& v, A...a) { t.Do(v, a...); };
   }

   /// Get the verb functor for the given type and arguments                  
   ///   @return the function, or nullptr if not available                    
   template<CT::Dense T, CT::Data...A>
   constexpr auto Do::Of() noexcept {
      if constexpr (CT::Constant<T>) {
         return [](const void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<const T*>(context);
            typedContext->Do(verb, args...);
         };
      }
      else {
         return [](void* context, Flow::Verb& verb, A...args) {
            auto typedContext = static_cast<T*>(context);
            typedContext->Do(verb, args...);
         };
      }
   }

   /// Execute the do/undo verb in a specific context                         
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb to execute                                    
   ///   @return true if verb has been satisfied                              
   bool Do::ExecuteIn(CT::Dense auto& context, Verb& verb) {
      using T = Deref<decltype(context)>;
      static_assert(Do::AvailableFor<T>(),
         "Verb is not available for this context, this shouldn't be reached by flow");
      context.Do(verb);
      return verb.IsDone();
   }

   /// Default do/undo in an immutable context                                
   ///   @param context - the block to execute in                             
   ///   @param verb - do/undo verb                                           
   inline bool Do::ExecuteDefault(const Many&, Verb&) {
      //TODO
      return true;
   }

   /// Default do/undo in a mutable context                                   
   ///   @param context - the block to execute in                             
   ///   @param verb - do/undo verb                                           
   inline bool Do::ExecuteDefault(Many&, Verb&) {
      //TODO
      return true;
   }

   /// Stateless execution                                                    
   ///   @param verb - the do/undo verb                                       
   ///   @return true if verb was satisfied                                   
   inline bool Do::ExecuteStateless(Verb& verb) {
      if (not verb)
         return false;

      //TODO execute
      return true;
   }

   /// Wrap anything in a Do verb, executing stuff in a specific context      
   ///   @param context - the context to execute in                           
   ///   @param verb - the verb/flow to execute                               
   ///   @return the Do verb                                                  
   inline Do Do::In(auto&& context, auto&& verb) {
      using CS = IntentOf<decltype(context)>;
      using VS = IntentOf<decltype(verb)>;
      Do v = VS::Nest(verb);
      v.SetSource(CS::Nest(context));
      return Abandon(v);
   }

} // namespace Langulus::Verbs


namespace Langulus::Flow
{

   /// Invoke a single verb on a single context                               
   ///   @tparam DISPATCH - whether or not to use context's dispatcher, if    
   ///      any is statically available or reflected                          
   ///   @tparam DEFAULT - whether or not to attempt default verb execution   
   ///      if such is statically available or reflected this is done only    
   ///      if direct or dispatched execution fails                           
   ///   @tparam FALLBACK - for internal use by the function - used to nest   
   ///      with default functionality, if DEFAULT is enabled                 
   ///   @param context - the context in which to execute in                  
   ///   @param verb - the verb to execute                                    
   ///   @return the number of successful executions                          
   template<bool DISPATCH, bool DEFAULT, bool FALLBACK>
   Count Execute(CT::Data auto& context, CT::VerbBased auto& verb) {
      using T = Deref<decltype(context)>;

      // Always reset verb progress prior to execution                  
      verb.Undo();

      if constexpr (not FALLBACK and DISPATCH and CT::Dispatcher<T>) {
         // Custom reflected dispatcher is available                    
         // It's your responsibility to implement it adequately         
         // Keep in mind, that once you declare a custom Do for your    
         // type, you no longer rely on reflected bases' verbs or       
         // default verbs. You must invoke those by yourself in your    
         // dispatcher - the custom dispatcher provides full control!   
         context.Do(verb);
      }
      else {
         if constexpr (FALLBACK) {
            // Execute the default verb                                 
            Verb::GenericExecuteDefault(context, verb);
         }
         else if constexpr (DISPATCH) {
            // Context might have a dispatcher                          
            // If that is the case, then it is the context's            
            // responsibility to dispatch the verb!                     
            if constexpr (CT::Deep<T>) {
               auto meta = context.GetType();
               if constexpr (CT::Constant<T>) {
                  if (meta->mDispatcherConstant)
                     meta->mDispatcherConstant(context.GetRaw(), verb);
                  else
                     Verb::GenericExecuteIn(context, verb);
               }
               else if (meta->mDispatcherConstant)
                  meta->mDispatcherConstant(context.GetRaw(), verb);
               else if (meta->mDispatcherMutable)
                  meta->mDispatcherMutable(context.GetRaw(), verb);
               else
                  Verb::GenericExecuteIn(context, verb);
            }
            else Verb::GenericExecuteIn(context, verb);
         }
         else Verb::GenericExecuteIn(context, verb);

         if (verb.IsDone())
            return verb.GetSuccesses();

         // If that fails, attempt executing the default verb           
         if constexpr (DEFAULT and not FALLBACK) {
            if (not verb.IsDone()) {
               // Verb wasn't executed neither in current element,      
               // nor in any of its bases, so we resort to the          
               // default abilities                                     
               Execute<false, false, true>(context, verb);
            }
         }
      }

      return verb.GetSuccesses();
   }

   /// Invoke a verb on a flat context of as much elements as you want        
   /// If an element is not able to execute verb, attempt calling the default 
   /// This should be called only in memory blocks that are flat              
   ///   @tparam RESOLVE - whether or not to perform runtime resolve of the   
   ///                     contexts, getting the most concrete type           
   ///   @tparam DISPATCH - whether or not to use custom dispatcher for       
   ///                      contexts, if any                                  
   ///   @tparam DEFAULT - whether or not to allow default/stateless verb     
   ///                     execution, if all else fails                       
   ///   @param context - the context in which to dispatch the verb           
   ///   @param verb - the verb to send over                                  
   ///   @return the number of successful executions                          
   template<bool RESOLVE, bool DISPATCH, bool DEFAULT>
   Count DispatchFlat(CT::Deep auto& context, CT::VerbBased auto& verb) {
      if (not context or verb.IsMonocast()) {
         if (context.IsInvalid()) {
            // Context is empty and doesn't have any relevant states,   
            // and execution happens only if DEFAULT verbs are allowed, 
            // as a stateless verb execution                            
            if constexpr (DEFAULT)
               return Verb::GenericExecuteStateless(verb);
            else
               return 0;
         }
         else {
            // Context is empty, but has relevant states, so directly   
            // forward it as context. Alternatively, the verb is not a  
            // multicast verb, and we're operating on context as one    
            //verb.SetSource(context);
            Execute<DISPATCH, DEFAULT, true>(context, verb);
            return verb.GetSuccesses();
         }
      }

      Count successCount = 0;
      auto output = Many::FromState(context);

      // Iterate elements in the current context                        
      for (Count i = 0; i < context.GetCount(); ++i) {
         //verb.SetSource(context.GetElement(i));
         auto ith = context.GetElement(i);
         if constexpr (RESOLVE)
            ith = ith.GetResolved();
         else
            ith = ith.GetDense();

         verb.SetSource(ith);
         Execute<DISPATCH, DEFAULT, false>(ith, verb);
         
         if (verb.IsDone()) {
            if (verb.GetOutput()) {
               // Cache output, conserving the context hierarchy        
               output.SmartPush(IndexBack, Langulus::Move(verb.GetOutput()));
            }

            ++successCount;
            verb.Undo();
         }
      }
      
      if (context.IsOr())
         return verb.template CompleteDispatch<true >(successCount, Abandon(output));
      else
         return verb.template CompleteDispatch<false>(successCount, Abandon(output));
   }

   /// Invoke a verb on a container, that is either deep or flat, either      
   /// AND, or OR. The verb will be executed for each flat element inside     
   /// this block. If a failure occurs inside a scope, that scope will be     
   /// considered failed, unless it's an OR scope - OR scopes stop execution  
   /// right after the first success and fail only if all branches fail       
   ///   @tparam RESOLVE - whether or not to perform runtime resolve of the   
   ///                     contexts, getting the most concrete type           
   ///   @tparam DISPATCH - whether or not to use custom dispatcher for       
   ///                      contexts, if any                                  
   ///   @tparam DEFAULT - whether or not to allow default/stateless verb     
   ///                     execution, if all else fails                       
   ///   @param context - the context in which scope will be dispatched to    
   ///   @param verb - the verb to execute                                    
   ///   @return the number of successful executions                          
   template<bool RESOLVE, bool DISPATCH, bool DEFAULT>
   Count DispatchDeep(CT::Deep auto& context, CT::VerbBased auto& verb) {
      if (not context or verb.IsMonocast()) {
         if (context.IsInvalid()) {
            // Context is empty and doesn't have any relevant states,   
            // and execution happens only if DEFAULT verbs are allowed, 
            // as a stateless verb execution                            
            if constexpr (DEFAULT)
               return Verb::GenericExecuteStateless(verb);
            else
               return 0;
         }
         else {
            // Context is empty, but has relevant states, so directly   
            // forward it as context. Alternatively, the verb is not a  
            // multicast verb, and we're operating on context as one    
            verb.SetSource(context);
            Execute<DISPATCH, DEFAULT, true>(context, verb);
            return verb.GetSuccesses();
         }
      }

      if (context.IsDeep()) {
         // Nest if context is deep                                     
         // There is no escape from this scope                          
         Count successCount = 0;
         auto output = Many::FromState(context);
         for (Count i = 0; i < context.GetCount(); ++i) {
            DispatchDeep<RESOLVE, DISPATCH, DEFAULT>(
               context.template Get<Many>(i), verb);

            if (verb.IsDone()) {
               if (verb.GetOutput()) {
                  // Cache output, conserving the context hierarchy     
                  output.SmartPush(IndexBack, Langulus::Move(verb.GetOutput()));
               }

               ++successCount;
               verb.Undo();
            }
         }

         if (context.IsOr())
            return verb.template CompleteDispatch<true >(successCount, Abandon(output));
         else
            return verb.template CompleteDispatch<false>(successCount, Abandon(output));
      }
      else if (context.template Is<Trait>()) {
         // Nest if context is trait                                    
         // Traits are considered deep only when executing in them      
         // There is no escape from this scope                          
         Count successCount = 0;
         auto output = Many::FromState(context);
         for (Count i = 0; i < context.GetCount(); ++i) {
            auto& t = context.template Get<Trait>(i);
            if constexpr (CT::Constant<decltype(context)>) {
               DispatchDeep<RESOLVE, DISPATCH, DEFAULT>(
                  static_cast<const Many&>(t), verb);
            }
            else {
               DispatchDeep<RESOLVE, DISPATCH, DEFAULT>(
                  static_cast<Many&>(t), verb);
            }

            if (verb.IsDone()) {
               if (verb.GetOutput()) {
                  // Cache output, conserving the context hierarchy     
                  output.SmartPush(IndexBack, Langulus::Move(verb.GetOutput()));
               }

               ++successCount;
               verb.Undo();
            }
         }

         if (context.IsOr())
            return verb.template CompleteDispatch<true >(successCount, Abandon(output));
         else
            return verb.template CompleteDispatch<false>(successCount, Abandon(output));
      }

      // If reached, then block is flat                                 
      // Execute implemented verbs if available, or fallback to         
      // default verbs, eventually                                      
      return DispatchFlat<RESOLVE, DISPATCH, DEFAULT>(context, verb);
   }

} // namespace Langulus::Flow
