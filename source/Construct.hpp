///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"

namespace Langulus::Flow
{

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


   ///                                                                        
   ///   CONSTRUCT                                                            
   ///                                                                        
   ///   Useful to describe complex (non-pod) content construction. This      
   /// applies to any domain - units, data, sound, geometry, materials,       
   /// textures, entity hierarchies, etc.                                     
   ///   It is essentially the instructions required to generate the          
   /// content - usually arguments to a creation verb. First the required     
   /// components from the required modules, then the traits that determine   
   /// various characteristics, to finally the additional raw data in case    
   /// of very specific custom contents.                                      
   ///                                                                        
   class Construct : public Any, public Charge {
      LANGULUS(POD) false;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS(DEEP) false;
      LANGULUS_CONVERSIONS(Code, Debug);
      LANGULUS_BASES(Any, Charge);

   private:
      DMeta mType {};
      mutable Hash mHash {};

   public:
      constexpr Construct() noexcept = default;
      Construct(const Construct&) noexcept;
      Construct(Construct&&) noexcept;
      template<CT::Semantic S>
      Construct(S&&) requires (CT::Exact<TypeOf<S>, Construct>);

      Construct(DMeta);

      template<CT::NotSemantic T = Any>
      Construct(DMeta, const T&, const Charge& = {});
      template<CT::NotSemantic T = Any>
      Construct(DMeta, T&, const Charge& = {});
      template<CT::NotSemantic T = Any>
      Construct(DMeta, T&&, const Charge& = {});

      template<CT::Semantic S>
      Construct(DMeta, S&&, const Charge& = {});

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         Construct(const Token&);
         template<CT::NotSemantic T = Any>
         Construct(const Token&, const T&, const Charge& = {});
         template<CT::NotSemantic T = Any>
         Construct(const Token&, T&, const Charge& = {});
         template<CT::NotSemantic T = Any>
         Construct(const Token&, T&&, const Charge& = {});

         template<CT::Semantic S>
         Construct(const Token&, S&&, const Charge& = {});
      #endif

      Construct& operator = (const Construct&) noexcept;
      Construct& operator = (Construct&&) noexcept;
      template<CT::Semantic S>
      Construct& operator = (S&&) requires (CT::Exact<TypeOf<S>, Construct>);

      NOD() explicit operator Code() const;
      NOD() explicit operator Debug() const;

   public:
      NOD() Hash GetHash() const;

      template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
      NOD() static Construct From(HEAD&&, TAIL&&...);
      template<CT::Data T>
      NOD() static Construct From();

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         template<CT::Data HEAD, CT::Data... TAIL>
         NOD() static Construct FromToken(const Token&, HEAD&&, TAIL&&...);
         NOD() static Construct FromToken(const Token&);
      #endif

   private:
      // Omit these inherited from Any                                  
      using Any::FromMeta;
      using Any::FromBlock;
      using Any::FromState;

   public:
      NOD() bool operator == (const Construct&) const;

      NOD() bool StaticCreation(Any&) const;

      NOD() bool CastsTo(DMeta type) const;
      template<CT::Data T>
      NOD() bool CastsTo() const;

      NOD() bool Is(DMeta) const;
      template<CT::Data T>
      NOD() bool Is() const;

      NOD() const Any& GetArgument() const noexcept;
      NOD() Any& GetArgument() noexcept;

      NOD() const Charge& GetCharge() const noexcept;
      NOD() Charge& GetCharge() noexcept;

      NOD() DMeta GetType() const noexcept;
      NOD() Token GetToken() const noexcept;
      NOD() DMeta GetProducer() const noexcept;

      void Clear();
      void ResetCharge() noexcept;

      template<CT::Data T>
      Construct& operator << (const T&);
      template<CT::Data T>
      Construct& operator << (T&&);
      template<CT::Data T>
      Construct& operator >> (const T&);
      template<CT::Data T>
      Construct& operator >> (T&&);

      template<CT::Data T>
      Construct& operator <<= (const T&);
      template<CT::Data T>
      Construct& operator <<= (T&&);
      template<CT::Data T>
      Construct& operator >>= (const T&);
      template<CT::Data T>
      Construct& operator >>= (T&&);

      Construct& Set(const Trait&, const Offset& = 0);
      const Trait* Get(TMeta, const Offset& = 0) const;

      template<CT::Trait T>
      const Trait* Get(const Offset& = 0) const;
   };

} // namespace Langulus::Flow

#include "Construct.inl"
