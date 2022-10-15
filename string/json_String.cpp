// json_String.cpp														
// 				
	
#include <iostream>

#include "json_String.h"
																		// class cl_jsonString
bool
cl_jsonString::activate() &
{
	if (_fl_active)		return true ;

	_base = _loader(_fname) ;  ;

#ifdef VERBOSE
	std::cout << std::endl << ": ACTIVATE ->  loaded in string:\n<" << _base << "\n>" ;
#endif

	return _fl_active = (_base != decltype(_base){}) ;
} // cl_jsonString activate()

cl_jsonString::iterator 
cl_jsonString::find(cl_jsonString::iterator pos, const std::string& list_delim) const&
{
	auto	it_end{this->end()} ;
	for ( ; pos != it_end ; ++pos) {
		if (list_delim.find(*pos) != std::string::npos)		return pos ;
	}

	return it_end ;
} // cl_jsonString find()

std::string
cl_jsonString::string_until_char(cl_jsonString::iterator pos, const std::string & list_delim) const&
{
	auto	it{this->find(pos, list_delim)} ;
	return it != this->end()
			? _base.substr(std::distance(this->begin(), pos), std::distance(pos, it))
			: std::string{} ; // throw std::runtime_error("") ;	// error
} // cl_jsonString string_until_char()
																		// eoc cl_jsonString
																		
// eof json_String.cpp
