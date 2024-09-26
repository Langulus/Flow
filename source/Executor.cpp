///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Executor.hpp"
#include "verbs/Do.inl"
#include "verbs/Interpret.inl"
#include "verbs/Create.inl"
#include "inner/Missing.hpp"

#if 0
   #define VERBOSE(...)      Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_TAB(...)  const auto tab = Logger::VerboseTab(__VA_ARGS__)
#else
   #define VERBOSE(...)      LANGULUS(NOOP)
   #define VERBOSE_TAB(...)  LANGULUS(NOOP)
#endif

#define FLOW_ERRORS(...)  Logger::Error(__VA_ARGS__)


namespace Langulus::Flow
{

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
   bool Execute(
      const Many& flow, Many& context, Many& output,
      const bool integrate, const bool silent
   ) {
      bool skipVerbs = false;
      return Execute(flow, context, output, integrate, skipVerbs, silent);
   }

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
      auto results = Many::FromState(flow);
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
         catch (const Except::Flow&) {
            // Execution failed                                         
            return false;
         }
      }

      output.SmartPush(IndexBack, Abandon(results));
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
      Count executed = 0;
      if (flow.IsDeep() and flow.IsDense()) {
         executed = flow.ForEach([&](const Many& block) {
            // Nest if deep                                             
            Many local;
            if (not Execute(block, context, local, integrate, skipVerbs, silent)) {
               if (silent)
                  LANGULUS_THROW(Flow, "Deep AND failure");
               else
                  LANGULUS_OOPS(Flow, "Deep AND failure: ", flow);
            }

            output.SmartPush(IndexBack, Abandon(local));
         });
      }
      else if (flow.IsDense()) {
         executed = flow.ForEach(
            [&](const Inner::Missing& missing) {
               // Nest if missing points                                
               Many local;
               if (not Execute(missing.mContent, context, local, integrate, skipVerbs, silent)) {
                  if (silent)
                     LANGULUS_THROW(Flow, "Missing point failure");
                  else
                     LANGULUS_OOPS(Flow, "Missing point failure: ", flow);
               }

               output.SmartPush(IndexBack, Abandon(local));
            },
            [&](const Trait& trait) {
               // Nest if traits, but retain each trait                 
               if (trait.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(IndexBack, trait);
                  return;
               }

               Many local;
               if (not Execute(trait, context, local, integrate, skipVerbs, silent)) {
                  if (silent)
                     LANGULUS_THROW(Flow, "Trait AND failure");
                  else
                     LANGULUS_OOPS(Flow, "Trait AND failure: ", flow);
               }

               output.SmartPush(IndexBack, Trait::From(trait.GetTrait(), Abandon(local)));
            },
            [&](const Construct& construct) {
               // Nest if constructs, but retain each construct         
               // Make a shallow copy of the construct, and strip all   
               // verbs from it. Some of them might get reinserted, if  
               // missing, but generally they will be substituted with  
               // the corresponding results                             
               VERBOSE("Executing construct: ", construct);
               Construct local = construct;
               local.GetDescriptor().template RemoveData<A::Verb>();
               VERBOSE("Executing construct (verbs stripped): ", local);
               bool constructIsMissing = false;

               construct.GetDescriptor().ForEach(
                  [&](const A::Verb& constVerb) {
                     if (constVerb.IsMissing()) {
                        // Never touch missing stuff, only propagate it 
                        local << constVerb;
                        constructIsMissing = true;
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
                           LANGULUS_OOPS(Flow, "Construct AND failure: ", flow);
                     }
                     else if (verb.GetOutput())
                        local << Abandon(verb.GetOutput());
                  }
               );

               VERBOSE("Executing construct (verbs executed): ", local);
               if (constructIsMissing
               or construct.GetType()->mProducerRetriever
               or not construct.GetType()->mDescriptorConstructor) {
                  // Just propagate if missing or not instantiatable at 
                  // compile-time                                       
                  output.SmartPush(IndexBack, Abandon(local));
                  return;
               }

               // We can attempt an implicit Verbs::Create to make      
               // the data at compile-time                              
               Verbs::Create creator {&local};
               if (DispatchDeep<true, true, false>(context, creator))
                  output.SmartPush(IndexBack, Abandon(creator.GetOutput()));
               else {
                  if (silent)
                     LANGULUS_THROW(Flow, "Construct creation failure");
                  else
                     LANGULUS_OOPS(Flow, "Construct creation failure: ", flow);
               }
            },
            [&](const A::Verb& constVerb) {
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
               auto verb = Verb::FromMeta(
                  constVerb.GetVerb(),
                  constVerb.GetArgument(),
                  constVerb,
                  constVerb.GetVerbState()
               );
               verb.SetSource(constVerb.GetSource());

               // Execute the verb                                      
               if (not ExecuteVerb(context, verb, silent)) {
                  if (silent)
                     LANGULUS_THROW(Flow, "Verb AND failure");
                  else
                     LANGULUS_OOPS(Flow, "Verb AND failure: ", verb);
               }

               // Make sure the original verb has been marked done, so  
               // that it isn't executed every time.                    
               const_cast<A::Verb&>(constVerb).Done();
               output.SmartPush(IndexBack, Abandon(verb.GetOutput()));
               return Loop::Continue;
            }
         );
      }

      if (not executed and integrate) {
         // If this is reached, then we had non-verb content            
         // Just propagate its contents                                 
         output.SmartPush(IndexBack, flow);
      }

      VERBOSE(Logger::Green, "AND scope done: ", flow);
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
      Count executed = 0;
      bool localSkipVerbs = false;

      if (flow.IsDeep() and flow.IsDense()) {
         executed = flow.ForEach([&](const Many& block) {
            // Nest if deep                                             
            Many local;
            if (Execute(block, context, local, integrate, localSkipVerbs, silent)) {
               executed = true;
               output.SmartPush(IndexBack, Abandon(local));
            }
         });
      }
      else if (flow.IsDense()) {
         executed = flow.ForEach(
            [&](const Trait& trait) {
               // Nest if traits, but retain each trait                 
               if (trait.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(IndexBack, trait);
                  return;
               }

               Many local;
               if (Execute(trait, context, local, integrate, silent)) {
                  executed = true;
                  output.SmartPush(IndexBack, Trait::From(trait.GetTrait(), Abandon(local)));
               }
            },
            [&](const Construct& construct) {
               // Nest if constructs, but retain each construct         
               // Make a shallow copy of the construct, and strip all   
               // verbs from it. Some of them might get reinserted, if  
               // missing, but generally they will be substituted with  
               // the corresponding results                             
               Construct local = construct;
               local.GetDescriptor().template RemoveData<A::Verb>();
               bool constructIsMissing = false;

               construct.GetDescriptor().ForEach(
                  [&](const A::Verb& constVerb) noexcept {
                     if (constVerb.IsMissing()) {
                        // Never touch missing stuff, only propagate it 
                        local << constVerb;
                        constructIsMissing = true;
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

               if (constructIsMissing
               or construct.GetType()->mProducerRetriever
               or not construct.GetType()->mDescriptorConstructor) {
                  // Just propagate if missing or not instantiatable at 
                  // compile-time                                       
                  output.SmartPush(IndexBack, Abandon(local));
                  return;
               }

               // We can attempt an implicit Verbs::Create to make      
               // the data at compile-time                              
               Verbs::Create creator {&local};
               if (DispatchDeep<true, true, false>(context, creator))
                  output.SmartPush(IndexBack, Abandon(creator.GetOutput()));
               else {
                  if (silent)
                     LANGULUS_THROW(Flow, "Construct creation failure");
                  else
                     LANGULUS_OOPS(Flow, "Construct creation failure: ", flow);
               }
            },
            [&](const Verb& constVerb) {
               // Execute verbs                                         
               if (localSkipVerbs)
                  return Loop::Break;

               // Shallow-copy the verb to make it mutable              
               // Also resets its output                                
               auto verb = Verb::FromMeta(
                  constVerb.GetVerb(),
                  constVerb.GetArgument(),
                  constVerb,
                  constVerb.GetVerbState()
               );

               if (not ExecuteVerb(context, verb, silent))
                  return Loop::Continue;

               executed = true;
               output.SmartPush(IndexBack, Abandon(verb.GetOutput()));
               return Loop::Continue;
            }
         );
      }

      skipVerbs |= localSkipVerbs;

      if (not executed and integrate) {
         // If this is reached, then we have non-verb flat content      
         // Just propagate it                                           
         output.SmartPush(IndexBack, flow);
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
      if (verb.IsMonocast()) {
         // We're executing on whole argument/source, so be lazy        
         if (verb.GetSource().IsInvalid())
            verb.SetSource(context);
         return true;
      }

      // Integrate the verb source to environment                       
      Many localSource;
      if (not Execute(verb.GetSource(), context, localSource, true, silent)) {
         // It's considered error only if verb is not monocast          
         if (not silent)
            FLOW_ERRORS("Error at source of: ", verb);
         return false;
      }

      if (localSource.IsInvalid())
         localSource = context;

      // Integrate the verb argument to the source                      
      Many localArgument;
      if (not Execute(verb.GetArgument(), localSource, localArgument, true, silent)) {
         // It's considered error only if verb is not monocast          
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
            if (verb)
               verb << Move(verb.GetArgument());
            else
               verb << Move(verb.GetSource());
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

} // namespace Langulus::Flow