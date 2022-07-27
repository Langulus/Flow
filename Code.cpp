#include "Code.hpp"
#include "Serial.hpp"
#include "verbs/Do.inl"
#include "verbs/Associate.inl"
#include "verbs/Add.inl"
#include "verbs/Multiply.inl"
#include "verbs/Exponent.inl"

#define VERBOSE_INNER(a) \
		Logger::Verbose() << LANGULUS(FUNCTION_NAME) << ": " << a << " at " << progress << ": " << \
		Logger::Verbose() << " -- " \
			<< Logger::Green << input.RightOf(progress) \
			<< Logger::Gray << input.LeftOf(progress)

#define PRETTY_ERROR(a) { \
		Logger::Error() << LANGULUS(FUNCTION_NAME) << ": " << a << " at " << progress << ": " << \
		Logger::Error() << " -- " \
			<< Logger::Green << input.RightOf(progress) \
			<< Logger::Red << input.LeftOf(progress); \
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

	constexpr Code::TokenProperties Code::Token[OpCounter] = {
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


	/// The syntax of Code is characterized by this expression tree:				
	///																								
	///																								
	///		|-> Escape (colors, formatting)												
	///		|-> Space ('\n', '\r', '\t', ' ')											
	///		|-> Comment Scope ('|')															
	///		|-> Comment Line ('||')															
	///		|																						
	///	Skipped <---- Known -----> Number literal										
	///				    /	     \																
	///				   v	      v																
	///		   Keyword   ->   Operator														
	///			|							 |														
	///			|-> Data ID				 |-> Content Scope ('(', ')')					
	///			|-> Verb ID				 |-> String Scope ('"' or '`')				
	///			|-> Trait ID			 |-> Character Scope ('\'') 					
	///			|-> Constant ID		 |-> Code Scope ('[', ']')						
	///			|							 |-> Negate ('-' w/o lhs) 						
	///			|-> Reserved			 |-> Polarize ('<', '>')						
	///			|-> User-Defined		 |-> Source (':')									
	///			|-> Index        		 |-> Output ("as")								
	///			|        				 |-> Missing ('?')								
	///			|        				 |-> Or-Separator ("or")						
	///			|        				 |-> And-Separator (',' or "and")			
	///			|        				 |-> Select ('.' w/o lhs/rhs digit)			
	///			|        				 |-> Fraction ('.' with lhs/rhs digit)		
	///			|        				 |-> Add ('+')										
	///			|        				 |-> Add one to lhs ('++')						
	///			|        				 |-> Subtract ('-' with lhs)					
	///			|        				 |-> Subtract one from lhs ('--')			
	///			|        				 |-> Multiply ('*')								
	///			|        				 |-> Divide ('/')									
	///			|        				 |-> Time ('@')									
	///			|        				 |-> Power/Periodicity ('^')					
	///			|        				 |-> Priority ('!')								
	///			|        				 |-> Copy/Associate ('=')						
	///			+----------+----------+														
	///						  |																	
	///						  v																	
	///					  Content																
	///																								
	/// Base abstract Code expression														
	struct Expression {
		static Offset Parse(const Code&, Any&, int priority, bool optimize);
	};

	/// Abstract keyword expression															
	struct Keyword : public Expression {
		NOD() static Offset Parse(const Code&, Any&);
		NOD() inline static bool Peek(const Code& input) noexcept {
			return input.StartsWithLetter();
		}
	};

	/// Abstract skippable expression														
	struct Skipped : public Expression {
		NOD() static Offset Parse(const Code&);
		NOD() inline static bool Peek(const Code& input) noexcept {
			return input.StartsWithSkippable();
		}
	};

	/// Code number expression																	
	struct Number : public Expression {
		NOD() static Offset Parse(const Code&, Any&);
		NOD() inline static bool Peek(const Code& input) noexcept {
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
	};

	/// Abstract operator expression															
	struct Operator : public Expression {
		NOD() static Offset Parse(const Code&, Any&, int priority, bool optimize);
		NOD() inline static bool Peek(const Code& input) noexcept {
			for (Offset i = 0; i < Code::OpCounter; ++i) {
				if (input.StartsWithOperator(i))
					return true;
			}
			return false;
		}
	};

	/// Content scope operator																	
	struct OperatorContent : public Operator {
		NOD() static Offset Parse(const Code&, Any&, bool optimize);
		static void InsertContent(Any& input, Any& output);
	};

	/// String/character/code scope operator												
	struct OperatorString : public Operator {
		NOD() static Offset Parse(Code::Operator, const Code&, Any&);
	};

	/// Polarizer operator																		
	struct OperatorPolarize : public Operator {
		NOD() static Offset Parse(Code::Operator, const Code&, Any&, bool optimize);
	};

	/// Context operator																			
	/*struct OperatorContext : public Operator {
		NOD() static Offset Parse(const Code&, Any&, bool optimize);
	};

	/// As operator ("as")																		
	struct OperatorAs : public Operator {
		NOD() static Offset Parse(const Code&, Any&, bool optimize);
	};*/

	/// Copy operator																				
	struct OperatorCopy : public Operator {
		NOD() static Offset Parse(const Code&, Any&, bool optimize);
	};

	/// Missing operator																			
	struct OperatorMissing : public Operator {
		NOD() static Offset Parse(const Code&, Any&);
	};

	/// Separator (and/or) operator															
	struct OperatorSeparator : public Operator {
		NOD() static Offset Parse(Code::Operator, const Code&, Any&, bool optimize);
	};

	/// Dot operator																				
	struct OperatorSelect : public Operator {
		NOD() static Offset Parse(const Code&, Any&, bool optimize);
	};

	/// Add/subtract operator																	
	struct OperatorAdd : public Operator {
		NOD() static Offset Parse(Code::Operator, const Code&, Any&, bool optimize);
	};

	/// Multiply/divide operator																
	struct OperatorMultiply : public Operator {
		NOD() static Offset Parse(Code::Operator, const Code&, Any&, bool optimize);
	};

	/// Power/log operator																		
	struct OperatorPower : public Operator {
		NOD() static Offset Parse(const Code&, Any&, bool optimize);
	};

	/// Charge operators																			
	struct OperatorCharge : public Operator {
		NOD() static Offset Parse(Code::Operator, const Code&, Any&);
		NOD() inline static bool IsChargable(const Any& output) noexcept {
			return !output.IsMissing() && output.GetCount() == 1 &&
				output.Is<MetaData, MetaVerb, Verb>();
		}
	};


	/// Parse any Code expression																
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param priority - the current lhs priority									
	///	@return number of parsed characters												
	Offset Expression::Parse(const Code& input, Any& lhs, int priority, bool optimize) {
		Any rhs;
		Offset progress = 0;
		if (!lhs.IsValid())
			VERBOSE("Parsing unknown..." << Logger::Tab);
		else
			VERBOSE("Parsing unknown with LHS(" << lhs << ") " << Logger::Tab);

		try {
			while (progress < input.GetCount()) {
				// Scan input until end													
				Code relevant = input.LeftOf(progress);
				Offset localProgress = 0;

				if (relevant[0] == '\0')
					break;
				else if (Skipped::Peek(relevant))
					localProgress = Skipped::Parse(relevant);
				else if (Operator::Peek(relevant))
					localProgress = Operator::Parse(relevant, rhs, priority, optimize);
				else if (!rhs.IsValid()) {
					if (Keyword::Peek(relevant))
						localProgress = Keyword::Parse(relevant, rhs);
					else if (Number::Peek(relevant))
						localProgress = Number::Parse(relevant, rhs);
					else
						PRETTY_ERROR("Unexpected symbol");
				}
				else {
					// There's already something in RHS, so nest					
					// Make sure we parse in a fresh container and then push	
					Any subrhs;
					localProgress = Expression::Parse(relevant, subrhs, priority, optimize);
					rhs.SmartPush(subrhs);

					// ... and do an early exit to avoid endless loops			
					progress += localProgress;
					VERBOSE(Logger::Green << "Unknown resulted in " << rhs << Logger::Untab);
					lhs = Move(rhs);
					return progress;
				}

				if (0 == localProgress)
					break;

				progress += localProgress;
			}
		}
		catch (const Except::Flow& e) {
			VERBOSE(Logger::Error() << "Failed to parse: " << input);
			VERBOSE(Logger::Error() << "Due to exception: " << e.what() << Logger::Untab);
			throw;
		}

		// Input was parsed, relay content to output								
		VERBOSE(Logger::Green << "Unknown resulted in " << rhs << Logger::Untab);
		lhs = Move(rhs);
		return progress;
	}

	/// Parse a skippable, no content produced											
	///	@param input - code that starts with a skippable character				
	///	@return number of parsed characters												
	Offset Skipped::Parse(const Code& input) {
		Offset progress = 0;
		while (progress < input.GetCount()) {
			const auto relevant = input.LeftOf(progress);
			if (Peek(relevant))
				++progress;
			else break;
		}

		return progress;
	}

	/// Parse keyword																				
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset Keyword::Parse(const Code& input, Any& lhs) {
		Offset progress = 0;
		VERBOSE("Parsing keyword...");
		while (progress < input.GetCount()) {
			// Collect all characters of the keyword								
			const auto relevant = input.LeftOf(progress);
			if (!Keyword::Peek(relevant) && !Number::Peek(relevant))
				break;

			++progress;
		}

		if (0 == progress)
			PRETTY_ERROR("No progress at keyword parse");

		Any rhs;
		const auto keyword = input.RightOf(progress);
		if (keyword.IsEmpty())
			PRETTY_ERROR("No keyword parsed");

		VERBOSE("Parsed keyword: " << keyword);

		// Search for token in meta definitions									
		const auto metaData = RTTI::Database.GetMetaData(keyword);
		if (metaData)
			rhs << metaData;

		const auto metaTrait = RTTI::Database.GetMetaTrait(keyword);
		if (metaTrait)
			rhs << metaTrait;

		const auto metaVerb = RTTI::Database.GetMetaVerb(keyword);
		if (metaVerb) {
			// Check if the keyword is for a reverse verb						
			// Some verbs might have same tokens, so make sure they differ	
			const bool reversed =
				!CompareTokens(keyword, metaVerb->mToken) &&
				 CompareTokens(keyword, metaVerb->mTokenReverse);

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

	/// Parse an integer or real number														
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset Number::Parse(const Code& input, Any& lhs) {
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

	/// Parse an operator, operate on current output (lhs) and content (rhs)	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param priority - the priority of the last parsed element				
	///	@param optimize - the priority of the last parsed element				
	///	@return number of parsed characters												
	Offset Operator::Parse(const Code& input, Any& lhs, int priority, bool optimize) {
		// Determine operator															
		Offset progress = 0;
		auto op = Code::OpCounter;
		for (Offset i = 0; i < Code::OpCounter; ++i) {
			const bool typeMatch = !Code::Token[i].mCharge || lhs.Is<MetaData, MetaVerb>();
			if (input.StartsWithOperator(i) && typeMatch) {
				op = Code::Operator(i);
				progress += Code::Token[i].mToken.size();
				break;
			}
		}

		if (op == Code::OpCounter)
			PRETTY_ERROR("Unknown operator");

		VERBOSE("Parsing operator...");
		const Code relevant = input.LeftOf(progress);

		if (Code::Token[op].mPriority && priority >= Code::Token[op].mPriority) {
			VERBOSE(Logger::Yellow << "Delaying because of a prioritized operator...");
			return 0;
		}

		switch (op) {
		//case Code::As:
		//	return progress + OperatorAs::Parse(relevant, lhs, optimize);
		case Code::OpenScope:
			return progress + OperatorContent::Parse(relevant, lhs, optimize);
		case Code::CloseScope:
			return 0;
		case Code::OpenString:
		case Code::OpenStringAlt:
		case Code::OpenCode:
		case Code::OpenCharacter:
			return progress + OperatorString::Parse(op, relevant, lhs);
		//case Code::OpenByte:
		//	TODO();
		case Code::Past:
		case Code::Future:
			return progress + OperatorPolarize::Parse(op, relevant, lhs, optimize);
		//case Code::Context:
		//	return progress + OperatorContext::Parse(relevant, lhs, optimize);
		case Code::Associate:
			return progress + OperatorCopy::Parse(relevant, lhs, optimize);
		case Code::Missing:
			return progress + OperatorMissing::Parse(relevant, lhs);
		case Code::And:
		case Code::Or:
			return progress + OperatorSeparator::Parse(op, relevant, lhs, optimize);
		case Code::Select:
			return progress + OperatorSelect::Parse(relevant, lhs, optimize);
		case Code::Mass:
		case Code::Frequency:
		case Code::Time:
		case Code::Priority:
			if (OperatorCharge::IsChargable(lhs))
				return progress + OperatorCharge::Parse(op, relevant, lhs);
			else
				PRETTY_ERROR("Mass operator on non-chargable LHS: " << lhs);
		case Code::Add:
		case Code::Subtract:
			return progress + OperatorAdd::Parse(op, relevant, lhs, optimize);
		case Code::Multiply:
		case Code::Divide:
			return progress + OperatorMultiply::Parse(op, relevant, lhs, optimize);
		case Code::Power:
			return progress + OperatorPower::Parse(relevant, lhs, optimize);
		default:
			PRETTY_ERROR("Unexpected operator ID: " << int(op));
		}
	}

	/*struct VerbHelper {
		Any source;
		Any argument;
		Any output;*/

		/*operator Debug () {
			Code result;
			result += Verb{ uvInvalid, source, argument, output };
			return result;
		}*/
	//};

	/// Parse a content scope																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset OperatorContent::Parse(const Code& input, Any& lhs, bool optimize) {
		Offset progress = 0;

		// Can define contents for one element at a time						
		if (lhs.GetCount() > 1)
			PRETTY_ERROR("Content scope for multiple elements is not allowed: " << lhs);

		// We don't know what to expect, so we attempt blind parse			
		Any rhs;
		progress = Expression::Parse(input, rhs, Code::Token[Code::OpenScope].mPriority, optimize);
		if (!input.LeftOf(progress).StartsWithOperator(Code::CloseScope))
			PRETTY_ERROR("Missing closing bracket");

		// Account for the closing content scope									
		progress += Code::Token[Code::CloseScope].mToken.size();

		// Insert to new content in rhs to the already available lhs		
		OperatorContent::InsertContent(rhs, lhs);
		return progress;
	}

	/// Insert content to lhs, instantiating it											
	///	@param rhs - the content to insert												
	///	@param lhs - the place where the content will be inserted				
	void OperatorContent::InsertContent(Any& rhs, Any& lhs) {
		if (lhs.IsUntyped()) {
			// If output is untyped, we directly push content, regardless	
			// if it's filled with something or not - a scope is a scope	
			// Combine states!															
			const auto stateBackup = lhs.GetState();
			lhs = Move(rhs);
			lhs.AddState(stateBackup);
			VERBOSE_ALT("Untyped content: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<DMeta>()) {
			// The content is for a data scope										
			if (lhs.GetCount() > 1) {
				Logger::Error() << "Can't submit content to multiple DataIDs: " << lhs;
				Logger::Error() << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				Throw<Except::Flow>();
			}

			lhs.Reset();

			Construct outputConstruct(lhs.Get<DMeta>(), Move(rhs));
			try {
				outputConstruct.StaticCreation(lhs);
			}
			catch (...) {
				// Failed to construct, so just propagate request				
				VERBOSE_ALT(Logger::Red << "Can't statically construct " << outputConstruct);
				lhs = Move(outputConstruct);
			}
		}
		else if (lhs.Is<VMeta>()) {
			// Verbs are always constructed in place								
			if (lhs.GetCount() > 1) {
				Logger::Error() << "Can't submit content to multiple VerbIDs: " << lhs;
				Logger::Error() << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				Throw<Except::Flow>();
			}

			/*if (rhs.Is<VerbHelper>()) {
				const auto& helper = rhs.Get<VerbHelper>();
				lhs = Verb {
					lhs.Get<VMeta>(),
					Move(helper.source),
					Move(helper.argument),
					Move(helper.output)
				};
			}
			else {*/
				lhs = Verb {lhs.Get<VMeta>(), {}, Move(rhs)};
			//}

			VERBOSE_ALT("Constructed from VerbID: " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<Verb>()) {
			// Modified verb id (expanded when using . primarily)				
			if (lhs.GetCount() > 1) {
				Logger::Error() << "Can't submit content to multiple Verbs: " << lhs;
				Logger::Error() << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				Throw<Except::Flow>();
			}

			auto& verb = lhs.Get<Verb>();
			/*if (rhs.Is<VerbHelper>()) {
				const auto& helper = rhs.Get<VerbHelper>();
				verb.GetSource().SmartPush(helper.source);
				verb.GetArgument().SmartPush(helper.argument);
				verb.GetOutput().SmartPush(helper.output);
			}
			else {*/
				// Push content scope to the argument								
				if (verb.GetArgument().IsEmpty()) {
					// Push as argument													
					verb.GetArgument().SmartPush(rhs);
				}
				else {
					// An argument already exists, and the content should be	
					// for it, so nest													
					OperatorContent::InsertContent(rhs, verb.GetArgument());
				}
			//}

			VERBOSE_ALT("Constructed from Verb " << Logger::Cyan << lhs);
		}
		else if (lhs.Is<TMeta>()) {
			// Trait scope																	
			if (lhs.GetCount() > 1) {
				Logger::Error() << "Can't submit content to multiple TraitIDs: " << lhs;
				Logger::Error() << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				Throw<Except::Flow>();
			}

			lhs = Trait::From(lhs.Get<TMeta>(), Move(rhs));

			VERBOSE_ALT("Constructed trait " << Logger::Cyan << lhs);
		}
		else {
			Logger::Error() << "Bad scope for " << lhs << " (" << lhs.GetToken() << ")";
			Logger::Error() << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
			Throw<Except::Flow>();
		}
	}

	/// String/character/code scope															
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	Offset OperatorString::Parse(const Code::Operator op, const Code& input, Any& lhs) {
		Offset progress = 0;
		Offset depth = 1;
		while (progress < input.GetCount()) {
			// Collect all characters in scope, essentially gobbling them	
			// up into a text container until matching token is reached		
			const auto relevant = input.LeftOf(progress);

			switch (op) {
			case Code::OpenString:
				// Finish up a "string"													
				if (relevant.StartsWithOperator(Code::CloseString)) {
					const auto tokenSize = Code::Token[Code::CloseString].mToken.size();
					lhs = Text {input.RightOf(progress)};
					VERBOSE("Constructed string " << Logger::Cyan << lhs);
					return tokenSize + progress;
				}
				break;

			case Code::OpenStringAlt:
				// Finish up a `string`													
				if (relevant.StartsWithOperator(Code::CloseStringAlt)) {
					const auto tokenSize = Code::Token[Code::CloseStringAlt].mToken.size();
					lhs = Text {input.RightOf(progress)};
					VERBOSE("Constructed string " << Logger::Cyan << lhs);
					return tokenSize + progress;
				}
				break;

			case Code::OpenCharacter:
				// Finish up a 'c'haracter												
				if (relevant.StartsWithOperator(Code::CloseCharacter)) {
					const auto tokenSize = Code::Token[Code::CloseCharacter].mToken.size();
					lhs = input[0];
					VERBOSE("Constructed character " << Logger::Cyan << lhs);
					return tokenSize + progress;
				}
				break;

			case Code::OpenCode:
				// Finish up a [code]													
				if (relevant.StartsWithOperator(Code::OpenCode))
					++depth;
				else if (relevant.StartsWithOperator(Code::CloseCode)) {
					--depth;
					if (0 == depth) {
						const auto tokenSize = Code::Token[Code::CloseCode].mToken.size();
						lhs = input.RightOf(progress);
						VERBOSE("Constructed code " << Logger::Cyan << lhs);
						return tokenSize + progress;
					}
				}
				break;

			default:
				PRETTY_ERROR("Unexpected operator");
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
	Offset OperatorPolarize::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		Any rhs;
		auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		
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
			<< Code::Token[op].mToken << ") " << rhs << " -> " << Logger::Cyan << lhs);
		return progress;
	}

	/// Context operator	':'																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	/*Offset OperatorContext::Parse(const Code& input, Any& lhs, bool optimize) {
		Offset progress = 0;
		if (!lhs.IsValid())
			PRETTY_ERROR("Invalid context");
		PC_PARSE_VERBOSE("Context -> " << ccCyan << lhs);

		// Parse right side now, to avoid interfering with context			
		Any rhs;
		progress += Expression::Parse(input, rhs, Code::Token[Code::Context].mPriority, optimize);

		// Reflect and push a helper													
		TAny<VerbHelper> helper;
		helper << VerbHelper {Move(lhs), Move(rhs), {}};
		lhs = Move(static_cast<Any&>(helper));
		return progress;
	}*/

	/// As operator																				
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	/*Offset OperatorAs::Parse(const Code& input, Any& lhs, bool optimize) {
		// Parse RHS, and if valid, push Output(RHS) trait to output		
		Any rhs;
		const Offset progress = Expression::Parse(input, rhs, Code::Token[Code::As].mPriority, optimize);
		if (!rhs.IsValid())
			PRETTY_ERROR("Invalid RHS for AS operator");
		PC_PARSE_VERBOSE("Output -> " << ccCyan << rhs);

		if (lhs.Is<VerbHelper>()) {
			// Add to helper, if available											
			lhs.Get<VerbHelper>().output = Move(rhs);
		}
		else {
			// Reflect and push a helper												
			TAny<VerbHelper> helper;
			helper << VerbHelper {{}, Move(lhs), Move(rhs)};
			lhs = Move(static_cast<Any&>(helper));
		}

		return progress;
	}*/

	/// Missing operator '?'																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorMissing::Parse(const Code&, Any& lhs) {
		lhs.MakeMissing();
		VERBOSE_ALT("Missing -> " << Logger::Cyan << lhs);
		return 0;
	}

	/// Separator operators (and, or)														
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorSeparator::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
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
			progress = OperatorContent::Parse(input.LeftOf(scopeOffset), lhs, optimize);
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
			progress = OperatorContent::Parse(input.LeftOf(scopeOffset), lhs, optimize);
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
	}

	/// Add/subtract operator (+, -)															
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorAdd::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Perform addition or substraction on LHS and RHS						
		// Naturally, LHS is output; RHS is the newly parsed content		
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		if (optimize) {
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
		}

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
	Offset OperatorMultiply::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
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
	}

	/// Mass/time/frequency/priority operators (+, -, *, /, @, ^, !)				
	/// Can be applied to things with charge, like verbs and constructs			
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	Offset OperatorCharge::Parse(const Code::Operator op, const Code& input, Any& lhs) {
		// LHS must be chargable														
		Offset progress = 0;
		if (lhs.IsEmpty() || !IsChargable(lhs))
			PRETTY_ERROR("Invalid LHS(" << lhs << ") for charge operator");

		// RHS must evaluate to a number												
		// It can be either a number literal, or a number constant			
		Any rhs;
		if (Number::Peek(input))
			progress += Number::Parse(input, rhs);
		else if (Keyword::Peek(input))
			progress += Keyword::Parse(input, rhs);

		if (!rhs.CastsTo<A::Number>(1) || rhs.IsEmpty()) {
			PRETTY_ERROR("Invalid RHS(" << rhs << ") for charge operator '"
				<< Code::Token[op].mToken << "' on LHS(" << lhs << ")");
		}

		const auto asReal = rhs.AsCast<Real>();
		if (lhs.Is<VMeta>()) {
			// After operating on a verb ID, we always end up with a			
			// modified verb id, that is later used to construct a verb		
			Verb verb {lhs.Get<VMeta>()};

			switch (op) {
			case Code::Mass:
				verb.SetMass(asReal);
				break;
			case Code::Time:
				verb.SetTime(asReal);
				break;
			case Code::Frequency:
				verb.SetFrequency(asReal);
				break;
			case Code::Priority:
				verb.SetPriority(asReal);
				break;
			default:
				PRETTY_ERROR("Invalid verb charge operator: " << Code::Token[op].mToken);
			}

			VERBOSE(lhs << " " << Code::Token[op].mToken
				<< " " << rhs << " = " << Logger::Cyan << verb);
			lhs = verb;
		}
		else if (lhs.Is<DMeta>()) {
			Construct construct;
			construct = Construct {lhs.Get<DMeta>()};

			switch (op) {
			case Code::Mass:
				construct.mMass = asReal;
				break;
			case Code::Time:
				construct.mTime = asReal;
				break;
			case Code::Frequency:
				construct.mFrequency = asReal;
				break;
			case Code::Priority:
				construct.mPriority = asReal;
				break;
			default:
				PRETTY_ERROR("Invalid data charge operator: " << Code::Token[op].mToken);
			}

			VERBOSE(lhs << " " << Code::Token[op].mToken
				<< " " << rhs << " = " << Logger::Cyan << construct);
			lhs = construct;
		}
		else PRETTY_ERROR("Uncharged meta " << lhs << " for operator: " << Code::Token[op].mToken);

		return progress;
	}

	/// Generate code from operator															
	///	@param op - the operator to stringify											
	Code::Code(Operator op)
		: Text {Disowned(Token[op].mTokenWithSpacing.data())} { }

	/// Parse Code code																			
	///	@param optimize - whether or not to precalculate constexpr				
	///	@returned the parsed data															
	Any Code::Parse(bool optimize) const {
		Any output;
		const auto parsed = Expression::Parse(*this, output, 0, optimize);
		if (parsed != GetCount()) {
			Logger::Warning() << "Some characters were left out at the end, while parsing Code code:";
			Logger::Warning() << " -- " 
				<< Logger::Green << RightOf(parsed) 
				<< Logger::Red << LeftOf(parsed);
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
		for (auto& a : Code::Token) {
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

