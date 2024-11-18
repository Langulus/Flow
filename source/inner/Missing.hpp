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

   struct MissingPast;
   struct MissingFuture;
   using Pasts   = TMany<MissingPast*>;
   using Futures = TMany<MissingFuture*>;


   ///                                                                        
   ///   A missing point inside a flow                                        
   ///                                                                        
   struct Missing {
      LANGULUS_CONVERTS_TO(Text);

      TMany<DMeta> mFilter;
      Many mContent;
      Real mPriority {0};

      Missing() = default;
      explicit Missing(Missing&&) = default;
      explicit Missing(const Missing&) = default;
      explicit Missing(const TMany<DMeta>&, Real priority = 0);
      explicit Missing(const Many&,         Real priority = 0);

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
   };

} // namespace Langulus::Flow::Inner