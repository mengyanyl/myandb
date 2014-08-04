AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=myandb
myandb_SOURCES=\
      pmd/pmdMain.cpp os/osSocket.cpp utils/logger.cpp

myandb_CXXFLAGS=-I../../emeralddb/boost -I../../emeralddb/bson/src -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
myandb_LDADD=-lpthread -lm -lboost_system -lboost_thread -lboost_program_options -lrt 
myandb_LDFLAGS=-fPIC -rdynamic -L../../emeralddb/boost/stage/lib -pthread
