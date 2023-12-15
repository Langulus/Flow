///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// Distributed under GNU General Public License v3+                          
/// See LICENSE file, or https://www.gnu.org/licenses                         
///                                                                           
#pragma once
#include "Common.hpp"
#include "Code.hpp"


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
      Verb(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      Verb(T1&&, T2&&, TAIL&&...);

      LANGULUS_API(FLOW) ~Verb();

      LANGULUS_API(FLOW) Verb& operator = (const Verb&);
      LANGULUS_API(FLOW) Verb& operator = (Verb&&);
      Verb& operator = (CT::Semantic auto&&);

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

      NOD() LANGULUS_API(FLOW)
      const Charge& GetCharge() const noexcept;

      NOD() LANGULUS_API(FLOW)
      VMeta GetVerb() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Real GetMass() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Real GetRate() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Real GetTime() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Real GetPriority() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Any& GetSource() noexcept;
      NOD() LANGULUS_API(FLOW)
      const Any& GetSource() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Any& GetArgument() noexcept;
      NOD() LANGULUS_API(FLOW)
      const Any& GetArgument() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Any& GetOutput() noexcept;
      NOD() LANGULUS_API(FLOW)
      const Any& GetOutput() const noexcept;

      NOD() const Any* operator -> () const noexcept;
      NOD() Any* operator -> () noexcept;

      NOD() LANGULUS_API(FLOW)
      bool Validate(const Index&) const noexcept;

      LANGULUS_API(FLOW) Verb& ShortCircuit(bool) noexcept;
      LANGULUS_API(FLOW) Verb& Multicast(bool) noexcept;
      LANGULUS_API(FLOW) Verb& SetVerbState(const VerbState&) noexcept;

      NOD() LANGULUS_API(FLOW)
      Token GetToken() const;

      NOD() LANGULUS_API(FLOW)
      bool IsDone() const noexcept;

      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;

      NOD() LANGULUS_API(FLOW)
      const VerbState& GetVerbState() const noexcept;

      NOD() LANGULUS_API(FLOW)
      Count GetSuccesses() const noexcept;

      NOD() LANGULUS_API(FLOW)
      bool IsMissing() const noexcept;

      NOD() LANGULUS_API(FLOW)
      bool IsMissingDeep() const noexcept;

      LANGULUS_API(FLOW) void Done(Count) noexcept;
      LANGULUS_API(FLOW) void Done() noexcept;
      LANGULUS_API(FLOW) void Undo() noexcept;
      LANGULUS_API(FLOW) Verb& Invert() noexcept;

      template<CT::Data>
      Verb& SetVerb();
      LANGULUS_API(FLOW) Verb& SetVerb(VMeta) noexcept;
      LANGULUS_API(FLOW) Verb& SetMass(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetRate(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetTime(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetPriority(Real) noexcept;
      LANGULUS_API(FLOW) Verb& SetCharge(const Charge&) noexcept;

      Verb& SetSource(const CT::NotSemantic auto&);
      Verb& SetSource(CT::NotSemantic auto&);
      Verb& SetSource(CT::NotSemantic auto&&);
      Verb& SetSource(CT::Semantic auto&&);
      
      Verb& SetArgument(const CT::NotSemantic auto&);
      Verb& SetArgument(CT::NotSemantic auto&);
      Verb& SetArgument(CT::NotSemantic auto&&);
      Verb& SetArgument(CT::Semantic auto&&);
      
      Verb& SetOutput(const CT::NotSemantic auto&);
      Verb& SetOutput(CT::NotSemantic auto&);
      Verb& SetOutput(CT::NotSemantic auto&&);
      Verb& SetOutput(CT::Semantic auto&&);

      NOD() LANGULUS_API(FLOW)
      bool operator == (const Verb&) const;
      NOD() LANGULUS_API(FLOW)
      bool operator == (VMeta) const noexcept;
      NOD() LANGULUS_API(FLOW)
      bool operator == (bool) const noexcept;

      NOD() LANGULUS_API(FLOW)
      bool operator <  (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW)
      bool operator >  (const Verb&) const noexcept;

      NOD() LANGULUS_API(FLOW)
      bool operator >= (const Verb&) const noexcept;
      NOD() LANGULUS_API(FLOW)
      bool operator <= (const Verb&) const noexcept;

      Verb& operator << (const CT::NotSemantic auto&);
      Verb& operator << (CT::NotSemantic auto&);
      Verb& operator << (CT::NotSemantic auto&&);
      Verb& operator << (CT::Semantic auto&&);
      
      Verb& operator >> (const CT::NotSemantic auto&);
      Verb& operator >> (CT::NotSemantic auto&);
      Verb& operator >> (CT::NotSemantic auto&&);
      Verb& operator >> (CT::Semantic auto&&);
      
      Verb& operator <<= (const CT::NotSemantic auto&);
      Verb& operator <<= (CT::NotSemantic auto&);
      Verb& operator <<= (CT::NotSemantic auto&&);
      Verb& operator <<= (CT::Semantic auto&&);
      
      Verb& operator >>= (const CT::NotSemantic auto&);
      Verb& operator >>= (CT::NotSemantic auto&);
      Verb& operator >>= (CT::NotSemantic auto&&);
      Verb& operator >>= (CT::Semantic auto&&);

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

      StaticVerb(const CT::NotSemantic auto&);
      StaticVerb(CT::NotSemantic auto&);
      StaticVerb(CT::NotSemantic auto&&);
      StaticVerb(CT::Semantic auto&&);

      template<CT::Data T1, CT::Data T2, CT::Data... TAIL>
      StaticVerb(T1&&, T2&&, TAIL&&...);

      StaticVerb& operator = (const StaticVerb&);
      StaticVerb& operator = (StaticVerb&&);
      StaticVerb& operator = (CT::Semantic auto&&);

      NOD() static VMeta GetVerb();
   };

} // namespace Langulus::Flow


namespace Langulus::CT
{

   /// A VerbBased type is any type that inherits (or is) Verb, and is        
   /// binary compatible to a Verb                                            
   template<class... T>
   concept VerbBased = ((DerivedFrom<T, Flow::Verb>
         and sizeof(T) == sizeof(Flow::Verb)
      ) and ...);

   /// A reflected verb type is any type that inherits Verb, is not Verb      
   /// itself, and is binary compatible to a Verb                             
   template<class... T>
   concept Verb = VerbBased<T...> and ((not Same<T, Flow::Verb>) and ...);

} // namespace Langulus::CT
