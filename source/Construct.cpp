///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
/*#include <Langulus/Verbs/Interpret.inl>
#include <Langulus/Verbs/Do.inl>
#include <Langulus/Verbs/Create.inl>
#include <Langulus/Verbs/Select.inl>
#include <Langulus/Construct.hpp>

#if 0
   #define VERBOSE_CONSTRUCT(...) Logger::Verbose(__VA_ARGS__)
#else
   #define VERBOSE_CONSTRUCT(...) LANGULUS(NOOP)
#endif


namespace Langulus::Annies
{*/

   /// Attempt to create construct statically if possible                     
   /// If not possible, simply propagate the construct                        
   ///   @param output - [out] results go here                                
   //TODO this function might be removed in the future, because its use is 
   //questionable - statically creating constructs might take away the
   //opportunity to create items without producer at runtime. Like, what if
   //some module produces specific Text for example, with some stuff always appended
   //or, like, creating an integer in the context of something that will never allow zero
   //there results should also be probably paired with the producer that made them, so that
   //a solver can differentiate between them, and trust ones that are relevant
   /*bool Construct::StaticCreation(Many& output) const {
      if (mType->mProducerRetriever)
         return false;

      // If reached, data doesn't rely on a producer                    
      // Make sure we're creating something concrete                    
      Verbs::Create creator {this};
      if (Verbs::Create::ExecuteStateless(creator)) {
         VERBOSE_CONSTRUCT("Constructed from initializer-list: ", Logger::Cyan, creator.GetOutput());
         output.SmartPush(IndexBack, Move(creator.GetOutput()));
         return true;
      }

      return false;
   }

}*/ // namespace Langulus::Annies
