#pragma once

#include <iostream>
#include <unordered_map>

class URLhandler
{
	public:

	URLhandler();
	~URLhandler();

	void decode(std::string &str);
	void encode(std::string &str);


	private:

	std::unordered_map<std::string, std::string> m_decodeMap;
	std::unordered_map<char, std::string> m_encodeMap;




};