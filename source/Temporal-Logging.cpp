///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Time.inl"
#include "Code.inl"
#include "Resolvable.inl"
#include "inner/Missing.hpp"
#include "inner/Entangled.hpp"
#include "Temporal.hpp"

using namespace Langulus::Flow;

namespace {
   constexpr int CarryOverLimit = 100;
}


/// Dump the contents of the flow to the log in a pretty, colorized and       
/// easily readable way                                                       
void Temporal::Dump() const {
   auto tab = Logger::VerboseTab(*this, ": DUMPING CONTENTS...");

   if (mPriorityStack) {
      bool first = true;
      DumpInner(mPriorityStack, true, first);
   }

   for (auto pair : mTimeStack) {
      auto tab = Logger::Section(Logger::PushPurple, "At time ", pair.mKey, ":");
      pair.mValue.Dump();
   }

   for (auto pair : mFrequencyStack) {
      auto tab = Logger::Section(Logger::PushBlue, "At rate ", pair.mKey, ":");
      pair.mValue.Dump();
   }
}

/// Dump a separator, that depends on AND/OR data container, newline, and     
/// whether it's the first element                                            
void Temporal::DumpSeparator(const Many& data, bool newline, bool& first) {
   if (not first) {
      if (data.IsOr()) {
         if (newline)
            Logger::Verbose(Logger::PushDarkYellow, "or ", Logger::Pop);
         else
            Logger::Append(Logger::PushDarkYellow, " or ", Logger::Pop);
      }
      else {
         Logger::Append(", ");

         if (newline)
            Logger::Verbose("");
      }
   }
   else if (newline) {
      if (data.IsOr())
         Logger::Verbose("   ");
      else
         Logger::Verbose("");
   }

   first = false;
}

/// Inner nested dumper                                                       
///   @return true if stuff was dumped on a new line                          
bool Temporal::DumpInner(const Many& data, bool newline, bool& first) {
   Text serial; data.Serialize(serial);
   const bool tooLong = serial.GetCount() > CarryOverLimit;

   if (data.IsDeep()) {
      // Nest                                                           
      data.ForEach([&](const Many& group) {
         if (group.IsOr()) {
            DumpSeparator(data, newline or tooLong, first);
            Logger::Append(Logger::PushDarkYellow, '(', Logger::Pop);
            if (tooLong)
               Logger::Append(Logger::Tab);

            bool inner = true;
            group.ForEachElement([&](const Many& subgroup) {
               DumpSeparator(group, tooLong, inner);
               bool unused = true;
               DumpInner(subgroup, false, unused);
            });

            if (tooLong) {
               Logger::Append(Logger::Untab);
               Logger::Verbose(Logger::PushDarkYellow, ')', Logger::Pop);
            }
            else Logger::Append(Logger::PushDarkYellow, ')', Logger::Pop);
         }
         else DumpInner(group, newline or tooLong, first);
      });

      return tooLong;
   }

   // Flat if reached                                                   
   // Do some special formatting for specific things                    
   const auto done = data.ForEach(
      [&](const Verb& v) {
         DumpSeparator(data, newline or tooLong, first);
         DumpVerb(v);
      },
      [&](const Construct& c) {
         DumpSeparator(data, newline or tooLong, first);
         DumpConstruct(c);
      },
      [&](const Trait& t) {
         DumpSeparator(data, newline or tooLong, first);
         DumpTrait(t);
      },
      [&](const Inner::MissingFuture& p) {
         if (p.mSuspended) {
            if (p.mContent) {
               bool unused = true;
               DumpSeparator(data, newline or tooLong, first);
               DumpInner(p.mContent, false, unused);
            }
            return;
         }

         // Write a missing future linking point                        
         DumpSeparator(data, newline or tooLong, first);

         if (p.mContent) {
            bool unused = true;
            DumpInner(p.mContent, false, unused);
            DumpSeparator(data, newline or tooLong, first);
         }

         DumpMissing(p);
      },
      [&](const Inner::MissingPast& p) {
         // Write a missing past linking point                          
         DumpSeparator(data, newline or tooLong, first);

         if (p.mContent) {
            bool unused = true;
            DumpInner(p.mContent, false, unused);
            DumpSeparator(data, newline or tooLong, first);
         }

         DumpMissing(p);
      },
      [&](const AMeta& meta) {
         // Write meta definition                                       
         DumpSeparator(data, newline or tooLong, first);

         auto dmeta = static_cast<DMeta>(meta);
         if (dmeta) {
            Logger::Append(dmeta);
            return;
         }

         auto tmeta = static_cast<TMeta>(meta);
         if (tmeta) {
            Logger::Append(Logger::PushPurple, tmeta, Logger::Pop);
            return;
         }

         auto cmeta = static_cast<CMeta>(meta);
         if (cmeta) {
            Logger::Append(Logger::PushYellow, cmeta, Logger::Pop);
            return;
         }

         auto vmeta = static_cast<VMeta>(meta);
         if (vmeta) {
            Logger::Append(Logger::PushBlue, vmeta, Logger::Pop);
            return;
         }
      },
      [&](const Code& p) {
         // Write code                                                  
         DumpSeparator(data, newline or tooLong, first);
         Logger::Append(Logger::PushDarkCyan, Logger::Invert, '{', p, '}', Logger::Pop);
      },
      [&](const Text& p) {
         // Write text literal                                          
         DumpSeparator(data, newline or tooLong, first);
         Logger::Append(Logger::PushRed, '`', p, '`', Logger::Pop);
      }
   );

   // Just dump anything else                                           
   if (not done) {
      if (data.IsMissing()) {
         DumpSeparator(data, newline or tooLong, first);

         if (data.IsPast())
            Logger::Append(Logger::PushYellow, data, Logger::Pop);
         else
            Logger::Append(Logger::PushGreen, data, Logger::Pop);
      }
      else {
         data.ForEachElement([&](const Many& element) {
            DumpSeparator(data, newline or tooLong, first);
            Logger::Append(element);
         });
      }
   }

   return tooLong;
}

/// Dump a missing point as a hexxed address                                  
///   @param p - the missing point to dump                                    
void Temporal::DumpMissing(const Inner::Missing& p) {
   const auto color = p.mFilter.IsPast() ? Logger::PushDarkYellow : Logger::PushDarkGreen;

   if (p.mPriority) {
      Logger::Append(color, Logger::Invert, '(', Logger::Hex(&p), ' ', p.mFilter);
      if (p.mPriority)
         Logger::Append(" !", p.mPriority);
      Logger::Append(')', Logger::Pop);
   }
   else Logger::Append(color, Logger::Invert, '(', Logger::Hex(&p), ' ', p.mFilter, ')', Logger::Pop);
}

/// Dump a verb                                                               
///   @param v - the verb to dump                                             
void Temporal::DumpVerb(const A::Verb& v) {
   using Rules = typename Text::SerializationRules;

   // Can we fit the verb on a single line?                             
   const auto serv = static_cast<Text>(v);
   const auto separated = serv.GetCount() > CarryOverLimit;

   if (v.IsDone() and v.GetOutput()) {
      // If verb has been executed with output, just dump the           
      // output                                                         
      bool unused = true;
      DumpInner(v.GetOutput(), false, unused);
      return;
   }

   // If reached, then verb hasn't been executed yet                    
   // Let's check if there's a source in which verb is executed         
   if (v.GetSource().IsValid()) {
      Text srcScope;
      if (Rules::BeginScope(v.GetSource(), srcScope))
         Logger::Append(Logger::PushBlue, srcScope, Logger::Pop);
      else if (separated)
         Logger::Append(Logger::PushBlue, '(', Logger::Pop, Logger::Tab);

      bool unused = true;
      DumpInner(v.GetSource(), separated, unused);

      srcScope.Clear();
      if (Rules::EndScope(v.GetSource(), srcScope))
         Logger::Append(Logger::PushBlue, srcScope, Logger::Pop);
      else if (separated) {
         Logger::Append(Logger::Untab);
         Logger::Verbose(Logger::PushBlue, ')', Logger::Pop);
      }
   }

   // After the source, we decide whether to write verb token or        
   // verb operator, depending on the verb definition, state and        
   // charge                                                            
   bool writtenAsToken = false;
   const auto token = v.GetOperatorToken(writtenAsToken);
   if (writtenAsToken and v.GetSource().IsValid())
      Logger::Append(' ');

   Logger::Append(Logger::PushBlue, token, Logger::Pop);

   if (not v.GetArgument().IsValid())
      return;

   if (writtenAsToken and v.GetArgument().IsValid())
      Logger::Append(' ');

   Text argScope;
   if (Rules::BeginScope(v.GetArgument(), argScope))
      Logger::Append(Logger::PushBlue, argScope, Logger::Pop);
   else if (separated)
      Logger::Append(Logger::PushBlue, '(', Logger::Pop, Logger::Tab);

   bool unused = true;
   DumpInner(v.GetArgument(), separated, unused);

   argScope.Clear();
   if (Rules::EndScope(v.GetArgument(), argScope))
      Logger::Append(Logger::PushBlue, argScope, Logger::Pop);
   else if (separated) {
      Logger::Append(Logger::Untab);
      Logger::Verbose(Logger::PushBlue, ')', Logger::Pop);
   }
}

/// Dump a construct                                                          
///   @param c - the construct to dump                                        
void Temporal::DumpConstruct(const Construct& c) {
   // Can we fit the construct on a single line?                        
   const auto serv = Verbs::Interpret::To<Anyness::Text>(c);
   const auto separated = serv.GetCount() > CarryOverLimit;

   // Write the type, charge, and open the scope                        
   Logger::Append(c.GetType(), c.GetCharge(), '(');
   if (separated)
      Logger::Append(Logger::Tab);

   // Write the descriptor contents                                     
   bool unused = true;
   DumpInner(c.GetDescriptor(), separated, unused);

   // Close the scope                                                   
   if (separated) {
      Logger::Append(Logger::Untab);
      Logger::Verbose(')');
   }
   else Logger::Append(')');
}

/// Dump a trait                                                              
///   @param t - the trait to dump                                            
void Temporal::DumpTrait(const Trait& t) {
   // Can we fit the trait on a single line?                            
   const auto serv = Verbs::Interpret::To<Anyness::Text>(t);
   const auto separated = serv.GetCount() > CarryOverLimit;

   // Write the trait and open the scope                                
   Logger::Append(Logger::PushPurple, t.GetTrait(), '(', Logger::Pop);
   if (separated)
      Logger::Append(Logger::Tab);

   // Write the trait contents                                          
   bool unused = true;
   DumpInner(static_cast<const Many&>(t), separated, unused);

   // Close the scope                                                   
   if (separated) {
      Logger::Append(Logger::Untab);
      Logger::Verbose(Logger::PushPurple, ')', Logger::Pop);
   }
   else Logger::Append(Logger::PushPurple, ')', Logger::Pop);
}