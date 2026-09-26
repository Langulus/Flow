///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/Flow/Export.hpp>
#include <Langulus/Verbs/Do.hpp>


namespace Langulus::Flow
{
   ///                                                                        
   /// Tools for executing containers as flows                                
   ///                                                                        
   LANGULUS_API(FLOW)
   bool Execute(const Many&, Many&, Many& output, bool integration, bool& skipVerbs, bool silent = false);

   LANGULUS_API(FLOW)
   bool ExecuteAND(const Many&, Many&, Many& output, bool integration, bool& skipVerbs, bool silent = false);
   LANGULUS_API(FLOW)
   bool ExecuteOR(const Many&, Many&, Many& output, bool integration, bool& skipVerbs, bool silent = false);

   LANGULUS_API(FLOW)
   bool ExecuteVerb(Many&, Verb&, bool silent = false);
   LANGULUS_API(FLOW)
   bool IntegrateVerb(Many&, Verb&, bool silent = false);
}

namespace Langulus::Annies
{
   /// Execute the verb.                                                      
   /// Implemented in Langulus::Flow, as it requires some prime verbs         
   /// being defined, such as Verbs::Do                                       
   template<class V>
   bool TVerb<V>::Run() const {
      if (not mContext) {
         // Context is empty and doesn't have any relevant states,      
         // and execution happens only in stateless mode by using       
         // verb argument as the context. This sometimes happens with   
         // unary operators, like -5. Since 5 is a number, stateless    
         // subtraction on numbers will be sought and executed.         
         // Another example is selecting global objects, like the       
         // logger, by using `.logger`                                  
         return RunStateless();
      }

      mOutput = Many::CopyStates(mContext);

      // What kind of data does 'mContext' hold? Is it capable of       
      // dispatching? If so, do that and ignore anything else.          
      // This is where deep containers get nested before executing      
      // any verbs. The verb's argument will get eventually run in      
      // a context that cares about it.                                 
      auto resolver = mContext.GetType().GetResolver();
      if (not resolver) {
         // No resolver is available, which means no dynamic_casts,     
         // so all types are just as they appear, and we can check      
         // all of them for abilities once.                             
         auto& abilities = mContext.GetType().GetVerbs();
         auto found_dispatcher = abilities.find(MetaVerbOf<Verbs::Do>().GetDefinition());
         if (found_dispatcher != abilities.end()) {
            // Custom reflected dispatcher is available.                
            // It's your responsibility to implement it adequately.     
            // Keep in mind, that once you declare a custom Do for      
            // your type, you no longer rely on reflected bases'        
            // verbs or default verbs. You must invoke those by         
            // yourself in your dispatcher - the custom dispatcher      
            // provides full control!                                   
            auto dispatch = IsVerb<Verbs::Do>() ? Verbs::Do::Like(*this) : Verbs::Do(*this);
            size_t successCount = 0;
            for (auto handle : mContext) {
               if (found_dispatcher(handle.GetRaw(), dispatch)) {
                  mOutput.Compose(Move(dispatch.GetOutput()));
                  ++successCount;
                  dispatch.Clear();
               }
            }
            return successCount > 0;
         }

         //                                                             
         // If reached, then contained type has no dispatcher. Time     
         // to run the verb's argument inside whatever context there    
         // is. Thing is, the argument might contain a whole            
         // hierarchy of verbs, and we must preserve that.              
         auto const& flow = GetArgument();
         size_t successCount = 0;
         for (auto handle : mContext) {
            if (Execute(flow, mContext, mOutput/*, integrate, skipVerbs, silent*/))
               ++successCount;
         }
         return successCount > 0;
      }
      else {
         // Type is resolvable - probably abstract pointer. This        
         // means that each element must be resolved to its concrete    
         // type using dynamic_cast, and might end up possessing        
         // completely different abilities.                             
         size_t successCount = 0;
         for (auto handle : mContext) {
            auto resolved = handle.GetResolved();
            auto& abilities = resolved.GetType().GetVerbs();
            auto found_dispatcher = abilities.find(MetaVerbOf<Verbs::Do>().GetDefinition());
            if (found_dispatcher != abilities.end()) {
               // Custom reflected dispatcher is available              
               auto dispatch = IsVerb<Verbs::Do>() ? Verbs::Do::Like(*this) : Verbs::Do(*this);
               if (found_dispatcher(resolved.GetRaw(), dispatch)) {
                  mOutput.Compose(Move(dispatch.GetOutput()));
                  ++successCount;
               }
            }
            else {
               // No dispatcher for that particular resolved element    
               if (Execute(GetArgument(), resolved, mOutput/*, integrate, skipVerbs, silent*/))
                  ++successCount;
            }
         }
         return successCount > 0;
      }
   }
}