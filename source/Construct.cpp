///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
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

#define VERBOSE_CONSTRUCT(a) //Logger::Verbose() << a

namespace Langulus::Anyness
{

   /// Attempt to create construct statically if possible                     
   /// If not possible, simply propagate the construct                        
   ///   @param output - [out] results go here                                
   bool Construct::StaticCreation(Any& output) const {
      if (mType->mProducer)
         return false;

      // If reached, data doesn't rely on a producer                    
      // Make sure we're creating something concrete                    
      Verbs::Create creator {this};
      if (Verbs::Create::ExecuteStateless(creator)) {
         VERBOSE_CONSTRUCT("Constructed from initializer-list: "
            << Logger::Cyan << creator.GetOutput());
         output << Move(creator.GetOutput());
         return true;
      }

      return false;
   }

} // namespace Langulus::Anyness
