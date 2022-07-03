#pragma once
#include <Langulus.Logger.hpp>
#include <Langulus.Anyness.hpp>

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
	/// Charge, carrying the four verb dimensions, and state							
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
	/// in a Code flow. Piception is based around natural language processing	
	/// theory based around verbs, so this is the natural name for such thing	
	///																								
	class Verb : public Charge {
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

	public:
		NOD() Hash GetHash() const;

		NOD() Verb PartialCopy() const noexcept;
		NOD() Verb Clone() const;
		void Reset();

		NOD() bool Is(VMeta) const noexcept;
		template<CT::Verb... T>
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
		Verb& operator >>= (const T&);

	public:
		static bool ExecuteScope(Any&, const Any&);
		static bool ExecuteScope(Any&, const Any&, Any&);
		static bool ExecuteScope(Any&, const Any&, Any&, bool& skipVerbs);
		static bool ExecuteVerb(Any&, Verb&);
		static Count DispatchEmpty(Verb&);
		static Count DispatchDeep(Block&, Verb&, bool resolveElements = true, bool allowCustomDispatch = true, bool allowDefaultVerbs = true);
		static Count DispatchFlat(Block&, Verb&, bool resolveElements = true, bool allowCustomDispatch = true, bool allowDefaultVerbs = true);
		NOD() static bool IsScopeExecutable(const Block&) noexcept;
		NOD() static bool IsScopeExecutableDeep(const Block&);
		static void DefaultCreateInner(Any&, const Any&, Any&);
		static void SetMembers(Any&, const Any&);

	protected:
		static bool DefaultDo(Block&, Verb&);
		static void DefaultInterpret(Block&, Verb&);
		static void DefaultAssociate(Block&, Verb&);
		static void DefaultSelect(Block&, Verb&);
		static void DefaultConjunct(Block&, Verb&);
		static void DefaultDisjunct(Block&, Verb&);
		static void DefaultCreate(Block&, Verb&);
		static void DefaultScope(const Block&, Verb&);
		static bool ExecuteScopeOR(Any&, const Any&, Any&, bool& skipVerbs);
		static bool ExecuteScopeAND(Any&, const Any&, Any&, bool& skipVerbs);
		static bool IntegrateScope(Any&, Any&);
		static bool IntegrateVerb(Any&, Verb&);
	};

	/// A handy container for verbs															
	using Script = TAny<Verb>;

} // namespace Langulus::Flow


namespace Langulus
{

	/// Namespace containing all built-in Langulus verbs								
	namespace Verbs
	{

		using Flow::Verb;
		using Flow::Any;
		using Flow::Charge;

		/// Create/destroy verb																	
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
		};

		/// Select/deselect verb																
		/// Used to focus on a part of a container, or access members				
		struct Select : public Verb {
			LANGULUS(POSITIVE_VERB) "Select";
			LANGULUS(NEGATIVE_VERB) "Deselect";
			LANGULUS(INFO)
				"Used to focus on a part of a container, or access members";

			Select(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		/// Associate/disassociate verb														
		/// Either performs a shallow copy, or aggregates associations,			
		/// depending on the context's complexity											
		struct Associate : public Verb {
			LANGULUS(POSITIVE_VERB) "Associate";
			LANGULUS(NEGATIVE_VERB) "Disassocate";
			LANGULUS(INFO)
				"Either performs a shallow copy, or aggregates associations, "
				"depending on the context's complexity";

			Associate(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		/// Add/subtract verb																	
		/// Performs arithmetic addition or subtraction									
		struct Add : public Verb {
			LANGULUS(POSITIVE_VERB) "Add";
			LANGULUS(NEGATIVE_VERB) "Subtract";
			LANGULUS(INFO)
				"Performs arithmetic addition or subtraction";

			Add(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		/// Multiply/divide verb																
		/// Performs arithmetic multiplication or division								
		/// If context is no specified, it is always 1									
		struct Multiply : public Verb {
			LANGULUS(POSITIVE_VERB) "Multiply";
			LANGULUS(NEGATIVE_VERB) "Divide";
			LANGULUS(INFO)
				"Performs arithmetic multiplication or division. "
				"If context is no specified, it is always 1";

			Multiply(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		/// Exponentiate/logarithm verb														
		/// Performs exponentiation or logarithm											
		struct Exponent : public Verb {
			LANGULUS(POSITIVE_VERB) "Exponent";
			LANGULUS(NEGATIVE_VERB) "Logarithm";
			LANGULUS(INFO)
				"Performs exponentiation or logarithm";

			Exponent(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		/// Catenate/break verb																	
		/// Catenates anything catenable, or breaks stuff apart using a mask		
		struct Catenate : public Verb {
			LANGULUS(POSITIVE_VERB) "Catenate";
			LANGULUS(NEGATIVE_VERB) "Break";
			LANGULUS(INFO)
				"Catenates anything catenable, or breaks stuff apart using a mask";

			Catenate(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		/// Interpret																				
		/// Performs conversion																	
		struct Interpret : public Verb {
			LANGULUS(NAME) "Interpret";
			LANGULUS(INFO) "Performs conversion";

			Interpret(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);

			template<class TO, class FROM>
			static TO To(const FROM&);
		};

		/// Do verb																					
		/// Used as a runtime dispatcher of composite types							
		struct Do : public Verb {
			LANGULUS(NAME) "Do";
			LANGULUS(INFO) "Used as a runtime dispatcher of composite types";

			Do(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

	} // namespace Langulus::Verbs


	/// Extend the logger to be capable of logging Block								
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (Logger::A::Interface& lhs, const Flow::Block& rhs) {
		return lhs << Verbs::Interpret::To<Flow::Debug>(rhs);
	}

} // namespace Langulus


/// Start an OR sequence of operations, that relies on short-circuiting to		
/// abort on a successful operation															
#define EitherDoThis [[maybe_unused]] volatile ::Langulus::Count _____________________sequence = 0 <

/// Add another operation to an OR sequence of operations, relying on			
/// short-circuiting to abort on a successful operation								
#define OrThis || 0 <

/// Return if any of the OR sequence operations succeeded							
#define AndReturnIfDone ; if (_____________________sequence) return

/// Enter scope if any of the OR sequence operations succeeded						
#define AndIfDone ; if (_____________________sequence)

#include "Verb.inl"