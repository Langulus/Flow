///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Scope.hpp"
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

   /// Nested AND/OR scope execution (discarding outputs)                     
   /// TODO optimize for unneeded outputs                                     
   ///   @param environment - the environment in which scope will be executed 
   ///   @return true of no errors occured                                    
   bool Scope::Execute(Any& environment) const {
      Any output;
      bool skipVerbs = false;
      return Execute(environment, output, skipVerbs);
   }

   /// Nested AND/OR scope execution with output                              
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @return true of no errors occured                                    
   bool Scope::Execute(Any& environment, Any& output) const {
      bool skipVerbs = false;
      return Execute(environment, output, skipVerbs);
   }

   /// Nested AND/OR scope execution with output                              
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [in/out] whether to skip verbs after OR success   
   ///   @return true of no errors occured                                    
   bool Scope::Execute(Any& environment, Any& output, bool& skipVerbs) const {
      auto results = Any::FromState(*this);
      if (not IsEmpty()) {
         VERBOSE_TAB("Executing scope: [", this, ']');

         try {
            if (IsOr() and GetCount() > 1)
               ExecuteOR(environment, results, skipVerbs);
            else
               ExecuteAND(environment, results, skipVerbs);
         }
         catch (const Except::Flow&) {
            // Execution failed                                         
            return false;
         }
      }

      output.SmartPush(Abandon(results));
      return true;
   }

   /// Nested AND scope execution                                             
   ///   @param environment - the environment in which scope will be executed 
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [in/out] whether to skip verbs after OR success   
   ///   @return true of no errors occured                                    
   bool Scope::ExecuteAND(Any& environment, Any& output, bool& skipVerbs) const {
      Count executed {};
      if (IsDeep()) {
         // Nest if deep                                                
         executed = ForEach([&](const Block& block) {
            const auto& scope = ReinterpretCast<Scope>(block);
            Any local;
            if (!scope.Execute(environment, local, skipVerbs)) {
               VERBOSE(Logger::Red, "Deep AND flow failed: ", *this);
               LANGULUS_THROW(Flow, "Deep AND failure");
            }

            output.SmartPush(Abandon(local));
         });
      }
      else {
         executed = ForEach(
            // Nest if traits, but retain each trait                    
            [&](const Trait& trait) {
               if (trait.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(trait);
                  return;
               }

               const auto& scope = ReinterpretCast<Scope>(static_cast<const Block&>(trait));
               Any local;
               if (not scope.Execute(environment, local, skipVerbs)) {
                  VERBOSE(Logger::Red, "Trait AND flow failed: ", *this);
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

               const auto& scope = ReinterpretCast<Scope>(construct.GetArgument());
               Any local;
               if (not scope.Execute(environment, local, skipVerbs)) {
                  VERBOSE(Logger::Red, "Construct AND flow failed: ", *this);
                  LANGULUS_THROW(Flow, "Construct AND failure");
               }

               Construct newc {construct.GetType(), Move(local), construct};
               if (newc.StaticCreation(local))
                  output.SmartPush(Abandon(local));
               else {
                  // Construction failed, so just propagate construct   
                  // A new attempt will be made at runtime              
                  Verbs::Create creator {&newc};
                  if (DispatchDeep<true, true, false>(environment, creator))
                     output.SmartPush(Abandon(creator.GetOutput()));
                  else {
                     VERBOSE(Logger::Red, "Construct runtime creation failed in: ", *this);
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
                  constVerb.mVerb,
                  constVerb.GetArgument(),
                  constVerb,
                  constVerb.GetVerbState()
               );
               verb.SetSource(constVerb.GetSource());

               // Execute the verb                                      
               if (not Scope::ExecuteVerb(environment, verb)) {
                  VERBOSE(Logger::Red, "Verb AND flow failed: ", *this);
                  LANGULUS_THROW(Flow, "Verb AND failure");
               }

               output.SmartPush(Abandon(verb.mOutput));
               return Flow::Continue;
            }
         );
      }

      if (not executed) {
         // If this is reached, then we had non-verb content            
         // Just propagate its contents                                 
         output.SmartPush(static_cast<const Any&>(*this));
      }

      VERBOSE(Logger::Green, "AND scope done: ", *this);
      return true;
   }

   /// Nested OR execution                                                    
   ///   @param environment - the context in which scope will be executed     
   ///   @param output - [out] verb result will be pushed here                
   ///   @param skipVerbs - [out] whether to skip verbs after OR success      
   ///   @return true of no errors occured                                    
   bool Scope::ExecuteOR(Any& environment, Any& output, bool& skipVerbs) const {
      Count executed {};
      bool localSkipVerbs {};

      if (IsDeep()) {
         // Nest if deep                                                
         executed = ForEach([&](const Block& block) {
            const auto& scope = ReinterpretCast<Scope>(block);
            Any local;
            if (scope.Execute(environment, local, localSkipVerbs)) {
               executed = true;
               output.SmartPush(Abandon(local));
            }
         });
      }
      else {
         executed = ForEach(
            // Nest if traits, but retain each trait                    
            [&](const Trait& trait) {
               if (trait.IsMissing()) {
                  // Never touch missing stuff, only propagate it       
                  output.SmartPush(trait);
                  return;
               }

               const auto& scope = 
                  ReinterpretCast<Scope>(static_cast<const Block&>(trait));

               Any local;
               if (scope.Execute(environment, local)) {
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

               const auto& scope = 
                  ReinterpretCast<Scope>(construct.GetArgument());

               Any local;
               if (scope.Execute(environment, local)) {
                  executed = true;
                  output << Construct {construct.GetType(), Abandon(local), construct};
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
                  constVerb.mVerb,
                  constVerb.GetArgument(),
                  constVerb,
                  constVerb.GetVerbState()
               );

               if (not Scope::ExecuteVerb(environment, verb))
                  return Flow::Continue;

               executed = true;
               output.SmartPush(Abandon(verb.mOutput));
               return Flow::Continue;
            }
         );
      }

      skipVerbs |= localSkipVerbs;

      if (!executed) {
         // If this is reached, then we have non-verb flat content      
         // Just propagate it                                           
         output.SmartPush(static_cast<const Any&>(*this));
         ++executed;
      }

      if (executed) {
         VERBOSE(Logger::Green, "OR scope done: ", *this);
      }
      else {
         VERBOSE(Logger::Red, "OR scope failed: ", *this);
      }

      return executed;
   }

   /// Integrate all parts of a verb inside this environment                  
   ///   @param context - [in/out] the context where verb will be integrated  
   ///   @param verb - [in/out] verb to integrate                             
   ///   @return true of no errors occured                                    
   bool Scope::IntegrateVerb(Any& environment, Verb& verb) {
      if (verb.IsMonocast()) {
         // We're executing on whole argument/source, so be lazy        
         if (verb.GetSource().IsInvalid())
            verb.GetSource() = environment;
         return true;
      }

      // Integrate the verb source to environment                       
      Any localSource;
      if (not ReinterpretCast<Scope>(verb.GetSource()).Execute(environment, localSource)) {
         // It's considered error only if verb is not monocast          
         FLOW_ERRORS("Error at source of: ", verb);
         return false;
      }

      if (localSource.IsInvalid())
         localSource = environment;

      // Integrate the verb argument to the source                      
      Any localArgument;
      if (not ReinterpretCast<Scope>(verb.GetArgument()).Execute(localSource, localArgument)) {
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
   bool Scope::ExecuteVerb(Any& context, Verb& verb) {
      // Integration (and execution of subverbs if any)                 
      // Source and argument will be executed locally if scripts, and   
      // substituted with their results in the verb                     
      if (not Scope::IntegrateVerb(context, verb)) {
         FLOW_ERRORS("Error integrating verb: ", verb, " (", verb.GetVerb(), ')');
         return false;
      }

      if (verb.Is<Verbs::Do>()) {
         // A Do verb is done at this point, because the subverbs       
         // inside (if any) should be done in the integration phase     
         // Just making sure that the integrated argument & source are  
         // propagated to the verb's output                             
         if (not verb.mOutput) {
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
      if (not DispatchDeep(verb.mSource, verb)) {
         FLOW_ERRORS("Error executing verb: ", verb, " (", verb.GetVerb(), ')');
         return false;
      }

      VERBOSE("Executed: ", Logger::Green, verb, " (", verb.GetVerb(), ')');
      return true;
   }

} // namespace Langulus::Flow
