///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
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
      Missing(const Many&,         Real priority);

      NOD() bool Accepts(const Many&) const;
      NOD() bool IsSatisfied() const;

      bool Push(const Many&);
      Many Link(const Many&, const Missing&) const;
      //Many Link(const Neat&, const Missing&) const;
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
      static Futures Find(const Many& scope, const Real priority, Real& priorityFeedback);
   };

} // namespace Langulus::Flow::Inner