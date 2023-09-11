///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Executor.hpp"
#include "Verb.hpp"
#include "verbs/Do.inl"
#include "verbs/Interpret.inl"
#include "verbs/Create.inl"

#define VERBOSE(...)      //Logger::Verbose(__VA_ARGS__)
#define VERBOSE_TAB(...)  //const auto tab = Logger::Verbose(__VA_ARGS__, Logger::Tabs{})
#define FLOW_ERRORS(...)  Logger::Error(__VA_ARGS__)

namespace Langulus::Flow
{

   /// Flat check if block contains verbs                                     
   ///   @param block - the block to scan for verbs                           
   ///   @return true if the block contains immediate verbs                   
   bool IsExecutable(const Block& block) noexcept {
      if (block.Is<Verb>())
         return true;

      bool executable = false;
      block.ForEach(
         // Scan deeper into traits, because they're not deep           
         // They are deep only with respect to execution                
         [&executable](const Trait& trait) noexcept {
            executable = IsExecutable(trait);
            return not executable;
         },
         // Scan deeper into constructs, because they're not deep       
         // They are deep only with respect to execution                
         [&executable](const Construct& construct) noexcept {
            executable = IsExecutable(construct);
            return not executable;
         }
      );

      return executable;
   }

   /// Deep (nested and slower) check if block contains verbs                 
   ///   @param block - the block to scan for verbs                           
   ///   @return true if the deep or flat block contains verbs                
   bool IsExecutableDeep(const Block& block) noexcept {
      if (IsExecutable(block))
         return true;

      bool executable = false;
      block.ForEachDeep([&executable](const Block& group) noexcept {
         executable = IsExecutable(group);
         return not executable;
      });

      return executable;
   }
   
   /// Flat check if Neat contains verbs                                      
   ///   @param block - the Neat to scan for verbs                            
   ///   @return true if the block contains immediate verbs                   
   bool IsExecutable(const Neat& block) noexcept {
      bool executable = false;
      block.ForEach([&](const Block& group) noexcept {
         executable = IsExecutable(group);
         return not executable;
      });
      return executable;
   }

   /// Deep check if Neat contains verbs                                      
   ///   @param block - the Neat to scan for verbs                            
   ///   @return true if the deep or flat block contains verbs                
   bool IsExecutableDeep(const Neat& block) noexcept {
      // Neats are always flat, so it is the same as IsExecutable       
      return IsExecutable(block);
   }

   /// Nested AND/OR scope execution (discarding outputs)                     
   /// TODO optimize for unneeded outputs                                     
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the environment in which flow will be executed  
   ///   @return true of no errors occured                                    
   bool Execute(const Block& flow, Any& environment) {
      Any output;
      bool skipVerbs = false;
      return Execute(flow, environment, output, skipVerbs);
   }

   /// Nested AND/OR scope execution with output                              
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @return true of no errors occured                                    
   bool Execute(const Block& flow, Any& environment, Any& output) {
      bool skipVerbs = false;
      return Execute(flow, environment, output, skipVerbs);
   }

   /// Nested AND/OR scope execution with output                              
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [in/out] whether to skip verbs after OR success   
   ///   @return true of no errors occured                                    
   bool Execute(const Block& flow, Any& environment, Any& output, bool& skipVerbs) {
      auto results = Any::FromState(flow);
      if (flow) {
         VERBOSE_TAB("Executing scope: [", flow, ']');

         try {
            if (flow.IsOr() and flow.GetCount() > 1)
               ExecuteOR(flow, environment, results, skipVerbs);
            else
               ExecuteAND(flow, environment, results, skipVerbs);
         }
         catch (const Except::Flow&) {
            // Execution failed                                         
            return false;
         }
      }

      output.SmartPush(Abandon(results));
      return true;
   }
   
   /// Nested neat scope execution with output                                
   /// Iterates all verbs inside Neat, and executes them, producing new Neat  
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @return true of no errors occured                                    
   bool Execute(const Neat& flow, Any& environment, Neat& output) {
      TODO();
   }

   /// Nested neat scope execution with output                                
   /// Iterates all verbs inside Neat, and executes them, producing new Neat  
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [in/out] whether to skip verbs after OR success   
   ///   @return true of no errors occured                                    
   bool Execute(const Neat& flow, Any& environment, Neat& output, bool& skipVerbs) {
      TODO();
   }

   /// Nested AND scope execution                                             
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [in/out] whether to skip verbs after OR success   
   ///   @return true of no errors occured                                    
   bool ExecuteAND(const Block& flow, Any& environment, Any& output, bool& skipVerbs) {
      Count executed {};
      if (flow.IsDeep()) {
         // Nest if deep                                                
         executed = flow.ForEach([&](const Block& block) {
            Any local;
            if (not Execute(block, environment, local, skipVerbs)) {
               VERBOSE(Logger::Red, "Deep AND flow failed: ", flow);
               LANGULUS_THROW(Flow, "Deep AND failure");
            }

            output.SmartPush(Abandon(local));
         });
      }
      else {
         executed = flow.ForEach(
            // Nest if traits, but retain each trait                    
            [&](const Trait& trait) {
               if (trait.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(trait);
                  return;
               }

               Any local;
               if (not Execute(trait, environment, local, skipVerbs)) {
                  VERBOSE(Logger::Red, "Trait AND flow failed: ", flow);
                  LANGULUS_THROW(Flow, "Trait AND failure");
               }

               output.SmartPush(Trait::From(trait.GetTrait(), Abandon(local)));
            },
            // Nest if constructs, but retain each construct            
            [&](const Construct& construct) {
               if (construct.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(construct);
                  return;
               }

               Construct local;
               if (not Execute(construct, environment, local, skipVerbs)) {
                  VERBOSE(Logger::Red, "Construct AND flow failed: ", flow);
                  LANGULUS_THROW(Flow, "Construct AND failure");
               }

               // Attempt to produce the thing at compile-time          
               Any constExpr;
               if (local.StaticCreation(constExpr))
                  output.SmartPush(Abandon(constExpr));
               else {
                  // Construction failed, so just propagate construct   
                  // A new attempt will be made at runtime              
                  Verbs::Create creator {&local};
                  if (DispatchDeep<true, true, false>(environment, creator))
                     output.SmartPush(Abandon(creator.GetOutput()));
                  else {
                     VERBOSE(Logger::Red, "Construct runtime creation failed in: ", flow);
                     LANGULUS_THROW(Flow, "Construct runtime creation failure");
                  }
               }
            },
            // Execute verbs                                            
            [&](const Verb& constVerb) {
               if (skipVerbs)
                  return Flow::Break;

               if (constVerb.GetCharge().IsFlowDependent()) {
                  // The verb hasn't been integrated into a flow, just  
                  // forward it                                         
                  output.SmartPush(constVerb);
                  return Flow::Continue;
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
               if (not ExecuteVerb(environment, verb)) {
                  VERBOSE(Logger::Red, "Verb AND flow failed: ", flow);
                  LANGULUS_THROW(Flow, "Verb AND failure");
               }

               output.SmartPush(Abandon(verb.GetOutput()));
               return Flow::Continue;
            }
         );
      }

      if (not executed) {
         // If this is reached, then we had non-verb content            
         // Just propagate its contents                                 
         output.SmartPush(flow);
      }

      VERBOSE(Logger::Green, "AND scope done: ", flow);
      return true;
   }

   /// Nested OR execution                                                    
   ///   @param flow - the flow to execute                                    
   ///   @param environment - the context in which scope will be executed     
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [out] whether to skip verbs after OR success      
   ///   @return true of no errors occured                                    
   bool ExecuteOR(const Block& flow, Any& environment, Any& output, bool& skipVerbs) {
      Count executed {};
      bool localSkipVerbs {};

      if (flow.IsDeep()) {
         // Nest if deep                                                
         executed = flow.ForEach([&](const Block& block) {
            Any local;
            if (Execute(block, environment, local, localSkipVerbs)) {
               executed = true;
               output.SmartPush(Abandon(local));
            }
         });
      }
      else {
         executed = flow.ForEach(
            // Nest if traits, but retain each trait                    
            [&](const Trait& trait) {
               if (trait.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(trait);
                  return;
               }

               Any local;
               if (Execute(trait, environment, local)) {
                  executed = true;
                  output.SmartPush(Trait::From(trait.GetTrait(), Abandon(local)));
               }
            },
            // Nest if constructs, but retain each construct            
            [&](const Construct& construct) {
               if (construct.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(construct);
                  return;
               }

               Construct local;
               if (Execute(construct, environment, local)) {
                  executed = true;
                  output << Abandon(local);
               }
            },
            // Execute verbs                                            
            [&](const Verb& constVerb) {
               if (localSkipVerbs)
                  return Flow::Break;

               if (constVerb.GetCharge().IsFlowDependent()) {
                  // The verb hasn't been integrated into a flow, just  
                  // forward it                                         
                  output.SmartPush(constVerb);
                  return Flow::Continue;
               }

               // Shallow-copy the verb to make it mutable              
               // Also resets its output                                
               auto verb = Verb::FromMeta(
                  constVerb.GetVerb(),
                  constVerb.GetArgument(),
                  constVerb,
                  constVerb.GetVerbState()
               );

               if (not ExecuteVerb(environment, verb))
                  return Flow::Continue;

               executed = true;
               output.SmartPush(Abandon(verb.GetOutput()));
               return Flow::Continue;
            }
         );
      }

      skipVerbs |= localSkipVerbs;

      if (not executed) {
         // If this is reached, then we have non-verb flat content      
         // Just propagate it                                           
         output.SmartPush(flow);
         ++executed;
      }

      if (executed) {
         VERBOSE(Logger::Green, "OR scope done: ", flow);
      }
      else {
         VERBOSE(Logger::Red, "OR scope failed: ", flow);
      }

      return executed;
   }

   /// Integrate all parts of a verb inside this environment                  
   ///   @param environment - [in/out] the context for integration            
   ///   @param verb - [in/out] verb to integrate                             
   ///   @return true of no errors occured                                    
   bool IntegrateVerb(Any& environment, Verb& verb) {
      if (verb.IsMonocast()) {
         // We're executing on whole argument/source, so be lazy        
         if (verb.GetSource().IsInvalid())
            verb.GetSource() = environment;
         return true;
      }

      // Integrate the verb source to environment                       
      Any localSource;
      if (not Execute(verb.GetSource(), environment, localSource)) {
         // It's considered error only if verb is not monocast          
         FLOW_ERRORS("Error at source of: ", verb);
         return false;
      }

      if (localSource.IsInvalid())
         localSource = environment;

      // Integrate the verb argument to the source                      
      Any localArgument;
      if (not Execute(verb.GetArgument(), localSource, localArgument)) {
         // It's considered error only if verb is not monocast          
         FLOW_ERRORS("Error at argument of: ", verb);
         return false;
      }

      verb.GetSource() = Abandon(localSource);
      verb.GetArgument() = Abandon(localArgument);
      return true;
   }

   /// Execute a single verb, and all subverbs in it, if any                  
   ///   @param context - [in/out] the context in which verb will be executed 
   ///   @param verb - [in/out] verb to execute                               
   ///   @return true of no errors occured                                    
   bool ExecuteVerb(Any& context, Verb& verb) {
      // Integration (and execution of subverbs if any)                 
      // Source and argument will be executed locally if scripts, and   
      // substituted with their results in the verb                     
      if (not IntegrateVerb(context, verb)) {
         FLOW_ERRORS("Error integrating verb: ", verb, " (", verb.GetVerb(), ')');
         return false;
      }

      if (verb.Is<Verbs::Do>()) {
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

      VERBOSE_TAB("Executing verb: ", Logger::Cyan, verb, " (", verb.GetVerb(), ')');

      // Dispatch the verb to the context, executing it                 
      // Any results should be inside verb.mOutput afterwards           
      if (not DispatchDeep(verb.GetSource(), verb)) {
         FLOW_ERRORS("Error executing verb: ", verb, " (", verb.GetVerb(), ')');
         return false;
      }

      VERBOSE("Executed: ", Logger::Green, verb, " (", verb.GetVerb(), ')');
      return true;
   }

} // namespace Langulus::Flow
