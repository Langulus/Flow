#pragma once
#include <Langulus.Logger.hpp>
#include <Langulus.Anyness.hpp>

namespace Langulus::Flow
{

	using namespace ::Langulus::Anyness;


	///																								
	/// Charge, carrying the four verb dimensions, and state							
	///																								
	struct Charge {
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
		NOD() constexpr bool operator != (const Charge&) const noexcept;

		NOD() constexpr bool IsDefault() const noexcept;
		NOD() Hash GetHash() const noexcept;

	public:
		// Mass of the verb																
		Real mMass = DefaultMass;
		// Frequency of the verb														
		Real mFrequency = DefaultFrequency;
		// Time of the verb																
		Real mTime = DefaultTime;
		// Priority of the verb															
		Real mPriority = DefaultPriority;
	};

	class Construct;
	class Code;


	///																								
	///	THE UNIVERSAL VERB																	
	///																								
	/// It's practically a single call to the framework, or a single statement	
	/// in a Code flow. Piception is based around natural language processing	
	/// theory based around verbs, so this is the natural name for such thing	
	///																								
	class Verb {
	private:
		// Verb meta, mass, frequency, time and priority						
		VMeta mVerb {};
		// Verb charge and state														
		Charge mCharge {};
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

		template<CT::Verb T>
		NOD() static Verb From(const Charge&, const Any& = {}, const Any& = {}, const Any& = {});

		Verb& operator = (const Verb&) = default;
		Verb& operator = (Verb&&) noexcept = default;

		NOD() explicit operator Code() const;
		//NOD() explicit operator Debug() const;

	public:
		NOD() Hash GetHash() const;

		NOD() Verb PartialCopy() const noexcept;
		NOD() Verb Clone() const;
		void Reset();

		NOD() bool Is(VMeta) const noexcept;
		template<CT::Verb T>
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

		NOD() bool operator == (const Verb&) const;
		NOD() bool operator == (VMeta) const noexcept;
		NOD() bool operator == (bool) const noexcept;
		NOD() bool operator <  (const Verb&) const noexcept;
		NOD() bool operator >  (const Verb&) const noexcept;
		NOD() bool operator >= (const Verb&) const noexcept;
		NOD() bool operator <= (const Verb&) const noexcept;

		template<CT::Data T> Verb& operator << (const T&);
		template<CT::Data T> Verb& operator << (T&&);
		template<CT::Data T> Verb& operator >> (const T&);
		template<CT::Data T> Verb& operator >> (T&&);
		template<CT::Data T> Verb& operator <<= (const T&);
		template<CT::Data T> Verb& operator >>= (const T&);

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

	using Script = TAny<Verb>;

	namespace Verbs
	{

		class Create : public Verb {
		public:
			Create(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Select : public Verb {
		public:
			Select(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Associate : public Verb {
		public:
			Associate(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Scope : public Verb {
		public:
			Scope(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Add : public Verb {
		public:
			Add(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Multiply : public Verb {
		public:
			Multiply(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Exponent : public Verb {
		public:
			Exponent(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Catenate : public Verb {
		public:
			Catenate(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Interpret : public Verb {
		public:
			Interpret(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

		class Do : public Verb {
		public:
			Do(const Any& = {}, const Any& = {}, const Any& = {}, const Charge& = {}, bool = true);
		};

	} // namespace Langulus::Anyness::Traits

} // namespace Langulus::Flow


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