///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include <Langulus/TVerb.hpp>
#include <Langulus/TTag.hpp>
#include <Langulus/Flow/Export.hpp>
//#include "Langulus/CT/Executable.hpp"
//#include "Langulus/CT/Deep.hpp"


///                                                                           
/// MARK: Do/Undo                                                             
///   Serves as a level of indirection between context and verb.              
/// When added as an ability to a thing, that thing can now route all         
/// verbs through itself, before executing them (or not). It basically        
/// states that the entity has the ability to "do things differently".        
/// Useful for implementing dispatchers, debuggers, interpreters of entire    
/// flows, etc.                                                               
///   For example, in the game Mindmaze, the maze is essentially              
/// an interpreter that converts a script into a maze. Different verbs        
/// do completely different things in that context - they attach hallways,    
/// rooms, place stuff in the rooms, etc. All events that happen to a         
/// maze object go through the maze's Do ability and get interpreted          
/// accordingly to the local rules.                                           
///   @attention this verb goes through all branches without doing            
///      any short-circuiting.                                                
LANGULUS_DEFINE_VERB(Do, Undo, 0, 
   "An indirection between context and flow. "
   "Not short-circuited."
);


namespace Langulus::Flow
{
   ///                                                                        
   /// Tools for executing containers as flows                                
   ///                                                                        
   LANGULUS_API(FLOW)
   bool Execute(Many const&, Many const&, Many& output, bool integration, bool& skipVerbs, bool silent = false);
   LANGULUS_API(FLOW)
   bool ExecuteVerb(Many const&, Verb&, bool silent = false);
   LANGULUS_API(FLOW)
   bool IntegrateVerb(Many const&, Verb&, bool silent = false);
}

namespace Langulus::CTTI
{
   /// Perform any flow in a deep context by dispatching the argument to      
   /// each subcontainer, preserving hierarchy in the outputs. Example:       
   ///    {1 or 2 or 3} add 1 --> {{1 add 1} or {2 add 1} or {3 add 1}}       
   ///                         resulting in:                                  
   ///                         {2 or 3 or 4}                                  
   LglsImplementAbilitiesFor(Annies::Many) {
      using Can = Verbs::Do;

      static bool Default(Annies::Many const& lhs, Verbs::Do& verb) {
         auto const& flow = verb.GetArgument();
         auto& output = verb.GetOutput();
         output = Many::CopyStates(lhs);

         // What kind of data does 'mContext' hold? Is it capable of    
         // dispatching? If so, do that and ignore anything else.       
         // This is where deep containers get nested before executing   
         // any verbs. The verb's argument will get eventually run in   
         // a context that cares about it.                              
         auto resolver = lhs.GetType().GetResolver();
         if (not resolver) {
            // No resolver is available, which means no dynamic_casts,  
            // so all types are just as they appear, and we can check   
            // all of them for abilities once.                          
            auto& abilities = lhs.GetType().GetVerbs();
            auto found_dispatcher = abilities.find(MetaVerbOf<Verbs::Do>().GetDefinition());
            if (found_dispatcher != abilities.end()) {
               // Custom reflected dispatcher is available.             
               // It's your responsibility to implement it adequately.  
               // Keep in mind, that once you declare a custom Do for   
               // your type, you no longer rely on reflected bases'     
               // verbs or default verbs. You must invoke those by      
               // yourself in your dispatcher - the custom dispatcher   
               // provides full control!                                
               auto dispatch = Verbs::Do::Like(verb);
               size_t successCount = 0;
               for (auto handle : lhs) {
                  if (found_dispatcher->second(handle.GetRaw(), reinterpret_cast<Verb&>(dispatch))) {
                     output.Compose(Move(dispatch.GetOutput()));
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
            size_t successCount = 0;
            for (auto handle : lhs) {
               //TODO magic bools here. maybe carry state with verbs and use it as arguments here?
               bool unusedSkipVerbs = false;
               if (Flow::Execute(flow, lhs, output, false, unusedSkipVerbs, false))
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
            for (auto handle : lhs) {
               auto resolved = handle.GetResolved();
               auto& abilities = resolved.GetType().GetVerbs();
               auto found_dispatcher = abilities.find(MetaVerbOf<Verbs::Do>().GetDefinition());
               if (found_dispatcher != abilities.end()) {
                  // Custom reflected dispatcher is available           
                  auto dispatch = Verbs::Do::Like(verb);
                  if (found_dispatcher->second(resolved.GetRaw(), reinterpret_cast<Verb&>(dispatch))) {
                     output.Compose(Move(dispatch.GetOutput()));
                     ++successCount;
                  }
               }
               else {
                  // No dispatcher for that particular resolved element 
                  //TODO magic bools here. maybe carry state with verbs and use it as arguments here?
                  bool unusedSkipVerbs = false;
                  if (Flow::Execute(flow, resolved, output, false, unusedSkipVerbs, false))
                     ++successCount;
               }
            }
            return successCount > 0;
         }
      }
   };
   
   /// Execute a flow inside a tag, and preserve that tag. Example:           
   ///         tag(1 or 2) add 1 --> tag({1 add 1} or {2 add 1})              
   ///                         resulting in:                                  
   ///                          tag(2 or 3)                                   
   LglsImplementAbilitiesFor(Annies::Tag) {
      using Can = Verbs::Do;
      using Tag = Annies::Tag;

      static bool Default(Tag const& tag, Verbs::Do& verb) {
         Langulus::InvokeAbility(tag.GetData(), verb);
         verb.GetOutput() = Tag::From(tag, Move(verb.GetOutput()));
         return true;
      }
   };
}

namespace Langulus::Annies
{
   /// Execute a verb.                                                        
   /// Implemented in Langulus::Flow, as it requires some prime verbs         
   /// being defined, such as Verbs::Do.                                      
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

      // Dispatcher for Many should always exist                        
      if (IsVerb<Verbs::Do>())
         return Langulus::InvokeAbility(mContext, reinterpret_cast<Verbs::Do&>(*this));
      else
         return Langulus::InvokeAbility(mContext, Verbs::Do(*this));
   }
}