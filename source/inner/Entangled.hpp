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

   ///                                                                        
   ///   Entangled contents                                                   
   ///                                                                        
   ///   When an OR scope is pushed into a temporal flow, each branch gets    
   /// inserted into future points. However, some insertions my end up in     
   /// completely different places inside the flow - one might end on the     
   /// proprity stack, while another in the frequency stack for example.      
   /// Since memory locality is destroyed by this separation (branches end    
   /// up in separate blocks), we must keep track of when a branch has been   
   /// satisfied, so that the rest are ignored regardless where they end up.  
   /// This is achieved by pushing branch contents into an Entangled element, 
   /// that has a reference to a shared bool flag.                            
   ///                                                                        
   struct Entangled {
      // A reference to a shared boolean flag                           
      Ref<bool> mDone;
      // The actual contents of the branch                              
      Many mActiveContent;
      // Fallback contents for when branch is no longer active          
      Many mPassiveContent;

      LANGULUS_CONVERTS_TO(Text);

      /// Construct an entangled branch                                       
      ///   @param done - a reference to the shared completion flag           
      ///   @param active - active branch contents                            
      ///   @param passive - passive branch contents                          
      Entangled(bool* done, Many&& active, Many&& passive)
         : mDone           {done}
         , mActiveContent  {Forward<Many>(active)}
         , mPassiveContent {Forward<Many>(passive)} {
         LANGULUS_ASSUME(DevAssumes, done, "Invalid entanglement handle");
      }

      /// Just stringify the contents                                         
      explicit operator Text() const {
         Text result;
         if (IsActive())
            mActiveContent.Serialize(result);
         else
            mPassiveContent.Serialize(result);
         return result;
      }

      /// Check if the branch is still active (if it must be executed)        
      ///   @return true if branch has to be executed                         
      bool IsActive() const noexcept {
         return not *mDone;
      }

      /// Get the branch contents                                             
      ///   @return the contents                                              
      auto GetContent() const noexcept -> const Many& {
         return IsActive() ? mActiveContent : mPassiveContent;
      }
   };

} // namespace Langulus::Flow::Inner