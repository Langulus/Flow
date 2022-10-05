///																									
/// Langulus::Flow																				
/// Copyright(C) 2017 Dimo Markov <langulusteam@gmail.com>							
///																									
/// Distributed under GNU General Public License v3+									
/// See LICENSE file, or https://www.gnu.org/licenses									
///																									
#pragma once
#include "Common.hpp"

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
	///	It is essentially the instructions required to generate the				
	/// content - usually arguments to a creation verb. First the required		
	/// components from the required modules, then the traits that determine	
	/// various characteristics, to finally the additional raw data in case		
	/// of very specific custom contents.													
	///																								
	class Construct : public Any, public Charge {
		LANGULUS(POD) false;
		LANGULUS(NULLIFIABLE) false;
		LANGULUS(DEEP) false;
		LANGULUS_CONVERSIONS(Code, Debug);
		LANGULUS_BASES(Any, Charge);
	private:
		DMeta mType {};
		Hash mHash;

	public:
		Construct() = default;
		Construct(const Construct&) = default;
		Construct(Construct&&) noexcept = default;

		Construct(Disowned<Construct>&&) noexcept;
		Construct(Abandoned<Construct>&&) noexcept;

		Construct(DMeta);
		template<CT::Data T = Any>
		Construct(DMeta, const T&, const Charge& = {});
		template<CT::Data T = Any>
		Construct(DMeta, T&, const Charge& = {});
		template<CT::Data T = Any>
		Construct(DMeta, T&&, const Charge& = {});

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
		Construct(const Token&);
		template<CT::Data T = Any>
		Construct(const Token&, const T&, const Charge& = {});
		template<CT::Data T = Any>
		Construct(const Token&, T&, const Charge& = {});
		template<CT::Data T = Any>
		Construct(const Token&, T&&, const Charge& = {});
#endif
		Construct& operator = (const Construct&) = default;
		Construct& operator = (Construct&&) noexcept = default;

		Construct& operator = (Disowned<Construct>&&) noexcept;
		Construct& operator = (Abandoned<Construct>&&) noexcept;

		NOD() explicit operator Code() const;
		NOD() explicit operator Debug() const;

	public:
		NOD() Hash GetHash() const;

		template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
		NOD() static Construct From(const HEAD&, const TAIL&...);
		template<CT::Data T, CT::Data HEAD, CT::Data... TAIL>
		NOD() static Construct From(HEAD&&, TAIL&&...);
		template<CT::Data T>
		NOD() static Construct From();

		// Omit these inherited from Any												
		Any FromMeta() = delete;
		Any FromBlock() = delete;
		Any FromState() = delete;

		NOD() bool operator == (const Construct&) const;

		NOD() bool StaticCreation(Any&) const;

		NOD() bool CastsTo(DMeta type) const;
		template<CT::Data T>
		NOD() bool CastsTo() const;

		NOD() bool Is(DMeta) const;
		template<CT::Data T>
		NOD() bool Is() const;

		NOD() const Any& GetArgument() const noexcept;
		NOD() Any& GetArgument() noexcept;

		NOD() const Charge& GetCharge() const noexcept;
		NOD() Charge& GetCharge() noexcept;

		NOD() DMeta GetType() const noexcept;
		NOD() DMeta GetProducer() const noexcept;

		void Clear();
		NOD() Construct Clone(DMeta = nullptr) const;

		template<CT::Data T>
		NOD() Construct CloneAs() const;

		template<CT::Data T>
		Construct& operator << (const T&);
		template<CT::Data T>
		Construct& operator << (T&&);
		template<CT::Data T>
		Construct& operator >> (const T&);
		template<CT::Data T>
		Construct& operator >> (T&&);

		template<CT::Data T>
		Construct& operator <<= (const T&);
		template<CT::Data T>
		Construct& operator <<= (T&&);
		template<CT::Data T>
		Construct& operator >>= (const T&);
		template<CT::Data T>
		Construct& operator >>= (T&&);

		Construct& Set(const Trait&, const Offset& = 0);
		const Trait* Get(TMeta, const Offset& = 0) const;

		template<CT::Trait T>
		const Trait* Get(const Offset& = 0) const;
	};

} // namespace Langulus::Flow

#include "Construct.inl"
