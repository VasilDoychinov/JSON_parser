// json_Iterators.h: iterators to operate on a descriptor file ;
//                 NB: designed to be used within cl_jsonDescr class ;
//                     to be included in "parser.h", only.
//                 member functions in "parser.cpp"
// 
//      - scope_iterator: forward_iterator_tag
//              - iterates from {} to {} of a same scope: defined by cl_sectionUnit
//              - operations: constructors, ++, !=, limit(), 
//                            *: returns std:vector<>::const_iterator
//      - data_interator: forward_iterator_tag+ 
//              - iterates within {} by a defined step of delimiters
//              - operations: constructors, ++, !=, limit(), + address: positions it over a field 
//                            *: returns std:string with the data field
//


#ifndef _DEFS_JSON_ITERATORS_
#define _DEFS_JSON_ITERATORS_

#include <assert.h>
#include <string>

#include "json_Params.h"    // basic constants, type definitions, etc

class cl_jsonScope {     // iterates within a scope=::{}, ie from a Section {} to next one {} on the same level
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = typename s_sectionAtom ;
        using size_type = typename size_t ;

        using pointer = typename const s_sectionAtom * ;
        using reference = typename const s_sectionAtom & ;

    private:
        std::string     _scn{} ;                // the parent section

        jsonSections::const_iterator _it ;      // current in the scope
        jsonSections::const_iterator _it_end ;  // limit of the scope

    private:
        // size_t  _count_levels() { return 0 ; /*size_t num_levels{0} ; for (const auto& e :*/ }

    public:
        cl_jsonScope() : _it{}, _it_end{}, _scn{} { }						// default C.
        cl_jsonScope(const jsonSections& ss, const std::string& parent_scope = std::string{}) ;
        // all specials  = default

        bool operator !=(const cl_jsonScope& ri) const& { return _it != ri._it ; } // ??? id check

        cl_jsonScope limit() { cl_jsonScope temp(*this) ; temp._it = temp._it_end ; return temp ; }
        
        // operators
        cl_jsonScope&   operator ++() & { auto level{_it_end->section_level() + 1} ; // ??? check for name match ommitted
            if (_it != _it_end)   {
                for (++_it ; _it != _it_end ; ++_it) { if (_it->section_level() == level) break ; }
            }
            return *this ; 
        }
        reference       operator * () const& { return  *_it ; }
        pointer         operator ->() const& { return &(*_it) ; }

        // identifiers
        const std::string& scope() const& { return _scn ; } // or, _it_end->name
        const std::string& id() const& { return _it->_name ; }
}; // class cl_jsonScope

template <typename JCont>
class cl_jsonData {     // iterates within a data section by a defined step =:: # of delimiters
    public:
        using iterator_category = std::forward_iterator_tag ;
        using value_type = typename jsonData_type ;
        // using size_type = typename size_t ;
        using reference = typename jsonData_type& ;

    private:                                                                         
        const JCont&            _jcont ;        // to the JSON data
        const s_sectionAtom*    _scope ;        // the data scope
                                                                            // OLD: s_sectionAtom _s_unit{} ;
        size_t              _step_size{0} ; // measured in # of _delim (until _end?)
        long                _addr_c{-1L} ;  // current position in the scope
        mutable long        _a_n{-1L} ;     // address of a next element, -1L if not set
        mutable value_type  _data{} ;       // buffer for the value

    public:
        cl_jsonData(const JCont& jc, const cl_jsonScope& s_it, const std::string& data_name, size_t ss) ;
        // all specials  = default

        bool is_valid() const& { return _scope != nullptr ; }   // if cl_jsonData is valid
        long tell() const& { return _addr_c ; }                 // the position of ...

        // mark the End of data scope
        cl_jsonData limit() const& { cl_jsonData temp(*this) ; temp._addr_c = _scope ? _scope->_addr_end : -1L ; return temp ; }

        // operators
        bool operator !=(const cl_jsonData& ri) const& { return _addr_c != ri._addr_c ; }
        value_type operator *() const & ;        // de-reference

        // Position operators
        cl_jsonData& operator ++() & ;
        // cl_jsonData  operator +(long addr) const& { cl_jsonData t{*this} ; t._addr_c = addr ; return t ; }

        friend std::ostream& operator <<(std::ostream& os, const cl_jsonData& di) {
        os << "data section-> current addr: " << di._addr_c << ", next addr: " << di._a_n << ", step:  " << di._step_size 
            // << ", data : " // << di._data
            << "\n___ section: " ;
        if (di._scope != nullptr)   os << *(di._scope) ;
        else                        os << "<nullptr>" ;
        return os ;
    }
}; // class cl_jsonData

template <typename JCont>
cl_jsonData<JCont>::cl_jsonData(const JCont& jc, const cl_jsonScope& sc, const std::string& data_name, size_t ss)
                                : _jcont{jc}, _scope{nullptr}, _addr_c{-1L}, _step_size{ss}, _a_n{-1L}, _data{}
{ 
                                                                        // std::cout << std::endl << "--- _addr_c: " << _addr_c
                                                                        // 	<< "; section: " << _s_unit << "; step: " << ss ;
    if (_jcont.invalid())      return ;         // container not valid

    auto    s_it{sc}, s_end{s_it.limit()} ;     // mark the scope to iterate through

    for (auto pos{(sc.scope()).size()} ; s_it != s_end ; ++s_it) {
        if (((s_it.id()).size() == pos + 1 + data_name.size()) && 
            ((s_it.id()).find(data_name, pos) != std::string::npos))  break ;
    }
    if (!(s_it != s_end))       return ;        // empty section or data name not found

    _scope = &(*s_it) ;                         // the data section
    _addr_c = _scope->_addr_start + 1, _a_n = _addr_c ;

    return ;
} // cl_jsonData()

template <typename JCont> typename cl_jsonData<JCont>::value_type
cl_jsonData<JCont>::operator *() const &
{
    if (_data.size() > 0)     return _data ;

    unsigned char	c_buff{} ;		        // work variable
                                            // initiate [ start(in _addr), end(in _addr_end) )
    auto            addr{_addr_c} ;
    const auto&     addr_end{_scope->_addr_end} ;

    int		        count{0} ;
    auto            jc_it{_jcont.begin()} ; // start of JCont 
    for (_jcont.advance(jc_it, addr) ; addr < addr_end ; addr++) {                                            
       c_buff = *jc_it, ++jc_it ;
       if (c_buff == JSON_delimiter && (++count == _step_size))  break ;

       if (std::isgraph(c_buff))	_data.push_back(c_buff) ;
    }
    _a_n = (addr < addr_end) ? addr + 1 : addr ; // position of _fh

    return _data ;
} // cl_jsonData operator *()

template <typename JCont> typename cl_jsonData<JCont>&
cl_jsonData<JCont>::operator ++() & // move after the defined _step_size (in delimiters)
{
                                                                        // std::cout << std::endl 
                                                                        // << "- operator++ on _data_iter: \n"
                                                                        // << *this ;
    // as it usually act in pair with operator *()
    // check if the address of the next elements's been set
    if (_a_n != -1L) { _addr_c = _a_n, _a_n = -1L, _data = cl_jsonData::value_type{} ; return *this ; }
    const auto&     addr_end{_scope->_addr_end} ;
    if (_addr_c >= addr_end)	throw std::runtime_error("___data_iter ++: position out of scope") ;

    // find the next element
    int		        count{0} ;
    auto            jc_it{_jcont.begin()} ; // start of JCont 
    for (_jcont.advance(jc_it, _addr_c) ; _addr_c < addr_end ; _addr_c++, ++jc_it) {
        if (*jc_it == JSON_delimiter && (++count == _step_size))  break ;
    }
    if (_addr_c < addr_end)    _addr_c++ ; // position in JCONT; here _a_n is already -1L
    _data = cl_jsonData::value_type{} ;             // clear the data buffer: just in case

    return *this ;
} // cl_jsonData operator ++()

_DEFS_JSON_ITERATORS_
#endif
// eof json_Iterators.h