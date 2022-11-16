#include "config.h"

Config gConfig = {
	.style = "default",
	.sleepy = true,
	.tabBarWidth = 16
};

void PrintEnumValuesError(Message& error,const char** values){
	std::string msg = "Value must be one of ";
	
	const char* value;
	for (size_t i=0;;++i){
		value = values[i];
		if (value==NULL)
			break;
		
		msg += "'";
		msg += value;
		msg += "', ";
	}
	msg = msg.substr(0,msg.size()-2);
	error.Set(std::move(msg));
}

bool SetConfigVar(const ModeOption& op,const TokenVector& tokens,Message& error){
	if (tokens.size()<=2){
		error.Set("No value supplied!");
		return false;
	}

	switch (op.type){
		case OptionType::Bool: {
			std::string val = tokens[2].Stringify();
			if (!ParseBool(*op.boolLoc,val)){
				error.Set("Could not parse '"+val+"' as bool!");
				return false;
			}
			return true;
		}
		case OptionType::Int: {
			std::string val = tokens[2].Stringify();
			if (!ParseInt(*op.intLoc,val)){
				error.Set("Could not parse '"+val+"' as int!");
				return false;
			}
			return true;
		}
		case OptionType::PositiveInt: {
			std::string val = tokens[2].Stringify();
			if (!ParsePositiveInt(*op.intLoc,val)){
				error.Set("Could not parse '"+val+"' as a positive int!");
				return false;
			}
			return true;
		}
		case OptionType::NonNegativeInt: {
			std::string val = tokens[2].Stringify();
			if (!ParseNonNegativeInt(*op.intLoc,val)){
				error.Set("Could not parse '"+val+"' as a non-negative int!");
				return false;
			}
			return true;
		}
		case OptionType::String: {
			std::string val = tokens[2].Stringify();
			*op.strLoc = val;
			return true;
		}
		case OptionType::Float: {
			error.Set("Not implemented!");
			return false;
		}
		
		case OptionType::Enum: {
			std::string val = tokens[2].Stringify();
			const char* enumValue;
			for (size_t i=0;;++i){
				enumValue = op.enumValues[i];
				if (enumValue==NULL)
					break;
				
				if (val==enumValue){
					*op.enumLoc = i;
					return true;
				}
			}
			PrintEnumValuesError(error,op.enumValues);
			return false;
		}
	}
	return false;
}

bool FindAndSetConfigVar(ModeIndex modeIndex,const std::string& name,const TokenVector& tokens,Message& error){
	size_t count;
	const ModeOption* entry = ModeIndexToOptionArray(modeIndex,count);
	
	for (size_t i=0;i<count;++i){
		if (entry[i].name==name){
			return SetConfigVar(entry[i],tokens,error);
		}
	}
	
	const char* mode = ModeIndexToName(modeIndex);
	std::string msg = "'";
	msg += mode;
	msg += '.';
	msg += name;
	msg += "' is not a config var!";
	error.Set(std::move(msg));
	return false;
}

ModeOption gCtxOps[3] = {
	{
		.type = OptionType::String,
		.name = "style",
		.strLoc = &gConfig.style
	},
	{
		.type = OptionType::Bool,
		.name = "sleepy",
		.boolLoc = &gConfig.sleepy
	},
	{
		.type = OptionType::PositiveInt,
		.name = "tabBarWidth",
		.intLoc = &gConfig.tabBarWidth
	}
};

