///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include <Core/Exceptions.hpp>
#include <RTTI/MetaData.hpp>
#include <Anyness/Trait.hpp>
#include <Anyness/Text.hpp>

#if defined(LANGULUS_EXPORT_ALL) || defined(LANGULUS_EXPORT_FLOW)
   #define LANGULUS_API_FLOW() LANGULUS_EXPORT()
#else
   #define LANGULUS_API_FLOW() LANGULUS_IMPORT()
#endif

LANGULUS_EXCEPTION(Flow);
LANGULUS_EXCEPTION(Link);

namespace Langulus::Flow
{

   using namespace Anyness;
   using RTTI::VMeta;
   using RTTI::TMeta;
   using RTTI::DMeta;
   using RTTI::CMeta;
   using RTTI::MetaData;
   using RTTI::MetaVerb;
   using RTTI::MetaTrait;
   using RTTI::MetaConst;


   ///                                                                        
   ///   Bits for seek functions                                              
   ///                                                                        
   enum class Seek : uint8_t {
      // Seek entities that are children of the context                 
      Below = 1,
      // Seek entities that are parents of the context                  
      Above = 2,
      // Seek objects in both directions - in parents and children      
      Duplex = Below | Above,
      // Include the current entity in the seek operation               
      Here = 4,
      // Seek everywhere                                                
      Everywhere = Duplex | Here,
      // Seek parents and this context included                         
      HereAndAbove = Above | Here,
      // Seek children and this context included                        
      HereAndBelow = Below | Here
   };

   constexpr bool operator & (const Seek& lhs, const Seek& rhs) {
      return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
   }

   class Charge;
   class Construct;
   struct Code;
   class Verb;
   struct Resolvable;
   class Temporal;
   struct Scope;

   
   ///                                                                        
   ///   Charge, carrying the four verb dimensions                            
   ///                                                                        
   class Charge {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) false;

      // Mass of the verb                                               
      Real mMass = DefaultMass;
      // Frequency of the verb                                          
      Real mRate = DefaultRate;
      // Time of the verb                                               
      Real mTime = DefaultTime;
      // Priority of the verb                                           
      Real mPriority = DefaultPriority;

   public:
      static constexpr Real DefaultMass {1};
      static constexpr Real DefaultRate {0};
      static constexpr Real DefaultTime {0};

      static constexpr Real DefaultPriority {0};
      static constexpr Real MinPriority {-10000};
      static constexpr Real MaxPriority {+10000};

      constexpr Charge(
         Real = DefaultMass,
         Real = DefaultRate,
         Real = DefaultTime,
         Real = DefaultPriority
      ) noexcept;

      NOD() constexpr bool operator == (const Charge&) const noexcept;

      NOD() constexpr Charge operator * (const Real&) const noexcept;
      NOD() constexpr Charge operator ^ (const Real&) const noexcept;

      NOD() constexpr Charge& operator *= (const Real&) noexcept;
      NOD() constexpr Charge& operator ^= (const Real&) noexcept;

      NOD() constexpr bool IsDefault() const noexcept;
      NOD() constexpr bool IsFlowDependent() const noexcept;
      NOD() Hash GetHash() const noexcept;
      void Reset() noexcept;

      NOD() explicit operator Code() const;
      NOD() explicit operator Debug() const;
   };

} // namespace Langulus::Flow

LANGULUS_DEFINE_TRAIT(Mass,
   "Mass of anything with charge, amplitude, or literally physical mass");
LANGULUS_DEFINE_TRAIT(Rate,
   "Rate of anything with charge, or with physical frequency");
LANGULUS_DEFINE_TRAIT(Time,
   "Time of anything with charge, or with a temporal component");
LANGULUS_DEFINE_TRAIT(Priority,
   "Priority of anything with charge, or some kind of priority");

/// Make the rest of the code aware, that Langulus::Flow has been included    
#define LANGULUS_LIBRARY_FLOW() 1
