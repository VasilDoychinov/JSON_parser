# PROJECT: a prototype (ver. 0.1) of a JSON format parser
#    Copyright: the project is meant for illustration purposes only.
#               Vasil Doychinov carries no responsibility for any
#               misuse of the presented contents
.
#    Goals: - data to be stored in User defined containers that 
#             provide certain "interface". The containers are to
#             be parameters to the Parser
#           - access (read only, to start with) to be based on
#             iterators (forward to start with)
#
#    Solution: cl_jsonDescr<> template parameterized with JCont
#              see ./src/json_Parser.h
#
#    Platform: C++, std: 17, 
#
#    Consists of:
#     ./       : test_Parser.cpp - a sample program to test and 
#                illustrate the idea
#     ./src/   : source files as follows
#                - json_Parser.h, .hpp: major definitions, etc
#                - json_Iterators.h, .cpp: the iterator support
#                - json_Params.h: constants, etc
#     ./file/  : example of using a binary file as a container
#                - json_File.h, .cpp: 
#     ./string/: example of using std::string as a container
#                - json_String.h, .cpp
#     ./data/  : sample data used for testing
#
#   Usage: - integrate ./src/*.* in your build system
#          - initiate the template with the desired container.
#            The Container must provide the required interface.
#            (for example see json_File.*, json_String.*, 
#            test_Parser.c
#
#   for more details look at the source provided or 
#                    email doychinov.vasil@gmail.com
#------------------------------------------------------------------