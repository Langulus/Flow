///                                                                           
/// Langulus::Flow                                                            
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>                    
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Construct.hpp"

namespace Langulus::Flow
{
   
   ///                                                                        
   ///   Verb state flags                                                     
   ///                                                                        
   struct VerbState {
      LANGULUS(POD) true;
      LANGULUS(NULLIFIABLE) true;

      enum Enum : ::std::uint8_t {
         // Default verb state                                          
         // Default state is short-circuited multicast                  
         Default = 0,

         // When verb is long-circuited (as oposed to short-circuited), 
         // it will not cease executing on success, and be executed for 
         // each element in the context if multicasted. Used usually in 
         // interpretation, when you want to guarantee all elements are 
         // converted                                                   
         LongCircuited = 1,

         // When verb is monocast (as opposite to multicast), it will   
         // not iterate deep items, but be executed on the context once 
         // as a whole. Used extensively when executing at compile-time 
         Monocast = 2
      };

      using Type = TypeOf<Enum>;

      Type mState {Default};

   public:
      constexpr VerbState() noexcept = default;
      constexpr VerbState(const Type&) noexcept;

      explicit constexpr operator bool() const noexcept;
      constexpr bool operator == (const VerbState&) const noexcept = default;
      
      NOD() constexpr VerbState operator + (const VerbState&) const noexcept;
      NOD() constexpr VerbState operator - (const VerbState&) const noexcept;
      constexpr VerbState& operator += (const VerbState&) noexcept;
      constexpr VerbState& operator -= (const VerbState&) noexcept;
      
      NOD() constexpr bool operator & (const VerbState&) const noexcept;
      NOD() constexpr bool operator % (const VerbState&) const noexcept;
      
      NOD() constexpr bool IsDefault() const noexcept;
      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;
      
      constexpr void Reset() noexcept;
   };


   ///                                                                        
   ///   THE UNIVERSAL VERB                                                   
   ///                                                                        
   /// It's practically a single call to the framework, or a single statement 
   /// in a code flow. Langulus is based around natural language processing   
   /// theory based around verbs, so this is the natural name for such thing  
   ///                                                                        
   class Verb : public Any, public Charge {
      LANGULUS(POD) false;
      LANGULUS(NULLIFIABLE) false;
      LANGULUS(DEEP) false;
      LANGULUS_CONVERSIONS(Code, Debug);
      LANGULUS_BASES(Any, Charge);

   protected:
      friend struct Scope;

      // Verb meta, mass, frequency, time and priority                  
      VMeta mVerb {};
      // The number of successful executions                            
      Count mSuccesses {};
      // Verb short-circuiting                                          
      VerbState mState {};
      // Verb context                                                   
      Any mSource;
      // The container where output goes                                
      Any mOutput;

   public:
      constexpr Verb() noexcept = default;

      LANGULUS_API(FLOW) Verb(const Verb&);
      LANGULUS_API(FLOW) Verb(Verb&&);

      Verb(const CT::NotSemantic auto&);
      Verb(CT::NotSemantic auto&);
      Verb(CT::NotSemantic auto&&);

      template<CT::Semantic S>
      Verb(S&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      Verb(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      LANGULUS_API(FLOW) ~Verb();

      LANGULUS_API(FLOW) Verb& operator = (const Verb&);
      LANGULUS_API(FLOW) Verb& operator = (Verb&&);
      template<CT::Semantic S>
      Verb& operator = (S&&);

      LANGULUS_API(FLOW) Verb operator * (const Real&) const;
      LANGULUS_API(FLOW) Verb operator ^ (const Real&) const;

      LANGULUS_API(FLOW) Verb& operator *= (const Real&) noexcept;
      LANGULUS_API(FLOW) Verb& operator ^= (const Real&) noexcept;

      NOD() LANGULUS_API(FLOW)
      explicit operator Code() const;

      NOD() LANGULUS_API(FLOW)
      explicit operator Debug() const;

      template<CT::Dense T>
      bool GenericAvailableFor() const noexcept;
      template<CT::Dense T, CT::Data V>
      static bool GenericExecuteIn(T&, V&);
      template<CT::Data V>
      static bool GenericExecuteDefault(const Block&, V&);
      template<CT::Data V>
      static bool GenericExecuteDefault(Block&, V&);
      template<CT::Data V>
      static bool GenericExecuteStateless(V&);

   public:
      template<CT::Data VERB>
      NOD() static Verb From(const Charge& = {}, const VerbState& = {});
      template<CT::Data VERB, CT::Data DATA>
      NOD() static Verb From(DATA&&, const Charge& = {}, const VerbState& = {});
      template<CT::Data DATA>
      NOD() static Verb FromMeta(VMeta, DATA&&, const Charge& = {}, const VerbState& = {});

      NOD() LANGULUS_API(FLOW)
      static Verb FromMeta(VMeta, const Charge& = {}, const VerbState& = {});

      NOD() LANGULUS_API(FLOW)
      Hash GetHash() const;

      NOD() LANGULUS_API(FLOW)
      Verb PartialCopy() const noexcept;

      LANGULUS_API(FLOW) void Reset();

      NOD() LANGULUS_API(FLOW)
      bool VerbIs(VMeta) const noexcept;

      template<CT::Data... T>
      NOD() bool VerbIs() const noexcept;

      NOD() LANGULUS_API(FLOW) const Charge& GetCharge() const noexcept;
      NOD() LANGULUS_API(FLOW) VMeta GetVerb() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetMass() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetFrequency() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetTime() const noexcept;
      NOD() LANGULUS_API(FLOW) Real GetPriority() const noexcept;

      NOD() LANGULUS_API(FLOW) Any& GetSource() noexcept;
      NOD() LANGULUS_API(FLOW) const Any& GetSource() const noexcept;
      NOD() LANGULUS_API(FLOW) Any& GetArgument() noexcept;
      NOD() LANGULUS_API(FLOW) const Any& GetArgument() const noexcept;
      NOD() LANGULUS_API(FLOW) Any& GetOutput() noexcept;
      NOD() LANGULUS_API(FLOW) const Any& GetOutput() const noexcept;

      NOD() LANGULUS_API(FLOW) bool Validate(const Index&) const noexcept;
      LANGULUS_API(FLOW) Verb& ShortCircuit(bool) noexcept;
      LANGULUS_API(FLOW) Verb& Multicast(bool) noexcept;
      LANGULUS_API(FLOW) Verb& SetVerbState(const VerbState&) noexcept;
      NOD() LANGULUS_API(FLOW) Token GetToken() const;
      NOD() LANGULUS_API(FLOW) bool IsDone() const noexcept;
      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;
      NOD() LANGULUS_API(FLOW) const VerbState& GetVerbState() const noexcept;
      NOD() LANGULUS_API(FLOW) Count GetSuccesses() const noexcept;
      NOD() LANGULUS_API(FLOW) bool IsMissing() const noexcept;
      NOD() LANGULUS_API(FLOW) bool IsMissingDeep() const noexcept;

      LANGULUS_API(FLOW) void Done(Count) noexcept;
      LANGULUS_API(FLOW) void Done() noexcept;
      LANGULUS_API(FLOW) void Undo() noexcept;
      LANGULUS_API(FLOW) Verb& Invert() noexcept;

      template<CT::Data>
      Verb& SetVerb();
      LANGULUS_API(FLOW) Verb& SetVerb(VMeta) noexcept;
      LANGULUS_API(FLOW) Verb& SetMass(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetFrequency(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetTime(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetPriority(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetCharge(const Charge&) noexcept;

      template<CT::Data T>
      Verb& SetSource(const T&);
      template<CT::Data T>
      Verb& SetSource(T&);
      template<CT::Data T>
      Verb& SetSource(T&&) requires CT::Mutable<T>;

      template<CT::Data T>
      Verb& SetArgument(const T&);
      template<CT::Data T>
      Verb& SetArgument(T&&) requires CT::Mutable<T>;

      template<CT::Data T>
      Verb& SetOutput(const T&);
      template<CT::Data T>
      Verb& SetOutput(T&&) requires CT::Mutable<T>;

      NOD() LANGULUS_API(FLOW) bool operator == (const Verb&) const;
      NOD() LANGULUS_API(FLOW) bool operator == (VMeta) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator == (bool) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator <  (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator >  (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator >= (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW) bool operator <= (const Verb&) const noexcept;

      template<CT::Data T>
      Verb& operator << (const T&);
      template<CT::Data T>
      Verb& operator << (T&&);
      template<CT::Data T>
      Verb& operator >> (const T&);
      template<CT::Data T>
      Verb& operator >> (T&&);

      template<CT::Data T>
      Verb& operator <<= (const T&);
      template<CT::Data T>
      Verb& operator <<= (T&&);
      template<CT::Data T>
      Verb& operator >>= (const T&);
      template<CT::Data T>
      Verb& operator >>= (T&&);

      template<bool OR>
      Count CompleteDispatch(const Count, Abandoned<Any>&&);

   protected:
      template<CT::Text T>
      T SerializeVerb() const;
   };

   /// A handy container for verbs                                            
   using Script = TAny<Verb>;


   ///                                                                        
   /// Statically typed verb, used as CRTP for all specific verbs             
   ///                                                                        
   template<class VERB>
   struct StaticVerb : Verb {
      LANGULUS_BASES(Verb);

      StaticVerb();
      StaticVerb(const StaticVerb&);
      StaticVerb(StaticVerb&&);

      template<CT::NotSemantic T>
      StaticVerb(const T&);
      template<CT::NotSemantic T>
      StaticVerb(T&);
      template<CT::NotSemantic T>
      StaticVerb(T&&);

      template<CT::Semantic S>
      StaticVerb(S&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      StaticVerb(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      StaticVerb& operator = (const StaticVerb&);
      StaticVerb& operator = (StaticVerb&&);
      template<CT::Semantic S>
      StaticVerb& operator = (S&&);
   };


   ///                                                                        
   /// Statically typed verb, used as CRTP for arithmetic verbs               
   ///                                                                        
   template<class VERB, bool NOEXCEPT>
   struct ArithmeticVerb : public StaticVerb<VERB> {
      template<CT::Data T>
      using Operator = Conditional<NOEXCEPT,
         T(*)(const T*, const T*) noexcept, 
         T(*)(const T*, const T*)
      >;

      template<CT::Data T>
      using OperatorMutable = Conditional<NOEXCEPT,
         void(*)(T*, const T*) noexcept,
         void(*)(T*, const T*)
      >;

      using StaticVerb<VERB>::StaticVerb;

      template<CT::Data T>
      static bool Vector(const Block&, const Block&, Verb&, Operator<T>) noexcept(NOEXCEPT);
      template<CT::Data T>
      static bool Vector(const Block&, Block&, Verb&, OperatorMutable<T>) noexcept(NOEXCEPT);

      template<CT::Data T>
      static bool Scalar(const Block&, const Block&, Verb&, Operator<T>) noexcept(NOEXCEPT);
      template<CT::Data T>
      static bool Scalar(const Block&, Block&, Verb&, OperatorMutable<T>) noexcept(NOEXCEPT);
   };

} // namespace Langulus::Flow


namespace Langulus
{
   namespace CT
   {

      /// A reflected verb type is any type that inherits Verb, and is binary 
      /// compatible to a Verb                                                
      template<class... T>
      concept Verb = ((DerivedFrom<T, ::Langulus::Flow::Verb>
         && sizeof(T) == sizeof(::Langulus::Flow::Verb)) && ...);

   } // namespace Langulus::CT

   /// Get the meta of some stuff, just for convenience                       
   ///   @tparam T - type to get meta definition of                           
   ///   @return the meta definition of the provided stuff                    
   template<class T>
   NOD() auto MetaOf() {
      if constexpr (CT::Trait<T>)
         return RTTI::MetaTrait::Of<Decay<T>>();
      else if constexpr (CT::Verb<T>)
         return RTTI::MetaVerb::Of<Decay<T>>();
      else
         return RTTI::MetaData::Of<T>();
   }

} // namespace Langulus


/// Namespace containing all built-in Langulus verbs                          
namespace Langulus::Verbs
{

   struct Create;
   struct Select;
   struct Catenate;
   struct Exponent;
   struct Multiply;
   struct Add;
   struct Associate;
   struct Conjunct;
   struct Interpret;
   struct Interact;
   struct Do;

} // namespace Langulus::Verbs