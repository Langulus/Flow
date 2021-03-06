#pragma once
#include <Langulus.Logger.hpp>
#include <Langulus.Anyness.hpp>

#ifndef LANGULUS_ENABLE_FEATURE_MANAGED_MEMORY
	#error Langulus::Flow can be compiled only with enabled LANGULUS_FEATURE_MANAGED_MEMORY
#endif

#ifndef LANGULUS_ENABLE_FEATURE_MANAGED_REFLECTION
	#error Langulus::Flow can be compiled only with enabled LANGULUS_ENABLE_MANAGED_REFLECTION
#endif

LANGULUS_EXCEPTION(Flow);

namespace Langulus::Flow
{

	using Anyness::DataState;
	using Anyness::Index;
	using Anyness::Block;
	using Anyness::Any;
	using Anyness::Text;
	using Anyness::Debug;
	using Anyness::Bytes;
	using Anyness::Trait;
	using Anyness::Map;
	using Anyness::TAny;
	using Anyness::TMap;
	using Anyness::THashMap;
	using Anyness::Inner::Allocator;
	using RTTI::VMeta;
	using RTTI::TMeta;
	using RTTI::DMeta;
	using RTTI::MetaData;
	using RTTI::MetaVerb;
	using RTTI::MetaTrait;

	class Charge;
	class Construct;
	class Code;
	class Verb;


	///																								
	/// Charge, carrying the four verb dimensions										
	///																								
	struct Charge {
		// Mass of the verb																
		Real mMass = DefaultMass;
		// Frequency of the verb														
		Real mFrequency = DefaultFrequency;
		// Time of the verb																
		Real mTime = DefaultTime;
		// Priority of the verb															
		Real mPriority = DefaultPriority;

	public:
		static constexpr Real DefaultMass {1};
		static constexpr Real DefaultFrequency {0};
		static constexpr Real DefaultTime {0};

		static constexpr Real DefaultPriority {0};
		static constexpr Real MinPriority {-10000};
		static constexpr Real MaxPriority {+10000};

		constexpr Charge(
			Real = DefaultMass,
			Real = DefaultFrequency,
			Real = DefaultTime,
			Real = DefaultPriority
		) noexcept;

		NOD() constexpr bool operator == (const Charge&) const noexcept;

		NOD() constexpr Charge operator * (const Real&) const noexcept;
		NOD() constexpr Charge operator ^ (const Real&) const noexcept;

		NOD() constexpr Charge& operator *= (const Real&) noexcept;
		NOD() constexpr Charge& operator ^= (const Real&) noexcept;

		NOD() constexpr bool IsDefault() const noexcept;
		NOD() Hash GetHash() const noexcept;
		void Reset() noexcept;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;
	};


	///																								
	///	THE UNIVERSAL VERB																	
	///																								
	/// It's practically a single call to the framework, or a single statement	
	/// in a code flow. Langulus is based around natural language processing	
	/// theory based around verbs, so this is the natural name for such thing	
	///																								
	class Verb : public Charge {
	friend class Scope;
	private:
		// Verb meta, mass, frequency, time and priority						
		VMeta mVerb {};
		// The number of successful executions										
		Count mSuccesses {};
		// Verb context																	
		Any mSource;
		// Argument for the call														
		Any mArgument;
		// The container where output goes											
		Any mOutput;
		// Verb short-circuiting														
		bool mShortCircuited {true};

	public:
		Verb() noexcept {}
		Verb(const Verb&) = default;
		Verb(Verb&&) noexcept = default;
		~Verb() = default;

		Verb(VMeta, const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		Verb& operator = (const Verb&) = default;
		Verb& operator = (Verb&&) noexcept = default;

		Verb operator * (const Real&) const;
		Verb operator ^ (const Real&) const;

		Verb& operator *= (const Real&) noexcept;
		Verb& operator ^= (const Real&) noexcept;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;

		/// Replace these in your verbs, to specify their behavior statically	
		/// Otherwise, these functions fallback and perform slow RTTI checks		
		template<CT::Data T>
		bool AvailableFor() const noexcept;
		template<CT::Data T, CT::Data V>
		static bool ExecuteIn(T&, V&);
		template<CT::Data V>
		static bool ExecuteDefault(const Block&, V&);
		template<CT::Data V>
		static bool ExecuteDefault(Block&, V&);
		template<CT::Data V>
		static bool ExecuteStateless(V&);

	public:
		NOD() Hash GetHash() const;

		NOD() Verb PartialCopy() const noexcept;
		NOD() Verb Clone() const;
		void Reset();

		NOD() bool Is(VMeta) const noexcept;
		template<CT::Data... T>
		NOD() bool Is() const noexcept;

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
		template<CT::Trait T>
		NOD() bool OutputsTo() const noexcept;

		NOD() bool Validate(const Index&) const noexcept;
		NOD() Verb& ShortCircuit(bool) noexcept;
		NOD() Token GetToken() const;
		NOD() bool IsDone() const noexcept;
		NOD() bool IsShortCircuited() const noexcept;
		NOD() Count GetSuccesses() const noexcept;
		void Done(Count) noexcept;
		void Done() noexcept;
		void Undo() noexcept;
		Verb& Invert() noexcept;

		Verb& SetVerb(VMeta) noexcept;
		Verb& SetMass(Real) noexcept;
		Verb& SetFrequency(Real) noexcept;
		Verb& SetTime(Real) noexcept;
		Verb& SetPriority(Real) noexcept;
		Verb& SetCharge(const Charge&) noexcept;
		Verb& SetSource(const Any&);
		Verb& SetSource(Any&&) noexcept;
		Verb& SetArgument(const Any&);
		Verb& SetArgument(Any&&) noexcept;
		Verb& SetOutput(const Any&);
		Verb& SetOutput(Any&&) noexcept;
		Verb& SetAll(const Any&, const Any&, const Any&);

		NOD() bool operator == (const Verb&) const noexcept;
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
	};

	/// A handy container for verbs															
	using Script = TAny<Verb>;

} // namespace Langulus::Flow


namespace Langulus::CT
{

	/// A reflected verb type is any type that inherits Verb, and is binary		
	/// compatible to a Verb																	
	template<class... T>
	concept Verb = ((DerivedFrom<T, ::Langulus::Flow::Verb>
		&& sizeof(T) == sizeof(::Langulus::Flow::Verb)) && ...);

} // namespace Langulus::CT


/// Namespace containing all built-in Langulus verbs									
namespace Langulus::Verbs
{
	using namespace ::Langulus::Flow;

	/// Create/Destroy verb																		
	/// Used for allocating new elements. If the type you're creating has		
	/// a producer, you need to execute the verb in the correct context			
	struct Create : public Verb {
		LANGULUS(POSITIVE_VERB) "Create";
		LANGULUS(NEGATIVE_VERB) "Destroy";
		LANGULUS(INFO) 
			"Used for allocating new elements. "
			"If the type you're creating has	a producer, "
			"you need to execute the verb in a matching producer, "
			"or that producer will be created automatically for you, if possible";

		Create(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(Block&, Verb&);
		static bool ExecuteStateless(Verb&);

	protected:
		static void SetMembers(Any&, const Any&);
	};

	/// Select/Deselect verb																	
	/// Used to focus on a part of a container, or access members					
	struct Select : public Verb {
		LANGULUS(POSITIVE_VERB) "Select";
		LANGULUS(NEGATIVE_VERB) "Deselect";
		LANGULUS(INFO)
			"Used to focus on a part of a container, or access members";

		Select(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
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

	/// Associate/Disassociate verb															
	/// Either performs a shallow copy, or aggregates associations,				
	/// depending on the context's complexity												
	struct Associate : public Verb {
		LANGULUS(POSITIVE_VERB) "Associate";
		LANGULUS(NEGATIVE_VERB) "Disassocate";
		LANGULUS(INFO)
			"Either performs a shallow copy, or aggregates associations, "
			"depending on the context's complexity";

		Associate(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(Block&, Verb&);
	};

	/// Add/Subtract verb																		
	/// Performs arithmetic addition or subtraction										
	struct Add : public Verb {
		LANGULUS(POSITIVE_VERB) "Add";
		LANGULUS(NEGATIVE_VERB) "Subtract";
		LANGULUS(INFO)
			"Performs arithmetic addition or subtraction";

		Add(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteStateless(Verb&);
	};

	/// Multiply/Divide verb																	
	/// Performs arithmetic multiplication or division									
	/// If context is no specified, it is always 1										
	struct Multiply : public Verb {
		LANGULUS(POSITIVE_VERB) "Multiply";
		LANGULUS(NEGATIVE_VERB) "Divide";
		LANGULUS(INFO)
			"Performs arithmetic multiplication or division. "
			"If context is no specified, it is always 1";

		Multiply(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteStateless(Verb&);
	};

	/// Exponent/Logarithm verb																
	/// Performs exponentiation or logarithm												
	struct Exponent : public Verb {
		LANGULUS(POSITIVE_VERB) "Exponent";
		LANGULUS(NEGATIVE_VERB) "Logarithm";
		LANGULUS(INFO)
			"Performs exponentiation or logarithm";

		Exponent(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);
	};

	/// Catenate/Split verb																		
	/// Catenates anything catenable, or split stuff apart using a mask			
	struct Catenate : public Verb {
		LANGULUS(POSITIVE_VERB) "Catenate";
		LANGULUS(NEGATIVE_VERB) "Split";
		LANGULUS(INFO)
			"Catenates anything catenable, or splits stuff apart using a mask";

		Catenate(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(const Block&, Verb&);
		static bool ExecuteDefault(Block&, Verb&);
		static bool ExecuteStateless(Verb&);
	};

	/// Conjunct/Disjunct verb																	
	/// Either combines LHS and RHS as one AND container, or separates them		
	/// as one OR container - does only shallow copying								
	struct Conjunct : public Verb {
		LANGULUS(POSITIVE_VERB) "Conjunct";
		LANGULUS(NEGATIVE_VERB) "Disjunct";
		LANGULUS(INFO)
			"Either combines LHS and RHS as one AND container, or separates them "
			"as one OR container - does only shallow copying";

		Conjunct(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(const Block&, Verb&);
		static bool ExecuteDefault(Block&, Verb&);
		static bool ExecuteStateless(Verb&);
	};

	/// Interpret																					
	/// Performs conversion																		
	struct Interpret : public Verb {
		LANGULUS(NAME) "Interpret";
		LANGULUS(INFO) "Performs conversion";

		Interpret(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<class TO, class FROM>
		static TO To(const FROM&);

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(const Block&, Verb&);
	};

	/// Do/Undo verb																				
	/// Used as a runtime dispatcher of composite types								
	struct Do : public Verb {
		LANGULUS(POSITIVE_VERB) "Do";
		LANGULUS(NEGATIVE_VERB) "Undo";
		LANGULUS(INFO) "Used as a runtime dispatcher of composite types";

		Do(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(const Block&, Verb&);
		static bool ExecuteDefault(Block&, Verb&);
		static bool ExecuteStateless(Verb&);
	};

} // namespace Langulus::Verbs

#include "Verb.inl"
