#include "../GASM.hpp"
#include "IncludeLogger.hpp"

#define PC_SCOPE_VERBOSE(a) pcLogFuncVerbose << a

namespace PCFW::Flow
{

	/// Helper that is analogous to Block::Gather, but also gives the option	
	/// to do a runtime interpretation to elements, that are not compatible		
	/// This is very useful for extracting relevant data from Idea(s)				
	///	@param input - source container													
	///	@param output - [in/out] container that collects results					
	///	@param direction - the direction to search from								
	///	@return the number of gathered elements										
	pcptr GatherAndInterpret(const Block& input, Block& output, const Index direction) {
		if (output.IsUntyped() || output.IsDeep()) {
			// Output is deep/any, so no need to iterate or interpret		
			return output.InsertBlock(input);
		}

		pcptr count = 0;
		if (input.IsDeep()) {
			// Iterate all subpacks														
			if (direction == uiFront) {
				for (pcptr i = 0; i < input.GetCount(); ++i)
					count += GatherAndInterpret(input.As<Block>(i), output, direction);
			}
			else {
				for (pcptr i = input.GetCount(); i > 0; --i)
					count += GatherAndInterpret(input.As<Block>(i - 1), output, direction);
			}

			return count;
		}

		if (output.IsConcatable(input)) {
			// Catenate input if compatible											
			count += output.InsertBlock(input);
		}
		else {
			// Attempt a slow interpretation to the output type				
			auto interpreter = Verb::From<Verbs::Interpret>({}, output.GetMeta());
			Verb::DispatchDeep(const_cast<Block&>(input), interpreter);
			try { count += output.InsertBlock(interpreter.GetOutput()); }
			catch (const Except::BadMutation&) {}
		}

		return count;
	}

	/// Produces constructs, strings, etc.													
	///	@param context - the context to execute in									
	///	@param verb - the scope's stuffing												
	void Verb::DefaultScope(const Block& context, Verb& verb) {
		// Scan source for what we're constructing								
		context.ForEach([&verb](const DataID& type) {
			auto meta = type.GetMeta();
			Construct scope(meta);

			// Scan for charges first													
			TAny<real> mass;
			GatherAndInterpret(verb.GetArgument(), mass, uiFront);
			if (!mass.IsEmpty()) {
				// Mass is collected, but may be fragmented, so use the		
				// concat verb to coalesce the numbers inside					
				PC_SCOPE_VERBOSE(ccGreen << "Mass " << mass << " for " << scope);
				real concatenatedMass = 0;
				for (auto& n : mass)
					concatenatedMass = pcConcat(concatenatedMass, n);
				scope.GetCharge().mMass = concatenatedMass;
			}

			// Scan for DataIDs															
			TAny<DataID> dataIds;
			GatherAndInterpret(verb.GetArgument(), dataIds, uiFront);
			if (!dataIds.IsEmpty()) {
				PC_SCOPE_VERBOSE(ccGreen << "DataID(s) for "
					<< meta->GetToken() << ": " << dataIds);
				scope << dataIds;
			}

			// Scan for MetaDatas														
			TAny<DMeta> metaDatas;
			GatherAndInterpret(verb.GetArgument(), metaDatas, uiFront);
			if (!metaDatas.IsEmpty()) {
				PC_SCOPE_VERBOSE(ccGreen << "DMeta(s) for "
					<< meta->GetToken() << ": " << metaDatas);
				scope << metaDatas;
			}

			// Scan for Constructs														
			TAny<Construct> constructs;
			GatherAndInterpret(verb.GetArgument(), constructs, uiFront);
			if (!constructs.IsEmpty()) {
				PC_SCOPE_VERBOSE(ccGreen << "Construct(s) for "
					<< meta->GetToken() << ": " << constructs);
				scope << constructs;
			}

			// Scan for Traits															
			TAny<Trait> traits;
			GatherAndInterpret(verb.GetArgument(), traits, uiFront);
			if (!constructs.IsEmpty()) {
				PC_SCOPE_VERBOSE(ccGreen << "Trait(s) for "
					<< meta->GetToken() << ": " << traits);
				scope << traits;
			}

			PC_SCOPE_VERBOSE(ccGreen << "Resulting scope: " << scope);
			verb << scope;
		});
	}

} // namespace PCFW::PCGASM
