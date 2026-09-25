///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Executor.hpp"
#include "inner/Missing.hpp"
#include "inner/Redundant.hpp"

#include <Langulus/Verbs/Do.hpp>
#include <Langulus/Verbs/Interpret.hpp>
#include <Langulus/Verbs/Create.hpp>
#include "Langulus/Except.hpp"
#include "Langulus/TTag.hpp"
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
using namespace Langulus::Flow;


/// Nested AND/OR scope execution with output                                 
///   @param flow - the flow to execute                                       
///   @param context - the environment in which scope will be executed        
///   @param output - [out] verb result will be pushed here                   
///   @param integrate - execution happens in two styles:                     
///      1. integration - everything not executed will still be pushed to     
///         output, preserving the hierarchy. useful when integrating verbs   
///      2. not integration - only unexecuted verbs will push to output,      
///         useful for collecting side-effects when updating                  
///   @param silent - whether or not to silence logging, in case we're        
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
/*bool Execute(
   const Many& flow, Many& context, Many& output,
   const bool integrate, const bool silent
) {
   bool skipVerbs = false;
   return Execute(flow, context, output, integrate, skipVerbs, silent);
}*/

/// Nested AND/OR scope execution with output                                 
///   @param flow - the flow to execute                                       
///   @param context - the environment in which scope will be executed        
///   @param output - [out] verb result will be pushed here                   
///   @param integrate - execution happens in two styles:                     
///      1. integration - everything not executed will still be pushed to     
///         output, preserving the hierarchy. useful when integrating verbs   
///      2. not integration - only unexecuted verbs will push to output,      
///         useful for collecting side-effects when updating                  
///   @param skipVerbs - [in/out] whether to skip verbs after OR success      
///   @param silent - whether or not to silence logging, in case we're        
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
bool Execute(
   const Many& flow, Many& context, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   auto results = Many::CopyStates(flow);
   if (flow) {
      if (integrate)
         VERBOSE_TAB("Executing scope (integrating): [", flow, ']');
      else
         VERBOSE_TAB("Executing scope: [", flow, ']');

      try {
         if (flow.IsOr())
            ExecuteOR(flow, context, results, integrate, skipVerbs, silent);
         else
            ExecuteAND(flow, context, results, integrate, skipVerbs, silent);
      }
      catch (...) {
         // Execution failed                                            
         return false;
      }
   }

   output.Compose(Abandon(results));
   return true;
}

/// Nested AND scope execution                                                
///   @param flow - the flow to execute                                       
///   @param context - the environment in which scope will be executed        
///   @param output - [out] verb result will be pushed here                   
///   @param integrate - execution happens in two styles:                     
///      1. integration - everything not executed will still be pushed to     
///         output, preserving the hierarchy. useful when integrating verbs   
///      2. not integration - only unexecuted verbs will push to output,      
///         useful for collecting side-effects when updating                  
///   @param skipVerbs - [in/out] whether to skip verbs after OR success      
///   @param silent - whether or not to silence logging, in case we're        
///      executing at compile-time, for example                               
///   @return true of no errors occured                                       
bool ExecuteAND(
   const Many& flow, Many& context, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   size_t executed = 0;
   if (flow.IsDeep() and not flow.IsSparse()) {
      executed = flow.ForEach([&](const Many& block) {
         // Nest if deep                                                
         Many local;
         if (not Execute(block, context, local, integrate, skipVerbs, silent)) {
            if (silent)
               throw Exception("Deep AND failure");
            else
               LglsError("Deep AND failure: "/*, flow*/);
         }

         output.Compose(Abandon(local));
      });
   }
   else if (not flow.IsSparse()) {
      executed = flow.ForEach(
         [&](const Missing& missing) {
            // Nest if missing points                                   
            Many local;
            if (not Execute(missing.mContent, context, local, integrate, skipVerbs, silent)) {
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
            if (not Execute(tag, context, local, integrate, skipVerbs, silent)) {
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
            if (not Execute(recipe.GetDescriptor(), context, local, integrate, skipVerbs, silent)) {
               if (silent)
                  throw Exception("Construct AND failure");
               else
                  LglsError("Construct AND failure: "/*, flow*/);
            }

            auto solved = Recipe::From(recipe, Abandon(local));

            // We can attempt an implicit Verbs::Create to make         
            // the data at compile-time. Allowed only if no producer    
            // was specified and if construct is not flow-dependent.    
            if (not recipe.GetTarget().GetProducer() /*and not recipe.GetCharge().IsFlowDependent()*/) {
               Verbs::Create creator {&solved};
               if (Verb::GenericExecuteStateless(creator)) {
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
            auto verb = Verb::From(constVerb, constVerb.GetArgument());
            verb.SetSource(constVerb.GetSource());

            if (verb.IsMissing()) {
               if (integrate) {
                  output.Compose(verb);
                  return Loop::Continue;
               }
               else FLOW_ERRORS("Trying to execute a missing verb: ", verb);
            }

            // Execute the verb                                      
            if (not ExecuteVerb(context, verb, silent)) {
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
///   @param flow - the flow to execute                                    
///   @param context - the context in which scope will be executed         
///   @param output - [out] verb result will be pushed here                
///   @param integrate - execution happens in two styles:                  
///      1. integration - everything not executed will still be pushed to  
///         output, preserving the hierarchy. useful when integrating verbs
///      2. not integration - only unexecuted verbs will push to output,   
///         useful for collecting side-effects when updating               
///   @param skipVerbs - [out] whether to skip verbs after OR success      
///   @param silent - whether or not to silence logging, in case we're     
///      executing at compile-time, for example                            
///   @return true of no errors occured                                    
bool ExecuteOR(
   const Many& flow, Many& context, Many& output,
   const bool integrate, bool& skipVerbs, const bool silent
) {
   size_t executed = 0;
   bool localSkipVerbs = false;

   if (flow.IsDeep() and not flow.IsSparse()) {
      executed = flow.ForEach([&](const Many& block) {
         // Nest if deep                                             
         Many local;
         if (Execute(block, context, local, integrate, localSkipVerbs, silent)) {
            executed = true;
            output.Compose(Abandon(local));
         }
      });
   }
   else if (not flow.IsSparse()) {
      executed = flow.ForEach(
         [&](const Tag& tag) {
            // Nest if traits, but retain each trait                 
            if (tag.IsMissing()) {
               // Never touch missing stuff, only propagate it       
               output.Compose(tag);
               return;
            }

            Many local;
            if (Execute(tag, context, local, integrate, silent)) {
               executed = true;
               output.Compose(Tag::From(tag, Abandon(local)));
            }
         },
         [&](const Recipe& recipe) {
            // Nest if constructs, but retain each construct         
            Many local;
            if (Execute(recipe.GetDescriptor(), context, local, integrate, skipVerbs, silent)) {
               executed = true;
               auto solved = Recipe::From(recipe, Abandon(local));

               // We can attempt an implicit Verbs::Create to make   
               // the data at compile-time. Allowed only if no       
               // producer was specified.                            
               if (not recipe.GetTarget().GetProducer() /*and not construct.GetCharge().IsFlowDependent()*/) {
                  Verbs::Create creator {&solved};
                  if (Verb::GenericExecuteStateless(creator)) {
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
            auto verb = Verb::From(constVerb, constVerb.GetArgument());
            if (verb.IsMissing()) {
               if (integrate) {
                  output.Compose(verb);
                  return Loop::Continue;
               }
               else FLOW_ERRORS("Trying to execute a missing verb: ", verb);
            }

            if (not ExecuteVerb(context, verb, silent))
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
bool IntegrateVerb(Many& context, Verb& verb, const bool silent) {
   /*if (verb.IsMonocast()) {
      // We're executing on whole argument/source, so be lazy        
      if (verb.GetSource().IsInvalid())
         verb.SetSource(context);
      return true;
   }*/

   // Integrate the verb source to environment                       
   Many localSource;
   if (not verb.GetSource().Is<Redundant>()) {
      if (not Execute(verb.GetSource(), context, localSource, true, silent)) {
         if (not silent)
            FLOW_ERRORS("Error at source of: ", verb);
         return false;
      }
   }
   else localSource = verb.GetSource().Get<Redundant>().mContent;

   if (localSource.IsInvalid())
      localSource = context;

   // Integrate the verb argument to the source                      
   Many localArgument;
   if (not Execute(verb.GetArgument(), localSource, localArgument, true, silent)) {
      if (not silent)
         FLOW_ERRORS("Error at argument of: ", verb);
      return false;
   }

   verb.SetSource(Abandon(localSource));
   verb.SetArgument(Abandon(localArgument));
   return true;
}

/// Execute a single verb, and all subverbs in it, if any                  
///   @param context - [in/out] the context in which verb will be executed 
///   @param verb - [in/out] verb to execute                               
///   @param silent - whether or not to silence logging, in case we're     
///      executing at compile-time, for example                            
///   @return true of no errors occured                                    
bool ExecuteVerb(Many& context, Verb& verb, const bool silent) {
   // Integration (and execution of subverbs if any)                 
   // Source and argument will be executed locally if scripts, and   
   // substituted with their results in the verb                     
   if (not IntegrateVerb(context, verb, silent)) {
      if (not silent) {
         FLOW_ERRORS("Error integrating verb: ",
            verb, " (", verb.GetVerb(), ')');
      }
      return false;
   }

   if (verb.IsVerb<Verbs::Do>()) {
      // A Do verb is done at this point, because the subverbs       
      // inside (if any) should be done in the integration phase     
      // Just making sure that the integrated argument & source are  
      // propagated to the verb's output                             
      if (not verb.GetOutput()) {
         if (verb)   verb << Move(verb.GetArgument());
         else        verb << Move(verb.GetSource());
      }

      return true;
   }

   VERBOSE_TAB("Executing verb: ",
      Logger::Cyan, verb, " (", verb.GetVerb(), ')');

   // Dispatch the verb to the context, executing it                 
   // Any results should be inside verb.mOutput afterwards           
   Many contextCopy = verb.GetSource();
   if (not DispatchDeep(contextCopy, verb)) {
      if (not silent) {
         FLOW_ERRORS("Error executing verb: ",
            verb, " (", verb.GetVerb(), ')');
      }
      return false;
   }

   VERBOSE("Executed: ",
      Logger::Green, verb, " (", verb.GetVerb(), ')');
   return true;
}