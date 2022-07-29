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

	constexpr Code::OperatorProperties Code::mOperators[OpCounter] = {
		{ "(", "(", 0, false },		// OpenScope
		{ ")", ")", 0, false },		// CloseScope
		{ "[", "[", 0, false },		// OpenCode
		{ "]", "]", 0, false },		// CloseCode
		{ "|", "|", 0, false },		// OpenComment
		{ "|", "|", 0, false },		// CloseComment
		{ "\"", "\"", 0, false },	// OpenString
		{ "\"", "\"", 0, false },	// CloseString
		{ "`", "`", 0, false },		// OpenStringAlt
		{ "`", "`", 0, false },		// CloseStringAlt
		{ "'", "'", 0, false },		// OpenCharacter
		{ "'", "'", 0, false },		// CloseCharacter
		{ "<", "<", 12, false },	// PolarizeLeft
		{ ">", ">", 12, false },	// PolarizeRight
		//{ ":", ": ", 1, false },	// Context
		{ "=", " = ", 1, false },	// Copy
		{ "?", "?", 13, false },	// Missing
		{ ",", ", ", 2, false },	// AndSeparator
		{ "or", " or ", 2, false },// OrSeparator
		{ ".", ".", 7, false },		// Select
		{ "*", "*", 20, true },		// Mass
		{ "^", "^", 20, true },		// Frequency
		{ "@", "@", 20, true },		// Time
		{ "!", "!", 20, true },		// Priority
		{ "+", " + ", 4, false },	// Add
		{ "-", " - ", 4, false },	// Subtract
		{ "*", " * ", 5, false },	// Multiply
		{ "/", " / ", 5, false },	// Divide
		{ "^", " ^ ", 6, false },	// Power
		//{ "as", " as ", 1, false },// As
		{ "0x", "0x", 0, false },	// OpenByte
		{ "", "", 0, false }			// CloseByte
	};

	inline bool Code::ChargeParser::IsChargable(const Any& output) noexcept {
		return !output.IsMissing() && output.GetCount() == 1 &&
			output.Is<MetaData, MetaVerb, Verb>();
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
				else if (!rhs.IsValid()) {
					// RHS is empty, so we can parse a keyword or a number	
					// in it																	
					if (KeywordParser::Peek(relevant))
						localProgress = KeywordParser::Parse(relevant, rhs);
					else if (NumberParser::Peek(relevant))
						localProgress = NumberParser::Parse(relevant, rhs);
					else
						PRETTY_ERROR("Unexpected symbol");
				}
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
				}

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
		lhs = Abandon(rhs);
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

	/// Parse keyword																				
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param allowCharge - whether to parse charge (internal use)				
	///	@return number of parsed characters												
	Offset Code::KeywordParser::Parse(const Code& input, Any& lhs, bool allowCharge) {
		Offset progress = 0;
		VERBOSE("Parsing keyword...");
		while (progress < input.GetCount()) {
			// Collect all characters of the keyword								
			const auto relevant = input.RightOf(progress);
			if (!KeywordParser::Peek(relevant) && !NumberParser::Peek(relevant))
				break;

			++progress;
		}

		if (0 == progress)
			PRETTY_ERROR("No progress at keyword parse");

		// Isolate the keyword															
		Any rhs;
		const auto keyword = input.LeftOf(progress);
		if (keyword.IsEmpty())
			PRETTY_ERROR("No keyword parsed");

		VERBOSE("Parsed keyword: " << keyword);

		// Search for token in meta definitions									
		const auto metaData = RTTI::Database.GetMetaData(keyword);
		if (metaData) {
			//TODO parse charge and materialize if needed to
			rhs << metaData;
		}

		const auto metaTrait = RTTI::Database.GetMetaTrait(keyword);
		if (metaTrait) {
			//TODO parse charge and materialize if needed to
			rhs << metaTrait;
		}

		const auto metaVerb = RTTI::Database.GetMetaVerb(keyword);
		if (metaVerb) {
			// Check if the keyword is for a reverse verb						
			// Some verbs might have same tokens, so make sure they differ	
			const bool reversed =
				!CompareTokens(keyword, metaVerb->mToken) &&
				 CompareTokens(keyword, metaVerb->mTokenReverse);

			//TODO parse charge and materialize if needed to
			if (reversed) {
				// The keyword resulted in a negatively charged verb			
				rhs << Verb(metaVerb) * -1;
			}
			else {
				// The keyword resulted in a metaverb								
				rhs << metaVerb;
			}
		}

		if (rhs.IsEmpty())
			PRETTY_ERROR("Missing meta");

		VERBOSE("Keyword resulted in "
			<< Logger::Push << Logger::Cyan << keyword << Logger::Pop << " -> " 
			<< Logger::Cyan << rhs << " (" << rhs.GetToken() << ")");

		lhs = Move(rhs);
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

		VERBOSE(Logger::Green << "Number resulted in " << rhs);
		lhs << rhs;
		return progress;
	}

	/// Peek inside input, and return true if it begins with one of the			
	/// builtin or reflected operators														
	///	@param input - the code to peek into											
	///	@return true if input begins with an operators								
	Code::Operator Code::OperatorParser::Peek(const Code& input) noexcept {
		for (Offset i = 0; i < Code::OpCounter; ++i) {
			if (!mOperators[i].mCharge && input.StartsWithOperator(i))
				return Operator(i);
		}

		return NoOperator;
	}

	/// Parse op-expression, operate on current output (lhs) and content (rhs)	
	/// Beware, that charge-expressions are not handled here							
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param priority - the priority of the last parsed element				
	///	@param optimize - the priority of the last parsed element				
	///	@return number of parsed characters												
	Offset Code::OperatorParser::Parse(Operator op, const Code& input, Any& lhs, int priority, bool optimize) {
		Offset progress = 0;
		SAFETY(if (op == Code::NoOperator)
			PRETTY_ERROR("Unknown operator"));

		// Operators have priority, we can't execute a higher priority		
		// operator, before all lower priority operators have been taken	
		// care of																			
		if (mOperators[op].mPriority && priority >= mOperators[op].mPriority) {
			VERBOSE(Logger::Yellow << "Delaying operator " << mOperators[op].mToken
				<< " due to a prioritized operator");
			return 0;
		}

		// Skip the operator, we already know it									
		VERBOSE("Parsing operator " << mOperators[op].mToken);
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
		//case Code::OpenByte:
		//	TODO();
		case Past:
		case Future:
			return progress + ParsePolarize(op, relevant, lhs, optimize);
		case Missing:
			return progress + ParseMissing(relevant, lhs);
		// FInally, execute reflected operators									
		default:
			return progress + ParseReflected(op, relevant, lhs, optimize);
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
			const auto stateBackup = lhs.GetState();
			lhs = Move(rhs);
			lhs.AddState(stateBackup);
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
				VERBOSE_ALT(Logger::Red << "Can't statically construct " << outputConstruct
					<< "; Reason: " << e.what());

				if (lhs.GetCount() > 1) {
					lhs.RemoveIndex(-1);
					lhs << Abandon(outputConstruct);
				}
				else lhs = Abandon(outputConstruct);

				VERBOSE_ALT("Constructed from DMeta: " << Logger::Cyan << lhs);
				return;
			}

			// Precompiled successfully, append it to LHS						
			if (lhs.GetCount() > 1) {
				lhs.RemoveIndex(-1);
				lhs << Abandon(precompiled);
			}
			else lhs = Abandon(precompiled);

			VERBOSE_ALT("Constructed from DMeta: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<VMeta>()) {
			// The content is for an uninstantiated verb scope					
			Verb verb {lhs.As<VMeta>(-1), {}, Move(rhs)};
			if (lhs.GetCount() > 1) {
				lhs.RemoveIndex(-1);
				lhs << Abandon(verb);
			}
			else lhs = Abandon(verb);

			VERBOSE_ALT("Constructed from VMeta: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<TMeta>()) {
			// The content is for an uninstantiated trait scope				
			auto trait = Trait::From(lhs.As<TMeta>(-1), Move(rhs));
			if (lhs.GetCount() > 1) {
				lhs.RemoveIndex(-1);
				lhs << Abandon(trait);
			}
			else lhs = Abandon(trait);

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
					lhs = Text {input.LeftOf(progress)};
					VERBOSE("Constructed string " << Logger::Cyan << lhs);
					return tokenSize + progress;
				}
				break;
			}
			case Code::OpenCharacter: {
				// Finish up a 'c'haracter												
				//TODO handle escapes!
				if (relevant.StartsWithOperator(CloseCharacter)) {
					const auto tokenSize = mOperators[CloseCharacter].mToken.size();
					lhs = input[0];
					VERBOSE("Constructed character " << Logger::Cyan << lhs);
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
						lhs = input.LeftOf(progress);
						VERBOSE("Constructed code " << Logger::Cyan << lhs);
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

	/// Polarize contents																		
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParsePolarize(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		Any rhs;
		auto progress = UnknownParser::Parse(input, rhs, mOperators[op].mPriority, optimize);
		
		if (lhs.IsValid() && rhs.IsValid()) {
			// If both LHS and RHS are available, the polarizer acts as		
			// AND separator and concatenates LHS << RHS, polarizing both	
			// Example: do(left > right), do(right < left)						
			Any deeper;
			if (op == Code::Future) {
				lhs.MakePast();
				rhs.MakeFuture();
				deeper << Move(lhs) << Move(rhs);
			}
			else {
				lhs.MakeFuture();
				rhs.MakePast();
				deeper << Move(lhs) << Move(rhs);
			}

			lhs = Move(deeper);
		}
		else if (lhs.IsValid()) {
			// Only LHS is available, so polarize it								
			// Example: do(left>), do(right<)										
			if (op == Code::Future)
				lhs.MakeFuture();
			else
				lhs.MakePast();
		}
		else if (rhs.IsValid()) {
			// Only RHS is available, so polarize it								
			// Example: do(>right), do(<left)										
			if (op == Code::Past) {
				rhs.MakeFuture();
				lhs = Move(rhs);
			}
			else {
				rhs.MakePast();
				lhs = Move(rhs);
			}
		}

		VERBOSE("Polarize ("
			<< mOperators[op].mToken << ") " << rhs << " -> " << Logger::Cyan << lhs);
		return progress;
	}

	/// Missing operator '?'																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseMissing(const Code&, Any& lhs) {
		lhs.MakeMissing();
		VERBOSE_ALT("Missing -> " << Logger::Cyan << lhs);
		return 0;
	}

	/// Separator operators (and, or)														
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	/*Offset OperatorSeparator::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Output is LHS, new content is RHS, just do a smart push to LHS	
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		lhs.SmartPush(rhs, (op == Code::Or ? DataState::Or : DataState::Default));
		VERBOSE("Pushing (" 
			<< Code::Token[op].mToken << ") " << rhs << " -> " << Logger::Cyan << lhs);
		return progress;
	}

	/// Select operator (.)																		
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorSelect::Parse(const Code& input, Any& lhs, bool optimize) {
		Offset progress = 0;
		if (input.StartsWithOperator(Code::OpenScope)) {
			// Selection arguments provided as content on RHS					
			const auto scopeOffset = Code::Token[Code::OpenScope].mToken.size();
			lhs = Verbs::Select(Move(lhs));
			progress = OperatorContent::Parse(input.RightOf(scopeOffset), lhs, optimize);
			VERBOSE("Select operator: " << lhs);
			return scopeOffset + progress;
		}

		// Search for anything on the right side									
		// LHS is the context, RHS is what we select in it						
		// If no LHS exists, we select in current environment					
		// We wrap both RHS, and LHS into a Verbs::Select						
		Any rhs;
		progress = Expression::Parse(input, rhs, Code::Token[Code::Select].mPriority, optimize);
		if (rhs.IsEmpty()) {
			PRETTY_ERROR("Empty RHS for selection operator");
		}
		else if (rhs.GetCount() > 1) {
			PRETTY_ERROR("RHS(" << rhs << ") is too big");
		}

		if (rhs.Is<VMeta>())
			lhs = Verb(rhs.Get<VMeta>()).SetSource(Move(lhs));
		else if (rhs.Is<Verb>())
			lhs = Move(rhs.Get<Verb>().SetSource(Move(lhs)));
		else
			lhs = Verbs::Select(Move(lhs), Move(rhs));

		VERBOSE("Select operator: " << lhs);
		return progress;
	}

	/// Copy operator (=)																		
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorCopy::Parse(const Code& input, Any& lhs, bool optimize) {
		Offset progress = 0;
		if (input.StartsWithOperator(Code::OpenScope)) {
			// Copy specification provided as content on RHS					
			const auto scopeOffset = Code::Token[Code::OpenScope].mToken.size();
			lhs = Verbs::Associate(Move(lhs)).SetPriority(2);
			progress = OperatorContent::Parse(input.RightOf(scopeOffset), lhs, optimize);
			VERBOSE("Copy operator: " << lhs);
			return scopeOffset + progress;
		}

		// LHS is the context, RHS is what we associate with					
		// If no LHS exists, we simply copy in current environment			
		// We wrap both RHS, and LHS into a Verbs::Associate					
		Any rhs;
		progress = Expression::Parse(input, rhs, Code::Token[Code::Associate].mPriority, optimize);
		if (rhs.IsInvalid())
			PRETTY_ERROR("Invalid RHS for copy operator");

		// Invoke the verb in the context (slower)								
		Verbs::Associate copier({}, rhs);
		if (!DispatchDeep(lhs, copier)) {
			// If execution failed, just push the verb							
			lhs = Move(copier.SetSource(Move(lhs)).SetPriority(2));
			VERBOSE("Copy operator: " << lhs);
			return progress;
		}

		VERBOSE("Copied: " << copier);
		return progress;
	}*/

	/// Execute a reflected verb operator													
	///	@param op - the operator to execute												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] result of the operator goes here					
	///	@return number of parsed characters												
	Offset Code::OperatorParser::ParseReflected(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Parse RHS for the operator													
		Any rhs;
		const auto progress = UnknownParser::Parse(input, rhs, mOperators[op].mPriority, false);
		/*if (optimize) {
			if (op == Code::Subtract && lhs.IsEmpty() && rhs.Is<Real>() && !rhs.IsEmpty()) {
				// Quick optimization for inverting number in RHS				
				VERBOSE("Built-in inversion (-): " << rhs << " "
					<< Code::Token[op].mToken << " = " << -rhs.Get<Real>());
				lhs = -rhs.Get<Real>();
				return progress;
			}
			else if (lhs.Is<Real>() && rhs.Is<Real>() && !lhs.IsEmpty() && !rhs.IsEmpty()) {
				// LHS + RHS and LHS - RHS if both reals (optimization)		
				if (op == Code::Subtract) {
					VERBOSE("Built-in subtract: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<Real>() - rhs.Get<Real>()));
					lhs = lhs.Get<Real>() - rhs.Get<Real>();
				}
				else {
					VERBOSE("Built-in add: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<Real>() + rhs.Get<Real>()));
					lhs = lhs.Get<Real>() + rhs.Get<Real>();
				}
				return progress;
			}
		}*/
		TODO();
		if (lhs.IsEmpty() && op == Code::Subtract) {
			// No LHS, so we execute in RHS to invert								
			auto inverter = Verbs::Add().Invert();
			if (!optimize || !DispatchDeep(rhs, inverter)) {
				// If execution failed, just push the verb						
				lhs = Move(inverter.SetSource(Move(rhs)));
				VERBOSE("Invert (-) operator: " << lhs);
				return progress;
			}

			VERBOSE("Inverted (-): " << rhs << " " << Code::Token[op].mToken
				<< " = " << inverter.GetOutput());
			lhs = Move(inverter.GetOutput());
			return progress;
		}

		// Invoke the verb in the context (slower)								
		auto adder = Verbs::Add({}, rhs);
		if (op == Code::Subtract)
			adder.Invert();

		if (!optimize || !DispatchDeep(lhs, adder)) {
			// If execution failed, just push the verb							
			lhs = Move(adder.SetSource(Move(lhs)));
			VERBOSE("Add/subtract operator: " << lhs);
			return progress;
		}
		
		VERBOSE("Added/subtracted: " << lhs << " " << Code::Token[op].mToken
			<< " " << rhs << " = " << adder.GetOutput());
		lhs = Move(adder.GetOutput());
		return progress;
	}

	/// Multiply/divide operator (*, /)														
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	/*Offset OperatorMultiply::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Perform multiplication or division on LHS and RHS					
		// Naturally, LHS is output; RHS is the newly parsed content		
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		if (optimize) {
			if (op == Code::Divide && lhs.IsEmpty() && rhs.Is<Real>() && !rhs.IsEmpty()) {
				// Quick optimization for inverting number in RHS				
				VERBOSE("Built-in inversion (/): " << rhs << " "
					<< Code::Token[op].mToken << " = " << (Real(1) / rhs.Get<Real>()));
				lhs = Real(1) / rhs.Get<Real>();
				return progress;
			}
			else if (lhs.Is<Real>() && rhs.Is<Real>() && !lhs.IsEmpty() && !rhs.IsEmpty()) {
				// LHS * RHS and LHS / RHS if both reals (optimization)		
				if (op == Code::Multiply) {
					VERBOSE("Built-in multiply: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<Real>() * rhs.Get<Real>()));
					lhs = lhs.Get<Real>() * rhs.Get<Real>();
				}
				else {
					VERBOSE("Built-in divide: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<Real>() / rhs.Get<Real>()));
					lhs = lhs.Get<Real>() / rhs.Get<Real>();
				}
				return progress;
			}
		}

		if (lhs.IsEmpty() && op == Code::Divide) {
			// No LHS, so we execute in RHS to invert (1/x)						
			auto inverter = Verbs::Multiply().Invert();
			if (!optimize || !DispatchDeep(rhs, inverter)) {
				// If execution failed, just push the verb						
				lhs = Move(inverter.SetSource(Move(rhs)));
				VERBOSE("Invert (/) operator: " << lhs);
				return progress;
			}

			VERBOSE("Inverted (/): " << rhs << " " << Code::Token[op].mToken
				<< " = " << inverter.GetOutput());
			lhs = Move(inverter.GetOutput());
			return progress;
		}

		// Invoke the verb in the context (slower)								
		auto multiplier = Verbs::Multiply({}, rhs);
		if (op == Code::Divide)
			multiplier.Invert();

		if (!optimize || !DispatchDeep(lhs, multiplier)) {
			// If execution failed, just push the verb							
			lhs = Move(multiplier.SetSource(Move(lhs)));
			VERBOSE("Multiply/divide operator: " << lhs);
			return progress;
		}
		
		VERBOSE("Multiplied/divided: " << lhs << " " << Code::Token[op].mToken
			<< " " << rhs << " = " << multiplier.GetOutput());
		lhs = Move(multiplier.GetOutput());
		return progress;
	}

	/// Power operator (^, ^(1/...))															
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorPower::Parse(const Code& input, Any& lhs, bool optimize) {
		// Perform power on LHS and RHS												
		// Naturally, LHS is output; RHS is the newly parsed content		
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[Code::Power].mPriority, optimize);
		if (optimize) {
			if (lhs.Is<Real>() && rhs.Is<Real>() && !lhs.IsEmpty() && !rhs.IsEmpty()) {
				// LHS ^ RHS when both reals (optimization)						
				Real result = std::pow(lhs.Get<Real>(), rhs.Get<Real>());
				VERBOSE("Built-in exponentiation: " << lhs << " " << Code::Token[Code::Power].mToken
					<< " " << rhs << " = " << result);
				lhs = result;
				return progress;
			}
		}

		// Try invoking the verb in the context (slower)						
		auto exponentiator = Verbs::Exponent({}, rhs);
		if (!optimize || !DispatchDeep(lhs, exponentiator)) {
			// If execution failed, just push the verb							
			lhs = Move(exponentiator.SetSource(Move(lhs)));
			VERBOSE("Power operator: " << lhs);
			return progress;
		}
		
		VERBOSE("Exponentiated: " << lhs << " " << Code::Token[Code::Power].mToken
			<< " " << rhs << " = " << exponentiator.GetOutput());
		lhs = Move(exponentiator.GetOutput());
		return progress;
	}*/

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
	///	@param lhs - [in/out] parsed content goes here								
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

	/// Generate code from operator															
	///	@param op - the operator to stringify											
	Code::Code(Operator op)
		: Text {Disowned(mOperators[op].mTokenWithSpacing.data())} { }

	/// Parse code																					
	///	@param optimize - whether or not to precalculate constexpr				
	///	@returned the parsed data															
	Any Code::Parse(bool optimize) const {
		Any output;
		const auto parsed = UnknownParser::Parse(*this, output, 0, optimize);
		if (parsed != GetCount()) {
			Logger::Warning() << "Some characters were left out at the end, while parsing Code code:";
			Logger::Warning() << " -- " 
				<< Logger::Green << LeftOf(parsed) 
				<< Logger::Red << RightOf(parsed);
		}
		return output;
	}

	/// Clone the Code container retaining type											
	///	@return the cloned Code container												
	Code Code::Clone() const {
		return Text::Clone();
	}

	/// Check if a string is reserved as a Code keyword/operator					
	///	@param text - the text to check													
	///	@return true if text is reserved													
	bool Code::IsReserved(const Text& text) {
		for (auto& a : mOperators) {
			if (CompareTokens(text, a.mToken))
				return true;
		}
		return false;
	}

	/// A Code keyword must be made of only letters and numbers						
	///	@param text - the text to check													
	///	@return true if text is a valid Code keyword									
	bool Code::IsValidKeyword(const Text& text) {
		if (text.IsEmpty() || !::std::isalpha(text[0]))
			return false;

		for (auto a : text) {
			if (::std::isdigit(a) || ::std::isalpha(a))
				continue;
			return false;
		}

		return true;
	}

} // namespace Langulus::Flow

