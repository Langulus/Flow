#include "Code.hpp"
#include "Serial.hpp"

#define PC_PARSE_VERBOSE_INNER(a) \
		pcLogFuncVerbose << a << " at " << progress << ": " << \
		pcLogVerbose << " -- " << ccGreen << LiteralText{input.CropRight(progress)} \
			<< ccGray << LiteralText{input.CropLeft(progress)}

#define PC_PARSE_VERBOSE(a) //PC_PARSE_VERBOSE_INNER(a)
#define PC_PARSE_VERBOSE_ALT(a) //pcLogFuncVerbose << a

#define PRETTY_ERROR(txt) \
	throw Except::Code( \
		pcLogFuncError << txt << " at " << progress << ": " << \
		pcLogError << " -- " << ccGreen << LiteralText{input.CropRight(progress)} \
			<< ccRed << LiteralText{input.CropLeft(progress)} \
	)

namespace Langulus::Flow
{

	/// Stringify charge																			
	///	@param charge - the charge to serialize										
	Code::Code(const Charge& charge) {
		// Write mass																		
		if (charge.mMass != Charge::DefaultMass) {
			(*this) += Code::Mass;
			(*this) += charge.mMass;
		}

		// Write frequency																
		if (charge.mFrequency != Charge::DefaultFrequency) {
			(*this) += Code::Frequency;
			(*this) += charge.mFrequency;
		}

		// Write time																		
		if (charge.mTime != Charge::DefaultTime) {
			(*this) += Code::Time;
			(*this) += charge.mTime;
		}

		// Write priority																	
		if (charge.mPriority != Charge::DefaultPriority) {
			(*this) += Code::Priority;
			(*this) += charge.mPriority;
		}
	}

	/// Stringify charged verb id, taking mass into account							
	///	@param verb - the charged verb to serialize									
	Code::Code(const Verb& verb) {
		Charge temp = verb.mCharge;
		if (!verb.mID)
			(*this) += VerbID::DefaultToken;
		else if (verb.mCharge.mMass < 0) {
			(*this) += verb.mID->GetTokenReverse();
			temp.mMass *= real(-1);
		}
		else (*this) += verb.mID->GetToken();
		(*this) += Code(temp);
	}

	/// Serialize a map																			
	///	@param map - the map to serialize												
	Code::Code(const Map& map) {
		(*this) += Map::GetMapToken(map.Keys().GetMeta(), map.Values().GetMeta());
		(*this) += Code::OpenScope;
			(*this) += Code::OpenScope;
				(void)Detail::SerializeBlockToText(map.Keys(), *this);
			(*this) += Code::CloseScope;
			(*this) += Code::AndSeparator;
			(*this) += Code::OpenScope;
				(void)Detail::SerializeBlockToText(map.Values(), *this);
			(*this) += Code::CloseScope;
		(*this) += Code::CloseScope;
	}

	/// Stringify Code operator																
	///	@param op - the operator to stringify											
	Code::Code(Code::Operator op)
		: Text {Code::Token[op].mTokenWithSpacing} {}

	/// Construct from a hash																	
	///	@param from - the hash to stringify												
	Code::Code(const Hash& from) : Code() {
		(*this) += DataID::Of<Hash>;
		(*this) += Code::OpenScope;
		(*this) += pcToHex(from.GetValue());
		(*this) += Code::CloseScope;
	}

	const Code::TokenProperties Code::Token[OpCounter] = {
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
		{ ":", ": ", 1, false },	// Context
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
		{ "as", " as ", 1, false },// As
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
		static pcptr Parse(const Code& input, Any& output, int priority, bool optimize);
	};

	/// Abstract Code keyword expression													
	struct Keyword : public Expression {
		NOD() static pcptr Parse(const Code& input, Any& output);
		NOD() inline static bool Peek(const Code& input) noexcept {
			return input.IsLetter();
		}
	};

	/// Abstract Code skippable expression													
	struct Skipped : public Expression {
		NOD() static pcptr Parse(const Code& input);
		NOD() inline static bool Peek(const Code& input) noexcept {
			return input.IsSkippable();
		}
	};

	/// Code number expression																	
	struct Number : public Expression {
		NOD() static pcptr Parse(const Code& input, Any& output);
		NOD() inline static bool Peek(const Code& input) noexcept {
			if (input.IsNumber())
				return true;
			else for (auto c : input) {
				if (c == '-' || pcIsSpecialChar(c))
					continue;
				else return pcIsNumber(c);
			}
			return false;
		}
	};

	/// Abstract Code operator expression													
	struct Operator : public Expression {
		NOD() static pcptr Parse(const Code& input, Any& output, int priority, bool optimize);
		NOD() inline static bool Peek(const Code& input) noexcept {
			for (pcptr i = 0; i < Code::OpCounter; ++i) {
				if (input.IsOperator(i))
					return true;
			}
			return false;
		}
	};

	/// Content scope operator																	
	struct OperatorContent : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output, bool optimize);
		static void InsertContent(Any& input, Any& output);
	};

	/// String/character/code scope operator												
	struct OperatorString : public Operator {
		NOD() static pcptr Parse(Code::Operator, const Code& input, Any& output);
	};

	/// Polarizer operator																		
	struct OperatorPolarize : public Operator {
		NOD() static pcptr Parse(Code::Operator, const Code& input, Any& output, bool optimize);
	};

	/// Context operator																			
	struct OperatorContext : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output, bool optimize);
	};

	/// As operator ("as")																		
	struct OperatorAs : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output, bool optimize);
	};

	/// Copy operator																				
	struct OperatorCopy : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output, bool optimize);
	};

	/// Missing operator																			
	struct OperatorMissing : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output);
	};

	/// Separator (and/or) operator															
	struct OperatorSeparator : public Operator {
		NOD() static pcptr Parse(Code::Operator, const Code& input, Any& output, bool optimize);
	};

	/// Dot operator																				
	struct OperatorSelect : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output, bool optimize);
	};

	/// Add/subtract operator																	
	struct OperatorAdd : public Operator {
		NOD() static pcptr Parse(Code::Operator, const Code& input, Any& output, bool optimize);
	};

	/// Multiply/divide operator																
	struct OperatorMultiply : public Operator {
		NOD() static pcptr Parse(Code::Operator, const Code& input, Any& output, bool optimize);
	};

	/// Power/log operator																		
	struct OperatorPower : public Operator {
		NOD() static pcptr Parse(const Code& input, Any& output, bool optimize);
	};

	/// Charge operators																			
	struct OperatorCharge : public Operator {
		NOD() static pcptr Parse(Code::Operator, const Code& input, Any& output);
		NOD() inline static bool IsChargable(const Any& output) noexcept {
			return !output.IsMissing() && output.GetCount() == 1 && (
				output.Is<VerbID>() ||
				output.Is<ChargedVerbID>() ||
				output.Is<DataID>() /*TODO ||
				output.Is<ChargedDataID>()*/
			);
		}
	};


	/// Parse any Code expression																
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param priority - the current lhs priority									
	///	@return number of parsed characters												
	pcptr Expression::Parse(const Code& input, Any& lhs, int priority, bool optimize) {
		Any rhs;
		pcptr progress = 0;
		if (!lhs.IsValid())
			PC_PARSE_VERBOSE("Parsing unknown..." << ccTab);
		else
			PC_PARSE_VERBOSE("Parsing unknown with LHS(" << lhs << ") " << ccTab);

		while (progress < input.GetCount()) {
			// Scan input until end														
			Code relevant = input.CropLeft(progress);
			pcptr localProgress = 0;

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
				PC_PARSE_VERBOSE(ccGreen << "Unknown resulted in " << rhs << ccUntab);
				lhs = pcMove(rhs);
				return progress;
			}

			if (0 == localProgress)
				break;
			progress += localProgress;
		}

		// Input was parsed, relay content to output								
		PC_PARSE_VERBOSE(ccGreen << "Unknown resulted in " << rhs << ccUntab);
		lhs = pcMove(rhs);
		return progress;
	}

	/// Parse a skippable, no content produced											
	///	@param input - code that starts with a skippable character				
	///	@return number of parsed characters												
	pcptr Skipped::Parse(const Code& input) {
		pcptr progress = 0;
		while (progress < input.GetCount()) {
			const auto relevant = input.CropLeft(progress);
			if (Peek(relevant))
				++progress;
			else break;
		}

		return progress;
	}

	/// Parse an abstract keyword																
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	pcptr Keyword::Parse(const Code& input, Any& lhs) {
		pcptr progress = 0;
		PC_PARSE_VERBOSE("Parsing keyword...");
		while (progress < input.GetCount()) {
			// Collect all characters of the keyword								
			const auto relevant = input.CropLeft(progress);
			if (!Keyword::Peek(relevant) && !Number::Peek(relevant))
				break;

			++progress;
		}

		if (0 == progress)
			PRETTY_ERROR("No progress at keyword parse");

		const auto keyword = input.CropRight(progress);
		if (keyword.IsEmpty())
			PRETTY_ERROR("No keyword parsed");
		PC_PARSE_VERBOSE("Parsed keyword: " << keyword);

		// Search for token in meta definitions									
		const AMeta* meta = nullptr;
		try { meta = PCMEMORY.GetMeta(keyword); }
		catch (const Except::Meta&) {
			PRETTY_ERROR("Missing meta");
		}

		// Check the type of the meta definition									
		Any rhs;
		switch (meta->GetMetaClass().GetHash().GetValue()) {
		case DataID::Switch<MetaData>():
			// The keyword resulted in a DataID										
			rhs << static_cast<DMeta>(meta)->GetID();
			break;
		case DataID::Switch<MetaVerb>(): {
			// Check if the keyword is for a reverse verb						
			auto metav = static_cast<VMeta>(meta);
			const bool reversed =
				!metav->GetToken().CompareLoose(keyword)
				&& metav->GetTokenReverse().CompareLoose(keyword);

			if (reversed)
				// The keyword resulted in a ChargedVerbID						
				rhs << ChargedVerbID(metav, real(-1));
			else
				// The keyword resulted in a VerbID									
				rhs << metav->GetID();
			break;
		}
		case DataID::Switch<MetaTrait>():
			// The keyword resulted in a TraitID									
			rhs << static_cast<TMeta>(meta)->GetID();
			break;
		case DataID::Switch<MetaConst>():
			// The keyword resulted in a constant value							
			static_cast<CMeta>(meta)->GetBlock().Clone(rhs);
			break;
		}

		PC_PARSE_VERBOSE(pcLogVerbose 
			<< "Keyword resulted in " << ccPush << ccCyan << keyword 
			<< ccPop << " -> " << ccCyan << rhs << " (" << rhs.GetToken() << ")");
		lhs = pcMove(rhs);
		return progress;
	}

	/// Parse an integer or real number														
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	pcptr Number::Parse(const Code& input, Any& lhs) {
		real rhs = 0;
		pcptr progress = 0;
		PC_PARSE_VERBOSE("Parsing number...");

		if (auto [p, ec] = std::from_chars(input.GetRaw(), input.GetRaw() + input.GetCount(), rhs); 
			ec == std::errc()) {
			progress = p - input.GetRaw();
		}

		PC_PARSE_VERBOSE(ccGreen << "Number resulted in " << rhs);
		lhs << rhs;
		return progress;
	}

	/// Parse an operator, operate on current output (lhs) and content (rhs)	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@param priority - the priority of the last parsed element				
	///	@param optimize - the priority of the last parsed element				
	///	@return number of parsed characters												
	pcptr Operator::Parse(const Code& input, Any& lhs, int priority, bool optimize) {
		// Determine operator															
		pcptr progress = 0;
		auto op = Code::OpCounter;
		for (pcptr i = 0; i < Code::OpCounter; ++i) {
			const bool typeMatch = 
				!Code::Token[i].mCharge || 
				lhs.Is<DataID>() || 
				lhs.Is<VerbID>();
			if (input.IsOperator(i) && typeMatch) {
				op = Code::Operator(i);
				progress += Code::Token[i].mToken.size();
				break;
			}
		}

		if (op == Code::OpCounter)
			PRETTY_ERROR("Unknown operator");

		PC_PARSE_VERBOSE("Parsing operator...");
		const Code relevant = input.CropLeft(progress);

		if (Code::Token[op].mPriority && priority >= Code::Token[op].mPriority) {
			PC_PARSE_VERBOSE(ccYellow << "Delaying because of a prioritized operator...");
			return 0;
		}

		switch (op) {
		case Code::As:
			return progress + OperatorAs::Parse(relevant, lhs, optimize);
		case Code::OpenScope:
			return progress + OperatorContent::Parse(relevant, lhs, optimize);
		case Code::CloseScope:
			return 0;
		case Code::OpenString:
		case Code::OpenStringAlt:
		case Code::OpenCode:
		case Code::OpenCharacter:
			return progress + OperatorString::Parse(op, relevant, lhs);
		case Code::OpenByte:
			TODO();
		case Code::PolarizeLeft:
		case Code::PolarizeRight:
			return progress + OperatorPolarize::Parse(op, relevant, lhs, optimize);
		case Code::Context:
			return progress + OperatorContext::Parse(relevant, lhs, optimize);
		case Code::Copy:
			return progress + OperatorCopy::Parse(relevant, lhs, optimize);
		case Code::Missing:
			return progress + OperatorMissing::Parse(relevant, lhs);
		case Code::AndSeparator:
		case Code::OrSeparator:
			return progress + OperatorSeparator::Parse(op, relevant, lhs, optimize);
		case Code::Select:
			return progress + OperatorSelect::Parse(relevant, lhs, optimize);
		case Code::Mass:
		case Code::Frequency:
		case Code::Time:
		case Code::Priority:
			if (OperatorCharge::IsChargable(lhs))
				return progress + OperatorCharge::Parse(op, relevant, lhs);
			else PRETTY_ERROR("Mass operator on non-chargable LHS: " << lhs);
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

	struct VerbHelper {
		Any source;
		Any argument;
		Any output;

		/*operator Debug () {
			Code result;
			result += Verb{ uvInvalid, source, argument, output };
			return result;
		}*/
	};

	/// Parse a content scope																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	pcptr OperatorContent::Parse(const Code& input, Any& lhs, bool optimize) {
		pcptr progress = 0;

		// Can define contents for one element at a time						
		if (lhs.GetCount() > 1)
			PRETTY_ERROR("Content scope for multiple elements is not allowed: " << lhs);

		// We don't know what to expect, so we attempt blind parse			
		Any rhs;
		progress = Expression::Parse(input, rhs, Code::Token[Code::OpenScope].mPriority, optimize);
		if (!input.CropLeft(progress).IsOperator(Code::CloseScope))
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
			auto stateBackup = lhs.GetUnconstrainedState();
			lhs = pcMove(rhs);
			lhs.ToggleState(stateBackup, true);
			PC_PARSE_VERBOSE_ALT("Untyped content: " << ccCyan << lhs);
		}
		else if (lhs.Is<DataID>()) {
			// The content is for a data scope										
			if (lhs.GetCount() > 1) {
				pcLogFuncError << "Can't submit content to multiple DataIDs: " << lhs;
				pcLogFuncError << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				throw Except::Code();
			}

			const auto meta = lhs.Get<DataID>().GetMeta();
			lhs.Reset();

			Construct outputConstruct(meta, pcMove(rhs));
			try
			{
				outputConstruct.StaticCreation(lhs);
			}
			catch (...) {
				// Failed to construct, so just propagate request				
				PC_PARSE_VERBOSE_ALT(ccRed << "Can't statically construct " << outputConstruct);
				lhs = pcMove(outputConstruct);
			}
		}
		else if (lhs.Is<VerbID>()) {
			// Verbs are always constructed in place								
			if (lhs.GetCount() > 1) {
				pcLogFuncError << "Can't submit content to multiple VerbIDs: " << lhs;
				pcLogFuncError << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				throw Except::Code();
			}

			const auto meta = lhs.Get<VerbID>().GetMeta();
			if (rhs.Is<VerbHelper>()) {
				const auto& helper = rhs.Get<VerbHelper>();
				lhs = Verb {
					ChargedVerbID{ meta },
					pcMove(helper.source),
					pcMove(helper.argument),
					pcMove(helper.output)
				};
			}
			else {
				lhs = Verb{ ChargedVerbID{ meta }, {}, pcMove(rhs) };
			}
			PC_PARSE_VERBOSE_ALT("Constructed from VerbID: " << ccCyan << lhs);
		}
		else if (lhs.Is<ChargedVerbID>()) {
			// Modified verb id (expanded when energy was provided)			
			if (lhs.GetCount() > 1) {
				pcLogFuncError << "Can't submit content to multiple ChargedVerbIDs: " << lhs;
				pcLogFuncError << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				throw Except::Code();
			}

			const auto& chargedVerb = lhs.Get<ChargedVerbID>();
			if (rhs.Is<VerbHelper>()) {
				const auto& helper = rhs.Get<VerbHelper>();
				lhs = Verb {
					chargedVerb,
					pcMove(helper.source),
					pcMove(helper.argument),
					pcMove(helper.output)
				};
			}
			else {
				lhs = Verb{ chargedVerb, {}, pcMove(rhs) };
			}
			PC_PARSE_VERBOSE_ALT("Constructed from ChargedVerbID " << ccCyan << lhs);
		}
		else if (lhs.Is<Verb>()) {
			// Modified verb id (expanded when using . primarily)				
			if (lhs.GetCount() > 1) {
				pcLogFuncError << "Can't submit content to multiple Verbs: " << lhs;
				pcLogFuncError << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				throw Except::Code();
			}

			auto& verb = lhs.Get<Verb>();
			if (rhs.Is<VerbHelper>()) {
				const auto& helper = rhs.Get<VerbHelper>();
				verb.GetSource().SmartPush(helper.source);
				verb.GetArgument().SmartPush(helper.argument);
				verb.GetOutput().SmartPush(helper.output);
			}
			else {
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
			}

			PC_PARSE_VERBOSE_ALT("Constructed from Verb " << ccCyan << lhs);
		}
		else if (lhs.Is<TraitID>()) {
			// Trait scope																	
			if (lhs.GetCount() > 1) {
				pcLogFuncError << "Can't submit content to multiple TraitIDs: " << lhs;
				pcLogFuncError << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
				throw Except::Code();
			}

			auto meta = lhs.Get<TraitID>().GetMeta();
			lhs = Trait{ meta, pcMove(rhs) };
			PC_PARSE_VERBOSE_ALT("Constructed trait " << ccCyan << lhs);
		}
		else {
			pcLogFuncError << "Bad scope for " << lhs << " (" << lhs.GetToken() << ")";
			pcLogFuncError << "Content is: " << rhs << " (" << rhs.GetToken() << ")";
			throw Except::Code();
		}
	}

	/// String/character/code scope															
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here (lhs)						
	///	@return number of parsed characters												
	pcptr OperatorString::Parse(const Code::Operator op, const Code& input, Any& lhs) {
		pcptr progress = 0;
		pcptr depth = 1;
		while (progress < input.GetCount()) {
			// Collect all characters in scope, essentially gobbling them	
			// up into a text container until matching token is reached		
			const auto relevant = input.CropLeft(progress);

			switch (op) {
			case Code::OpenString:
				// Finish up a "string"													
				if (relevant.IsOperator(Code::CloseString)) {
					const auto tokenSize = Code::Token[Code::CloseString].mToken.size();
					lhs = Text{ input.CropRight(progress) };
					PC_PARSE_VERBOSE("Constructed string " << ccCyan << lhs);
					return tokenSize + progress;
				}
				break;

			case Code::OpenStringAlt:
				// Finish up a `string`													
				if (relevant.IsOperator(Code::CloseStringAlt)) {
					const auto tokenSize = Code::Token[Code::CloseStringAlt].mToken.size();
					lhs = Text{ input.CropRight(progress) };
					PC_PARSE_VERBOSE("Constructed string " << ccCyan << lhs);
					return tokenSize + progress;
				}
				break;

			case Code::OpenCharacter:
				// Finish up a 'c'haracter												
				if (relevant.IsOperator(Code::CloseCharacter)) {
					const auto tokenSize = Code::Token[Code::CloseCharacter].mToken.size();
					lhs = input[0];
					PC_PARSE_VERBOSE("Constructed character " << ccCyan << lhs);
					return tokenSize + progress;
				}
				break;

			case Code::OpenCode:
				// Finish up a [code]													
				if (relevant.IsOperator(Code::OpenCode))
					++depth;
				else if (relevant.IsOperator(Code::CloseCode)) {
					--depth;
					if (0 == depth) {
						const auto tokenSize = Code::Token[Code::CloseCode].mToken.size();
						lhs = input.CropRight(progress);
						PC_PARSE_VERBOSE("Constructed code " << ccCyan << lhs);
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
	pcptr OperatorPolarize::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		Any rhs;
		auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		
		if (lhs.IsValid() && rhs.IsValid()) {
			// If both LHS and RHS are available, the polarizer acts as		
			// AND separator and concatenates LHS << RHS, polarizing both	
			// Example: do(left > right), do(right < left)						
			Any deeper;
			if (op == Code::PolarizeRight)
				deeper << pcMove(lhs.MakeLeft()) << pcMove(rhs.MakeRight());
			else
				deeper << pcMove(lhs.MakeRight()) << pcMove(rhs.MakeLeft());
			lhs = pcMove(deeper);
		}
		else if (lhs.IsValid()) {
			// Only LHS is available, so polarize it								
			// Example: do(left>), do(right<)										
			if (op == Code::PolarizeRight)
				lhs.MakeRight();
			else
				lhs.MakeLeft();
		}
		else if (rhs.IsValid()) {
			// Only RHS is available, so polarize it								
			// Example: do(>right), do(<left)										
			if (op == Code::PolarizeRight)
				lhs = pcMove(rhs.MakeRight());
			else
				lhs = pcMove(rhs.MakeLeft());
		}

		PC_PARSE_VERBOSE("Polarize (" 
			<< Code::Token[op].mToken << ") " << rhs << " -> " << ccCyan << lhs);
		return progress;
	}

	/// Context operator	':'																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorContext::Parse(const Code& input, Any& lhs, bool optimize) {
		pcptr progress = 0;
		if (!lhs.IsValid())
			PRETTY_ERROR("Invalid context");
		PC_PARSE_VERBOSE("Context -> " << ccCyan << lhs);

		// Parse right side now, to avoid interfering with context			
		Any rhs;
		progress += Expression::Parse(input, rhs, Code::Token[Code::Context].mPriority, optimize);

		// Reflect and push a helper													
		TAny<VerbHelper> helper;
		helper << VerbHelper { pcMove(lhs), pcMove(rhs), {} };
		lhs = pcMove(static_cast<Any&>(helper));
		return progress;
	}

	/// As operator																				
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorAs::Parse(const Code& input, Any& lhs, bool optimize) {
		// Parse RHS, and if valid, push Output(RHS) trait to output		
		Any rhs;
		const pcptr progress = Expression::Parse(input, rhs, Code::Token[Code::As].mPriority, optimize);
		if (!rhs.IsValid())
			PRETTY_ERROR("Invalid RHS for AS operator");
		PC_PARSE_VERBOSE("Output -> " << ccCyan << rhs);

		if (lhs.Is<VerbHelper>()) {
			// Add to helper, if available											
			lhs.Get<VerbHelper>().output = pcMove(rhs);
		}
		else {
			// Reflect and push a helper												
			TAny<VerbHelper> helper;
			helper << VerbHelper { {}, pcMove(lhs), pcMove(rhs) };
			lhs = pcMove(static_cast<Any&>(helper));
		}

		return progress;
	}

	/// Missing operator '?'																	
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorMissing::Parse(const Code&, Any& lhs) {
		lhs.MakeMissing();
		PC_PARSE_VERBOSE_ALT("Missing -> " << ccCyan << lhs);
		return 0;
	}

	/// Separator operators (and, or)														
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorSeparator::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Output is LHS, new content is RHS, just do a smart push to LHS	
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		lhs.SmartPush(rhs, (op == Code::OrSeparator ? DState::Or : DState::Default));
		PC_PARSE_VERBOSE("Pushing (" 
			<< Code::Token[op].mToken << ") " << rhs << " -> " << ccCyan << lhs);
		return progress;
	}

	/// Select operator (.)																		
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorSelect::Parse(const Code& input, Any& lhs, bool optimize) {
		pcptr progress = 0;
		if (input.IsOperator(Code::OpenScope)) {
			// Selection arguments provided as content on RHS					
			const auto scopeOffset = Code::Token[Code::OpenScope].mToken.size();
			lhs = Verb::From<Verbs::Select>().SetSource(pcMove(lhs));
			progress = OperatorContent::Parse(input.CropLeft(scopeOffset), lhs, optimize);
			PC_PARSE_VERBOSE("Select operator: " << lhs);
			return scopeOffset + progress;
		}

		// Search for anything on the right side									
		// LHS is the context, RHS is what we select in it						
		// If no LHS exists, we select in current environment					
		// We wrap both RHS, and LHS into a Verbs::Select						
		Any rhs;
		progress = Expression::Parse(input, rhs, Code::Token[Code::Select].mPriority, optimize);
		if (rhs.IsEmpty())
			PRETTY_ERROR("Empty RHS for selection operator");
		else if (rhs.GetCount() > 1)
			PRETTY_ERROR("RHS(" << rhs << ") is too big");

		if (rhs.Is<VerbID>())
			lhs = Verb(rhs.Get<VerbID>()).SetSource(pcMove(lhs));
		else if (rhs.Is<ChargedVerbID>())
			lhs = Verb(rhs.Get<ChargedVerbID>()).SetSource(pcMove(lhs));
		else if (rhs.Is<Verb>())
			lhs = pcMove(rhs.Get<Verb>().SetSource(pcMove(lhs)));
		else
			lhs = Verb::From<Verbs::Select>().SetSource(pcMove(lhs)).SetArgument(pcMove(rhs));

		PC_PARSE_VERBOSE("Select operator: " << lhs);
		return progress;
	}

	/// Copy operator (=)																		
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorCopy::Parse(const Code& input, Any& lhs, bool optimize) {
		pcptr progress = 0;
		if (input.IsOperator(Code::OpenScope)) {
			// Copy specification provided as content on RHS					
			const auto scopeOffset = Code::Token[Code::OpenScope].mToken.size();
			lhs = Verb::From<Verbs::Associate>().SetSource(pcMove(lhs)).SetPriority(2);
			progress = OperatorContent::Parse(input.CropLeft(scopeOffset), lhs, optimize);
			PC_PARSE_VERBOSE("Copy operator: " << lhs);
			return scopeOffset + progress;
		}

		// LHS is the context, RHS is what we associate with					
		// If no LHS exists, we simply copy in current environment			
		// We wrap both RHS, and LHS into a Verbs::Associate					
		Any rhs;
		progress = Expression::Parse(input, rhs, Code::Token[Code::Copy].mPriority, optimize);
		if (rhs.IsInvalid())
			PRETTY_ERROR("Invalid RHS for copy operator");

		// Invoke the verb in the context (slower)								
		auto copier = Verb::From<Verbs::Associate>({}, rhs);
		if (!Verb::DispatchDeep(lhs, copier)) {
			// If execution failed, just push the verb							
			lhs = pcMove(copier.SetSource(pcMove(lhs)).SetPriority(2));
			PC_PARSE_VERBOSE("Copy operator: " << lhs);
			return progress;
		}

		PC_PARSE_VERBOSE("Copied: " << copier);
		return progress;
	}

	/// Add/subtract operator (+, -)															
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorAdd::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Perform addition or substraction on LHS and RHS						
		// Naturally, LHS is output; RHS is the newly parsed content		
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		if (optimize) {
			if (op == Code::Subtract && lhs.IsEmpty() && rhs.Is<real>() && !rhs.IsEmpty()) {
				// Quick optimization for inverting number in RHS				
				PC_PARSE_VERBOSE("Built-in inversion (-): " << rhs << " "
					<< Code::Token[op].mToken << " = " << -rhs.Get<real>());
				lhs = -rhs.Get<real>();
				return progress;
			}
			else if (lhs.Is<real>() && rhs.Is<real>() && !lhs.IsEmpty() && !rhs.IsEmpty()) {
				// LHS + RHS and LHS - RHS if both reals (optimization)		
				if (op == Code::Subtract) {
					PC_PARSE_VERBOSE("Built-in subtract: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<real>() - rhs.Get<real>()));
					lhs = lhs.Get<real>() - rhs.Get<real>();
				}
				else {
					PC_PARSE_VERBOSE("Built-in add: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<real>() + rhs.Get<real>()));
					lhs = lhs.Get<real>() + rhs.Get<real>();
				}
				return progress;
			}
		}

		if (lhs.IsEmpty() && op == Code::Subtract) {
			// No LHS, so we execute in RHS to invert								
			auto inverter = Verb::From<Verbs::Add>().Invert();
			if (!optimize || !Verb::DispatchDeep(rhs, inverter)) {
				// If execution failed, just push the verb						
				lhs = pcMove(inverter.SetSource(pcMove(rhs)));
				PC_PARSE_VERBOSE("Invert (-) operator: " << lhs);
				return progress;
			}

			PC_PARSE_VERBOSE("Inverted (-): " << rhs << " " << Code::Token[op].mToken
				<< " = " << inverter.GetOutput());
			lhs = pcMove(inverter.GetOutput());
			return progress;
		}

		// Invoke the verb in the context (slower)								
		auto adder = Verb::From<Verbs::Add>({}, rhs);
		if (op == Code::Subtract)
			adder.Invert();

		if (!optimize || !Verb::DispatchDeep(lhs, adder)) {
			// If execution failed, just push the verb							
			lhs = pcMove(adder.SetSource(pcMove(lhs)));
			PC_PARSE_VERBOSE("Add/subtract operator: " << lhs);
			return progress;
		}
		
		PC_PARSE_VERBOSE("Added/subtracted: " << lhs << " " << Code::Token[op].mToken
			<< " " << rhs << " = " << adder.GetOutput());
		lhs = pcMove(adder.GetOutput());
		return progress;
	}

	/// Multiply/divide operator (*, /)														
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorMultiply::Parse(const Code::Operator op, const Code& input, Any& lhs, bool optimize) {
		// Perform multiplication or division on LHS and RHS					
		// Naturally, LHS is output; RHS is the newly parsed content		
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[op].mPriority, optimize);
		if (optimize) {
			if (op == Code::Divide && lhs.IsEmpty() && rhs.Is<real>() && !rhs.IsEmpty()) {
				// Quick optimization for inverting number in RHS				
				PC_PARSE_VERBOSE("Built-in inversion (/): " << rhs << " "
					<< Code::Token[op].mToken << " = " << (real(1) / rhs.Get<real>()));
				lhs = real(1) / rhs.Get<real>();
				return progress;
			}
			else if (lhs.Is<real>() && rhs.Is<real>() && !lhs.IsEmpty() && !rhs.IsEmpty()) {
				// LHS * RHS and LHS / RHS if both reals (optimization)		
				if (op == Code::Multiply) {
					PC_PARSE_VERBOSE("Built-in multiply: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<real>() * rhs.Get<real>()));
					lhs = lhs.Get<real>() * rhs.Get<real>();
				}
				else {
					PC_PARSE_VERBOSE("Built-in divide: " << lhs << " " << Code::Token[op].mToken
						<< " " << rhs << " = " << (lhs.Get<real>() / rhs.Get<real>()));
					lhs = lhs.Get<real>() / rhs.Get<real>();
				}
				return progress;
			}
		}

		if (lhs.IsEmpty() && op == Code::Divide) {
			// No LHS, so we execute in RHS to invert (1/x)						
			auto inverter = Verb::From<Verbs::Multiply>().Invert();
			if (!optimize || !Verb::DispatchDeep(rhs, inverter)) {
				// If execution failed, just push the verb						
				lhs = pcMove(inverter.SetSource(pcMove(rhs)));
				PC_PARSE_VERBOSE("Invert (/) operator: " << lhs);
				return progress;
			}

			PC_PARSE_VERBOSE("Inverted (/): " << rhs << " " << Code::Token[op].mToken
				<< " = " << inverter.GetOutput());
			lhs = pcMove(inverter.GetOutput());
			return progress;
		}

		// Invoke the verb in the context (slower)								
		auto multiplier = Verb::From<Verbs::Multiply>({}, rhs);
		if (op == Code::Divide)
			multiplier.Invert();

		if (!optimize || !Verb::DispatchDeep(lhs, multiplier)) {
			// If execution failed, just push the verb							
			lhs = pcMove(multiplier.SetSource(pcMove(lhs)));
			PC_PARSE_VERBOSE("Multiply/divide operator: " << lhs);
			return progress;
		}
		
		PC_PARSE_VERBOSE("Multiplied/divided: " << lhs << " " << Code::Token[op].mToken
			<< " " << rhs << " = " << multiplier.GetOutput());
		lhs = pcMove(multiplier.GetOutput());
		return progress;
	}

	/// Power operator (^, ^(1/...))															
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorPower::Parse(const Code& input, Any& lhs, bool optimize) {
		// Perform power on LHS and RHS												
		// Naturally, LHS is output; RHS is the newly parsed content		
		Any rhs;
		const auto progress = Expression::Parse(input, rhs, Code::Token[Code::Power].mPriority, optimize);
		if (optimize) {
			if (lhs.Is<real>() && rhs.Is<real>() && !lhs.IsEmpty() && !rhs.IsEmpty()) {
				// LHS ^ RHS when both reals (optimization)						
				real result = std::pow(lhs.Get<real>(), rhs.Get<real>());
				PC_PARSE_VERBOSE("Built-in exponentiation: " << lhs << " " << Code::Token[Code::Power].mToken
					<< " " << rhs << " = " << result);
				lhs = result;
				return progress;
			}
		}

		// Try invoking the verb in the context (slower)						
		auto exponentiator = Verb::From<Verbs::Exponent>({}, rhs);
		if (!optimize || !Verb::DispatchDeep(lhs, exponentiator)) {
			// If execution failed, just push the verb							
			lhs = pcMove(exponentiator.SetSource(pcMove(lhs)));
			PC_PARSE_VERBOSE("Power operator: " << lhs);
			return progress;
		}
		
		PC_PARSE_VERBOSE("Exponentiated: " << lhs << " " << Code::Token[Code::Power].mToken
			<< " " << rhs << " = " << exponentiator.GetOutput());
		lhs = pcMove(exponentiator.GetOutput());
		return progress;
	}

	/// Mass/time/frequency/priority operators (+, -, *, /, @, ^, !)				
	/// Can be applied to things with charge, like verbs and constructs			
	///	@param op - the starting operator												
	///	@param input - the code to parse													
	///	@param lhs - [in/out] parsed content goes here								
	///	@return number of parsed characters												
	pcptr OperatorCharge::Parse(const Code::Operator op, const Code& input, Any& lhs) {
		// LHS must be chargable														
		pcptr progress = 0;
		if (lhs.IsEmpty() || !IsChargable(lhs))
			PRETTY_ERROR("Invalid LHS(" << lhs << ") for charge operator");

		// RHS must evaluate to a number												
		// It can be either a number literal, or a number constant			
		Any rhs;
		if (Number::Peek(input))
			progress += Number::Parse(input, rhs);
		else if (Keyword::Peek(input))
			progress += Keyword::Parse(input, rhs);

		if (!rhs.InterpretsAs<ANumber>(1) || rhs.IsEmpty()) {
			PRETTY_ERROR("Invalid RHS(" << rhs << ") for charge operator '"
				<< Code::Token[op].mToken << "' on LHS(" << lhs << ")");
		}

		const auto asReal = rhs.AsCast<real>();
		if (lhs.Is<VerbID>() || lhs.Is<ChargedVerbID>()) {
			// After operating on a verb ID, we always end up with a			
			// modified verb id, that is later used to construct a verb		
			ChargedVerbID verb;
			if (lhs.Is<VerbID>())
				verb = { lhs.Get<VerbID>().GetMeta() };
			else
				verb = lhs.Get<ChargedVerbID>();

			switch (op) {
			case Code::Mass:
				verb.mCharge.mMass = asReal;
				break;
			case Code::Time:
				verb.mCharge.mTime = asReal;
				break;
			case Code::Frequency:
				verb.mCharge.mFrequency = asReal;
				break;
			case Code::Priority:
				verb.mCharge.mPriority = asReal;
				break;
			default:
				PRETTY_ERROR("Invalid verb charge operator: " << Code::Token[op].mToken);
			}

			PC_PARSE_VERBOSE(lhs << " " << Code::Token[op].mToken
				<< " " << rhs << " = " << ccCyan << verb);
			lhs = verb;
		}
		else if (lhs.Is<DataID>()/* || lhs.Is<ChargedDataID>()*/) {
			Construct construct;
			if (lhs.Is<DataID>())
				construct = Construct{ lhs.Get<DataID>() };
			else TODO();
				//construct = lhs.Get<ChargedDataID>();

			switch (op) {
			case Code::Mass:
				construct.mCharge.mMass = asReal;
				break;
			case Code::Time:
				construct.mCharge.mTime = asReal;
				break;
			case Code::Frequency:
				construct.mCharge.mFrequency = asReal;
				break;
			case Code::Priority:
				construct.mCharge.mPriority = asReal;
				break;
			default:
				PRETTY_ERROR("Invalid data charge operator: " << Code::Token[op].mToken);
			}

			PC_PARSE_VERBOSE(lhs << " " << Code::Token[op].mToken
				<< " " << rhs << " = " << ccCyan << construct);
			lhs = construct;
		}
		else PRETTY_ERROR("Uncharged internal " << lhs << "for operator: " << Code::Token[op].mToken);
		return progress;
	}

	/// Parse Code code																			
	///	@param optimize - whether or not to precalculate constexpr				
	///	@returned the parsed data															
	Any Code::Parse(bool optimize) const {
		Any output;
		const auto parsed = Expression::Parse(*this, output, 0, optimize);
		if (parsed != GetCount()) {
			pcLogFuncWarning 
				<< "Some characters were left out at the end, while parsing Code code:";
			pcLogWarning << " -- " << ccGreen 
				<< CropRight(parsed) << ccRed << CropLeft(parsed);
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
			if (text.CompareLoose(a.mToken))
				return true;
		}
		return false;
	}

	/// A Code keyword must be made of only letters and numbers						
	///	@param text - the text to check													
	///	@return true if text is a valid Code keyword									
	bool Code::IsValidKeyword(const Text& text) {
		if (text.IsEmpty() || !pcIsLetter(text[0]))
			return false;

		for (auto a : text) {
			if (pcIsNumber(a) || pcIsLetter(a))
				continue;
			return false;
		}

		return true;
	}

} // namespace Langulus::Flow

