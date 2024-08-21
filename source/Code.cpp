///                                                                           
/// Langulus::Flow                                                            
/// Copyright (c) 2017 Dimo Markov <team@langulus.com>                        
/// Part of the Langulus framework, see https://langulus.com                  
///                                                                           
/// SPDX-License-Identifier: GPL-3.0-or-later                                 
///                                                                           
#include "Code.inl"
#include "Verb.hpp"
#include "Temporal.hpp"
#include "Time.hpp"

#include "verbs/Do.inl"
#include "verbs/Select.inl"
#include "verbs/Associate.inl"
#include "verbs/Create.inl"
#include "verbs/Catenate.inl"
#include "verbs/Conjunct.inl"
#include "verbs/Interpret.inl"

LANGULUS_RTTI_BOUNDARY(RTTI::MainBoundary)

#define ENABLE_VERBOSE() 0

#define VERBOSE_INNER(...) \
      Logger::Flow("Flow::Code: ", Logger::PushCyan, __VA_ARGS__ \
         , Logger::Pop, " at ", progress, ": " \
         , Logger::NewLine, "+-[", Logger::PushGreen, Logger::Underline \
         , input. LeftOf(progress).Replace('\n', "\\n"), Logger::PopAndPushWhite \
         , input.RightOf(progress).Replace('\n', "\\n"), Logger::Pop, ']')

#define PRETTY_ERROR(...) { \
      Logger::Error("Flow::Code: ", Logger::PushDarkYellow, __VA_ARGS__ \
         , Logger::Pop, " at ", progress, ": " \
         , Logger::NewLine, "+-[", Logger::PushDarkYellow, Logger::Underline \
         , input. LeftOf(progress).Replace('\n', "\\n"), Logger::Pop \
         , input.RightOf(progress).Replace('\n', "\\n"), ']'); \
      LANGULUS_THROW(Flow, "Parse error"); \
   }

#if ENABLE_VERBOSE()
   #define VERBOSE(...)       VERBOSE_INNER(__VA_ARGS__)
   #define VERBOSE_TAB(...)   auto tab = VERBOSE_INNER(__VA_ARGS__) << Logger::Tabs{}
   #define VERBOSE_ALT(...)   Logger::Flow(__VA_ARGS__)
#else
   #define VERBOSE(...)       LANGULUS(NOOP)
   #define VERBOSE_TAB(...)   LANGULUS(NOOP)
   #define VERBOSE_ALT(...)   LANGULUS(NOOP)
#endif


namespace Langulus::Flow
{

   /// Parse code                                                             
   ///   @param optimize - whether or not to precompile                       
   ///   @returned the parsed flow                                            
   Many Code::Parse(bool optimize) const {
      // Make sure that all default traits are registered before parsing
      (void)MetaOf<Traits::Logger>();
      (void)MetaOf<Traits::Count>();
      (void)MetaOf<Traits::Name>();
      (void)MetaOf<Traits::Path>();
      (void)MetaOf<Traits::Data>();
      (void)MetaOf<Traits::Index>();
      (void)MetaOf<Traits::Context>();
      (void)MetaOf<Traits::Trait>();
      (void)MetaOf<Traits::State>();
      (void)MetaOf<Traits::Child>();
      (void)MetaOf<Traits::Parent>();
      (void)MetaOf<Traits::Clipboard>();
      (void)MetaOf<Traits::Color>();
      (void)MetaOf<Traits::Min>();
      (void)MetaOf<Traits::Max>();
      (void)MetaOf<Traits::Input>();
      (void)MetaOf<Traits::Output>();

      // Make sure that all default types are registered before parsing 
      (void)MetaOf<Index>();
      (void)MetaOf<Temporal>();
      //(void)MetaOf<Time>(); //TODO causes conflict with the trait that isn't detected due to too lax constraints!
      (void)MetaOf<Code>();

      // Make sure that all default verbs are registered before parsing 
      (void)MetaOf<Verbs::Do>();
      (void)MetaOf<Verbs::Select>();
      (void)MetaOf<Verbs::Associate>();
      (void)MetaOf<Verbs::Create>();
      (void)MetaOf<Verbs::Catenate>();
      (void)MetaOf<Verbs::Conjunct>();
      (void)MetaOf<Verbs::Interpret>();

      // Parse                                                          
      Many output;
      const auto parsed = UnknownParser::Parse(*this, output, 0, optimize);
      if (parsed != GetCount()) {
         Logger::Warning("Some characters were left out at the end, while parsing code:");
         Logger::Warning("+-- ", 
            Logger::Green, LeftOf(parsed), 
            Logger::Red,   RightOf(parsed)
         );
      }
      return output;
   }
   
   /// Check if the code container begins with an operator                    
   ///   @param i - the operator to check for                                 
   ///   @return true if the operator matches                                 
   bool Code::StartsWithOperator(Offset i) const noexcept {
      const Code token {static_cast<Operator>(i)};
      if (not token or GetCount() < token.GetCount())
         return false;

      const Code remainder = RightOf(token.GetCount());
      const bool endsWithALetter = token.EndsWithLetter();
      return token.GetCount() > 0
         and (GetCount() == token.GetCount()
            or (endsWithALetter and (
               not     remainder.StartsWithLetter()
               and not remainder.StartsWithDigit()))
            or not endsWithALetter)
         and MatchesLoose(token) == token.GetCount();
   }

   /// Compare two tokens, ignoring case                                      
   ///   @param lhs - the left token                                          
   ///   @param rhs - the right token                                         
   ///   @return true if both loosely match                                   
   constexpr bool CompareTokens(const Token& lhs, const Token& rhs) noexcept {
      return (lhs.size() == rhs.size() and (
         lhs.size() == 0 or ::std::equal(lhs.begin(), lhs.end(), rhs.begin(),
            [](const char& c1, const char& c2) noexcept {
               return c1 == c2 or (::std::toupper(c1) == ::std::toupper(c2));
            })
         ));
   }

   /// Isolate an operator token                                              
   ///   @param token - the operator                                          
   ///   @return the isolated operator token                                  
   constexpr Token IsolateOperator(const Token& token) noexcept {
      auto l = token.data();
      auto r = token.data() + token.size();
      while (l < r and *l <= 32)
         ++l;
      while (r > l and *(r - 1) <= 32)
         --r;
      return token.substr(l - token.data(), r - l);
   }

   /// Compare two operators, ignoring case and spacing                       
   ///   @param lhs - the left operator                                       
   ///   @param rhs - the right operator                                      
   ///   @return true if both loosely match                                   
   constexpr bool CompareOperators(const Token& lhs, const Token& rhs) noexcept {
      return CompareTokens(IsolateOperator(lhs), IsolateOperator(rhs));
   }

   /// Check if a string is reserved as a keyword/operator                    
   ///   @param text - the text to check                                      
   ///   @return true if text is reserved                                     
   bool Code::IsReserved(const Text& text) {
      for (auto& a : SerializationRules::Operators) {
         if (CompareOperators(text, a.mToken))
            return true;
      }

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         if (not RTTI::GetAmbiguousMeta(text).empty())
            return true;
      #endif

      return false;
   }

   /// A keyword must be made of only letters and numbers, namespace operator 
   /// and/or underscores                                                     
   ///   @param text - the text to check                                      
   ///   @return true if text is a valid Code keyword                         
   bool IsKeywordSymbol(char a) {
      return IsDigit(a) or IsAlpha(a) or a == ':' or a == '_';
   }

   /// A keyword must be made of only letters and numbers, namespace operator 
   /// and/or underscores                                                     
   ///   @param text - the text to check                                      
   ///   @return true if text is a valid Code keyword                         
   bool Code::IsValidKeyword(const Text& text) {
      if (not text or not IsAlpha(text[0]))
         return false;

      for (auto a : text) {
         if (IsKeywordSymbol(a))
            continue;
         return false;
      }

      return true;
   }

   /// Parse any code expression, anticipate anything                         
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] parsed content goes here (lhs)                 
   ///   @param precedence - the last parsed operation precedence             
   ///   @param optimize - whether to attempt executing at compile-time       
   ///   @return number of parsed characters from input                       
   Offset Code::UnknownParser::Parse(const Code& input, Many& lhs, Real precedence, bool optimize) {
      Many rhs;
      Offset progress = 0;
      VERBOSE_TAB("Parsing unknown");
      #if ENABLE_VERBOSE()
         if (lhs.IsValid())
            VERBOSE_ALT("LHS: ", lhs);
      #endif

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
            localProgress = OperatorParser::Parse(op, relevant, rhs, precedence, optimize);
         else if (KeywordParser::Peek(relevant))
            localProgress = KeywordParser::Parse(relevant, rhs);
         else if (NumberParser::Peek(relevant))
            localProgress = NumberParser::Parse(relevant, rhs);
         else
            PRETTY_ERROR("Unexpected symbol");

         if (0 == localProgress) {
            // This occurs often, when a lower priority operator is     
            // waiting for higher priority stuff to be parsed first     
            break;
         }

         progress += localProgress;
      }

      // Input was parsed, relay content to output                      
      VERBOSE(Logger::Green, "Unknown parsed: ", rhs);
      lhs.SmartPush(IndexBack, Abandon(rhs));
      return progress;
   }

   /// Peek inside input, and return true if first symbol is skippable        
   ///   @param input - the code to peek into                                 
   ///   @return true if input is skippable                                   
   bool Code::SkippedParser::Peek(const Code& input) noexcept {
      return input.StartsWithSkippable();
   }

   /// Parse a skippable, no content produced                                 
   ///   @param input - code that starts with a skippable character           
   ///   @return number of parsed characters                                  
   Offset Code::SkippedParser::Parse(const Code& input) {
      Offset progress = 0;
      while (progress < input.GetCount()) {
         const auto relevant = input.RightOf(progress);
         const auto asview = Token {relevant};

         if (relevant[0] > 0 and relevant[0] <= 32) {
            // Skip a single skippable character                     
            ++progress;
            continue;
         }
         else if (asview.starts_with("//")) {
            // Skip an entire line comment                           
            while (progress < input.GetCount() and input[progress] != '\n')
               ++progress;
            continue;
         }
         else if (asview.starts_with("/*")) {
            // Skip a block comment (across multiple new lines)      
            while (progress + 1 < input.GetCount() and (input[progress] != '*' or input[progress + 1] != '/'))
               ++progress;

            if (progress + 1 < input.GetCount())
               // Skip the "*/" tag                                  
               progress += 2;
            else 
               // Skip to end of input, "*/" was never found         
               progress = input.GetCount();

            continue;
         }

         // If reached, then something valuable was encountered      
         break;
      }

      VERBOSE("Skipped ", progress, " characters");
      return progress;
   }

   /// Peek inside input, and return true if first symbol is a character      
   ///   @param input - the code to peek into                                 
   ///   @return true if input is a character                                 
   bool Code::KeywordParser::Peek(const Code& input) noexcept {
      return input.StartsWithLetter();
   }
   
   /// Gather all symbols of a keyword                                        
   ///   @param input - the code to peek into                                 
   ///   @return the isolated keyword token                                   
   Token Code::KeywordParser::Isolate(const Code& input) noexcept {
      Offset progress = 0;
      while (progress < input.GetCount()) {
         const auto c = input[progress];
         if (not IsKeywordSymbol(c))
            break;
         ++progress;
      }

      if (0 == progress)
         return {};

      return input.LeftOf(progress);
   }
   
   /// Parse keyword for a constant, data, or trait                           
   /// Verbs are considered operators, not keywords                           
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] parsed content goes here (lhs)                 
   ///   @param allowCharge - whether to parse charge (internal use)          
   ///   @return number of parsed characters                                  
   Offset Code::KeywordParser::Parse(const Code& input, Many& lhs, bool allowCharge) {
      // Isolate the keyword                                            
      Offset progress = 0;
      const auto keyword = Isolate(input);
      if (keyword.empty())
         PRETTY_ERROR("No keyword parsed");

      progress += keyword.size();
      VERBOSE_TAB("Keyword isolated: ", keyword);

   #if LANGULUS_FEATURE(MANAGED_REFLECTION)
      // Search for an exact token in meta definitions                  
      const auto dmeta = RTTI::GetMetaData(keyword);
      const auto tmeta = RTTI::GetMetaTrait(keyword);
      const auto cmeta = RTTI::GetMetaConstant(keyword);

      if (dmeta and not tmeta and not cmeta) {
         // Exact non-ambiguous data definition found                   
         if (allowCharge) {
            const auto relevant = input.RightOf(progress);
            if (ChargeParser::Peek(relevant) != Operator::NoOperator) {
               // Parse charge for the keyword                          
               Charge charge;
               progress += ChargeParser::Parse(relevant, charge);
               lhs << Construct {dmeta, Many {}, charge};
            }
            else lhs << dmeta;
         }
         else lhs << dmeta;
      }
      else if (not dmeta and tmeta and not cmeta) {
         // Exact non-ambiguous trait definition found                  
         lhs << tmeta;
      }
      else if (not dmeta and not tmeta and cmeta) {
         const Block<> constant {
            {}, cmeta->mValueType, 1, cmeta->mPtrToValue, nullptr
         };
         lhs.SmartPush(IndexBack, Clone(constant));
      }
      else {
         // If this is reached, then exactly one match in symbols       
         // Push found meta data, if any                                
         const auto meta = Disambiguate(progress, input, keyword);
         if (not meta) {
            PRETTY_ERROR("Disambiguation of `", keyword, "` failed");
         }

         const auto dmeta = meta.As<DMeta>();
         const auto tmeta = meta.As<TMeta>();
         const auto cmeta = meta.As<CMeta>();

         if (dmeta) {
            if (allowCharge) {
               const auto relevant = input.RightOf(progress);
               if (ChargeParser::Peek(relevant) != Operator::NoOperator) {
                  // Parse charge for the keyword                       
                  Charge charge;
                  progress += ChargeParser::Parse(relevant, charge);
                  lhs << Construct {dmeta, Many {}, charge};
               }
               else lhs << dmeta;
            }
            else lhs << dmeta;
         }

         if (tmeta)
            lhs << tmeta;

         if (cmeta) {
            const Block<> constant {{}, cmeta};
            lhs.SmartPush(IndexBack, Clone(constant));
         }
      }

      VERBOSE("Keyword parsed: `", keyword, "` as ", lhs, " (of type ", lhs.GetToken(), ")");
      return progress;
   #else    // LANGULUS_FEATURE(MANAGED_REFLECTION)
      (void)lhs;
      (void)allowCharge;
      PRETTY_ERROR("Can't parse keyword, managed reflection feature is disabled");
   #endif   // LANGULUS_FEATURE(MANAGED_REFLECTION)
   }

#if LANGULUS_FEATURE(MANAGED_REFLECTION)
   /// Disambiguate a keyword                                                 
   ///   @param progress - current position in input                          
   ///   @param input - the input code (used only for debugging)              
   ///   @param keyword - the keyword we'll be disambiguating                 
   ///   @return the disambiguated definition                                 
   AMeta Code::KeywordParser::Disambiguate(
      const Offset progress, const Code& input, const Token& keyword
   ) {
      try
      {
         return RTTI::DisambiguateMeta(keyword);
      }
      catch (...) {
         PRETTY_ERROR("Unknown keyword: ", keyword);
      }
   }
#endif

   /// Peek inside input, and return true if first symbol is a digit, or a    
   /// minus followed by a digit                                              
   ///   @param input - the code to peek into                                 
   ///   @return true if input begins with a number                           
   bool Code::NumberParser::Peek(const Code& input) noexcept {
      return input.StartsWithDigit();
   }

   /// Parse an integer or real number                                        
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] parsed content goes here (lhs)                 
   ///   @return number of parsed characters                                  
   Offset Code::NumberParser::Parse(const Code& input, Many& lhs) {
      Real rhs = 0;
      Offset progress = 0;
      VERBOSE_TAB("Parsing number");

      if (auto [p, ec] = ::std::from_chars(input.GetRaw(), input.GetRaw() + input.GetCount(), rhs); 
         ec == ::std::errc()) {
         progress = p - input.GetRaw();
      }

      VERBOSE(Logger::Green, "Number parsed: ", rhs);
      lhs << rhs;
      return progress;
   }

   /// Peek inside input, and return true if it begins with one of the        
   /// builtin operators                                                      
   ///   @param input - the code to peek into                                 
   ///   @return true if input begins with an operators                       
   Code::Operator Code::OperatorParser::PeekBuiltin(const Code& input) noexcept {
      for (Offset i = 0; i < Operator::OpCounter; ++i) {
         if (not SerializationRules::Operators[i].mCharge and input.StartsWithOperator(i))
            return Operator(i);
      }

      return Operator::NoOperator;
   }

   /// Peek inside input, and return true if it begins with one of the        
   /// builtin or reflected operators                                         
   ///   @param input - the code to peek into                                 
   ///   @return true if input begins with an operators                       
   Code::Operator Code::OperatorParser::Peek(const Code& input) noexcept {
      const auto builtin = PeekBuiltin(input);
      if (builtin != Operator::NoOperator)
         return builtin;

      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         const auto word = Isolate(input);
         auto found = RTTI::GetOperator(word);
         if (found)
            return Operator::ReflectedOperator;

         found = RTTI::GetMetaVerb(word);
         if (found)
            return Operator::ReflectedVerb;
      #endif

      return Operator::NoOperator;
   }

   /// Isolate an operator                                                    
   ///   @param input - the code to parse                                     
   ///   @return the isolated operator                                        
   Token Code::OperatorParser::Isolate(const Code& input) noexcept {
      // These can be either a word separated by operators/spaces, or   
      // operators separated by spaces/numbers/chatacters               
      if (input.StartsWithLetter())
         return KeywordParser::Isolate(input);

      // Isolate an operator separated by spaces/letters/digits, or     
      // built-in operators, such as '(', '"', etc.                     
      Offset progress = 0;
      while (progress < input.GetCount()) {
         const auto relevant = input.RightOf(progress);
         if (KeywordParser::Peek(relevant)
            or NumberParser::Peek(relevant)
            or SkippedParser::Peek(relevant)
            or PeekBuiltin(relevant) != Operator::NoOperator)
            break;
         ++progress;
      }

      if (0 == progress)
         return {};

      return input.LeftOf(progress);
   }

   /// Parse op-expression, operate on current output (lhs) and content (rhs) 
   /// Beware, that charge-expressions are not handled here                   
   ///   @param op - the built-in operator if any, or Reflected               
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] the operator expression will go here           
   ///   @param priority - the priority of the last parsed element            
   ///   @param optimize - the priority of the last parsed element            
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::Parse(
      Operator op, const Code& input, Many& lhs, Real priority, bool optimize
   ) {
      Offset progress = 0;
      if (op < Operator::NoOperator) {
         // Handle a built-in operator                                  
         /*if (GlobalOperators[op].mPrecedence and priority >= GlobalOperators[op].mPrecedence) {
            VERBOSE(Logger::Yellow, 
               "Delaying built-in operator [", GlobalOperators[op].mToken,
               "] due to a prioritized operation");
            return 0;
         }*/

         // Skip the operator, we already know it                       
         progress += SerializationRules::Operators[op].mToken.size();
         VERBOSE_TAB("Parsing built-in operator: [",
            SerializationRules::Operators[op].mToken, ']');
         const Code relevant = input.RightOf(progress);

         switch (op) {
            // Handle built-in operators first                          
         case Operator::OpenScope:
            return progress + ParseContent(relevant, lhs, optimize);
         case Operator::CloseScope:
            return 0;
         case Operator::OpenString:
         case Operator::OpenStringAlt:
         case Operator::OpenCode:
         case Operator::OpenCharacter:
            return progress + ParseString(op, relevant, lhs);
         case Operator::OpenByte:
            return progress + ParseBytes(relevant, lhs);
         case Operator::Past:
         case Operator::Future:
            return progress + ParsePhase(op, lhs);
         case Operator::Constant:
            return progress + ParseConst(lhs);
         default:
            PRETTY_ERROR("Unhandled built-in operator");
         }
      }
      else if (op == Operator::ReflectedOperator) {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         // Handle a reflected operator                                 
         const auto word = Isolate(input);
         const auto found = RTTI::GetOperator(word);

         if (found->mPrecedence and priority >= found->mPrecedence) {
            VERBOSE(Logger::Yellow,
               "Delaying reflected operator [", found,
               "] due to a prioritized operation");
            return 0;
         }

         VERBOSE_TAB("Parsing reflected operator: [", word, "] (", found, ")");
         progress += word.size();
         const Code relevant = input.RightOf(progress);
         auto operation = Verb::FromMeta(found);
         if (CompareOperators(word, found->mOperatorReverse))
            operation.SetMass(-1);

         return progress + ParseReflected(operation, relevant, lhs, optimize);
      #else    //LANGULUS_FEATURE(MANAGED_REFLECTION)
         PRETTY_ERROR("Can't parse reflected operator, managed reflection feature is disabled");
      #endif   //LANGULUS_FEATURE(MANAGED_REFLECTION)
      }
      else {
      #if LANGULUS_FEATURE(MANAGED_REFLECTION)
         // Handle a reflected verb                                     
         const auto word = Isolate(input);
         const auto found = RTTI::GetMetaVerb(word);

         if (found->mPrecedence and priority >= found->mPrecedence) {
            VERBOSE(Logger::Yellow,
               "Delaying reflected operator [", found, 
               "] due to a prioritized operation");
            return 0;
         }

         progress += word.size();
         VERBOSE_TAB("Parsing reflected verb: [", word, "] (", found, ")");
         const Code relevant = input.RightOf(progress);
         auto operation = Verb::FromMeta(found);
         if (CompareOperators(word, found->mTokenReverse))
            operation.SetMass(-1);

         return progress + ParseReflected(operation, relevant, lhs, optimize);
      #else    //LANGULUS_FEATURE(MANAGED_REFLECTION)
         PRETTY_ERROR("Can't parse reflected verb, managed reflection feature is disabled");
      #endif   //LANGULUS_FEATURE(MANAGED_REFLECTION)
      }
   }

   /// Parse a content scope                                                  
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] parsed content goes here (lhs)                 
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::ParseContent(const Code& input, Many& lhs, bool optimize) {
      Offset progress = 0;

      // Can define contents for one element at a time                  
      if (lhs.GetCount() > 1)
         PRETTY_ERROR("Content scope for multiple elements is not allowed: ", lhs);

      // We don't know what to expect, so we attempt blind parse        
      Many rhs;
      progress = UnknownParser::Parse(input, rhs, 0, optimize);
      if (not input.RightOf(progress).StartsWithOperator(Operator::CloseScope))
         PRETTY_ERROR("Missing closing bracket");

      // Account for the closing content scope                          
      progress += SerializationRules::Operators[Operator::CloseScope].mToken.size();

      // Insert to new content in rhs to the already available lhs      
      InsertContent(rhs, lhs);
      return progress;
   }

   /// Insert content to lhs, instantiating it if we have to                  
   /// Content is always inserted to the last element in LHS, if multiple     
   /// elements are present. If last element is a meta definition, the        
   /// definition will be replaced by the instantiated element                
   ///   @param rhs - the content to insert                                   
   ///   @param lhs - the place where the content will be inserted            
   void Code::OperatorParser::InsertContent(Many& rhs, Many& lhs) {
      if (lhs.IsUntyped() or not lhs) {
         // If output is untyped, we directly push content, regardless  
         // if it's filled with something or not - a scope is a scope   
         // If empty, just merge states                                 
         const auto stateBackup = lhs.GetState();
         lhs.ResetState();
         lhs.SmartPush(IndexBack, Move(rhs));
         lhs.AddState(stateBackup);
         VERBOSE_ALT("Untyped content: ", Logger::Cyan, lhs);
      }
      else if (lhs.Is<DMeta>()) {
         // The content is for an uninstantiated data scope             
         const auto meta = lhs.As<DMeta>(IndexLast);
         LANGULUS_ASSERT(meta, Flow, "Bad data id");

         if (meta->Is<Verb>()) {
            lhs.RemoveIndex(IndexLast);
            lhs.SmartPush(IndexBack, Verb {Move(rhs)});
         }
         else if (meta->Is<Trait>()) {
            lhs.RemoveIndex(IndexLast);
            lhs.SmartPush(IndexBack, Trait {Move(rhs)});
         }
         else {
            if (not rhs and not meta->mProducerRetriever
            and meta->mDefaultConstructor) {
               // Invoke default-construction                           
               Many constExpr;
               constExpr.SetType(meta);
               constExpr.New(1);
               lhs.RemoveIndex(IndexLast);
               lhs.SmartPush(IndexBack, Abandon(constExpr));
            }
            else {
               // Invoke the descriptor-constructor only if we have to  
               Construct outputConstruct {meta, Move(rhs)};
               Many precompiled;
               if (outputConstruct.StaticCreation(precompiled)) {
                  // Precompiled successfully, append it to LHS         
                  lhs.RemoveIndex(IndexLast);
                  lhs.SmartPush(IndexBack, Abandon(precompiled));
                  VERBOSE_ALT("Statically constructed from DMeta: ", Logger::Cyan, lhs);
                  return;
               }

               lhs.RemoveIndex(IndexLast);
               lhs.SmartPush(IndexBack, Abandon(outputConstruct));
            }
         }
         VERBOSE_ALT("Constructed from DMeta: ", Logger::Cyan, lhs);
      }
      else if (lhs.Is<VMeta>()) {
         // The content is for an uninstantiated verb scope             
         const auto meta = lhs.As<VMeta>(IndexLast);
         LANGULUS_ASSERT(meta, Flow, "Bad verb id");

         auto verb = Verb::FromMeta(meta, Move(rhs));
         lhs.RemoveIndex(IndexLast);
         lhs.SmartPush(IndexBack, Abandon(verb));
         VERBOSE_ALT("Constructed from VMeta: ", Logger::Cyan, lhs);
      }
      else if (lhs.Is<TMeta>()) {
         // The content is for an uninstantiated trait scope            
         const auto meta = lhs.As<TMeta>(IndexLast);
         LANGULUS_ASSERT(meta, Flow, "Bad trait id");

         auto trait = Trait::From(meta, Move(rhs));
         lhs.RemoveIndex(IndexLast);
         lhs.SmartPush(IndexBack, Abandon(trait));
         VERBOSE_ALT("Constructed from TMeta: ", Logger::Cyan, lhs);
      }
      else if (lhs.Is<Verb>()) {
         // The content is for an instantiated verb scope               
         auto& verb = lhs.As<Verb>(IndexLast);
         verb.GetArgument().SmartPush(IndexBack, Move(rhs));
         VERBOSE_ALT("Constructed from Verb ", Logger::Cyan, lhs);
      }
      else if (lhs.Is<Construct>()) {
         // The content is for an instantiated data scope               
         auto& construct = lhs.As<Construct>(IndexLast);
         construct << Move(rhs);
         VERBOSE_ALT("Constructed from Construct ", Logger::Cyan, lhs);
      }
      else {
         Logger::Error("Bad scope for ", lhs, " (", lhs.GetToken(), ')');
         Logger::Error("Content to insert is: ", rhs, " (", rhs.GetToken(), ')');
         LANGULUS_THROW(Flow, "Syntax error - bad scope");
      }
   }

   /// String/character/code scope                                            
   ///   @param op - the starting operator                                    
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] parsed content goes here (lhs)                 
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::ParseString(
      const Code::Operator op, const Code& input, Many& lhs
   ) {
      Offset progress = 0;
      Offset depth = 1;
      while (progress < input.GetCount()) {
         // Collect all characters in scope, essentially gobbling them  
         // up into a text container until matching token is reached    
         const auto relevant = input.RightOf(progress);

         switch (op) {
         case Operator::OpenString:
         case Operator::OpenStringAlt: {
            // Finish up a "string" or `string`                         
            //TODO handle escapes!
            const auto closer = op == Operator::OpenString
               ? Operator::CloseString : Operator::CloseStringAlt;

            if (relevant.StartsWithOperator(closer)) {
               const auto tokenSize = SerializationRules::Operators
                  [closer].mToken.size();
               lhs << Text {input.LeftOf(progress)};
               VERBOSE("String parsed: ", lhs);
               return tokenSize + progress;
            }
            break;
         }
         case Operator::OpenCharacter: {
            // Finish up a 'c'haracter                                  
            //TODO handle escapes!
            if (relevant.StartsWithOperator(Operator::CloseCharacter)) {
               const auto tokenSize = SerializationRules::Operators
                  [Operator::CloseCharacter].mToken.size();
               lhs << input[0];
               VERBOSE("Character parsed: ", lhs);
               return tokenSize + progress;
            }
            break;
         }
         case Operator::OpenCode: {
            // Finish up a [code]                                       
            // Nested code scopes are handled gracefully                
            if (relevant.StartsWithOperator(Operator::OpenCode))
               ++depth;
            else if (relevant.StartsWithOperator(Operator::CloseCode)) {
               --depth;

               if (0 == depth) {
                  const auto tokenSize = SerializationRules::Operators
                     [Operator::CloseCode].mToken.size();
                  lhs << input.LeftOf(progress);
                  VERBOSE("Code parsed: ", lhs);
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
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] here goes the byte sequence                    
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::ParseBytes(const Code& input, Many& lhs) {
      Offset progress = 0;
      while (progress < input.GetCount()) {
         const auto c = input[progress];
         if (IsDigit(c)) {
            ++progress;
            continue;
         }

         const auto lc = ::std::tolower(c);
         if (lc >= 'a' and lc <= 'f') {
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
         stager |= uint8_t(*i - (IsDigit(*i) ? '0' : 'a')) << shifter;

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
   ///   @param op - the phase operator                                       
   ///   @param lhs - [in/out] phased content goes here                       
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::ParsePhase(const Code::Operator op, Many& lhs) {
      if (op == Operator::Past)
         lhs.MakePast();
      else
         lhs.MakeFuture();
      return 0;
   }

   /// Const contents                                                         
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] constant content goes here                     
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::ParseConst(Many& lhs) {
      lhs.MakeConst();
      return 0;
   }

   /// Execute a reflected verb operator                                      
   ///   @param op - the operator to execute                                  
   ///   @param input - the code to parse                                     
   ///   @param lhs - [in/out] result of the operator goes here               
   ///   @param optimize - whether or not to attempt executing at compile-time
   ///   @return number of parsed characters                                  
   Offset Code::OperatorParser::ParseReflected(
      Verb& op, const Code& input, Many& lhs, bool optimize
   ) {
      Offset progress = 0;
      Code relevant = input;

      // Parse charge if any                                            
      if (ChargeParser::Peek(relevant) != Operator::NoOperator) {
         progress += ChargeParser::Parse(relevant, op);
         relevant = input.RightOf(progress);
      }
      
      // Parse RHS for the operator                                     
      progress += UnknownParser::Parse(
         relevant, op.GetArgument(), op.GetVerb()->mPrecedence, optimize);

      if (optimize and not op.GetCharge().IsFlowDependent()) {
         // Try executing operator at compile-time                      
         // We must disable multicast for this                          
         VERBOSE_TAB("Attempting compile-time execution... ");
         const auto opStateBackup = op.GetVerbState();
         op.Multicast(false);
         Many output;
         Many scope {op};
         if (Execute(scope, lhs, output, true)) {
            // The verb was executed at compile-time, so directly       
            // substitute LHS with the result                           
            VERBOSE("Verb was executed at compile time: ", output);
            lhs = Abandon(output);
            return progress;
         }
         else op.SetVerbState(opStateBackup);
      }

      // Either compile-time execution is impossible, or we don't       
      // want it, so directly substitute LHS with the verb              
      op.SetSource(Move(lhs));
      lhs = Move(op);
      return progress;
   }
   
   /// Peek inside input, and return true if it begins with one of the        
   /// built-in operators for charging                                        
   ///   @param input - the code to peek into                                 
   ///   @return true if input begins with an operator for charging           
   Code::Operator Code::ChargeParser::Peek(const Code& input) noexcept {
      // Parse skippables if any                                     
      auto relevant = input;
      if (SkippedParser::Peek(relevant)) {
         const auto offset = SkippedParser::Parse(relevant);
         relevant = input.RightOf(offset);
      }

      // Find the charge operator                                    
      for (Offset i = 0; i < Operator::OpCounter; ++i) {
         if (SerializationRules::Operators[i].mCharge
         and relevant.StartsWithOperator(i))
            return Operator(i);
      }

      return Operator::NoOperator;
   }

   /// Parse mass/time/frequency/priority operators                           
   ///   @param input - the code to parse                                     
   ///   @param charge - [out] parsed charge goes here                        
   ///   @return number of parsed characters                                  
   Offset Code::ChargeParser::Parse(const Code& input, Charge& charge) {
      Offset progress = 0;
      VERBOSE_TAB("Parsing charge");

      while (progress < input.GetCount()) {
         // Scan input until end of charge operators/code               
         auto relevant = input.RightOf(progress);
         if (not relevant or relevant[0] == '\0')
            break;

         // Parse skippables if any                                     
         if (SkippedParser::Peek(relevant)) {
            progress += SkippedParser::Parse(relevant);
            relevant = input.RightOf(progress);
         }

         // Find the charge operator                                    
         auto op = Operator::NoOperator;
         for (Offset i = 0; i < Operator::OpCounter; ++i) {
            if (SerializationRules::Operators[i].mCharge
            and relevant.StartsWithOperator(i)) {
               op = Operator(i);
               progress += SerializationRules::Operators[i].mToken.size();
               relevant = input.RightOf(progress);
               break;
            }
         }

         if (op == Operator::NoOperator)
            return progress;

         VERBOSE("Parsing charge operator: [",
            SerializationRules::Operators[op].mToken, ']');

         // Skip any spacing and consume '-' operators here             
         bool reverse = false;
         while (SkippedParser::Peek(relevant) or relevant[0] == '-') {
            progress += SkippedParser::Parse(relevant);
            relevant = input.RightOf(progress);
            if (relevant[0] == '-') {
               ++progress;
               reverse = not reverse;
               relevant = input.RightOf(progress);
            }
         }

         // For each charge operator encountered - parse a RHS          
         Many rhs;
         if (KeywordParser::Peek(relevant)) {
            // Charge parameter can be a keyword, like a constant,      
            // but is not allowed to have charge on its own, to         
            // avoid endless nesting - you must wrap it in a scope      
            progress += KeywordParser::Parse(relevant, rhs, false);
         }
         else if (NumberParser::Peek(relevant)) {
            // Can be a literal number                                  
            progress += NumberParser::Parse(relevant, rhs);
         }
         else if (OperatorParser::Peek(relevant) == Operator::OpenScope) {
            // Can be anything wrapped in a scope                       
            progress += OperatorParser::Parse(Operator::OpenScope, relevant, rhs, 0, true);
         }
         else PRETTY_ERROR("Unexpected symbol");

         // Save changes                                                
         // AsCast may throw here, if RHS did not evaluate or convert   
         // to real - this is later caught and handled gracefully       
         auto asReal = rhs.AsCast<Real>();
         if (reverse)
            asReal *= Real {-1};

         switch (op) {
         case Operator::Mass:
            charge.mMass = asReal;
            break;
         case Operator::Rate:
            charge.mRate = asReal;
            break;
         case Operator::Time:
            charge.mTime = asReal;
            break;
         case Operator::Priority:
            charge.mPriority = asReal;
            break;
         default:
            PRETTY_ERROR("Invalid charge operator: ",
               SerializationRules::Operators[op].mToken);
         }
      }

      VERBOSE("Charge parsed: ", charge);
      return progress;
   }

} // namespace Langulus::Flow
