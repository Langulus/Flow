#include "Code.hpp"
#include "Serial.hpp"
#include "verbs/Do.inl"
#include "verbs/Select.inl"
#include "verbs/Associate.inl"
#include "verbs/Add.inl"
#include "verbs/Multiply.inl"
#include "verbs/Exponent.inl"

#define VERBOSE_INNER(a) \
		Logger::Verbose() << LANGULUS(FUNCTION_NAME) << ": " << a << " at " << progress << ": " << \
		Logger::Verbose() << " -- " \
			<< Logger::Green << input.LeftOf(progress) \
			<< Logger::Gray << input.RightOf(progress)

#define PRETTY_ERROR(a) { \
		Logger::Error() << LANGULUS(FUNCTION_NAME) << ": " << a << " at " << progress << ": " << \
		Logger::Error() << " -- " \
			<< Logger::Green << input.LeftOf(progress) \
			<< Logger::Red << input.RightOf(progress); \
		Throw<Except::Flow>("Parse error"); \
	}

#define VERBOSE(a) VERBOSE_INNER(a)
#define VERBOSE_ALT(a) Logger::Verbose() << a


namespace Langulus::Flow
{

	/// Built-in operator properties															
	constexpr Code::OperatorProperties Code::mOperators[OpCounter] = {
		{ "(", 0, false },	// OpenScope
		{ ")", 0, false },	// CloseScope
		{ "[", 0, false },	// OpenCode
		{ "]", 0, false },	// CloseCode
		{ "|", 0, false },	// OpenComment
		{ "|", 0, false },	// CloseComment
		{ "\"", 0, false },	// OpenString
		{ "\"", 0, false },	// CloseString
		{ "`", 0, false },	// OpenStringAlt
		{ "`", 0, false },	// CloseStringAlt
		{ "'", 0, false },	// OpenCharacter
		{ "'", 0, false },	// CloseCharacter
		{ "0x", 0, false },	// OpenByte
		{ "past", 13, false },		// Past
		{ "future", 13, false },	// Future
		{ "?", 13, false },			// Missing
		{ "const", 13, false },		// Constant
		{ "sparse", 13, false },	// Sparse
		{ "*", 20, true },	// Mass
		{ "^", 20, true },	// Frequency
		{ "@", 20, true },	// Time
		{ "!", 20, true }		// Priority
	};
	
	/// Generate code from operator															
	///	@param op - the operator to stringify											
	Code::Code(Operator op)
		: Text {Disowned(mOperators[op].mToken.data())} { }

	/// Parse code																					
	///	@param optimize - whether or not to precompile 								
	///	@returned the parsed flow															
	Any Code::Parse(bool optimize) const {
		Any output;
		const auto parsed = UnknownParser::Parse(*this, output, 0, optimize);
		if (parsed != GetCount()) {
			Logger::Warning() << "Some characters were left out at the end, while parsing code:";
			Logger::Warning() << " -- " 
				<< Logger::Green << LeftOf(parsed) 
				<< Logger::Red << RightOf(parsed);
		}
		return output;
	}

	/// Clone the Code container retaining type											
	///	@return the cloned code																
	Code Code::Clone() const {
		return Text::Clone();
	}

	/// Compare two tokens, ignoring case													
	///	@param lhs - the left token														
	///	@param rhs - the right token														
	///	@return true if both loosely match												
	constexpr bool CompareTokens(const Token& lhs, const Token& rhs) noexcept {
		return (lhs.size() == rhs.size() && (
			lhs.size() == 0 || ::std::equal(lhs.begin(), lhs.end(), rhs.begin(),
				[](const char& c1, const char& c2) noexcept {
					return c1 == c2 || (::std::toupper(c1) == ::std::toupper(c2));
				})
			));
	}

	/// Isolate an operator token																
	///	@param token - the operator														
	///	@return the isolated operator token												
	constexpr Token IsolateOperator(const Token& token) noexcept {
		// Skip skippable at the front and the back of token					
		auto l = token.data();
		auto r = token.data() + token.size();
		while (l < r && *l <= 32)
			++l;
		while (r > l && *(r - 1) <= 32)
			--r;
		return token.substr(l - token.data(), r - l);
	}

	/// Compare two operators, ignoring case and spacing								
	///	@param lhs - the left operator													
	///	@param rhs - the right operator													
	///	@return true if both loosely match												
	constexpr bool CompareOperators(const Token& lhs, const Token& rhs) noexcept {
		// Compare isolated operators													
		return CompareTokens(IsolateOperator(lhs), IsolateOperator(rhs));
	}

	/// Check if a string is reserved as a keyword/operator							
	///	@param text - the text to check													
	///	@return true if text is reserved													
	bool Code::IsReserved(const Text& text) {
		for (auto& a : mOperators) {
			if (CompareOperators(text, a.mToken))
				return true;
		}

		if (!RTTI::Database.GetAmbiguousMeta(text).empty())
			return true;

		return false;
	}

	/// A keyword must be made of only letters and numbers, namespace operator	
	/// and/or underscores																		
	///	@param text - the text to check													
	///	@return true if text is a valid Code keyword									
	bool IsKeywordSymbol(char a) {
		return ::std::isdigit(a) || ::std::isalpha(a) || a == ':' || a == '_';
	}

	/// A keyword must be made of only letters and numbers, namespace operator	
	/// and/or underscores																		
	///	@param text - the text to check													
	///	@return true if text is a valid Code keyword									
	bool Code::IsValidKeyword(const Text& text) {
		if (text.IsEmpty() || !::std::isalpha(text[0]))
			return false;

		for (auto a : text) {
			if (IsKeywordSymbol(a))
				continue;
			return false;
		}

		return true;
	}

	/// Parse any Code expression																
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param priority - the current lhs priority									
	///	@return number of parsed characters												
	Offset Code::UnknownParser::Parse(const Code& input, Any& lhs, int priority, bool optimize) {
		Any rhs;
		Offset progress = 0;
		if (!lhs.IsValid())
			VERBOSE("Parsing unknown..." << Logger::Tab);
		else
			VERBOSE("Parsing unknown with LHS(" << lhs << ") " << Logger::Tab);

		try {
			while (progress < input.GetCount()) {
				// Scan input until end													
				Code relevant = input.RightOf(progress);
				Offset localProgress = 0;
				Operator op;

				if (relevant[0] == '\0')
					break;
				else if (SkippedParser::Peek(relevant))
					localProgress = SkippedParser::Parse(relevant);
				else if ((op = OperatorParser::Peek(relevant)) != Operator::NoOperator)
					localProgress = OperatorParser::Parse(op, relevant, rhs, priority, optimize);
				else //if (!rhs.IsValid()) {
					// RHS is empty, so we can parse a keyword or a number	
					// in it																	
					if (KeywordParser::Peek(relevant))
						localProgress = KeywordParser::Parse(relevant, rhs);
					else if (NumberParser::Peek(relevant))
						localProgress = NumberParser::Parse(relevant, rhs);
					else
						PRETTY_ERROR("Unexpected symbol");
				/*}
				else {
					// There's already something in RHS, so nest					
					// Make sure we parse in a fresh container and then push	
					Any subrhs;
					localProgress = UnknownParser::Parse(relevant, subrhs, priority, optimize);
					rhs.SmartPush(subrhs);

					// ... and do an early exit to avoid endless loops			
					progress += localProgress;
					VERBOSE(Logger::Green << "Unknown parsed: " << rhs << Logger::Untab);
					lhs = Abandon(rhs);
					return progress;
				}*/

				if (0 == localProgress) {
					// This occurs often, when a higher priority operator is	
					// waiting for lower priority stuff to be parsed first	
					break;
				}

				progress += localProgress;
			}
		}
		catch (const Except::Flow& e) {
			VERBOSE(Logger::Error() << "Failed to parse: " << input 
				<< "; Reason: " << e.what() << Logger::Untab);
			throw;
		}

		// Input was parsed, relay content to output								
		VERBOSE(Logger::Green << "Unknown parsed: " << rhs << Logger::Untab);
		lhs.SmartPush(Abandon(rhs));
		return progress;
	}

	/// Peek inside input, and return true if first symbol is skippable			
	///	@param input - the code to peek into											
	///	@return true if input is skippable												
	bool Code::SkippedParser::Peek(const Code& input) noexcept {
		return input.StartsWithSkippable();
	}

	/// Parse a skippable, no content produced											
	///	@param input - code that starts with a skippable character				
	///	@return number of parsed characters												
	Offset Code::SkippedParser::Parse(const Code& input) {
		Offset progress = 0;
		while (progress < input.GetCount()) {
			const auto relevant = input.RightOf(progress);
			if (Peek(relevant))
				++progress;
			else break;
		}

		return progress;
	}

	/// Peek inside input, and return true if first symbol is a character		
	///	@param input - the code to peek into											
	///	@return true if input is a character											
	bool Code::KeywordParser::Peek(const Code& input) noexcept {
		return input.StartsWithLetter();
	}
	
	/// Gather all symbols of a keyword														
	///	@param input - the code to peek into											
	///	@return the isolated keyword token												
	Token Code::KeywordParser::Isolate(const Code& input) noexcept {
		Offset progress = 0;
		while (progress < input.GetCount()) {
			const auto c = input[progress];
			if (!IsKeywordSymbol(c))
				break;
			++progress;
		}

		if (0 == progress)
			return {};

		return input.LeftOf(progress);
	}
	
	/// Parse keyword for a constant, data, or trait									
	/// Verbs are considered operators, not keywords									
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param allowCharge - whether to parse charge (internal use)				
	///	@return number of parsed characters												
	Offset Code::KeywordParser::Parse(const Code& input, Any& lhs, bool allowCharge) {
		Offset progress = 0;
		VERBOSE("Parsing keyword...");

		// Isolate the keyword															
		const auto keyword = Isolate(input);
		if (keyword.empty())
			PRETTY_ERROR("No keyword parsed");

		progress += keyword.size();
		VERBOSE("Keyword collected: " << keyword);

		// Search for an exact token in meta definitions						
		const auto dmeta = RTTI::Database.GetMetaData(keyword);
		const auto tmeta = RTTI::Database.GetMetaTrait(keyword);
		const auto cmeta = RTTI::Database.GetMetaConstant(keyword);
		if (dmeta && !tmeta && !cmeta) {
			// Exact non-ambiguous data definition found							
			if (allowCharge) {
				const auto relevant = input.RightOf(progress);
				if (ChargeParser::Peek(relevant) != NoOperator) {
					// Parse charge for the keyword									
					Charge charge;
					progress += ChargeParser::Parse(relevant, charge);
					lhs << Construct {dmeta, {}, charge};
				}
				else lhs << dmeta;
			}
			else lhs << dmeta;
		}
		else if (!dmeta && tmeta && !cmeta) {
			// Exact non-ambiguous trait definition found						
			lhs << tmeta;
		}
		else if (!dmeta && !tmeta && cmeta) {
			lhs.SmartPush(Any {
				Block {{}, cmeta->mValueType, 1, cmeta->mPtrToValue}
			}.Clone());
		}
		else {
			// Search for ambiguous token in meta definitions					
			auto& symbols = RTTI::Database.GetAmbiguousMeta(keyword);
			if (symbols.size() > 1) {
				Logger::Error() << "Ambiguous symbol: " << keyword << "; Could be one of: " << Logger::Tab;
				for (auto& meta : symbols) {
					Logger::Verbose() << Logger::Red << meta->mToken << " (";
					switch (meta->GetMetaType()) {
					case RTTI::Meta::Data:
						Logger::Append() << "meta data)";
						break;
					case RTTI::Meta::Trait:
						Logger::Append() << "meta trait)";
						break;
					case RTTI::Meta::Constant:
						Logger::Append() << "meta constant)";
						break;
					}
				}
				Logger::Append() << Logger::Untab;
				PRETTY_ERROR("Ambiguous symbol");
			}

			// Push found meta data, if any											
			for (auto& meta : symbols) {
				switch (meta->GetMetaType()) {
				case RTTI::Meta::Data:
					if (allowCharge) {
						const auto relevant = input.RightOf(progress);
						if (ChargeParser::Peek(relevant) != NoOperator) {
							// Parse charge for the keyword							
							Charge charge;
							progress += ChargeParser::Parse(relevant, charge);
							lhs << Construct {static_cast<DMeta>(meta), {}, charge};
						}
						else lhs << static_cast<DMeta>(meta);
					}
					else lhs << static_cast<DMeta>(meta);
					break;

				case RTTI::Meta::Trait:
					lhs << static_cast<TMeta>(meta);
					break;
				
				case RTTI::Meta::Constant: {
					const auto metaConst = static_cast<CMeta>(meta);
					lhs.SmartPush(Any {
						Block {{}, metaConst->mValueType, 1, metaConst->mPtrToValue}
					}.Clone());
					break;
				}}
			}
		}

		VERBOSE("Keyword parsed: "
			<< Logger::Push << Logger::Cyan << keyword << Logger::Pop << " -> " 
			<< Logger::Cyan << lhs << " (" << lhs.GetToken() << ")");

		return progress;
	}

	/// Peek inside input, and return true if first symbol is a digit, or a		
	/// minus followed by a digit																
	///	@param input - the code to peek into											
	///	@return true if input begins with a number									
	bool Code::NumberParser::Peek(const Code& input) noexcept {
		if (input.StartsWithDigit())
			return true;
		else for (auto c : input) {
			if (c == '-' || c <= 32)
				continue;
			else
				return ::std::isdigit(c);
		}
		return false;
	}

	/// Parse an integer or real number														
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset Code::NumberParser::Parse(const Code& input, Any& lhs) {
		Real rhs = 0;
		Offset progress = 0;
		VERBOSE("Parsing number...");

		if (auto [p, ec] = ::std::from_chars(input.GetRaw(), input.GetRaw() + input.GetCount(), rhs); 
			ec == ::std::errc()) {
			progress = p - input.GetRaw();
		}

		VERBOSE(Logger::Green << "Number parsed: " << rhs);
		lhs << rhs;
		return progress;
	}

	/// Peek inside input, and return true if it begins with one of the			
	/// builtin or reflected operators														
	///	@param input - the code to peek into											
	///	@return true if input begins with an operators								
	Code::Operator Code::OperatorParser::Peek(const Code& input) noexcept {
		// Compare against the built-in operators									
		for (Offset i = 0; i < Code::OpCounter; ++i) {
			if (!mOperators[i].mCharge && input.StartsWithOperator(i))
				return Operator(i);
		}

		// Compare against reflected operators										
		const auto word = Isolate(input);
		auto found = RTTI::Database.GetOperator(word);
		if (found)
			return ReflectedOperator;

		found = RTTI::Database.GetMetaVerb(word);
		if (found)
			return ReflectedVerb;

		return NoOperator;
	}

	/// Isolate an operator																		
	///	@param input - the code to parse													
	///	@return the isolated operator														
	Token Code::OperatorParser::Isolate(const Code& input) noexcept {
		// These can be either a word separated by operators/spaces, or	
		// operators separated by spaces/numbers/chatacters					
		if (input.StartsWithLetter()) {
			// Isolate a keyword separated by spaces/operators					
			return KeywordParser::Isolate(input);
		}

		// Isolate an operator separated by spaces/letters/digits			
		Offset progress = 0;
		while (progress < input.GetCount()) {
			const auto relevant = input.RightOf(progress);
			if (KeywordParser::Peek(relevant) || NumberParser::Peek(relevant) || SkippedParser::Peek(relevant))
				break;
			++progress;
		}

		if (0 == progress)
			return {};

		return input.LeftOf(progress);
	}

	/// Parse op-expression, operate on current output (lhs) and content (rhs)	
	/// Beware, that charge-expressions are not handled here							
	///	@param op - the built-in operator if any, or Reflected					
	///	@param input - the code to parse													
	///	@param lhs - [in/out] the operator expression will go here				
	///	@param priority - the priority of the last parsed element				
	///	@param optimize - the priority of the last parsed element				
	///	@return number of parsed characters												
	Offset Code::OperatorParser::Parse(Operator op, const Code& input, Any& lhs, int priority, bool optimize) {
		Offset progress = 0;
		if (op < NoOperator) {
			// Handle a built-in operator												
			// Operators have priority, we can't execute a higher priority	
			// operator, before all lower priority operators have been		
			// taken	care of																
			if (mOperators[op].mPriority && priority >= mOperators[op].mPriority) {
				VERBOSE(Logger::Yellow << "Delaying operator " << mOperators[op].mToken
					<< " due to a prioritized operator");
				return 0;
			}

			// Skip the operator, we already know it								
			VERBOSE("Parsing built-in operator: " << mOperators[op].mToken);
			progress += mOperators[op].mToken.size();
			const Code relevant = input.RightOf(progress);

			switch (op) {
				// Handle built-in operators first									
			case OpenScope:
				return progress + ParseContent(relevant, lhs, optimize);
			case CloseScope:
				return 0;
			case OpenString:
			case OpenStringAlt:
			case OpenCode:
			case OpenCharacter:
				return progress + ParseString(op, relevant, lhs);
			case OpenByte:
				return progress + ParseBytes(relevant, lhs);
			case Past:
			case Future:
				return progress + ParsePhase(op, relevant, lhs, optimize);
			case Constant:
				return progress + ParseConst(relevant, lhs, optimize);
			case Sparse:
				return progress + ParseSparse(relevant, lhs, optimize);
			case Missing:
				return progress + ParseMissing(relevant, lhs);
			default:
				PRETTY_ERROR("Unhandled built-in operator");
			}
		}
		else if (op == ReflectedOperator) {
			// Handle a reflected operator											
			const auto word = Isolate(input);
			const auto found = RTTI::Database.GetOperator(word);

			VERBOSE("Parsing reflected operator: " << word << " (" << found->mToken << ")");
			progress += word.size();
			const Code relevant = input.RightOf(progress);
			Verb operation {found};
			if (CompareOperators(word, found->mOperatorReverse))
				operation.SetMass(-1);

			return progress + ParseReflected(operation, relevant, lhs, optimize);
		}
		else {
			// Handle a reflected verb													
			const auto word = Isolate(input);
			const auto found = RTTI::Database.GetMetaVerb(word);

			VERBOSE("Parsing reflected verb: " << word << " (" << found->mToken << ")");
			progress += word.size();
			const Code relevant = input.RightOf(progress);
			Verb operation {found};
			if (CompareOperators(word, found->mTokenReverse))
				operation.SetMass(-1);

			return progress + ParseReflected(operation, relevant, lhs, optimize);
		}
	}

	/// Parse a content scope																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseContent(const Code& input, Any& lhs, bool optimize) {
		Offset progress = 0;

		// Can define contents for one element at a time						
		if (lhs.GetCount() > 1)
			PRETTY_ERROR("Content scope for multiple elements is not allowed: " << lhs);

		// We don't know what to expect, so we attempt blind parse			
		Any rhs;
		progress = UnknownParser::Parse(input, rhs, mOperators[OpenScope].mPriority, optimize);
		if (!input.RightOf(progress).StartsWithOperator(CloseScope))
			PRETTY_ERROR("Missing closing bracket");

		// Account for the closing content scope									
		progress += mOperators[CloseScope].mToken.size();

		// Insert to new content in rhs to the already available lhs		
		InsertContent(rhs, lhs);
		return progress;
	}

	/// Insert content to lhs, instantiating it if we have to						
	/// Content is always inserted to the last element in LHS, if multiple		
	/// elements are present. If last element is a meta definition, the			
	/// definition will be replaced by the instantiated element						
	///	@param rhs - the content to insert												
	///	@param lhs - the place where the content will be inserted				
	void Code::OperatorParser::InsertContent(Any& rhs, Any& lhs) {
		if (lhs.IsUntyped()) {
			// If output is untyped, we directly push content, regardless	
			// if it's filled with something or not - a scope is a scope	
			// Also, don't forget to combine states!								
			//const auto stateBackup = lhs.GetState();
			lhs.SmartPush(Move(rhs));
			//lhs.AddState(stateBackup);
			VERBOSE_ALT("Untyped content: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<DMeta>()) {
			// The content is for an uninstantiated data scope					
			const auto meta = lhs.As<DMeta>(-1);
			Any precompiled;
			Construct outputConstruct {meta, Move(rhs)};
			try {
				outputConstruct.StaticCreation(precompiled);
			}
			catch (const Exception& e) {
				// Failed to precompile, so just propagate request				
				VERBOSE_ALT(Logger::Red 
					<< "Can't statically construct " << outputConstruct
					<< "; Reason: " << e.what());

				if (lhs.GetCount() > 1) {
					lhs.RemoveIndex(-1);
					lhs << Abandon(outputConstruct);
				}
				else lhs << Abandon(outputConstruct);

				VERBOSE_ALT("Constructed from DMeta: " << Logger::Cyan << lhs);
				return;
			}

			// Precompiled successfully, append it to LHS						
			if (lhs.GetCount() > 1) {
				lhs.RemoveIndex(-1);
				lhs << Abandon(precompiled);
			}
			else lhs << Abandon(precompiled);

			VERBOSE_ALT("Constructed from DMeta: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<VMeta>()) {
			// The content is for an uninstantiated verb scope					
			Verb verb {lhs.As<VMeta>(-1), Move(rhs)};
			if (lhs.GetCount() > 1) {
				lhs.RemoveIndex(-1);
				lhs << Abandon(verb);
			}
			else lhs << Abandon(verb);

			VERBOSE_ALT("Constructed from VMeta: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<TMeta>()) {
			// The content is for an uninstantiated trait scope				
			auto trait = Trait {lhs.As<TMeta>(-1), Move(rhs)};
			if (lhs.GetCount() > 1) {
				lhs.RemoveIndex(-1);
				lhs << Abandon(trait);
			}
			else lhs << Abandon(trait);

			VERBOSE_ALT("Constructed from TMeta: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<Verb>()) {
			// The content is for an instantiated verb scope					
			auto& verb = lhs.As<Verb>(-1);
			verb.GetArgument() = Move(rhs);
			VERBOSE_ALT("Constructed from Verb " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<Construct>()) {
			// The content is for an instantiated data scope					
			auto& construct = lhs.As<Construct>(-1);
			construct.GetAll() = Move(rhs);
			VERBOSE_ALT("Constructed from Construct " << Logger::Cyan << lhs);
		}
		else {
			Logger::Error() << "Bad scope for " << lhs << " (" << lhs.GetToken() << ")";
			Logger::Error() << "Content to insert is: " << rhs << " (" << rhs.GetToken() << ")";
			Throw<Except::Flow>("Syntax error - bad scope");
		}
	}

	/// String/character/code scope															
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseString(const Code::Operator op, const Code& input, Any& lhs) {
		Offset progress = 0;
		Offset depth = 1;
		while (progress < input.GetCount()) {
			// Collect all characters in scope, essentially gobbling them	
			// up into a text container until matching token is reached		
			const auto relevant = input.RightOf(progress);

			switch (op) {
			case Code::OpenString:
			case Code::OpenStringAlt: {
				// Finish up a "string" or `string`									
				//TODO handle escapes!
				const auto closer = op == OpenString ? CloseString : CloseStringAlt;
				if (relevant.StartsWithOperator(closer)) {
					const auto tokenSize = mOperators[closer].mToken.size();
					lhs << Text {input.LeftOf(progress)};
					VERBOSE("String parsed: " << Logger::Cyan << lhs);
					return tokenSize + progress;
				}
				break;
			}
			case Code::OpenCharacter: {
				// Finish up a 'c'haracter												
				//TODO handle escapes!
				if (relevant.StartsWithOperator(CloseCharacter)) {
					const auto tokenSize = mOperators[CloseCharacter].mToken.size();
					lhs << input[0];
					VERBOSE("Character parsed: " << Logger::Cyan << lhs);
					return tokenSize + progress;
				}
				break;
			}
			case Code::OpenCode: {
				// Finish up a [code]													
				// Nested code scopes are handled gracefully						
				if (relevant.StartsWithOperator(OpenCode))
					++depth;
				else if (relevant.StartsWithOperator(CloseCode)) {
					--depth;
					if (0 == depth) {
						const auto tokenSize = mOperators[CloseCode].mToken.size();
						lhs << input.LeftOf(progress);
						VERBOSE("Code parsed: " << Logger::Cyan << lhs);
						return tokenSize + progress;
					}
				}
				break;
			}
			default:
				PRETTY_ERROR("Unexpected string operator");
			}

			++progress;
		}

		PRETTY_ERROR("Unexpected EOF when parsing string/character/code");
	}

	/// Byte scope parser																		
	///	@param input - the code to parse													
	///	@param lhs - [in/out] here goes the byte sequence							
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseBytes(const Code& input, Any& lhs) {
		Offset progress = 0;
		while (progress < input.GetCount()) {
			const auto c = input[progress];
			if (::std::isdigit(c)) {
				++progress;
				continue;
			}

			const auto lc = ::std::tolower(c);
			if (lc >= 'a' && lc <= 'f') {
				++progress;
				continue;
			}

			break;
		}

		// Parse all bytes																
		Bytes result;
		auto i = input.GetRaw();
		const auto iEnd = i + progress;
		uint8_t stager {};
		uint8_t shifter {4};
		while (i != iEnd) {
			stager |= uint8_t(*i - (::std::isdigit(*i) ? '0' : 'a')) << shifter;

			if (shifter == 0) {
				result << Byte {stager};
				stager = {};
				shifter = 4;
			}
			else shifter = 0;

			++i;
		}

		// There might be a leftover byte											
		if (shifter == 0)
			result << Byte {stager};

		lhs << Abandon(result);
		return progress;
	}

	/// Phase contents																			
	///	@param op - the phase operator													
	///	@param input - the code to parse													
	///	@param lhs - [in/out] phased content goes here								
	///	@param optimize - whether or not to attempt precompile RHS				
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParsePhase(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		//Any rhs;
		//auto progress = UnknownParser::Parse(input, rhs, -1, optimize);
	
		if (op == Code::Past) {
			//rhs.MakeFuture();
			lhs.MakePast();// = Abandon(rhs);
		}
		else {
			//rhs.MakePast();
			lhs.MakeFuture();// = Abandon(rhs);
		}

		VERBOSE_ALT("Parsed phased: " << Logger::Cyan << lhs);
		return 0;
	}

	/// Const contents																			
	///	@param input - the code to parse													
	///	@param lhs - [in/out] constant content goes here							
	///	@param optimize - whether or not to attempt precompile RHS				
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseConst(const Code& input, Any& lhs, bool optimize) {
		//Any rhs;
		//auto progress = UnknownParser::Parse(input, rhs, -1, optimize);

		//rhs.MakeConst();
		//lhs = Abandon(rhs);
		lhs.MakeConst();

		VERBOSE_ALT("Parsed constant: " << Logger::Cyan << lhs);
		return 0;
	}

	/// Sparse contents																			
	///	@param input - the code to parse													
	///	@param lhs - [in/out] sparse content goes here								
	///	@param optimize - whether or not to attempt precompile RHS				
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseSparse(const Code& input, Any& lhs, bool optimize) {
		/*Any rhs;
		rhs.MakeSparse();
		auto progress = UnknownParser::Parse(input, rhs, -1, optimize);
		lhs = Abandon(rhs);*/
		lhs.MakeSparse();

		VERBOSE_ALT("Parsed sparse: " << Logger::Cyan << lhs);
		return 0;
	}

	/// Missing contents																			
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseMissing(const Code&, Any& lhs) {
		lhs.MakeMissing();
		VERBOSE_ALT("Parsed missing: " << Logger::Cyan << lhs);
		return 0;
	}

	/// Execute a reflected verb operator													
	///	@param op - the operator to execute												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] result of the operator goes here					
	///	@param optimize - whether or not to attempt executing at compile-time
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseReflected(Verb& op, const Code& input, Any& lhs, bool optimize) {
		// Parse RHS for the operator													
		const auto progress = UnknownParser::Parse(
			input, op.GetArgument(), op.GetPriority(), optimize);

		// Then dispatch the operation in lhs, with the given arguments,	
		// trying to execute it at compile-time. This is allowed only if	
		// lhs and rhs do not have anything missing inside						
		if (optimize && !op.IsMissingDeep() && DispatchDeep(lhs, op)) {
			// The verb was executed at compile-time, so directly				
			// substitute LHS with the result										
			lhs = Move(op.GetOutput());
		}
		else {
			// Either compile-time execution is impossible, or we don't		
			// want it, so directly substitute LHS with the verb				
			op.SetSource(Move(lhs));
			lhs = Move(op);
		}

		return progress;
	}

	/// Peek inside input, and return true if it begins with one of the			
	/// builtin operators for charging														
	///	@param input - the code to peek into											
	///	@return true if input begins with an operator for charging				
	Code::Operator Code::ChargeParser::Peek(const Code& input) noexcept {
		for (Offset i = 0; i < Code::OpCounter; ++i) {
			if (mOperators[i].mCharge && input.StartsWithOperator(i))
				return Operator(i);
		}

		return NoOperator;
	}

	/// Mass/time/frequency/priority operators (+, -, *, /, @, ^, !)				
	/// Can be applied to things with charge, like verbs and constructs			
	///	@param input - the code to parse													
	///	@param charge - [out] parsed charge goes here								
	///	@return number of parsed characters												
	Offset Code::ChargeParser::Parse(const Code& input, Charge& charge) {
		Offset progress = 0;
		VERBOSE("Parsing charge... " << Logger::Tab);

		try {
			while (progress < input.GetCount()) {
				// Scan input until end of charge operators/code				
				const auto relevant = input.RightOf(progress);
				if (relevant[0] == '\0')
					break;

				const auto op = ChargeParser::Peek(input);
				if (op == Operator::NoOperator)
					return progress;

				// For each charge operator encountered - parse a RHS			
				Offset localProgress = 0;
				Any rhs;
				if (SkippedParser::Peek(relevant))
					// Skip empty space and escape symbols							
					localProgress = SkippedParser::Parse(relevant);
				else if (KeywordParser::Peek(relevant))
					// Charge parameter can be a keyword, like a constant, 	
					// but is not allowed to have charge on its own, to		
					// avoid endless nesting - you must wrap it in a scope	
					localProgress = KeywordParser::Parse(relevant, rhs, false);
				else if (NumberParser::Peek(relevant))
					// Can be a literal number											
					localProgress = NumberParser::Parse(relevant, rhs);
				else if (OperatorParser::Peek(relevant) == Operator::OpenScope)
					// Can be anything wrapped in a scope							
					localProgress = OperatorParser::Parse(Operator::OpenScope, relevant, rhs, 0, true);
				else
					PRETTY_ERROR("Unexpected symbol");

				if (0 == localProgress)
					break;

				progress += localProgress;

				// Save changes															
				// AsCast may throw here, if RHS did not evaluate or convert
				// to real - this is later caught and handled gracefully		
				const auto asReal = rhs.AsCast<Real>();
				switch (op) {
				case Code::Mass:
					charge.mMass = asReal;
					break;
				case Code::Frequency:
					charge.mFrequency = asReal;
					break;
				case Code::Time:
					charge.mTime = asReal;
					break;
				case Code::Priority:
					charge.mPriority = asReal;
					break;
				default:
					PRETTY_ERROR("Invalid charge operator: " << mOperators[op].mToken);
				}
			}
		}
		catch (const Except::Flow& e) {
			VERBOSE(Logger::Error() << "Failed to parse charge: " << input 
				<< "; Reason: " << e.what() << Logger::Untab);
			throw;
		}

		VERBOSE("Charge parsed" << Logger::Untab);
		return progress;
	}

} // namespace Langulus::Flow

