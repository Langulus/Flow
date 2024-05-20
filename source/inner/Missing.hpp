///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "../Verb.hpp"


namespace Langulus::Flow::Inner
{

   constexpr auto NoPriority = Charge::MaxPriority;
   struct MissingPast;
   struct MissingFuture;
   using Pasts = TMany<MissingPast*>;
   using Futures = TMany<MissingFuture*>;


   ///                                                                        
   ///   A missing point inside a flow                                        
   ///                                                                        
   struct Missing {
      LANGULUS_CONVERTS_TO(Text);

      TMany<DMeta> mFilter;
      Many mContent;
      Real mPriority {NoPriority};

      Missing() = default;
      Missing(const TMany<DMeta>&, Real priority);
      Missing(const Block<>&, Real priority);

      NOD() bool Accepts(const Block<>&) const;
      NOD() bool IsSatisfied() const;

      bool Push(const Many&/*, const Block<>& environment*/);
      Many Link(const Block<>&, const Block<>& environment, bool& consumedPast) const;
      Many Link(const Neat&, const Block<>& environment, bool& consumedPast) const;
      Many Collapse() const;

      // Needs to be implicit, so that its inherited                    
      operator Text() const;
   };


   ///                                                                        
   ///   A missing past point inside a flow                                   
   ///                                                                        
   struct MissingPast : Missing {
      LANGULUS_BASES(Missing);
      using Missing::Missing;
      MissingPast();
   };


   ///                                                                        
   ///   A missing future point inside a flow                                 
   ///                                                                        
   struct MissingFuture : Missing {
      LANGULUS_BASES(Missing);
      using Missing::Missing;
      MissingFuture();
      static Futures Find(const Block<>& scope, const Real priority, Real& priorityFeedback);
   };

} // namespace Langulus::Flow::Inner