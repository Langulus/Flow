#pragma once
#include "Verb.hpp"

namespace Langulus::Flow
{

	///																								
	///	Bits for seek functions																
	///																								
	enum class SeekStyle : uint8_t {
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
		UpToHere = Above | Here,
		// Seek children and this context included								
		DownFromHere = Below | Here
	};

	constexpr bool operator & (const SeekStyle& lhs, const SeekStyle& rhs) {
		return (static_cast<int>(lhs) & static_cast<int>(rhs)) != 0;
	}


	///																								
	///	CONSTRUCT																				
	///																								
	///	Useful to describe complex (non-pod) content construction. This		
	/// applies to any domain - units, data, sound, geometry, materials,			
	/// textures, entity hierarchies, etc.													
	/// It is essentially the instructions required to generate this				
	/// content - usually arguments to a Create verb. First the required			
	/// components from the required modules, then the traits that determine	
	/// various characteristics, to finally the additional raw data in case		
	/// of specific custom content.															
	///																								
	class Construct : public Charge {
	private:
		DMeta mType {};
		Any mArguments;
		Hash mHash;

	public:
		Construct() = default;
		Construct(const Construct&) = default;
		Construct(Construct&&) noexcept = default;

		Construct(DMeta);
		Construct(DMeta, const Any&, const Charge&);
		Construct(DMeta, Any&&, const Charge&);

		Construct& operator = (const Construct&) = default;
		Construct& operator = (Construct&&) noexcept = default;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;

	public:
		NOD() Hash GetHash() const;

		template<CT::Data DATA>
		NOD() static Construct From(DMeta, DATA&&);
		template<CT::Data DATA>
		NOD() static Construct From(DMeta, const DATA&);

		template<CT::Data T, CT::Data DATA>
		NOD() static Construct From(DATA&&);
		template<CT::Data T, CT::Data DATA>
		NOD() static Construct From(const DATA&);

		template<CT::Data T>
		NOD() static Construct From();

		NOD() bool operator == (const Construct&) const noexcept;

		void StaticCreation(Any&) const;

		NOD() bool InterpretsAs(DMeta type) const;

		template<CT::Data T>
		NOD() bool InterpretsAs() const;

		NOD() bool Is(DMeta) const;

		template<CT::Data T>
		NOD() bool Is() const;

		NOD() const Any& GetAll() const noexcept;
		NOD() Any& GetAll() noexcept;

		NOD() const Charge& GetCharge() const noexcept;
		//NOD() Charge& GetCharge() noexcept;

		NOD() DMeta GetType() const noexcept;
		NOD() bool IsEmpty() const noexcept;

		void Clear();
		NOD() Construct Clone(DMeta = nullptr) const;

		template<CT::Data T>
		NOD() Construct CloneAs() const;

		template<CT::Data T>
		Construct& operator << (const T&);

		template<CT::Data T>
		Construct& operator <<= (const T&);

		Construct& Set(const Trait&, const Offset& = 0);
		const Trait* Get(TMeta, const Offset& = 0) const;

		template<CT::Trait T>
		const Trait* Get(const Offset& = 0) const;
	};

} // namespace Langulus::Flow


namespace Langulus
{

	/// Extend the logger to be capable of logging Construct							
	LANGULUS(ALWAYSINLINE) Logger::A::Interface& operator << (Logger::A::Interface& lhs, const Flow::Construct& rhs) {
		return lhs << Verbs::Interpret::To<Flow::Debug>(rhs);
	}

} // namespace Langulus

#include "Construct.inl"
