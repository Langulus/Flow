///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "Langulus/CT/Convertible.hpp"
#include <Langulus/Text.hpp>
#include <Langulus/Many.hpp>
#include <Langulus/TMany.hpp>


namespace Langulus::Flow
{
   using RTTI::DMeta;
   struct Temporal;
   struct MissingPast;
   struct MissingFuture;

   
   ///                                                                        
   ///   A missing point inside a flow                                        
   ///                                                                        
   struct Missing {
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
      Missing* mAbove = nullptr;

      Missing() = default;
      explicit Missing(Missing*, const TMany<DMeta>&, Real priority);
      explicit Missing(Missing*, Many const&,         Real priority);

      bool Accepts(Many const&) const;
      bool IsSatisfied() const;

      Many Link(Many const&, const MissingFuture&) const;

      // Needs to be implicit so that it's inherited                    
      operator Text() const;

      static void RemapFutures(MissingFuture&, Many const&);

   protected:
      template<class T>
      static decltype(auto) VerboseLinking(T const&, const MissingFuture&);
   };


   ///                                                                        
   ///   A missing past point inside a flow                                   
   ///                                                                        
   struct MissingPast : Missing {
      using Missing::Missing;
      MissingPast();

      void FillPast(Many const&);
   };


   ///                                                                        
   ///   A missing future point inside a flow                                 
   ///                                                                        
   struct MissingFuture : Missing {
      using Missing::Missing;
      MissingFuture();

      void FillFuture(Many const&, Temporal&);
      void Commit(Many const&, Temporal&);
   };
}

LANGULUS_MORPHISM(Flow::Missing, Annies::Text);
LANGULUS_MORPHISM(Flow::MissingPast, Annies::Text);
LANGULUS_MORPHISM(Flow::MissingFuture, Annies::Text);
