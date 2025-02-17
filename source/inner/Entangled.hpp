///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#pragma once
#include "../Temporal.hpp"


namespace Langulus::Flow
{


   ///                                                                        
   ///   Entanglement definition                                              
   ///                                                                        
   struct Temporal::Entanglement {
      Entanglement* mParent = nullptr;
      bool          mDone = false;
   };

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
   /// that has a reference to a shared Entanglement object owned by the flow 
   ///                                                                        
   struct Temporal::Entangled {
      LANGULUS_CONVERTS_TO(Text);

      // A reference to a shared boolean flag                           
      Ref<Entanglement> mDone;
      // Contents when mDone is true                                    
      Many mTrueContent;
      // Contents when mDone is false                                   
      Many mFalseContent;

      /// Construct an entangled branch                                       
      ///   @param done - a reference to the shared completion flag           
      ///   @param ontrue - true path branch contents                         
      ///   @param onfalse - false path branch contents                       
      Entangled(Entanglement* done, Many&& ontrue, Many&& onfalse)
         : mDone           {done}
         , mTrueContent    {Forward<Many>(ontrue)}
         , mFalseContent   {Forward<Many>(onfalse)} {
         LANGULUS_ASSUME(DevAssumes, done, "Invalid entanglement handle");
      }

      /// Just stringify the contents                                         
      explicit operator Text() const {
         Text result;
         if (IsActive())  mTrueContent.Serialize(result);
         else            mFalseContent.Serialize(result);
         return result;
      }

      /// Check if the branch is still active (if it must be executed)        
      ///   @return true if we're on the true path                            
      bool IsActive() const noexcept {
         return mDone->mDone;
      }

      /// Get the branch contents                                             
      ///   @return the contents                                              
      auto GetContent() const noexcept -> const Many& {
         return IsActive() ? mTrueContent : mFalseContent;
      }
   };

} // namespace Langulus::Flow