///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include <Langulus/Flow/Missing.hpp>
#include <Langulus/Flow/Redundant.hpp>

#include <Langulus/Except.hpp>
#include <Langulus/TTag.hpp>
#include <Langulus/Neat.hpp>
#include <Langulus/Recipe.hpp>
#include <Langulus/SerializeText.hpp>

#include <Langulus/Verbs/Do.hpp>
#include <Langulus/Verbs/Interpret.hpp>
#include <Langulus/Verbs/Create.hpp>

#include <Langulus/Annies/Functional/ForEach.hpp>
#include <Langulus/Annies/Functional/Compare.hpp>
#include <Langulus/Annies/Functional/Insert.hpp>
#include <Langulus/Annies/Functional/Deepen.hpp>

#define LglsVerboseEnabled 1
#include <Langulus/Logger/ToggleVerbose.hpp>

/*#if 0
   #define VERBOSE(...)      Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_TAB(...)  const auto tab = Logger::VerboseTab(__VA_ARGS__)
#else
   #define VERBOSE(...)      LANGULUS(NOOP)
   #define VERBOSE_TAB(...)  LANGULUS(NOOP)
#endif

#define FLOW_ERRORS(...)  Logger::Error(__VA_ARGS__)*/

using namespace Langulus;

namespace
{
/// Nested AND scope execution                                                
bool ExecuteAND(
   Many const& flow, Many const& environment, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   LglsAssumeDev(not flow.IsSparse(), "Can't execute sparse flows");
   size_t executed = 0;
   if (flow.IsDeep()) {
      executed = flow.ForEach([&](Many const& block) {
         // Nest if deep                                                
         Many local;
         if (not Flow::Execute(block, environment, local, integrate, skipVerbs, silent)) {
            if (silent)
               throw Exception("Deep AND failure");
            else
               LglsError("Deep AND failure: ", flow);
         }

         output.Compose(Abandon(local));
      });
   }
   else {
      executed = flow.ForEach(
         [&](const Flow::Missing& missing) {
            // Nest if missing points                                   
            Many local;
            if (not Flow::Execute(missing.mContent, environment, local, integrate, skipVerbs, silent)) {
               if (silent)
                  throw Exception("Missing point failure");
               else
                  LglsError("Missing point failure: ", flow);
            }

            output.Compose(Abandon(local));
         },
         [&](Tag const& tag) {
            // Nest if tags, but retain each tag                    
            if (tag.IsMissing()) {
               // Never touch missing stuff, only propagate it          
               output.Compose(tag);
               return;
            }

            Many local;
            if (not Flow::Execute(tag, environment, local, integrate, skipVerbs, silent)) {
               if (silent)
                  throw Exception("Tag AND failure");
               else
                  LglsError("Tag AND failure: ", flow);
            }

            output.Compose(Tag::From(tag, Abandon(local)));
         },
         [&](Recipe const& recipe) {
            // Nest if recipes, but retain each recipe                  
            LglsVerbose(Verbose, "Executing recipe: ", recipe);

            Many local;
            if (not Flow::Execute(recipe.GetDescriptor(), environment, local, integrate, skipVerbs, silent)) {
               if (silent)
                  throw Exception("Recipe AND failure");
               else
                  LglsError("Recipe AND failure: ", flow);
            }

            auto solved = Recipe::From(recipe, Abandon(local));

            // We can attempt an implicit Verbs::Create to make         
            // the data at compile-time. Allowed only if no producer    
            // was specified and if recipe is not flow-dependent.    
            if (not recipe.GetTarget().GetProducer()) {
               Verbs::Create creator {&solved};
               if (creator.RunUnary()) {
                  output.Compose(Abandon(creator.GetOutput()));
                  return;
               }
            }
            
            // Otherwise just propagate                                 
            output.Compose(Abandon(solved));
         },
         [&](Neat const& neat) {
            (void)neat;
            // And order-independent container                       
            // Make a shallow copy of the Neat, and strip all        
            // verbs from it. Some of them might get reinserted, if  
            // missing, but generally they will be substituted with  
            // the corresponding results                             
            LglsVerbose(Verbose, "Executing neat: ", neat);
            /*Neat local = neat;
            local.template RemoveData<Verb>();
            LglsVerbose(Verbose, "Executing neat (verbs stripped): ", local);

            local.ForEach(
               [&](Verb const& constVerb) {
                  if (constVerb.IsMissing()) {
                     // Never touch missing stuff, only propagate it 
                     local << constVerb;
                     return;
                  }

                  // Execute all verbs, push their outputs to the    
                  // local shallow-copied recipe                  
                  auto verb = Verb::FromMeta(
                     constVerb.GetVerb(),
                     constVerb.GetRHS(),
                     constVerb,
                     constVerb.GetVerbState()
                  );
                  verb.SetSource(constVerb.GetLHS());

                  if (not ExecuteVerb(context, verb, silent)) {
                     if (silent)
                        throw Exception(Flow, "Recipe AND failure");
                     else
                        LANGULUS_OOPS(Flow, "Recipe AND failure: ");
                  }
                  else if (verb.GetOutput())
                     local << Abandon(verb.GetOutput());
               }
            );

            VERBOSE("Executing neat (verbs executed): ", local);
            output.SmartPush(IndexBack, Abandon(local));*/
            TODO();
         },
         [&](Verb const& constVerb) {
            // Execute verbs                                         
            if (skipVerbs)
               return Loop::Break;

            if (constVerb.IsDone()) {
               // Verb has already been executed                     
               // Don't do anything                                  
               return Loop::Continue;
            }

            // Shallow-copy the verb to make it mutable              
            // Also resets its output                                
            auto verb = Verb::Like(constVerb).In(constVerb.GetLHS());
            if (verb.IsMissing()) {
               if (integrate) {
                  output.Compose(verb);
                  return Loop::Continue;
               }
               else Logger::Error("Trying to execute a missing verb: ", verb);
            }

            // Execute the verb                                      
            if (not Flow::ExecuteVerb(environment, verb, silent)) {
               if (silent)
                  throw Exception("Verb AND failure");
               else
                  LglsError("Verb AND failure: ", verb);
            }

            // Make sure the original verb has been marked done, so  
            // that it isn't executed every time.                    
            const_cast<Verb&>(constVerb).Done();
            output.Compose(Abandon(verb.GetOutput()));
            return Loop::Continue;
         }
      );
   }

   if (not executed and integrate) {
      // If this is reached, then we had non-verb content               
      // Just propagate its contents                                    
      output.Compose(flow);
   }

   LglsVerbose(Verbose, Logger::Green, "AND scope done: ", flow);
   return true;
}

/// Nested OR execution                                                       
bool ExecuteOR(
   Many const& flow, Many const& environment, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   LglsAssumeDev(not flow.IsSparse(), "Can't execute sparse flows");
   size_t executed = 0;
   bool localSkipVerbs = false;

   if (flow.IsDeep()) {
      executed = flow.ForEach([&](Many const& block) {
         // Nest if deep                                             
         Many local;
         if (Flow::Execute(block, environment, local, integrate, localSkipVerbs, silent)) {
            executed = true;
            output.Compose(Abandon(local));
         }
      });
   }
   else {
      executed = flow.ForEach(
         [&](Tag const& tag) {
            // Nest if tags, but retain each tag                 
            if (tag.IsMissing()) {
               // Never touch missing stuff, only propagate it       
               output.Compose(tag);
               return;
            }

            Many local;
            bool unusedSkipVerbs = false;
            if (Flow::Execute(tag.GetData(), environment, local, integrate, unusedSkipVerbs, silent)) {
               executed = true;
               output.Compose(Tag::From(tag, Abandon(local)));
            }
         },
         [&](Recipe const& recipe) {
            // Nest if recipes, but retain each recipe         
            Many local;
            if (Flow::Execute(recipe.GetDescriptor(), environment, local, integrate, skipVerbs, silent)) {
               executed = true;
               auto solved = Recipe::From(recipe, Abandon(local));

               // We can attempt an implicit Verbs::Create to make   
               // the data at compile-time. Allowed only if no       
               // producer was specified.                            
               if (not recipe.GetTarget().GetProducer() /*and not recipe.GetCharge().IsFlowDependent()*/) {
                  Verbs::Create creator {&solved};
                  if (creator.RunUnary()) {
                     output.Compose(Abandon(creator.GetOutput()));
                     return;
                  }
               }
            
               // Otherwise just propagate                           
               output.Compose(Abandon(solved));
            }
         },
         [&](Neat const& neat) {
            (void)neat;

            // Make a shallow copy of the neat, and strip all        
            // verbs from it. Some of them might get reinserted, if  
            // missing, but generally they will be substituted with  
            // the corresponding results                             
            /*Neat local = neat;
            local.template RemoveData<Verb>();

            local.ForEach(
               [&](Verb const& constVerb) noexcept {
                  if (constVerb.IsMissing()) {
                     // Never touch missing stuff, only propagate it 
                     local << constVerb;
                     return;
                  }

                  // Execute all verbs, push their outputs to the    
                  // local shallow-copied recipe                  
                  auto verb = Verb::FromMeta(
                     constVerb.GetVerb(),
                     constVerb.GetRHS(),
                     constVerb,
                     constVerb.GetVerbState()
                  );
                  verb.SetSource(constVerb.GetLHS());

                  if (ExecuteVerb(context, verb, silent))
                     executed = true;
               }
            );

            output.SmartPush(IndexBack, Abandon(local));*/
            TODO();
         },
         [&](Verb const& constVerb) {
            // Execute verbs                                            
            if (localSkipVerbs)
               return Loop::Break;

            // Shallow-copy the verb to make it mutable                 
            // Also resets its output                                   
            auto verb = Verb::Like(constVerb);
            if (verb.IsMissing()) {
               if (integrate) {
                  output.Compose(verb);
                  return Loop::Continue;
               }
               else Logger::Error("Trying to execute a missing verb: ", verb);
            }

            if (not Flow::ExecuteVerb(environment, verb, silent))
               return Loop::Continue;

            executed = true;
            output.Compose(Abandon(verb.GetOutput()));
            return Loop::Continue;
         }
      );
   }

   skipVerbs |= localSkipVerbs;

   if (not executed and integrate) {
      // If this is reached, then we have non-verb flat content         
      // Just propagate it                                              
      output.Compose(flow);
      ++executed;
   }

   if (executed)
      LglsVerbose(Verbose, Logger::Green, "OR scope done: ",   flow);
   else
      LglsVerbose(Verbose, Logger::Red,   "OR scope failed: ", flow);
   return executed;
}
}

/// Nested AND/OR flow execution                                              
///   @param flow the flow to execute                                         
///   @param environment the environment in which scope will be executed.     
///      Note: each verb in the flow can have its own context, but the        
///      environment is the overarching context, which is used to disambiguate
///      and link missing points inside the flow.                             
///   @param output [out] verb execution results and propagated data will be  
///      pushed here                                                          
///   @param integrate execution happens in two stages:                       
///      1. integration: everything not executed will still be pushed to      
///         output, preserving the hierarchy. Useful when integrating verbs.  
///      2. execution: only unexecuted verbs will push to output, especially  
///         useful for collecting side-effects when updating.                 
///   @param skipVerbs [in/out] whether to skip executing verbs in other      
///      branches after an OR-success of a short-circuited verb.              
///   @param silent whether or not to silence logging, in case we're          
///      executing at compile-time, for example.                              
///   @return true of no errors occured                                       
bool Flow::Execute(
   Many const& flow, Many const& environment, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   auto results = Many::CopyStates(flow);
   if (flow) {
      if (flow.IsSparse()) {
         // Sparse contents are always simply forwarded (if integrating)
         // and never executed.                                         
         if (integrate) {
            results.Compose(flow);
            LglsVerbose(Verbose, Logger::Green, "AND scope forwarded: {", flow, "}");
         }
      }
      else {
         // Dense flows will be integrated and/or executed              
         LglsVerboseSection(Verbose, "Executing scope");
         if (integrate)
            LglsVerboseAppend(" (integrating): {", flow, '}');
         else
            LglsVerboseAppend(": {", flow, '}');
         
         try {
            if (flow.IsOr())
               ExecuteOR(flow, environment, results, integrate, skipVerbs, silent);
            else
               ExecuteAND(flow, environment, results, integrate, skipVerbs, silent);
         }
         catch (...) {
            // Execution failed                                         
            return false;
         }
      }
   }

   output.Compose(Abandon(results));
   return true;
}

/// Integrate all parts of a verb inside this environment                     
///   @param context [in/out] the context for integration                     
///   @param verb [in/out] verb to integrate                                  
///   @param silent whether or not to silence logging, in case we're          
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
bool Flow::IntegrateVerb(Many const& environment, Verb& verb, const bool silent) {
   // Integrate the verb source to environment                          
   Many localSource;
   if (not verb.GetLHS().Is<Redundant>()) {
      bool unusedSkipVerbs = false;
      if (not Flow::Execute(verb.GetLHS(), environment, localSource, true, unusedSkipVerbs, silent)) {
         if (not silent)
            Logger::Error("Error at source of: ", verb);
         return false;
      }
   }
   else localSource = verb.GetLHS().As<Redundant>().mContent;

   if (not localSource.IsValid())
      localSource = environment;

   // Integrate the verb argument to the source                         
   const Many flow = verb.GetRHS();
   Many localArgument;
   bool unusedSkipVerbs = false;
   if (not Flow::Execute(flow, localSource, localArgument, true, unusedSkipVerbs, silent)) {
      if (not silent)
         Logger::Error("Error at argument of: ", verb);
      return false;
   }

   verb.SetArgument(Abandon(localArgument)).In(Abandon(localSource));
   return true;
}

/// Execute a single verb, and all subverbs in it, if any.                    
/// Ideally, this function is the last thing that gets called, after          
/// dispatching in all nested context, and after executing all nested branches
/// of a flow. Verb context should be a single flat element.                  
///   @param context [in/out] the context in which verb will be executed      
///   @param verb [in/out] verb to execute                                    
///   @param silent whether or not to silence logging, in case we're          
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
bool Flow::ExecuteVerb(Many const& environment, Verb& verb, const bool silent) {
   // Integration (and execution of subverbs if any).                   
   // Source and argument will be executed locally if scripts, and      
   // substituted with their results in the verb.                       
   if (not Flow::IntegrateVerb(environment, verb, silent)) {
      if (not silent) {
         Logger::Error("Error integrating verb: ", verb, " (a.k.a. ", verb.GetVerb().GetPositiveName(), ')');
      }
      return false;
   }

   if (verb.IsVerb<Verbs::Do>()) {
      // A Do verb is done at this point, because the subverbs          
      // inside (if any) should be done in the integration phase. This  
      // also acts as regression protection. All we need to make sure   
      // is that the integrated argument (or source) are propagated to  
      // the verb's output.                                             
      if (not verb.GetOutput()) {
         if (verb)   verb << verb.GetRHS();
         else        verb << verb.GetLHS();
      }

      return true;
   }

   LglsVerboseSection(Verbose, "Executing verb: ", Logger::Cyan, verb,
      " (a.k.a. ", verb.GetVerb().GetPositiveName(), ')');

   // Dispatch the verb to the context, executing it.                   
   // Any results should end up inside verb.mOutput afterwards.         
   auto& abilities = verb.GetLHS().GetType().GetVerbs();
   auto found_verb = abilities.find(verb.GetVerb().GetDefinition());
   if (found_verb != abilities.end() and found_verb->second.binary) {
      if (not found_verb->second.binary(verb.GetLHS().GetRawVoid(), verb)) {
         if (not silent) {
            Logger::Error("Error executing verb: ", verb, 
               " (a.k.a. ", verb.GetVerb().GetPositiveName(), ')');
         }
         return false;
      }
   }
   else {
      if (not silent) {
         Logger::Error("No ability to ", Logger::PushYellow, 
            verb.GetVerb().GetPositiveName(), Logger::Pop,
            " found for ", Logger::PushYellow, verb.GetLHS().GetType()
         );
      }
      return false;
   }

   //TODO unary

   LglsVerbose(Verbose, "Executed: ", Logger::Green, verb,
      " (a.k.a. ", verb.GetVerb().GetPositiveName(), ')');
   return true;
}