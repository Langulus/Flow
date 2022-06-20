#pragma once
#include "IncludeMemory.hpp"

namespace PCFW::Flow
{

	///																								
	/// Charge, carrying the four verb dimensions										
	///																								
	struct Charge {
		static constexpr real DefaultMass { 1 };
		static constexpr real DefaultFrequency { 0 };
		static constexpr real DefaultTime { 0 };

		static constexpr real DefaultPriority { 0 };
		static constexpr real MinPriority { -10000 };
		static constexpr real MaxPriority { +10000 };

		constexpr Charge(
			real = DefaultMass,
			real = DefaultFrequency,
			real = DefaultTime,
			real = DefaultPriority) noexcept;

		NOD() constexpr bool operator == (const Charge&) const noexcept;
		NOD() constexpr bool operator != (const Charge&) const noexcept;

		NOD() constexpr bool IsDefault() const noexcept;
		NOD() Hash GetHash() const noexcept;

	public:
		// Mass of the verb																
		real mMass = DefaultMass;
		// Frequency of the verb														
		real mFrequency = DefaultFrequency;
		// Time of the verb																
		real mTime = DefaultTime;
		// Priority of the verb															
		real mPriority = DefaultPriority;
	};


	///																								
	/// Verb ID that carries a charge														
	///																								
	struct EMPTY_BASE() ChargedVerbID : NAMED {
		REFLECT(ChargedVerbID);
	public:
		constexpr ChargedVerbID(VMeta = nullptr, Charge = {}, bool shortCircuited = true) noexcept;

		NOD() constexpr bool operator == (const ChargedVerbID&) const noexcept;
		NOD() constexpr bool operator != (const ChargedVerbID&) const noexcept;

		NOD() Hash GetHash() const noexcept;

	public:
		// Verb meta, as registered in the system									
		VMeta mID{};
		// Verb charge																		
		Charge mCharge{};
		// Verb short-circuiting														
		bool mShortCircuited{ true };
	};


	class Construct;
	class GASM;


	///																								
	///	THE UNIVERSAL VERB																	
	///																								
	/// It's practically a single call to the framework, or a single statement	
	/// in a GASM flow. Piception is based around natural language processing	
	/// theory based around verbs, so this is the natural name for such thing	
	///																								
	class EMPTY_BASE() LANGULUS_MODULE(FLOW) Verb : NAMED {
		REFLECT(Verb);
	public:
		Verb() noexcept {}
		Verb(const Verb&) = default;
		Verb(Verb&&) noexcept = default;
		~Verb() = default;

		Verb(VMeta, const Any& = {}, const Any& = {}, const Any& = {});
		Verb(const VerbID&, const Any& = {}, const Any& = {}, const Any& = {});
		Verb(const ChargedVerbID&, const Any& = {}, const Any& = {}, const Any& = {});
		Verb(const Text&, const Any& = {}, const Any& = {}, const Any& = {});

		template<RTTI::ReflectedVerb T>
		NOD() static Verb From(const Any& = {}, const Any& = {}, const Any& = {});

		Verb& operator = (const Verb&) = default;
		Verb& operator = (Verb&&) noexcept = default;

		NOD() explicit operator GASM() const;
		NOD() explicit operator Debug() const;

	public:
		NOD() Hash GetHash() const;

		NOD() Verb PartialCopy() const noexcept;
		NOD() Verb Clone() const;
		void Reset();

		NOD() bool Is(const VerbID&) const noexcept;
		template<RTTI::ReflectedVerb T>
		NOD() bool Is() const noexcept;

		NOD() VerbID GetID() const noexcept;
		NOD() const ChargedVerbID& GetChargedID() const noexcept;
		NOD() auto GetSwitch() const noexcept;
		NOD() VMeta GetMeta() const noexcept;
		NOD() real GetMass() const noexcept;
		NOD() real GetFrequency() const noexcept;
		NOD() real GetTime() const noexcept;
		NOD() real GetPriority() const noexcept;

		NOD() Any& GetSource() noexcept;
		NOD() const Any& GetSource() const noexcept;
		NOD() Any& GetArgument() noexcept;
		NOD() const Any& GetArgument() const noexcept;
		NOD() Any& GetOutput() noexcept;
		NOD() const Any& GetOutput() const noexcept;
		template<RTTI::ReflectedTrait T>
		NOD() bool OutputsTo() const noexcept;

		NOD() bool Validate(const Index&) const noexcept;
		NOD() Verb& ShortCircuit(bool) noexcept;
		NOD() LiteralText GetToken() const;
		NOD() bool IsDone() const noexcept;
		void Done() noexcept;
		void Undo() noexcept;
		Verb& Invert() noexcept;

		Verb& SetVerb(const VerbID&) noexcept;
		Verb& SetMass(real) noexcept;
		Verb& SetFrequency(real) noexcept;
		Verb& SetTime(real) noexcept;
		Verb& SetPriority(real) noexcept;
		Verb& SetCharge(const Charge&) noexcept;
		Verb& SetSource(const Any&);
		Verb& SetSource(Any&&) noexcept;
		Verb& SetArgument(const Any&);
		Verb& SetArgument(Any&&) noexcept;
		Verb& SetOutput(const Any&);
		Verb& SetOutput(Any&&) noexcept;
		Verb& SetAll(const Any&, const Any&, const Any&);

		NOD() bool operator == (const Verb&) const;
		NOD() bool operator != (const Verb&) const;

		NOD() bool operator == (const VerbID&) const noexcept;
		NOD() bool operator != (const VerbID&) const noexcept;

		NOD() bool operator == (bool) const noexcept;
		NOD() bool operator != (bool) const noexcept;
		NOD() bool operator <  (const Verb&) const noexcept;
		NOD() bool operator >  (const Verb&) const noexcept;
		NOD() bool operator >= (const Verb&) const noexcept;
		NOD() bool operator <= (const Verb&) const noexcept;

		template<RTTI::ReflectedData T> Verb& operator << (const T&);
		template<RTTI::ReflectedData T> Verb& operator << (T&&);
		template<RTTI::ReflectedData T> Verb& operator >> (const T&);
		template<RTTI::ReflectedData T> Verb& operator >> (T&&);
		template<RTTI::ReflectedData T> Verb& operator <<= (const T&);
		template<RTTI::ReflectedData T> Verb& operator >>= (const T&);

	public:
		static bool ExecuteScope(Any&, const Any&);
		static bool ExecuteScope(Any&, const Any&, Any&);
		static bool ExecuteScope(Any&, const Any&, Any&, bool& skipVerbs);
		static bool ExecuteVerb(Any&, Verb&);
		static pcptr DispatchEmpty(Verb&);
		static pcptr DispatchDeep(Block&, Verb&, bool resolveElements = true, bool allowCustomDispatch = true, bool allowDefaultVerbs = true);
		static pcptr DispatchFlat(Block&, Verb&, bool resolveElements = true, bool allowCustomDispatch = true, bool allowDefaultVerbs = true);
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

	private:
		// Verb meta, mass, frequency, time and priority						
		ChargedVerbID mVerb{};
		// The number of successful executions										
		pcptr mSuccesses = 0;
		// Verb context																	
		Any mSource;
		// Argument for the call														
		Any mArgument;
		// The container where output goes											
		Any mOutput;
	};

	using Script = TAny<Verb>;

} // namespace PCFW::PCGASM

#include "Verb.inl"