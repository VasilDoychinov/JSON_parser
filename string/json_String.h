// json_String.h: type to handle data in JSON format contained in std::strings
//      Possible usage: load the data from a file into a string, ...
//      Must provide:
//          - constructors, destructors as
//          - bool activate(): if succesfully activated
//          - void deactivate(): deactivate / destroy / release resources
//          - bool invalid(): if the container is invalid
//          - id(): JCont's identifier as a const string &
//          
//          The rest of required follow the forward constant_iterator model/support
//          - begin(): forward iterator to the start of JCont
//          - end():   forward iterator: jsut after the last character of
//          - .advance(iterator&, distance): follows std::advance()
//          - unsigned char operator *(iterator): 
//          - iterator& operator ++(): 
//          - std::string string_until_char(iterator, string of chars): returns the string
//            from iterator to any of "string of chars";
//          - find(iterator, string of chars): iterator to the 1st of "string of chars" found or,
//            end() if none
//          - .distance(start, end): follow std::distance()
//

#include <iostream>

#ifndef _DEFS_JSON_STRING_
#define _DEFS_JSON_STRING_

#include <string>

#include <assert.h>

/*
class cl_iteratorFile {
    public:
    using iterator_category = std::forward_iterator_tag ;
    using value_type = typename unsigned char ;

    private:
    int         _fh ;   // low level I/O File Handle
    long        _addr ;
    value_type  _buff ;

    public:
    explicit cl_iteratorFile(int fh, long a = 0L, value_type ch = value_type{}) : _fh{fh}, _addr{a}, _buff{ch} { assert(fh >= 0) ; }
    // all specials  = default 

    decltype(auto) tell() const& { return _addr ; }

    bool operator !=(const cl_iteratorFile& rs) const& { return _addr != rs._addr || _fh != rs._fh ; }
    value_type operator *() {
        if (_buff != value_type{})  return _buff ;
        _lseek(_fh, _addr, SEEK_SET) ;
        if (_read(_fh, static_cast<value_type *>(&_buff), sizeof(_buff)) != sizeof(_buff))   return value_type{} ;
        return _buff ;
    }

    // movements
    cl_iteratorFile& operator ++() { _addr += sizeof(value_type), _buff = value_type{} ; return *this ; }

    cl_iteratorFile _find_IF(const std::string& list_delim) & ;
    std::string     _such_IF(const std::string& list_delim) & ;

    friend long _distance_IF(const cl_iteratorFile& b, const cl_iteratorFile& e) { return e._addr - b._addr ; }
    friend void _advance_IF(cl_iteratorFile& it, long hops) { it._addr += hops, it._buff = value_type{} ; }

    friend std::ostream& operator <<(std::ostream& os, const cl_iteratorFile& it) {
        os << "itFile{f: " << it._fh << ", a: " << it._addr << ", b: " << it._buff << "}" ;
        return os ;
    }
}; // class cl_iteratorString
*/

#include <utility>
#include <functional>

class cl_jsonString {
    public:
    using   iterator = typename std::string::const_iterator ;
    using   value_type = iterator::value_type ;

    private:
        std::string   _fname{} ;
        std::string   _base ;       // data container
        bool          _fl_active ;  // if the String is active

        std::function<std::string(std::string&)> _loader ;   // to load data into _base

    public:
        explicit cl_jsonString(const std::string& name, std::function<std::string(std::string&)> f_load)
                                : _fname{name}, _base{}, _fl_active{false}, _loader{f_load} { /*_base = _loader(_fname) ;*/ }
        virtual ~cl_jsonString() { this->deactivate() ; }

        cl_jsonString(cl_jsonString&&) = default ;
        cl_jsonString& operator =(cl_jsonString&&) = default ;

        cl_jsonString(const cl_jsonString&) = delete ;
        cl_jsonString& operator =(const cl_jsonString&) = delete ;

        const std::string& id() const& { return _fname ; }

        bool activate() & ;
        void deactivate() & { _fl_active = false, _base = decltype(_base){} ; }
        bool invalid() const& { return !_fl_active || _base == (decltype(_base){}) ; }

        // iterator support
        iterator begin() const& { return _base.cbegin() ; }
        iterator end() const& { return _base.cend() ; }

        decltype(auto) distance(const iterator b, const iterator e) const& { return std::distance(b, e) ; }
        decltype(auto) advance(iterator& it, long hops) const& { return std::advance(it, hops) ; }

        iterator find(iterator pos, const std::string& list_delim) const& ;
        std::string string_until_char(cl_jsonString::iterator pos, const std::string& l_d) const& ;

        friend std::ostream& operator <<(std::ostream& os, cl_jsonString& js) {
            os << "String{" << js._base << "}" ;
            return os ;
        }
}; // class cl_jsonString

_DEFS_JSON_STRING_
#endif
// eof json_String.h