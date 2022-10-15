// test_Parser.cpp
//

#include <iostream>
using std::cout ;
using std::endl ;
 
#include <stdio.h>          // I/O support
#include <io.h>
#include <fcntl.h>
#include <share.h>

#include <utility>

#include "ownCIN.h"
#include "ownMenu.h"
#include "miscellaneous.h"

#include "src/json_String.h"
#include "src/json_Parser.h"

std::string load_file(std::string& fname)		// <fname> into a string
{
	int   fh ;
	std::string		tmp{} ;

	cout << endl << ": to open: " << fname ;
	errno_t err = _sopen_s(&fh, fname.c_str(), _O_BINARY | _O_RDONLY,
						   _SH_DENYRW, _S_IREAD | _S_IWRITE) ;
	if (err != 0)   return tmp ;

	const auto      bsize{sizeof(decltype(tmp)::value_type)} ; // just a practice

	auto	addr_eof{_lseek(fh, 0L, SEEK_END)} ;
	if (addr_eof <= 0L)   return tmp ;
	tmp.resize(static_cast<size_t>(addr_eof)) ;
	cout << " of length: " << addr_eof << " -> buffer size: " << tmp.size() << " of " << bsize << endl ;

	_lseek(fh, 0L, SEEK_SET) ;
	for (long addr = 0L ; addr < addr_eof ; addr += bsize) {
		if (_read(fh, &(tmp[addr]), bsize) != bsize) {
			tmp = decltype(tmp){} ; 
			break ;	// error
		}
	}
	if (fh != -1)	_close(fh) ;

	return tmp ;
} // load_file()

int
main()
{
	cout << endl << endl ;

	CMenu			 menu(string("> JSON "),
						  string("   (1) Activate   Show (2) Section   (3) All sections   (4) Data   (5) Deactivate") +
						  string("\n          E(x)it  ")) ;
	constexpr size_t chEx = 5 ;
	size_t			 mCh = 0 ;
	inp_str<80>		 txt ;
	auto		     fname{txt("> enter file name: ")} ;

	fname = "./data/" + fname + ".json";

	cl_jsonDescr<cl_jsonString>	jd{fname, load_file} ; // std::move(load_file)} ;
	cout << endl << ": loaded in string:\n<" << jd << "\n>" ;

	try {

		do {
			if (jd.invalid())	cout << endl << ": " << fname << " NOT active." ;
			cout << endl << endl << menu, mCh = menu.choice() ;

			if (mCh == 0) {
				if (jd.invalid())	cout << endl << ": " << fname << " active: " << std::boolalpha << jd.activate() ;
				else				jd.deactivate() ;
			} else if (mCh == 1) { 
				if (!jd.invalid()) {
					auto s_name{txt("> enter Section name(CR for Top): ")} ;
					cout << endl << endl, jd.show_section(cout, s_name) ;
				}
			} else if (mCh == 2) {
				cout << endl << endl, jd.show_all_sections(cout) ;
			} else if (mCh == 3) {
				if (!jd.invalid()) {
					auto s_name{txt("> enter Section name(CR for Top): ")} ;
					cout << endl << endl, jd.show_section(cout, s_name) << endl << endl ;

					auto d_name{txt("> enter Data Field name: ")} ;
					auto d_it{jd.begin_data(s_name, d_name, std::stoul(txt("> enter step size: ")))} ;
					if (d_it.is_valid()) {
						cout << endl << ": " << d_it ;
						cout << " : d_it != limit(): " << std::boolalpha << (d_it != d_it.limit()) << endl ;
						for (size_t i = 0 ; d_it != d_it.limit() ; i++, ++d_it) {
							cout << ": read: " << i << "\r" ;
#ifdef VERBOSE
							cout << endl << "___ value " << i << ": " << *d_it << "___ again: " << *d_it ;
#endif
						}
					} else {
						cout << endl << ": missing scope or data section" ;
					}
				}
			} else if (mCh == 4)	jd.deactivate() ;

			cout << endl ;
		} while (mCh != chEx) ;
	} catch (std::exception& e) {
		std::cerr << endl << endl
			<< endl << "___ exception Caught: " << e.what()
			<< endl << "___ Type            : " << typeid(e).name() << endl ;
	} catch (...) {
		std::cerr << endl << endl
			<< endl << "___ Unknown exception Caught: " << endl ;
	}

	cout << endl << endl << "--- That's it ..." ;
	return 0 ;
} // main()



// eof test_Parser.cpp