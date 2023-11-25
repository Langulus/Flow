///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#include "Serial.hpp"
#include "verbs/Interpret.inl"
#include "verbs/Do.inl"
#include "verbs/Create.inl"
#include "verbs/Select.inl"
#include <Anyness/Construct.hpp>

#define VERBOSE_CONSTRUCT(...) //Logger::Verbose(__VA_ARGS__)


namespace Langulus::Anyness
{

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
   bool Construct::StaticCreation(Any& output) const {
      if (mType->mProducerRetriever)
         return false;

      // If reached, data doesn't rely on a producer                    
      // Make sure we're creating something concrete                    
      Verbs::Create creator {this};
      if (Verbs::Create::ExecuteStateless(creator)) {
         VERBOSE_CONSTRUCT("Constructed from initializer-list: ", Logger::Cyan, creator.GetOutput());
         output << Move(creator.GetOutput());
         return true;
      }

      return false;
   }

} // namespace Langulus::Anyness
