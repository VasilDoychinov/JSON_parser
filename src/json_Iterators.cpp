// json_Iterators.cpp" the iterators interface to 
//

#include "json_Parser.h"

// a parent scope =:: full path; if just a section name - (and NOT unique) ...
																		// class cl_jsonScope (iterators)
cl_jsonScope::cl_jsonScope(const jsonSections& ss, const std::string& parent_scope)
							: _scn{}, _it{ss.cbegin()}, _it_end{ss.cend()}
{
																		// std::cout << "\n___ _scope_iter(): number of sections: " 
																		// << ss.size() << ", set for: " << scope ;
	_scn = parent_scope.size() == 0 ? JSON_TOP_Scope : parent_scope ;	// search the top

	// find the 1st occurence of parent_scope:
	{	auto	it_beg{_it} ;
		for (; it_beg != _it_end ; ++it_beg) {
			if (it_beg->section_name_contains(_scn))	break ;
		}
		if (!((_it = std::move(it_beg)) != _it_end))	return ;
	}
																		// cout << endl << ": looking for <" << _scn 
																		// << "> : found <" << _it->_name << ">" ;
	// find the parent: should be the "outmost"/last
	for (--_it_end ; _it_end != _it ; --_it_end) {
		if (_it_end->section_name_match(_scn))		break ;		// the outmost will be last
	}
	if (!(_it != _it_end))		return ;	// no sections in parent_scope: its wrong format actually

	_scn = _it_end->_name ;					// the scope
	auto	level{_it_end->section_level() + 1} ;
																		// cout << endl << ": found scope: " << _it_end->_name ;
																		// cout << " : setting level: " << level ;
	for ( ; _it != _it_end ; ++_it) {
		if (_it->section_level() == level)		break ;		// 1st within parent_scope
	}
																		// std::cout << "\n_____ _it_end->" << *_it_end << "\n" ;
	// _it: first occurence or _it_end(cend(ss))
	// _it_end: after the last matching or, _it (cend(ss))
	return ;
} // cl_jsonScope ()
																		// eoc cl_jsonScope(iterators)
// eof json_Iterators.cpp