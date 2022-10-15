// json_Parser.h: Prototype (ver 0.1): works on 'unsigned chars' only for now,
//                i.e. decltype(*JCont::iterator) is unsigned char
// 
//      Types and declarations for: parsing data in JSON format
//      Provides: iterators model READ ONLY (for know) access to JSON Data Format, held
//                in JCont
//      Containers ( template parameter <JCont>) must provide the following interface
//          - constructors, destructors as
//          - bool activate(): should give Access to the current state of Data; returns - if succesfully activated
//          - void deactivate(): deactivate & "free" Data
//          - bool invalid(): if the container is invalid
//          - id(): JCont's identifier as a const string &
//          
//          The rest of required follow the forward constant_iterator model
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
//      Specific examples for JCont "containers":
//      - file: 
//      - std::string
//


#ifndef _DEFS_JSON_PARSER_
#define _DEFS_JSON_PARSER_

#include <iostream>
#include <assert.h>

#include <utility>

#include <string>
#include <vector>


#include "json_Params.h"        // basic constants, type definitions, etc
// std::string load_file(std::string& fname) ;

#include "json_Iterators.h"

template <typename JCont>       // container of/providing access to data in json Format
class cl_jsonDescr {
    public:
        using iterator = typename cl_jsonScope ;
        using data_iterator = typename cl_jsonData<JCont> ;

    private:
        JCont           _jcont ;
        jsonSections    _sections{} ; //s_sectionUnit   _sections ; // jsonSections     _sections{} ;

    private:
        // recursive: check & load major sections
        jsonSections _parse(long fm, const std::string& name_parent, unsigned char sc, long& addr_reached) ;

    public:
        template <typename ...Args> explicit cl_jsonDescr(Args... args) : _jcont{std::forward<Args>(args)...}, _sections{} {}
        virtual ~cl_jsonDescr() { deactivate() ; }

        cl_jsonDescr(cl_jsonDescr&&) = default ;
        cl_jsonDescr& operator =(cl_jsonDescr&&) = default ;
        cl_jsonDescr(const cl_jsonDescr&) = delete ;
        cl_jsonDescr& operator =(const cl_jsonDescr&) = delete ;

        // operations:
        bool invalid() const& { return _jcont.invalid() ; }
        bool activate() & ;
        void deactivate() & { _jcont.deactivate() ; }

        std::ostream& show_section(std::ostream& os, const std::string& parent_scope = std::string{}) const& ;
        std::ostream& show_all_sections(std::ostream& os) const& ; // ???

        const std::string& id() const& { return _jcont.id() ; }
        const size_t size() const& { return _sections.size() ; }

        // scopes
        iterator begin_scope(const std::string& s_name = std::string{}) const& { return iterator(_sections, s_name) ; }

        data_iterator begin_data(const iterator& s_it, const std::string& d_n, size_t ss) const& { return data_iterator(_jcont, s_it, d_n, ss) ; }
        data_iterator begin_data(const std::string& scope, const std::string& d_n, size_t ss) const& { 
            return data_iterator(_jcont, begin_scope(scope), d_n, ss) ; 
        }

        friend std::ostream& operator<<(std::ostream& os, cl_jsonDescr& jd) {
            os << jd._jcont ;
            return os ;
        }
#ifdef XXXXXXXXXXX

        // index(temporary) elements of a data section
        std::vector<long> index_data(data_iterator&& iter) & {  std::vector<long>   v_addr{} ;
            for (auto iter_e = iter.limit() ; iter != iter_e ; ++iter) {
                v_addr.push_back(iter.addr()) ;
            }
            return v_addr ;
        }
        // load elements of a data section ( as std::string)
        std::vector<std::string> load_data(data_iterator&& iter) & { std::vector<std::string>   v_data{} ;
            for (auto iter_e = iter.limit() ; iter != iter_e ; ++iter) {
                v_data.push_back(*iter) ;
            }
            return v_data ;
        }

        // Friends
        friend std::vector<std::string> extract_data(cl_jsonDescr& scene,
                                                     const std::string& scope, 
                                                     const std::string& name, 
                                                     int ss);

        friend class _scope_iter ;
#endif
        friend class cl_jsonData<JCont> ;
}; // class cl_jsonDescr

#include "json_Parser.hpp"          // member functions templates

#ifdef _HELPER_FUNCTIONS
// Helper/converter functions
template <typename T> vector_RT<T>
vector_from_string(const std::string& str)  // there has to be two ',': defining three numbers 
{
    T       mark[3] = {{},} ;
                                                                            // cout << endl << endl 
                                                                            // << "___ in vector from_string("
                                                                            // << str << ")" ;
    try {
        size_t  i = 0 ;
        size_t  count = 0 ;
        for (size_t j = 0 ; j < str.size() && count < 2 ; j++) {
            if (str[j] == ',') {
                mark[count ++] = static_cast<T>(std::stod(str.substr(i, j - i))),
                    i = j + 1 ;
            }
        }
        if (count < 2) { throw std::runtime_error("___ parser VFS<" + str + ">: bad vector descriptor") ; }
        mark[2] = static_cast<T>(std::stod(str.substr(i))) ;
    } catch (...) { throw ; }

    return vector_RT<T>(mark[0], mark[1], mark[2]) ;
} // vector_from_string()

template <typename T> T
number_from_string(const std::string& str)  // there has to be a ':' - the number starts after it
{
    T       number{0} ;
    size_t  pos = 0 ;

    if ((pos = str.find(":")) != std::string::npos) {
        number = static_cast<T>(std::stod(str.substr(pos + 1))) ;
    }
    return number ;
} // number_from_string()

template <typename T> inline std::pair<triangle_RT<T>, vector_RT<unsigned int>>
triangle_from_indexes(_data_iter& it_tr, std::vector<long>& indexes)  // it_tr: assumed to be (ui,ui,ui)
{
    vector_RT<unsigned int> temp{vector_from_string<unsigned int>(*it_tr)} ;
    // temp holds the indexes of triangle's vertices in indexes

    // a bit of cheating but iter{it_tr+indexes[]} & it_tr are of the same scope
    return std::pair<triangle_RT<T>, vector_RT<unsigned int>>
           (triangle_RT<T>{vector_from_string<T>(*(it_tr + indexes[temp[0]])),
                           vector_from_string<T>(*(it_tr + indexes[temp[1]])),
                           vector_from_string<T>(*(it_tr + indexes[temp[2]]))
                          }, temp) ;
           // (triangle_RT<T>{vertices[0], vertices[1], vertices[2]}, std::move(temp)) ;
} // triangle_from_indexes()

#endif // _HELPER_FUNCTIONS


_DEFS_JSON_PARSER_
#endif
// eof json_Parser.h