///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Code.hpp"


namespace Langulus::CT
{

   /// A serializer is any Block type, that has an inner type called          
   /// SerializationRules, which holds instructions on how data is assembled  
   template<class...T>
   concept Serial = ((Block<T...>
       and requires { typename T::SerializationRules; }) and ...);

} // namespace Langulus::CT


namespace Langulus::Anyness
{

   template<bool HEADER = true>
   NOD() Count Serialize(const CT::Decayed auto&, CT::Serial auto&);

   template<bool HEADER = true>
   NOD() Count Deserialize(const CT::Serial auto&, CT::Decayed auto&);

} // namespace Langulus::Anyness