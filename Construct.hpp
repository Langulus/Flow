#pragma once
#include "Verb.hpp"

namespace PCFW::Flow
{

	/// Bits for seek functions																
	enum class SeekStyle : pcu8 {
		// Seek entities that are children of the context						
		Forward = 1,
		// Seek entities that are parents of the context						
		Backward = 2,
		// Seek objects in both - parents and children							
		Bidirectional = Forward | Backward,
		// Include the current entity in the seek operation					
		Here = 4,
		// Seek everywhere																
		Everywhere = Bidirectional | Here,
		// Seek parents and this context included									
		UpToHere = Backward | Here,
		// Seek children and this context included								
		DownFromHere = Forward | Here
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
	class LANGULUS_MODULE(FLOW) Construct : public Hashed {
		REFLECT(Construct);
	public:
		Construct() = default;
		Construct(const Construct&) = default;
		Construct(Construct&&) noexcept = default;

		Construct(DataID);
		Construct(DMeta);
		Construct(const Text&);

		Construct(DataID, Any&&);
		Construct(DMeta, Any&&);
		Construct(const Text&, Any&&);

		Construct(DataID, const Any&);
		Construct(DMeta, const Any&);
		Construct(const Text&, const Any&);

		Construct& operator = (const Construct&) = default;
		Construct& operator = (Construct&&) noexcept = default;

		NOD() explicit operator GASM() const;
		NOD() explicit operator Debug() const;

	public:
		NOD() Hash GetHash() const override;

		template<RTTI::ReflectedData DATA>
		NOD() static Construct From(DMeta, DATA&&);
		template<RTTI::ReflectedData DATA>
		NOD() static Construct From(DMeta, const DATA&);

		template<RTTI::ReflectedData T, RTTI::ReflectedData DATA>
		NOD() static Construct From(DATA&&);
		template<RTTI::ReflectedData T, RTTI::ReflectedData DATA>
		NOD() static Construct From(const DATA&);

		template<RTTI::ReflectedData T>
		NOD() static Construct From();

		NOD() bool operator == (const Construct&) const noexcept;
		NOD() bool operator != (const Construct&) const noexcept;

		void StaticCreation(Any&) const;

		NOD() bool InterpretsAs(DMeta type) const;

		template<RTTI::ReflectedData T>
		NOD() bool InterpretsAs() const;

		NOD() bool Is(DataID type) const;

		template<RTTI::ReflectedData T>
		NOD() bool Is() const;

		NOD() const Any& GetAll() const noexcept;
		NOD() Any& GetAll() noexcept;

		NOD() const Charge& GetCharge() const noexcept;
		NOD() Charge& GetCharge() noexcept;

		NOD() DMeta GetMeta() const noexcept;
		NOD() bool IsEmpty() const noexcept;

		void Clear();
		NOD() Construct Clone(DMeta = nullptr) const;

		template<RTTI::ReflectedData T>
		NOD() Construct CloneAs() const;

		template<RTTI::ReflectedData T>
		Construct& operator << (const T&);

		template<RTTI::ReflectedData T>
		Construct& operator <<= (const T&);

		Construct& Set(const Trait&, const pcptr& = 0);
		const Trait* Get(TMeta, const pcptr& = 0) const;

		template<RTTI::ReflectedTrait T>
		const Trait* Get(const pcptr& = 0) const;

		Charge mCharge{};

	private:
		DMeta mHeader{};
		Any mArguments;
	};

} // namespace PCFW::Flow

#include "Construct.inl"
