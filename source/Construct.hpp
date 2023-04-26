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
      LANGULUS_API(FLOW) Construct(const Construct&) noexcept;
      LANGULUS_API(FLOW) Construct(Construct&&) noexcept;

      template<CT::Semantic S>
      Construct(S&&) requires (CT::Exact<TypeOf<S>, Construct>);

      LANGULUS_API(FLOW) Construct(DMeta);

      template<CT::NotSemantic T = Any>
      Construct(DMeta, const T&, const Charge& = {});
      template<CT::NotSemantic T = Any>
      Construct(DMeta, T&, const Charge& = {});
      template<CT::NotSemantic T = Any>
      Construct(DMeta, T&&, const Charge& = {});

      template<CT::Semantic S>
      Construct(DMeta, S&&, const Charge& = {});

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         LANGULUS_API(FLOW) Construct(const Token&);
         template<CT::NotSemantic T = Any>
         Construct(const Token&, const T&, const Charge& = {});
         template<CT::NotSemantic T = Any>
         Construct(const Token&, T&, const Charge& = {});
         template<CT::NotSemantic T = Any>
         Construct(const Token&, T&&, const Charge& = {});

         template<CT::Semantic S>
         Construct(const Token&, S&&, const Charge& = {});
      #endif

      LANGULUS_API(FLOW) Construct& operator = (const Construct&) noexcept;
      LANGULUS_API(FLOW) Construct& operator = (Construct&&) noexcept;
      template<CT::Semantic S>
      Construct& operator = (S&&) requires (CT::Exact<TypeOf<S>, Construct>);

      NOD() LANGULUS_API(FLOW) explicit operator Code() const;
      NOD() LANGULUS_API(FLOW) explicit operator Debug() const;

   public:
      NOD() LANGULUS_API(FLOW) Hash GetHash() const;

      template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
      NOD() static Construct From(HEAD&&, TAIL&&...);
      template<CT::Data T>
      NOD() static Construct From();

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         template<CT::Data HEAD, CT::Data... TAIL>
         NOD() static Construct FromToken(const Token&, HEAD&&, TAIL&&...);
         NOD() LANGULUS_API(FLOW) static Construct FromToken(const Token&);
      #endif

   private:
      // Omit these inherited from Any                                  
      using Any::FromMeta;
      using Any::FromBlock;
      using Any::FromState;

   public:
      NOD() LANGULUS_API(FLOW) bool operator == (const Construct&) const;

      NOD() LANGULUS_API(FLOW) bool StaticCreation(Any&) const;

      NOD() LANGULUS_API(FLOW) bool CastsTo(DMeta type) const;
      template<CT::Data T>
      NOD() bool CastsTo() const;

      NOD() LANGULUS_API(FLOW) bool Is(DMeta) const;
      template<CT::Data T>
      NOD() bool Is() const;

      NOD() LANGULUS_API(FLOW) const Any& GetArgument() const noexcept;
      NOD() LANGULUS_API(FLOW) Any& GetArgument() noexcept;

      NOD() LANGULUS_API(FLOW) const Charge& GetCharge() const noexcept;
      NOD() LANGULUS_API(FLOW) Charge& GetCharge() noexcept;

      NOD() LANGULUS_API(FLOW) DMeta GetType() const noexcept;
      NOD() LANGULUS_API(FLOW) Token GetToken() const noexcept;
      NOD() LANGULUS_API(FLOW) DMeta GetProducer() const noexcept;

      LANGULUS_API(FLOW) void Clear();
      LANGULUS_API(FLOW) void ResetCharge() noexcept;

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

      LANGULUS_API(FLOW) Construct& Set(const Trait&, const Offset& = 0);
      LANGULUS_API(FLOW) const Trait* Get(TMeta, const Offset& = 0) const;

      template<CT::Trait T>
      const Trait* Get(const Offset& = 0) const;
   };

} // namespace Langulus::Flow

#include "Construct.inl"
