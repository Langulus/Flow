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
   class LANGULUS_API(FLOW) Verb : public Any, public Charge {
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

      Verb(const Verb&);
      Verb(Verb&&);

      Verb(const CT::NotSemantic auto&);
      Verb(CT::NotSemantic auto&);
      Verb(CT::NotSemantic auto&&);

      template<CT::Semantic S>
      Verb(S&&);

      template<CT::Data HEAD, CT::Data... TAIL>
      Verb(HEAD&&, TAIL&&...) requires (sizeof...(TAIL) >= 1);

      Verb& operator = (const Verb&);
      Verb& operator = (Verb&&);
      template<CT::Semantic S>
      Verb& operator = (S&&);

      Verb operator * (const Real&) const;
      Verb operator ^ (const Real&) const;

      Verb& operator *= (const Real&) noexcept;
      Verb& operator ^= (const Real&) noexcept;

      NOD() explicit operator Code() const;
      NOD() explicit operator Debug() const;

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
      NOD() static Verb FromMeta(VMeta, const Charge& = {}, const VerbState& = {});

      NOD() Hash GetHash() const;
      NOD() Verb PartialCopy() const noexcept;
      void Reset();

      NOD() bool VerbIs(VMeta) const noexcept;
      template<CT::Data... T>
      NOD() bool VerbIs() const noexcept;

      NOD() const Charge& GetCharge() const noexcept;
      NOD() VMeta GetVerb() const noexcept;
      NOD() Real GetMass() const noexcept;
      NOD() Real GetFrequency() const noexcept;
      NOD() Real GetTime() const noexcept;
      NOD() Real GetPriority() const noexcept;

      NOD() Any& GetSource() noexcept;
      NOD() const Any& GetSource() const noexcept;
      NOD() Any& GetArgument() noexcept;
      NOD() const Any& GetArgument() const noexcept;
      NOD() Any& GetOutput() noexcept;
      NOD() const Any& GetOutput() const noexcept;

      NOD() bool Validate(const Index&) const noexcept;
      Verb& ShortCircuit(bool) noexcept;
      Verb& Multicast(bool) noexcept;
      Verb& SetVerbState(const VerbState&) noexcept;
      NOD() Token GetToken() const;
      NOD() bool IsDone() const noexcept;
      NOD() constexpr bool IsMulticast() const noexcept;
      NOD() constexpr bool IsMonocast() const noexcept;
      NOD() constexpr bool IsShortCircuited() const noexcept;
      NOD() constexpr bool IsLongCircuited() const noexcept;
      NOD() const VerbState& GetVerbState() const noexcept;
      NOD() Count GetSuccesses() const noexcept;
      NOD() bool IsMissing() const noexcept;
      NOD() bool IsMissingDeep() const noexcept;

      void Done(Count) noexcept;
      void Done() noexcept;
      void Undo() noexcept;
      Verb& Invert() noexcept;

      template<CT::Data>
      Verb& SetVerb();
      Verb& SetVerb(VMeta) noexcept;
      Verb& SetMass(Real) noexcept;
      Verb& SetFrequency(Real) noexcept;
      Verb& SetTime(Real) noexcept;
      Verb& SetPriority(Real) noexcept;
      Verb& SetCharge(const Charge&) noexcept;

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

      NOD() bool operator == (const Verb&) const;
      NOD() bool operator == (VMeta) const noexcept;
      NOD() bool operator == (bool) const noexcept;
      NOD() bool operator <  (const Verb&) const noexcept;
      NOD() bool operator >  (const Verb&) const noexcept;
      NOD() bool operator >= (const Verb&) const noexcept;
      NOD() bool operator <= (const Verb&) const noexcept;

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
   using namespace Flow;

   /// Create/Destroy verb                                                    
   /// Used for allocating new elements. If the type you're creating has      
   /// a producer, you need to execute the verb in the correct context        
   struct Create : StaticVerb<Create> {
      LANGULUS(POSITIVE_VERB) "Create";
      LANGULUS(NEGATIVE_VERB) "Destroy";
      LANGULUS(PRECEDENCE) 1000;
      LANGULUS(INFO)
         "Used for allocating new elements. "
         "If the type you're creating has   a producer, "
         "you need to execute the verb in a matching producer, "
         "or that producer will be created automatically for you, if possible";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      static void SetMembers(Any&, const Any&);
   };

   /// Select/Deselect verb                                                   
   /// Used to focus on a part of a container, or access members              
   struct Select : StaticVerb<Select> {
      LANGULUS(POSITIVE_VERB) "Select";
      LANGULUS(NEGATIVE_VERB) "Deselect";
      LANGULUS(POSITIVE_OPERATOR) ".";
      LANGULUS(NEGATIVE_OPERATOR) "..";
      LANGULUS(PRECEDENCE) 100;
      LANGULUS(INFO)
         "Used to focus on a part of a container, or access members";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);

   protected:
      template<bool MUTABLE>
      static bool DefaultSelect(Block&, Verb&);
      template<bool MUTABLE, class META>
      static bool PerIndex(Block&, TAny<Trait>&, TMeta, META, const TAny<Index>&);
      template<bool MUTABLE>
      static bool SelectByMeta(const TAny<Index>&, DMeta, Block&, TAny<Trait>&, TAny<const RTTI::Ability*>&);
   };

   /// Catenate/Split verb                                                    
   /// Catenates anything catenable, or split stuff apart using a mask        
   struct Catenate : StaticVerb<Catenate> {
      LANGULUS(POSITIVE_VERB) "Catenate";
      LANGULUS(NEGATIVE_VERB) "Split";
      LANGULUS(POSITIVE_OPERATOR) " >< ";
      LANGULUS(NEGATIVE_OPERATOR) " <> ";
      LANGULUS(PRECEDENCE) 7;
      LANGULUS(INFO) "Catenates, or splits stuff apart";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };

   /// Exponent/Root verb                                                     
   /// Performs exponentiation or root                                        
   struct Exponent : ArithmeticVerb<Exponent, true> {
      LANGULUS(POSITIVE_VERB) "Exponent";
      LANGULUS(NEGATIVE_VERB) "Root";
      LANGULUS(POSITIVE_OPERATOR) "^";
      LANGULUS(NEGATIVE_OPERATOR) "^^";
      LANGULUS(PRECEDENCE) 6;
      LANGULUS(INFO) "Performs exponentiation or root";

      using ArithmeticVerb::ArithmeticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);

      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, const Block&, Verb&);
      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, Block&, Verb&);
   };

   /// Multiply/Divide verb                                                   
   /// Performs arithmetic multiplication or division                         
   /// If context is no specified, it is always 1                             
   struct Multiply : ArithmeticVerb<Multiply, false> {
      LANGULUS(POSITIVE_VERB) "Multiply";
      LANGULUS(NEGATIVE_VERB) "Divide";
      LANGULUS(POSITIVE_OPERATOR) "*";
      LANGULUS(NEGATIVE_OPERATOR) "/";
      LANGULUS(PRECEDENCE) 5;
      LANGULUS(INFO)
         "Performs arithmetic multiplication or division. "
         "If context is not specified, it is always 1";

      using ArithmeticVerb::ArithmeticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);

      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, const Block&, Verb&);
      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, Block&, Verb&);
      template<CT::Data... T>
      static bool OperateOnTypes(Block&, Verb&);
   };

   /// Add/Subtract verb                                                      
   /// Performs arithmetic addition or subtraction                            
   struct Add : ArithmeticVerb<Add, true> {
      LANGULUS(POSITIVE_VERB) "Add";
      LANGULUS(NEGATIVE_VERB) "Subtract";
      LANGULUS(POSITIVE_OPERATOR) " + ";
      LANGULUS(NEGATIVE_OPERATOR) " - ";
      LANGULUS(PRECEDENCE) 4;
      LANGULUS(INFO)
         "Performs arithmetic addition or subtraction";

      using ArithmeticVerb::ArithmeticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);

      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, const Block&, Verb&);
      template<CT::Data... T>
      static bool OperateOnTypes(const Block&, Block&, Verb&);
      template<CT::Data... T>
      static bool OperateOnTypes(Block&, Verb&);
   };

   /// Associate/Disassociate verb                                            
   /// Either performs a shallow copy, or aggregates associations,            
   /// depending on the context's complexity                                  
   struct Associate : StaticVerb<Associate> {
      LANGULUS(POSITIVE_VERB) "Associate";
      LANGULUS(NEGATIVE_VERB) "Disassocate";
      LANGULUS(POSITIVE_OPERATOR) " = ";
      LANGULUS(NEGATIVE_OPERATOR) " ~ ";
      LANGULUS(PRECEDENCE) 2;
      LANGULUS(INFO)
         "Either performs a shallow copy, or aggregates associations, "
         "depending on the context's complexity";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(Block&, Verb&);
   };

   /// Conjunct/Disjunct verb                                                 
   /// Either combines LHS and RHS as one AND container, or separates them    
   /// as one OR container - does only shallow copying                        
   struct Conjunct : StaticVerb<Conjunct> {
      LANGULUS(POSITIVE_VERB) "Conjunct";
      LANGULUS(NEGATIVE_VERB) "Disjunct";
      LANGULUS(POSITIVE_OPERATOR) ", ";
      LANGULUS(NEGATIVE_OPERATOR) " or ";
      LANGULUS(PRECEDENCE) 1;
      LANGULUS(INFO)
         "Either combines LHS and RHS as one AND container, or separates them "
         "as one OR container (does only shallow copying)";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };

   /// Interpret                                                              
   /// Performs conversion                                                    
   struct Interpret : StaticVerb<Interpret> {
      LANGULUS(VERB) "Interpret";
      LANGULUS(OPERATOR) " => ";
      LANGULUS(INFO) "Performs conversion";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<class TO, class FROM>
      static TO To(const FROM&);

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
   };

   /// Interact                                                               
   /// Used for processing user events, such as mouse movement, keyboard,     
   /// joystick and any other input                                           
   struct Interact : StaticVerb<Interact> {
      LANGULUS(VERB) "Interact";
      LANGULUS(INFO) 
         "Used for processing user events, such as mouse movement, "
         "keyboard, joystick and any other input";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);
   };

   /// Statically optimized interpret verb                                    
   ///   @tparam TO - what are we converting to?                              
   template<class TO>
   struct InterpretTo : Interpret {
      LANGULUS_BASES(Interpret);
      using Interpret::Interpret;
      using Type = TO;

      static bool ExecuteDefault(const Block&, Verb&);
   };

   /// Do/Undo verb                                                           
   /// Used as a runtime dispatcher of composite types                        
   struct Do : StaticVerb<Do> {
      LANGULUS(POSITIVE_VERB) "Do";
      LANGULUS(NEGATIVE_VERB) "Undo";
      LANGULUS(INFO) "Used as a runtime dispatcher of composite types";

      using StaticVerb::StaticVerb;

      template<CT::Dense T, CT::Data... A>
      static constexpr bool AvailableFor() noexcept;
      template<CT::Dense T, CT::Data... A>
      static constexpr auto Of() noexcept;

      template<CT::Dense T>
      static bool ExecuteIn(T&, Verb&);

      static bool ExecuteDefault(const Block&, Verb&);
      static bool ExecuteDefault(Block&, Verb&);
      static bool ExecuteStateless(Verb&);
   };

} // namespace Langulus::Verbs

#include "Verb.inl"
