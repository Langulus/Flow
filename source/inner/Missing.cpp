///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Missing.hpp"
#include "Redundant.hpp"
#include "../Temporal.hpp"
#include "../verbs/Do.inl"
#include "../verbs/Interpret.inl"

#if 1
   #define VERBOSE_MISSING_ENABLED() 1
   #define VERBOSE_MISSING_POINT(...)     Logger::Verbose(__VA_ARGS__)
   #define VERBOSE_MISSING_POINT_TAB(...) const auto tabs = Logger::VerboseTab(__VA_ARGS__)
   #define VERBOSE_FUTURE(...)            Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_MISSING_ENABLED() 0
   #define VERBOSE_MISSING_POINT(...)     LANGULUS(NOOP)
   #define VERBOSE_MISSING_POINT_TAB(...) LANGULUS(NOOP)
   #define VERBOSE_FUTURE(...)            LANGULUS(NOOP)
#endif

using namespace Langulus::Anyness;
using namespace Langulus::Flow::Inner;


/// Initialize a missing point by a precompiled filter                        
///   @param above - the missing point above this one                         
///   @param filter - the filter to set                                       
///   @param priority - the precedence of the point                           
Missing::Missing(Inner::Missing* above, const TMany<DMeta>& filter, Real priority)
   : mFilter   {filter}
   , mPriority {priority}
   , mAbove    {above} {}

/// Initialize a missing point by a filter, will be precompiled               
/// i.e. all meta data definitions will be gathered                           
///   @param above - the missing point above this one                         
///   @param filter - the filter to set                                       
///   @param priority - the precedence of the point                           
Missing::Missing(Inner::Missing* above, const Many& filter, Real priority)
   : mPriority {priority}
   , mAbove    {above} {
   mFilter.GatherFrom(filter, DataState::Missing);
   mFilter.SetState(filter.GetState());
}

/// Check if immediate contents are accepted by the filter of this point      
/// Verbs are always accepted                                                 
///   @param content - the content to check                                   
///   @return true if contents are accepted                                   
bool Missing::Accepts(const Many& content) const {
   if (not mFilter or content.CastsTo<Verb, true>())
      return true;

   for (auto type : mFilter) {
      if (content.template CastsToMeta<true>(type))
         return true;
   }

   return false;
}

/// Check if the missing point has been satisfied by pushed contents          
///   @return true if point was satisfied                                     
bool Missing::IsSatisfied() const {
   if (not mContent or not mFilter)
      return false;

   bool satisfied = false;
   mContent.ForEachDeep([&](const Many& b) {
      if (Accepts(b))
         satisfied = true;
      return not satisfied;
   });

   return satisfied;
}

/// Insert data into a past point                                             
///   @param content - the content to push                                    
///   @return true if mContent changed                                        
void MissingPast::FillPast(const Many& content) {
   if (not content) {
      #if VERBOSE_MISSING_ENABLED()
         Logger::Error("Can't push empty content");
      #endif
      LANGULUS_THROW(Link, "Can't push empty content");
   }

   if (content.IsDeep()) {
      // Always nest deep contents, we must filter each part and        
      // make sure branches are correctly inserted in forks             
      if (content.IsOr()) {
         // We're building a fork, we should take special care to       
         // preserve the hierarchy of the branches                      
         MissingPast fork {this, mFilter, mPriority};
         fork.mContent.MakeOr();
            
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            try {
               fork.FillPast(subcontent);
               atLeastOneSuccess = true;
            }
            catch (...) {}
         });

         if (not atLeastOneSuccess)
            LANGULUS_THROW(Link, "All branches failed the push");

         mContent.SmartPush(IndexBack, Abandon(fork.mContent));
      }
      else {
         // Just nest-push                                              
         content.ForEach([&](const Many& subcontent) {
            FillPast(subcontent);
         });
      }
      return;
   }
   else if (content.Is<Inner::Redundant>()) {
      // Redundant data serves only the purpose of filling past         
      // and acts as a deep container                                   
      content.ForEach([&](const Inner::Redundant& subcontent) {
         FillPast(subcontent.mContent);
      });
   }

   //                                                                   
   // If reached, we're pushing flat data                               
   // A handy lambda to commit any changes to the current mContent      
   const auto commit = [&](const Many& a) {
      if (a.IsSparse())
         mContent <<= a;               // Avoid duplications if sparse  
      else
         mContent << a;
   };

   if (mFilter) {
      // Filters are available, interpret source as requested           
      Verbs::Interpret interpreter {mFilter};
      if (DispatchDeep(content, interpreter)) {
         auto& output = interpreter.GetOutput();
         VERBOSE_MISSING_POINT("Satisfying filter by interpreting ", content, " as ", output);
         commit(output);
      }
   }
   //else commit(content);
}

/// Insert data into a future point. Any missing past points inside 'content' 
/// will be filled from either what's currently in this future point, or from 
/// contents of the future points above this one                              
///   @attention assumes 'content' has been Temporal::Compiled previously     
///   @param content - the content to push                                    
///   @return true if mContent changed                                        
void MissingFuture::FillFuture(const Many& content) {
   if (not content) {
      #if VERBOSE_MISSING_ENABLED()
         Logger::Error("Can't push empty content");
      #endif
      LANGULUS_THROW(Link, "Can't push empty content");
   }

   if (content.IsDeep()) {
      // Always nest deep contents, we must filter each part and        
      // make sure branches are correctly inserted in forks             
      if (content.IsOr() and content.IsDense()) {
         // We're building a fork, we should take special care to       
         // preserve the hierarchy of the branches                      
         MissingFuture fork {this, mFilter, mPriority};
         fork.mContent.MakeOr();
            
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            try {
               fork.FillFuture(subcontent);
               atLeastOneSuccess = true;
            }
            catch (...) {}
         });

         if (not atLeastOneSuccess)
            LANGULUS_THROW(Link, "All branches failed the push");

         mContent.SmartPush(IndexBack, Abandon(fork.mContent));
      }
      else if (content.IsDense()) {
         // Just nest-push                                              
         content.ForEach([&](const Many& subcontent) {
            FillFuture(subcontent);
         });
      }
      else {
         // Sparse blocks are always inserted as-is, and never repeated 
         // They are never linked, so not to affect contents outside    
         // this flow. This makes the flow impure, because it can be    
         // affected from the outside.                                  
         bool atLeastOneSuccess = false;
         content.ForEach([&](const Many& subcontent) {
            if (not mFilter) {
               mContent <<= &subcontent;
               atLeastOneSuccess = true;
            }
            else if (subcontent.GetType()) {
               for (const auto& type : mFilter) {
                  if (not subcontent.GetType()->CastsTo<false>(type))
                     continue;

                  mContent <<= &subcontent;
                  atLeastOneSuccess = true;
                  break;
               }
            }
         });

         if (not atLeastOneSuccess)
            LANGULUS_THROW(Link, "Nothing was pushed");
      }

      // Contents were modified, remap futures below                    
      Inner::Missing::RemapFutures(*this, mContent);
      return;
   }

   //                                                                   
   // If reached, we're pushing flat data                               
   // Fill any missing past points in the contents we're filling with   
   // If past fails to be satisfied with the current context, move      
   // to the one above and repeat until satisfied or nothing left above 
   Many linked;
   const MissingFuture* context = this;
   while (context) {
      try {
         linked = Link(content, *context);
         break;
      }
      catch (...) {
         context = static_cast<const MissingFuture*>(context->mAbove);
         continue;
      }
   }

   if (not context) {
      #if VERBOSE_MISSING_ENABLED()
         Logger::Error("None of the hierarchical past was satisfactory");
      #endif
      LANGULUS_THROW(Link, "None of the hierarchical past was satisfactory");
   }

   // A handy lambda to commit any changes to the current mContent      
   const auto commit = [&](Many& a) {
      if (a.IsSparse()) {
         // Avoid duplications if new content is sparse                 
         // No need to remap futures below, because sparse contents     
         // never link with anything                                    
         mContent <<= Abandon(a);
      }
      else mContent << Abandon(a);
   };

   if (mFilter) {
      // Filters are available, interpret contents as requested         
      Verbs::Interpret interpreter {mFilter};
      if (DispatchDeep(linked, interpreter)) {
         auto& output = interpreter.GetOutput();
         VERBOSE_MISSING_POINT("Satisfying filter by interpreting ", linked, " as ", output);
         commit(output);
      }
   }
   else commit(linked);

   // Contents were modified in a way that can introduce new            
   // futures below, so remap those                                     
   Inner::Missing::RemapFutures(*this, mContent);
}

/// Just a helper function for logging                                        
template<class T>
decltype(auto) Missing::VerboseLinking(const T& what, const MissingFuture& context) {
   #if VERBOSE_MISSING_ENABLED()
      if constexpr (CT::Same<T, Trait>) {
         Logger::Verbose("Linking trait ");
         Temporal::DumpTrait(what);
      }
      else if constexpr (CT::Same<T, Construct>) {
         Logger::Verbose("Linking construct ");
         Temporal::DumpConstruct(what);
      }
      else if constexpr (CT::Same<T, A::Verb>) {
         Logger::Verbose("Linking verb ");
         Temporal::DumpVerb(what);
      }
      else if constexpr (CT::DerivedFrom<T, Inner::Missing>) {
         Logger::Verbose("Linking point ");
         Temporal::DumpMissing(what);
      }
      else if constexpr (CT::Same<T, Many>) {
         Logger::Verbose("Linking subscope ");
         bool unused = true;
         Temporal::DumpInner(what, false, unused);
      }
      else static_assert(false, "Unsupported linking");

      Logger::Append(" to point ");
      Temporal::DumpMissing(context);
      return Logger::Append(Logger::Tabs {1});
   #else
      return 0;
   #endif
}

/// Links the missing past points with the provided context                   
///   @attention assumes argument is a valid scope                            
///   @param scope - the scope to link                                        
///   @param context - the future point we're using as past context           
///   @return the linked equivalent to the provided scope                     
Many Missing::Link(const Many& scope, const MissingFuture& context) const {
   Many result;
   if (scope.IsOr())
      result.MakeOr();

   if (scope.IsDeep()) {
      // Nest scopes, linking any past points in subscopes              
      scope.ForEach([&](const Many& subscope) {
         const auto tab = VerboseLinking(subscope, context);
         try { result << Link(subscope, context); }
         catch (const Except::Link&) {
            if (not scope.IsOr())
               throw;
            VERBOSE_MISSING_POINT(Logger::DarkYellow,
               "Skipped branch: ", subscope);
         }
      });

      if (result.GetCount() < 2)
         result.MakeAnd();
      return Abandon(result);
   }

   // Link all missing past points in the provided scope using context  
   const auto found = scope.ForEach(
      [&](const Trait& trait) {
         // Link a trait                                                
         const auto tab = VerboseLinking(trait, context);
         result << Trait::From(trait.GetTrait(), Link(trait, context));
      },
      [&](const Construct& construct) {
         // Link a construct                                            
         const auto tab = VerboseLinking(construct, context);
         result << Construct {
            construct.GetType(), Link(construct.GetDescriptor(), context)
         };
      },
      [&](const A::Verb& verb) {
         // Link a verb                                                 
         const auto tab = VerboseLinking(verb, context);
         auto source = Link(verb.GetSource(), context);
         result << Verb::FromMeta(
            verb.GetVerb(), 
            Link(verb.GetArgument(), context),
            verb.GetCharge(), 
            verb.GetVerbState()
         ).SetSource(Abandon(source));
      },
      [&](const Inner::MissingPast& past) {
         // Replace a missing past point with provided context          
         const auto tab = VerboseLinking(past, context);
         if (mPriority > past.mPriority) {
            #if VERBOSE_MISSING_ENABLED()
               Logger::Error("Skipped because of precedence");
            #endif
            LANGULUS_THROW(Link, "Skipped because of precedence");
         }

         if (past.mFilter) {
            Inner::MissingPast pastShallowCopy;
            pastShallowCopy.mFilter = past.mFilter;
            pastShallowCopy.FillPast(context.mContent);
            result << Abandon(pastShallowCopy.mContent);
         }
         else if (context.mContent) {
            // We're not allowed to modify the context! But we should   
            // modify the precedence, if it happens that the new        
            // contents should completely wrap around the old           
            auto& mutableContext = const_cast<MissingFuture&>(context);
            mutableContext.mPriority = past.mPriority;
            result << mutableContext.mContent;

            // Old contents become redundant                            
            mutableContext.mContent = Inner::Redundant {Move(mutableContext.mContent)};
         }  
         else {
            // Nothing to link with                                     
            #if VERBOSE_MISSING_ENABLED()
               Logger::Error("Nothing to link with - context is empty");
            #endif
            LANGULUS_THROW(Link, "Nothing to link with");
         }
      }
   );

   if (not found)
      result = scope;   // Anything else just gets propagated           
   return Abandon(result);
}

/// Scan for future points below a given context                              
/// If a future point contains other future points below it that are of the   
/// same priority, then it's considered suspended                             
///   @param context - the future point to search below                       
///   @param stack - used for nesting deep contents                           
///   @return the hierarchy of future points below the context                
Many Missing::RemapFutures(MissingFuture& context, const Many& stack) {
   if (&stack == &context.mContent)
      context.mBelow = {};

   if (not stack or stack.IsSparse())
      return {};    // No point in scanning sparse stacks - they're     
                    // never linked with                                

   Many result;
   if (stack.IsOr())
      result.MakeOr();

   if (stack.IsDeep()) {
      // Nest deep stack if dense                                       
      stack.ForEach([&](const Many& substack) {
         auto temp = RemapFutures(context, substack);
         result.SmartPush(stack.IsOr() ? IndexBack : IndexFront, Abandon(temp));
      });

      if (&stack == &context.mContent)
         context.mBelow = result;
      return result;
   }

   // Flat if reached                                                   
   stack.ForEach(
      [&](const Trait& trait) {
         // Nest inside traits                                          
         auto temp = RemapFutures(context, static_cast<const Many&>(trait));
         result.SmartPush(stack.IsOr() ? IndexBack : IndexFront, Abandon(temp));
      },
      [&](const Construct& con) {
         // Nest inside constructs                                      
         auto temp = RemapFutures(context, con.GetDescriptor());
         result.SmartPush(stack.IsOr() ? IndexBack : IndexFront, Abandon(temp));
      },
      [&](const A::Verb& verb) {
         // Nest inside verbs                                           
         Many temp;
         auto temps = RemapFutures(context, verb.GetSource());
         temp.SmartPush(IndexBack, Abandon(temps));
         auto tempa = RemapFutures(context, verb.GetArgument());
         temp.SmartPush(IndexBack, Abandon(tempa));

         result.SmartPush(stack.IsOr() ? IndexBack : IndexFront, Abandon(temp));
      },
      [&](const Inner::MissingFuture& below_const) {
         // Nest/register missing future points                         
         auto& below = const_cast<Inner::MissingFuture&>(below_const);
         auto temp = RemapFutures(below, below.mContent);

         if (below.mPriority == context.mPriority) {
            if (not temp) {
               // No more missing futures below, don't suspend this one 
               below.mSuspended = false;
               below.mAbove = &context;
               result >> &below;
            }
            else {
               // More missing futures below, suspend this one          
               below.mSuspended = true;
               temp.ForEachDeep([&](Inner::MissingFuture& next) {
                  next.mAbove = &context;
               });
               result.SmartPush(stack.IsOr() ? IndexBack : IndexFront, Abandon(temp));
            }
         }
         else {
            // Register the future point                                
            below.mSuspended = false;
            below.mAbove = &context;
            result >> &below;
         }
      }
   );

   if (&stack == &context.mContent)
      context.mBelow = result;
   return result;
}

/// Log the missing point                                                     
Missing::operator Text() const {
   Text result;

   if (mSuspended) {
      mContent.Serialize(result);
      return result;
   }

   if (mPriority or mContent) {
      result += '(';
      mFilter.Serialize(result);

      if (mPriority)
         result += Text {" !", mPriority};

      if (mContent) {
         result += ", ";
         mContent.Serialize(result);
      }

      result += ')';
   }
   else mFilter.Serialize(result);
   return result;
}

/// Default past point                                                        
MissingPast::MissingPast() {
   mFilter.MakePast();
}

/// Default future point                                                      
MissingFuture::MissingFuture() {
   mFilter.MakeFuture();
}
