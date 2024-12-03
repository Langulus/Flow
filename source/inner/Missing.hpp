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

      // A filter for the accepted contents                             
      TMany<DMeta> mFilter;
      // The contents that have been linked to this missing point       
      Many mContent;
      // The priority of the missing point for encoding precedence      
      Real mPriority {0};
      // Future points will be suspended if new future points of same   
      // priority are pushed to the contents                            
      bool mSuspended {false};

      // Missing points under this one (in reversed order, can be OR)   
      Many mBelow;
      // Missing point above this one                                   
      const Inner::Missing* mAbove = nullptr;

      Missing() = default;
      explicit Missing(Inner::Missing*, const TMany<DMeta>&, Real priority);
      explicit Missing(Inner::Missing*, const Many&,         Real priority);

      NOD() bool Accepts(const Many&) const;
      NOD() bool IsSatisfied() const;

      Many Link(const Many&, const MissingFuture&) const;

      // Needs to be implicit so that it's inherited                    
      operator Text() const;

      static Many RemapFutures(MissingFuture&, const Many&);

   protected:
      template<class T>
      static decltype(auto) VerboseLinking(const T&, const MissingFuture&);
   };


   ///                                                                        
   ///   A missing past point inside a flow                                   
   ///                                                                        
   struct MissingPast : Missing {
      LANGULUS_BASES(Missing);
      using Missing::Missing;
      MissingPast();

      void FillPast(const Many&);
   };


   ///                                                                        
   ///   A missing future point inside a flow                                 
   ///                                                                        
   struct MissingFuture : Missing {
      LANGULUS_BASES(Missing);
      using Missing::Missing;
      MissingFuture();

      void FillFuture(const Many&);
   };

} // namespace Langulus::Flow::Inner