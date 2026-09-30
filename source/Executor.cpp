///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "inner/Missing.hpp"
#include "inner/Redundant.hpp"

#include <Langulus/Verbs/Do.hpp>
#include <Langulus/Verbs/Interpret.hpp>
#include <Langulus/Verbs/Create.hpp>
#include "Langulus/Except.hpp"
#include "Langulus/TTag.hpp"
#include "Langulus/Neat.hpp"
#include "Langulus/Recipe.hpp"

#if 0
   #define VERBOSE(...)      Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_TAB(...)  const auto tab = Logger::VerboseTab(__VA_ARGS__)
#else
   #define VERBOSE(...)      LANGULUS(NOOP)
   #define VERBOSE_TAB(...)  LANGULUS(NOOP)
#endif

#define FLOW_ERRORS(...)  Logger::Error(__VA_ARGS__)

using namespace Langulus;

namespace
{
   bool ExecuteAND(Many const&, Many const&, Many&, bool, bool&, bool = false);
   bool ExecuteOR (Many const&, Many const&, Many&, bool, bool&, bool = false);
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
            VERBOSE(Logger::Green, "AND scope forwarded: {", flow, "}");
         }
      }
      else {
         // Dense flows will be integrated and/or executed              
         if (integrate)
            VERBOSE_TAB("Executing scope (integrating): [", flow, ']');
         else
            VERBOSE_TAB("Executing scope: [", flow, ']');
         
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

/// Nested AND scope execution                                                
bool ExecuteAND(
   Many const& flow, Many const& environment, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   LglsAssumeDev(not flow.IsSparse(), "Can't execute sparse flows");
   size_t executed = 0;
   if (flow.IsDeep()) {
      executed = flow.ForEach([&](const Many& block) {
         // Nest if deep                                                
         Many local;
         if (not Flow::Execute(block, environment, local, integrate, skipVerbs, silent)) {
            if (silent)
               throw Exception("Deep AND failure");
            else
               LglsError("Deep AND failure: "/*, flow*/);
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
                  LglsError("Missing point failure: "/*, flow*/);
            }

            output.Compose(Abandon(local));
         },
         [&](const Tag& tag) {
            // Nest if traits, but retain each trait                    
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
                  LglsError("Tag AND failure: "/*, flow*/);
            }

            output.Compose(Tag::From(tag, Abandon(local)));
         },
         [&](const Recipe& recipe) {
            // Nest if recipes, but retain each recipe                  
            VERBOSE("Executing recipe: ", recipe);

            Many local;
            if (not Flow::Execute(recipe.GetDescriptor(), environment, local, integrate, skipVerbs, silent)) {
               if (silent)
                  throw Exception("Construct AND failure");
               else
                  LglsError("Construct AND failure: "/*, flow*/);
            }

            auto solved = Recipe::From(recipe, Abandon(local));

            // We can attempt an implicit Verbs::Create to make         
            // the data at compile-time. Allowed only if no producer    
            // was specified and if construct is not flow-dependent.    
            if (not recipe.GetTarget().GetProducer()) {
               Verbs::Create creator {&solved};
               if (creator.RunStateless()) {
                  output.Compose(Abandon(creator.GetOutput()));
                  return;
               }
            }
            
            // Otherwise just propagate                                 
            output.Compose(Abandon(solved));
         },
         [&](const Neat& neat) {
            (void)neat;
            // And order-independent container                       
            // Make a shallow copy of the Neat, and strip all        
            // verbs from it. Some of them might get reinserted, if  
            // missing, but generally they will be substituted with  
            // the corresponding results                             
            VERBOSE("Executing neat: ", neat);
            /*Neat local = neat;
            local.template RemoveData<A::Verb>();
            VERBOSE("Executing neat (verbs stripped): ", local);

            local.ForEach(
               [&](const A::Verb& constVerb) {
                  if (constVerb.IsMissing()) {
                     // Never touch missing stuff, only propagate it 
                     local << constVerb;
                     return;
                  }

                  // Execute all verbs, push their outputs to the    
                  // local shallow-copied construct                  
                  auto verb = Verb::FromMeta(
                     constVerb.GetVerb(),
                     constVerb.GetArgument(),
                     constVerb,
                     constVerb.GetVerbState()
                  );
                  verb.SetSource(constVerb.GetSource());

                  if (not ExecuteVerb(context, verb, silent)) {
                     if (silent)
                        LANGULUS_THROW(Flow, "Construct AND failure");
                     else
                        LANGULUS_OOPS(Flow, "Construct AND failure: ");
                  }
                  else if (verb.GetOutput())
                     local << Abandon(verb.GetOutput());
               }
            );

            VERBOSE("Executing neat (verbs executed): ", local);
            output.SmartPush(IndexBack, Abandon(local));*/
            TODO();
         },
         [&](const Verb& constVerb) {
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
            auto verb = Verb::Like(constVerb).In(constVerb.GetSource());
            if (verb.IsMissing()) {
               if (integrate) {
                  output.Compose(verb);
                  return Loop::Continue;
               }
               else FLOW_ERRORS("Trying to execute a missing verb: ", verb);
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

   VERBOSE(Logger::Green, "AND scope done: "/*, flow*/);
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
      executed = flow.ForEach([&](const Many& block) {
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
         [&](const Tag& tag) {
            // Nest if traits, but retain each trait                 
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
         [&](const Recipe& recipe) {
            // Nest if constructs, but retain each construct         
            Many local;
            if (Flow::Execute(recipe.GetDescriptor(), environment, local, integrate, skipVerbs, silent)) {
               executed = true;
               auto solved = Recipe::From(recipe, Abandon(local));

               // We can attempt an implicit Verbs::Create to make   
               // the data at compile-time. Allowed only if no       
               // producer was specified.                            
               if (not recipe.GetTarget().GetProducer() /*and not construct.GetCharge().IsFlowDependent()*/) {
                  Verbs::Create creator {&solved};
                  if (creator.RunStateless()) {
                     output.Compose(Abandon(creator.GetOutput()));
                     return;
                  }
               }
            
               // Otherwise just propagate                           
               output.Compose(Abandon(solved));
            }
         },
         [&](const Neat& neat) {
            (void)neat;

            // Make a shallow copy of the neat, and strip all        
            // verbs from it. Some of them might get reinserted, if  
            // missing, but generally they will be substituted with  
            // the corresponding results                             
            /*Neat local = neat;
            local.template RemoveData<A::Verb>();

            local.ForEach(
               [&](const A::Verb& constVerb) noexcept {
                  if (constVerb.IsMissing()) {
                     // Never touch missing stuff, only propagate it 
                     local << constVerb;
                     return;
                  }

                  // Execute all verbs, push their outputs to the    
                  // local shallow-copied construct                  
                  auto verb = Verb::FromMeta(
                     constVerb.GetVerb(),
                     constVerb.GetArgument(),
                     constVerb,
                     constVerb.GetVerbState()
                  );
                  verb.SetSource(constVerb.GetSource());

                  if (ExecuteVerb(context, verb, silent))
                     executed = true;
               }
            );

            output.SmartPush(IndexBack, Abandon(local));*/
            TODO();
         },
         [&](const Verb& constVerb) {
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
               else FLOW_ERRORS("Trying to execute a missing verb: ", verb);
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

   if (executed) VERBOSE(Logger::Green, "OR scope done: ",   flow);
   else          VERBOSE(Logger::Red,   "OR scope failed: ", flow);
   return executed;
}

/// Integrate all parts of a verb inside this environment                     
///   @param context - [in/out] the context for integration                   
///   @param verb - [in/out] verb to integrate                                
///   @param silent - whether or not to silence logging, in case we're        
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
bool Flow::IntegrateVerb(Many const& environment, Verb& verb, const bool silent) {
   // Integrate the verb source to environment                          
   Many localSource;
   if (not verb.GetSource().Is<Redundant>()) {
      bool unusedSkipVerbs = false;
      if (not Flow::Execute(verb.GetSource(), environment, localSource, true, unusedSkipVerbs, silent)) {
         if (not silent)
            FLOW_ERRORS("Error at source of: ", verb);
         return false;
      }
   }
   else localSource = verb.GetSource().Get<Redundant>().mContent;

   if (not localSource.IsValid())
      localSource = environment;

   // Integrate the verb argument to the source                         
   Many localArgument;
   bool unusedSkipVerbs = false;
   if (not Flow::Execute(verb.GetArgument(), localSource, localArgument, true, unusedSkipVerbs, silent)) {
      if (not silent)
         FLOW_ERRORS("Error at argument of: ", verb);
      return false;
   }

   verb.SetArgument(Abandon(localArgument)).In(Abandon(localSource));
   return true;
}

/// Execute a single verb, and all subverbs in it, if any.                    
/// Ideally, this function is the last thing that gets called, after          
/// dispatching in all nested context, and after executing all nested branches
/// of a flow. Verb context should be a single flat element.                  
///   @param context - [in/out] the context in which verb will be executed    
///   @param verb - [in/out] verb to execute                                  
///   @param silent - whether or not to silence logging, in case we're        
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
bool Flow::ExecuteVerb(Many const& environment, Verb& verb, const bool silent) {
   // Integration (and execution of subverbs if any)                    
   // Source and argument will be executed locally if scripts, and      
   // substituted with their results in the verb                        
   if (not Flow::IntegrateVerb(environment, verb, silent)) {
      if (not silent) {
         FLOW_ERRORS("Error integrating verb: ", verb, " (", verb.GetVerb(), ')');
      }
      return false;
   }

   if (verb.IsVerb<Verbs::Do>()) {
      // A Do verb is done at this point, because the subverbs          
      // inside (if any) should be done in the integration phase. This  
      // also acts as regression protection. All we need to make sure   
      // is that the integrated argument or source are propagated to    
      // the verb's output.                                             
      if (not verb.GetOutput()) {
         if (verb)   verb << Move(verb.GetArgument());
         else        verb << Move(verb.GetSource());
      }

      return true;
   }

   VERBOSE_TAB("Executing verb: ", Logger::Cyan, verb, " (", verb.GetVerb(), ')');

   // Dispatch the verb to the context, executing it                    
   // Any results should be inside verb.mOutput afterwards              
   //TODO here was a context copy of verb, probably in case of destructive verbs. i don't like excessive copies, figure it out eventually
   if (verb.Run()) {
      if (not silent) {
         FLOW_ERRORS("Error executing verb: ", verb, " (", verb.GetVerb(), ')');
      }
      return false;
   }

   VERBOSE("Executed: ", Logger::Green, verb, " (", verb.GetVerb(), ')');
   return true;
}