#ifndef ARGUMENTS_DEFINITION
#define ARGUMENTS_DEFINITION


#include <vector>
#include <string>
#include <algorithm>
#include <map>
#include <set>
#include <iostream>
#include "errors.h"

using namespace std;

namespace arguments{

enum OptionType{
	INT_TYPE,
	BOOL_TYPE,
	STRING_TYPE
};

struct Argument{
	public:

	string name;
	string description;
};

Argument arg(const string & name, const string & description);

template<class OptionT>
struct Option{

	public:
		string shortname;
		string longname;
		OptionT option;
		OptionType type;
		string description;


		int ival;
		bool bval;
		string sval;
		vector<string> valid_values;

		string typeDesc() const{
			switch(type){
				case INT_TYPE:
					return "INT";
				break;

				case BOOL_TYPE:
					return "BOOL";
				break;

				default:
					if(valid_values.empty())
						return "STRING";
					else
						return "ENUM";
				break;
			}
			return "";
		}

		Option(){}
};

template<class OptionT>
Option<OptionT> sop(const string & shortname, const string & longname, OptionT option, const string & defval, const string & description){
	Option<OptionT> o;
	o.shortname = shortname;
	o.longname = longname;
	o.option = option;
	o.description = description;
	o.type = STRING_TYPE;
	o.sval = defval;
	return o;
}

template<class OptionT>
Option<OptionT> sop(const string & shortname, const string & longname, OptionT option, const string & defval, const vector<string> & values, const string & description){
	Option<OptionT> o;
	o.shortname = shortname;
	o.longname = longname;
	o.option = option;
	o.description = description;
	o.type = STRING_TYPE;
	o.sval = defval;
	o.valid_values = values;
	return o;
}

template<class OptionT>
Option<OptionT> iop(const string & shortname, const string & longname, OptionT option, int defval, const string & description){
	Option<OptionT> o;
	o.shortname = shortname;
	o.longname = longname;
	o.option = option;
	o.description = description;
	o.type = INT_TYPE;
	o.ival = defval;
	return o;
}

template<class OptionT>
Option<OptionT> bop(const string & shortname, const string & longname, OptionT option, bool defval, const string & description){
	Option<OptionT> o;
	o.shortname = shortname;
	o.longname = longname;
	o.option = option;
	o.description = description;
	o.type = BOOL_TYPE;
	o.bval = defval;
	return o;
}


template<class OptionT>
class Arguments{

private:
	string description;

	vector<string> arguments;
	vector<string> arguments_description;
	int mandatory;

	vector<string> argument_value;



	map<string,OptionT> name_option;
	map<OptionT,OptionType> option_type;
	map<OptionT,string> option_description;
	map<OptionT,vector<string> > option_svalues;

	map<OptionT,int> intOptions;
	map<OptionT,bool> boolOptions;
	map<OptionT,string> stringOptions;

protected:

	vector<Option<OptionT> > options;


public:

	Arguments(const vector<Argument> & arguments, int mandatory, const vector<Option<OptionT> > & options, const string & description);

	string getDescription() const;

	void addArgument(const string & arg);
	string getArgument(int idx) const;
	string getArgumentName(int idx) const;
	string getArgumentDesc(int idx) const;
	int getNArguments() const;
	int getAllowedArguments() const;
	int getMinimumArguments() const;

	bool hasOption(const string & s) const;
	OptionT getOptionRef(const string & s) const;
	OptionType getOptionType(OptionT option) const;
	void setOption(OptionT option, int value);
	void setOption(OptionT option, bool value);
	void setOption(OptionT option, const string & value);
	int getIntOption(OptionT option) const;
	bool getBoolOption(OptionT option) const;
	string getStringOption(OptionT option) const;
	int getNOptions() const;

	void printOptions() const;

	static Arguments<int> * nullProgArgs();


};

Arguments<int> * nullProgArgs();

template<class OptionT>
Arguments<OptionT>::Arguments(const vector<Argument> & arguments, int mandatory, const vector<Option<OptionT> > & options, const string & description){

	this->description = description;

	for(const Argument & a : arguments){
		this->arguments.push_back(a.name);
		this->arguments_description.push_back(a.description);
	}

	this->mandatory = mandatory;

	this->options = options;

	for(const Option<OptionT> & o: options){
		if(o.shortname != ""){
			if(name_option.find("-"+o.shortname) != name_option.end()){
				cerr << "Option " << ("-"+o.shortname) << " defined twice" << endl;
				exit(BADARGUMENTS_ERROR);
			}
			else
				name_option["-"+o.shortname] = o.option;
		}
		if(o.longname == ""){
			cerr << "It is required to specify a long name for each option" << endl;
			exit(BADARGUMENTS_ERROR);
		}
		else{
			if(name_option.find("--"+o.longname) != name_option.end()){
				cerr << "Option " << ("--"+o.longname) << " defined twice" << endl;
				exit(BADARGUMENTS_ERROR);
			}
			else
				name_option["--"+o.longname] = o.option;
		}
		option_type[o.option] = o.type;
		option_description[o.option] = o.description;
		option_svalues[o.option] = o.valid_values;

		switch(o.type){
			case INT_TYPE:
				this->setOption(o.option, o.ival);
				break;
			case BOOL_TYPE:
				this->setOption(o.option, o.bval);
				break;
			case STRING_TYPE:
				this->setOption(o.option,o.sval);
				break;
		}
	}

}

template<class OptionT>
string Arguments<OptionT>::getDescription() const{
	return description;
}

template<class OptionT>
void Arguments<OptionT>::addArgument(const string & arg){
	argument_value.push_back(arg);
}

template<class OptionT>
string Arguments<OptionT>::getArgument(int idx) const{
	return argument_value[idx];
}

template<class OptionT>
string Arguments<OptionT>::getArgumentName(int idx) const{
	return arguments[idx];
}

template<class OptionT>
string Arguments<OptionT>::getArgumentDesc(int idx) const{
	return arguments_description[idx];
}

template<class OptionT>
bool Arguments<OptionT>::hasOption(const string & s) const{
	return name_option.find(s) != name_option.end();
}

template<class OptionT>
OptionT Arguments<OptionT>::getOptionRef(const string & s) const{
	return name_option.find(s)->second;
}

template<class OptionT>
OptionType Arguments<OptionT>::getOptionType(OptionT option) const{
	return option_type.find(option)->second;
}

template<class OptionT>
void Arguments<OptionT>::setOption(OptionT option, int value){
	intOptions[option]=value;
}

template<class OptionT>
void Arguments<OptionT>::setOption(OptionT option, bool value){
	boolOptions[option]=value;
}

template<class OptionT>
void Arguments<OptionT>::setOption(OptionT option, const string & value){
	const vector<string> & values = option_svalues.find(option)->second;
	if(!values.empty() && find(values.begin(),values.end(),value) == values.end()){
		cerr << "Unsupported option value '" << value << "'" << endl;
		exit(BADARGUMENTS_ERROR);
	}
	stringOptions[option]=value;
}

template<class OptionT>
int Arguments<OptionT>::getIntOption(OptionT option) const{
	return intOptions.find(option)->second;
}

template<class OptionT>
bool Arguments<OptionT>::getBoolOption(OptionT option) const{
	return boolOptions.find(option)->second;
}

template<class OptionT>
string Arguments<OptionT>::getStringOption(OptionT option) const{
	return stringOptions.find(option)->second;
}

template<class OptionT>
int Arguments<OptionT>::getNOptions() const{
	return option_type.size();
}

template<class OptionT>
int Arguments<OptionT>::getNArguments() const{
	return argument_value.size();
}

template<class OptionT>
int Arguments<OptionT>::getAllowedArguments() const{
	return arguments.size();
}

template<class OptionT>
int Arguments<OptionT>::getMinimumArguments() const{
	return mandatory;
}

template<class OptionT>
void Arguments<OptionT>::printOptions() const{
	for(const arguments::Option<OptionT> & o : options){

		if(o.shortname == ""){
			printf("          --%s=%s",
			o.longname.c_str(),o.typeDesc().c_str());
		}
		else{
			printf("  -%s=%-4s --%s=%s",
			o.shortname.c_str(),o.typeDesc().c_str(),
			o.longname.c_str(),o.typeDesc().c_str());
		}

		if(!o.valid_values.empty()){
			cout << " : ";
			bool comma=false;
			for(const string & s : o.valid_values){
				if(comma)
					cout << ", ";
				else
					comma=true;
				cout << s;
			}
		}

		cout << endl;

		cout << "            " << o.description << endl << endl;
	}
}

}

#endif

