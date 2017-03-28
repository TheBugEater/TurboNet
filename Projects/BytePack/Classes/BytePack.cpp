#include "BytePack.h"
#include <stdlib.h>

BytePack::BytePack(void)
{
}


BytePack::~BytePack(void)
{
}

/////////////////////////////////////////////////////////
//Add Values           //////////////////////////////////
/////////////////////////////////////////////////////////
void BytePack::AddValue(string key,const char *value)
{
	m_valuesString[key] = value;
}

void BytePack::AddValue(string key,int value)
{
	m_valuesInt[key] = value;
}

void BytePack::AddValue(string key,float value)
{
	m_valuesFloat[key] = value;
}

void BytePack::AddValue(string key,bool value)
{
	m_valuesBool[key] = value;
}

/////////////////////////////////////////////////////////
//Get Values           //////////////////////////////////
/////////////////////////////////////////////////////////
void BytePack::GetValue(string key,string &value)
{
	value = m_valuesString[key];
}

void BytePack::GetValue(string key,int &value)
{
	value = m_valuesInt[key];
}

void BytePack::GetValue(string key,float &value)
{
	value = m_valuesFloat[key];
}

void BytePack::GetValue(string key,bool &value)
{
	value = m_valuesBool[key];
}

///////////////////////////////////////////////////////////
void BytePack::RemoveValue(string key)
{
	//Int
	map<string,int>::iterator itI;
	itI = m_valuesInt.find(key);
	if(itI != m_valuesInt.end())
		m_valuesInt.erase(itI);

	//Float
	map<string,float>::iterator itF;
	itF = m_valuesFloat.find(key);
	if(itF != m_valuesFloat.end())
		m_valuesFloat.erase(itF);

	//String
	map<string,string>::iterator itS;
	itS = m_valuesString.find(key);
	if(itS != m_valuesString.end())
		m_valuesString.erase(itS);

	//Bool
	map<string,bool>::iterator itB;
	itB = m_valuesBool.find(key);
	if(itB != m_valuesBool.end())
		m_valuesBool.erase(itB);
}

const char* BytePack::GetDataString()
{
	m_string.clear();

	//Int First
	map<string,int>::iterator Intit = m_valuesInt.begin();

	while(Intit != m_valuesInt.end())
	{
		string first = Intit->first;
		int second = Intit->second;
		string value = first + ":";

		char number[100];
		sprintf(number,"%d",second);
		number[strlen(number)] = '\0';

		value.append(number);
		m_string.append(value);
		Intit++;

		if(Intit != m_valuesInt.end())
		{
			m_string.append("/");
		}
	}

	m_string.append("|");

	//Float Second
	map<string,float>::iterator Floatit = m_valuesFloat.begin();

	while(Floatit != m_valuesFloat.end())
	{
		string first = Floatit->first;
		float second = Floatit->second;
		string value = first + ":";

		char number[100];
		sprintf(number,"%f",second);
		number[strlen(number)] = '\0';

		value.append(number);
		m_string.append(value);
		Floatit++;

		if(Floatit != m_valuesFloat.end())
		{
			m_string.append("/");
		}
	}

	m_string.append("|");

	//Bool Third
	map<string,bool>::iterator Boolit = m_valuesBool.begin();

	while(Boolit != m_valuesBool.end())
	{
		string first = Boolit->first;
		int second = Boolit->second;
		string value = first + ":";

		char number[100];
		sprintf(number,"%d",second);
		number[strlen(number)] = '\0';

		value.append(number);
		m_string.append(value);

		Boolit++;

		if(Boolit != m_valuesBool.end())
		{
			m_string.append("/");
		}
	}

	m_string.append("|");

	//String Fourth
	map<string,string>::iterator Stringit = m_valuesString.begin();

	while(Stringit != m_valuesString.end())
	{
		string first = Stringit->first;
		string second = Stringit->second;
		string value = first + ":" + second;

		m_string.append(value);
		Stringit++;

		if(Stringit != m_valuesString.end())
		{
			m_string.append("/");
		}
	}

	return m_string.c_str();
}

void BytePack::InitWithDataString(const char *data)
{
	m_valuesString.clear();
	m_valuesInt.clear();
	m_valuesFloat.clear();
	m_valuesBool.clear();

	vector<string> VariableValues = Parse(data,"|");

	string dataString = data;

	if(!VariableValues.size() || !dataString.size())
	{
		return;
	}

	/////////////////////////////////////////////////////////////////
	//Get the Last Updated String if there are more strings attached.
	vector<string> IntValues = Parse(VariableValues[VariableValues.size()-4].c_str(),"/");

	vector<string>::iterator it = IntValues.begin();

	while(it != IntValues.end())
	{
		string value = *it;

		if(value.size() < 1)
		{
			break;
		}

		vector<string> token = Parse(value,":");

		m_valuesInt[token[0]] = atoi(token[1].c_str());

		it++;
	}

	///////////////////////////////////////////////////////////////////
	vector<string> FloatValues = Parse(VariableValues[VariableValues.size()-3].c_str(),"/");

	it = FloatValues.begin();

	while(it != FloatValues.end())
	{
		string value = *it;

		if(value.size() < 1)
		{
			break;
		}

		vector<string> token = Parse(value,":");

		m_valuesFloat[token[0]] = atof(token[1].c_str());

		it++;
	}

	//////////////////////////////////////////////////////////////////
	vector<string> BoolValues = Parse(VariableValues[VariableValues.size()-2].c_str(),"/");

	it = BoolValues.begin();

	while(it != BoolValues.end())
	{
		string value = *it;

		if(value.size() < 1)
		{
			break;
		}

		vector<string> token = Parse(value,":");

		m_valuesBool[token[0]] = (bool)atoi(token[1].c_str());

		it++;
	}

	//////////////////////////////////////////////////////////////////
	vector<string> StringValues = Parse(VariableValues[VariableValues.size()-1].c_str(),"/");

	it = StringValues.begin();

	while(it != StringValues.end())
	{
		string value = *it;

		if(value.size() < 1)
		{
			break;
		}

		vector<string> token = Parse(value,":");

		m_valuesString[token[0]] = token[1];

		it++;
	}
}

void BytePack::Clean()
{
	m_valuesString.clear();
	m_valuesInt.clear();
	m_valuesFloat.clear();
	m_valuesBool.clear();
}

vector<string> BytePack::Parse(string str,string delimeter)
{
	std::vector<std::string> output;

	std::string::size_type prev_pos = 0, pos = 0;

	while((pos = str.find(delimeter, pos)) != std::string::npos)
	{
		std::string substring( str.substr(prev_pos, pos-prev_pos) );

		output.push_back(substring);

		prev_pos = ++pos;
	}

	output.push_back(str.substr(prev_pos, pos-prev_pos));

	return output;
}