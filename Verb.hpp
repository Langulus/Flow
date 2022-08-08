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

	using namespace Anyness;
	using Anyness::Inner::Allocator;

	using RTTI::VMeta;
	using RTTI::TMeta;
	using RTTI::DMeta;
	using RTTI::CMeta;
	using RTTI::MetaData;
	using RTTI::MetaVerb;
	using RTTI::MetaTrait;
	using RTTI::MetaConst;

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
	class Verb : public Any, public Charge {
		LANGULUS(DEEP) false;
		LANGULUS_CONVERSIONS(Code, Debug);
		LANGULUS_BASES(Any, Charge);
	friend class Scope;
	private:
		// Verb meta, mass, frequency, time and priority						
		VMeta mVerb {};
		// The number of successful executions										
		Count mSuccesses {};
		// Verb context																	
		Any mSource;
		// The container where output goes											
		Any mOutput;
		// Verb short-circuiting														
		bool mShortCircuited {true};

	public:
		Verb() noexcept {}

		Verb(const Verb&) = default;
		Verb(Verb&&) noexcept = default;

		Verb(Disowned<Verb>&&) noexcept;
		Verb(Abandoned<Verb>&&) noexcept;

		~Verb() = default;

		Verb(VMeta);
		template<CT::Data T = Any>
		Verb(VMeta, const T& = {}, const Charge& = {}, bool = true);
		template<CT::Data T = Any>
		Verb(VMeta, T&& = {}, const Charge& = {}, bool = true);

		Verb& operator = (const Verb&) = default;
		Verb& operator = (Verb&&) noexcept = default;

		Verb& operator = (Disowned<Verb>&&);
		Verb& operator = (Abandoned<Verb>&&);

		Verb operator * (const Real&) const;
		Verb operator ^ (const Real&) const;

		Verb& operator *= (const Real&) noexcept;
		Verb& operator ^= (const Real&) noexcept;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;

		template<CT::Data T>
		bool GenericAvailableFor() const noexcept;
		template<CT::Data T, CT::Data V>
		static bool GenericExecuteIn(T&, V&);
		template<CT::Data V>
		static bool GenericExecuteDefault(const Block&, V&);
		template<CT::Data V>
		static bool GenericExecuteDefault(Block&, V&);
		template<CT::Data V>
		static bool GenericExecuteStateless(V&);

	public:
		NOD() Hash GetHash() const;

		NOD() Verb PartialCopy() const noexcept;
		NOD() Verb Clone() const;
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
		NOD() Verb& ShortCircuit(bool) noexcept;
		NOD() Token GetToken() const;
		NOD() bool IsDone() const noexcept;
		NOD() bool IsShortCircuited() const noexcept;
		NOD() Count GetSuccesses() const noexcept;
		NOD() bool IsMissing() const noexcept;
		NOD() bool IsMissingDeep() const noexcept;

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

		template<CT::Data T>
		Verb& SetSource(const T&);
		template<CT::Data T>
		Verb& SetSource(T&&);

		template<CT::Data T>
		Verb& SetArgument(const T&);
		template<CT::Data T>
		Verb& SetArgument(T&&);

		template<CT::Data T>
		Verb& SetOutput(const T&);
		template<CT::Data T>
		Verb& SetOutput(T&&);

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
		LANGULUS_BASES(Verb);

		Create();
		template<CT::Data T>
		Create(const T&, const Charge& = {}, bool = true);
		template<CT::Data T>
		Create(T&&, const Charge& = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) ".";
		LANGULUS(NEGATIVE_OPERATOR) "..";
		LANGULUS(INFO)
			"Used to focus on a part of a container, or access members";
		LANGULUS_BASES(Verb);

		Select();
		template<CT::Data T>
		Select(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Select(T&&, const Charge & = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) " = ";
		LANGULUS(NEGATIVE_OPERATOR) " ~ ";
		LANGULUS(INFO)
			"Either performs a shallow copy, or aggregates associations, "
			"depending on the context's complexity";
		LANGULUS_BASES(Verb);

		Associate();
		template<CT::Data T>
		Associate(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Associate(T&&, const Charge & = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) " + ";
		LANGULUS(NEGATIVE_OPERATOR) " - ";
		LANGULUS(INFO)
			"Performs arithmetic addition or subtraction";
		LANGULUS_BASES(Verb);

		Add();
		template<CT::Data T>
		Add(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Add(T&&, const Charge & = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) "*";
		LANGULUS(NEGATIVE_OPERATOR) "/";
		LANGULUS(INFO)
			"Performs arithmetic multiplication or division. "
			"If context is no specified, it is always 1";
		LANGULUS_BASES(Verb);

		Multiply();
		template<CT::Data T>
		Multiply(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Multiply(T&&, const Charge & = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) "^";
		LANGULUS(NEGATIVE_OPERATOR) " log ";
		LANGULUS(INFO)
			"Performs exponentiation or logarithm";
		LANGULUS_BASES(Verb);

		Exponent();
		template<CT::Data T>
		Exponent(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Exponent(T&&, const Charge & = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) " >< ";
		LANGULUS(NEGATIVE_OPERATOR) " <> ";
		LANGULUS(INFO) "Catenates, or splits stuff apart";
		LANGULUS_BASES(Verb);

		Catenate();
		template<CT::Data T>
		Catenate(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Catenate(T&&, const Charge & = {}, bool = true);

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
		LANGULUS(POSITIVE_OPERATOR) ", ";
		LANGULUS(NEGATIVE_OPERATOR) " or ";
		LANGULUS(INFO)
			"Either combines LHS and RHS as one AND container, or separates them "
			"as one OR container (does only shallow copying)";
		LANGULUS_BASES(Verb);

		Conjunct();
		template<CT::Data T>
		Conjunct(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Conjunct(T&&, const Charge & = {}, bool = true);

		template<CT::Data T, CT::Data... A>
		static constexpr bool AvailableFor() noexcept;
		template<CT::Data T, CT::Data... A>
		static constexpr auto Of() noexcept;

		template<CT::Data T>
		static bool ExecuteIn(T&, Verb&);

		static bool ExecuteDefault(const Block&, Verb&);
		static bool ExecuteStateless(Verb&);
	};

	/// Interpret																					
	/// Performs conversion																		
	struct Interpret : public Verb {
		LANGULUS(NAME) "Interpret";
		LANGULUS(OPERATOR) " => ";
		LANGULUS(INFO) "Performs conversion";
		LANGULUS_BASES(Verb);

		Interpret();
		template<CT::Data T>
		Interpret(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Interpret(T&&, const Charge & = {}, bool = true);

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

	/// Statically optimized interpret verb												
	///	@tparam TO - what are we converting to?										
	template<class TO>
	struct InterpretTo : public Interpret {
		LANGULUS_BASES(Interpret);
		using Interpret::Interpret;
		using Type = TO;

		static bool ExecuteDefault(const Block&, Verb&);
	};

	/// Do/Undo verb																				
	/// Used as a runtime dispatcher of composite types								
	struct Do : public Verb {
		LANGULUS(POSITIVE_VERB) "Do";
		LANGULUS(NEGATIVE_VERB) "Undo";
		LANGULUS(INFO) "Used as a runtime dispatcher of composite types";
		LANGULUS_BASES(Verb);

		Do();
		template<CT::Data T>
		Do(const T&, const Charge & = {}, bool = true);
		template<CT::Data T>
		Do(T&&, const Charge & = {}, bool = true);

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
