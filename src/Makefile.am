AUTOMAKE_OPTIONS=foreign
bin_PROGRAMS=myandb mdb testc
myandb_SOURCES=\
      pmd/pmdMain.cpp os/osSocket.cpp utils/logger.cpp pmd/pmdAgent.cpp msg/msg.cpp \
      bson/src/bsonobj.cpp bson/src/util/json.cpp bson/src/oid.cpp \
      bson/src/lib/base64.cpp bson/src/lib/md5.cpp bson/src/lib/nonce.cpp \
      pmd/pmdTcpListener.cpp utils/threadPool.cpp os/osMmapFile.cpp os/osPrimitiveFileOp.cpp \
      dms/dmsFile.cpp

mdb_SOURCES=\
      os/osSocket.cpp utils/logger.cpp \
      client/command.cpp client/commandFactory.cpp client/mydb.cpp msg/msg.cpp \
      bson/src/bsonobj.cpp bson/src/util/json.cpp bson/src/oid.cpp \
      bson/src/lib/base64.cpp bson/src/lib/md5.cpp bson/src/lib/nonce.cpp \
      utils/threadPool.cpp

testc_SOURCES=\
      utils/logger.cpp \
      utils/threadPool.cpp test/test.cpp

myandb_CXXFLAGS=-I../../emeralddb/boost -I./bson/src -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
myandb_LDADD=-lpthread -lm -lboost_system -lboost_thread -lboost_program_options -lrt
myandb_LDFLAGS=-fPIC -rdynamic -L../../emeralddb/boost/stage/lib -pthread


mdb_CXXFLAGS=-I../../emeralddb/boost -I./bson/src -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
mdb_LDADD=-lpthread -lm -lboost_system -lboost_thread -lboost_program_options -lrt
mdb_LDFLAGS=-fPIC -rdynamic -L../../emeralddb/boost/stage/lib -pthread


testc_CXXFLAGS=-I../../emeralddb/boost -I./bson/src -Iinclude -D_FILE_OFFSET_BITS=64 -ggdb -Wall -O0
testc_LDADD=-lpthread -lm -lboost_system -lboost_thread -lboost_program_options -lrt
testc_LDFLAGS=-fPIC -rdynamic -L../../emeralddb/boost/stage/lib -pthread
