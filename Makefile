# This has to be a path to a checked-out, configured and _built_ stellar-core
# source tree.

# Use $(HOME)/src/stellar-core
# as the default locations of stellar-core and ivy.
# If you use different locations, use `env` to overwrite it.
# e.g., `env CORE_DIR=~/stellar-core IVY_DIR=~/ivy make executable`.
CORE_DIR?=~/desktop/BlockchainFall2023/StellarCoreAlgorandLibsodium/stellar-core

CORE_OBJS=$(CORE_DIR)/src/crypto/BLAKE2.o \
          $(CORE_DIR)/src/crypto/Hex.o \
          $(CORE_DIR)/src/crypto/KeyUtils.o \
          $(CORE_DIR)/src/crypto/Random.o \
          $(CORE_DIR)/src/crypto/SecretKey.o \
          $(CORE_DIR)/src/crypto/ShortHash.o \
          $(CORE_DIR)/src/crypto/SHA.o \
          $(CORE_DIR)/src/crypto/StrKey.o \
          $(CORE_DIR)/src/scp/BallotProtocol.o \
          $(CORE_DIR)/src/scp/LocalNode.o \
          $(CORE_DIR)/src/scp/NominationProtocol.o \
          $(CORE_DIR)/src/scp/QuorumSetUtils.o \
          $(CORE_DIR)/src/scp/SCP.o \
          $(CORE_DIR)/src/scp/SCPDriver.o \
          $(CORE_DIR)/src/scp/Slot.o \
          $(CORE_DIR)/src/util/Backtrace.o \
          $(CORE_DIR)/src/util/GlobalChecks.o \
          $(CORE_DIR)/src/util/HashOfHash.o \
          $(CORE_DIR)/src/util/Logging.o \
          $(CORE_DIR)/src/util/Math.o \
          $(CORE_DIR)/src/util/RandHasher.o \
          $(CORE_DIR)/src/util/ProtocolVersion.o \
          $(CORE_DIR)/src/util/Scheduler.o \
          $(CORE_DIR)/src/util/Timer.o \
          $(CORE_DIR)/src/util/numeric.o \
          $(CORE_DIR)/src/util/types.o

CORE_LIBDIRS=-L $(CORE_DIR)/lib \
             -L $(CORE_DIR)/lib/xdrpp/xdrpp/ \
             -L $(CORE_DIR)/lib/libsodium/src/libsodium/.libs

CORE_INCLUDES=-I $(CORE_DIR) \
              -I $(CORE_DIR)/src/ \
              -I $(CORE_DIR)/src/protocol-curr/ \
              -I $(CORE_DIR)/lib/xdrpp \
              -I $(CORE_DIR)/lib/fmt/include \
              -I $(CORE_DIR)/lib/libsodium/src/libsodium/include \
              -I $(CORE_DIR)/lib/spdlog/include

# Define compiler flags
CXXFLAGS := -O2 -g -std=c++17 -pthread -Wall $(CORE_INCLUDES)

# Define linker flags
LDFLAGS := $(CORE_LIBDIRS) -lpthread -lsodium -l3rdparty -lxdrpp #-lz3

# Define target executable
TARGET := main.exe

# Define source files
SRCS := main.cpp parser.cpp

# Define header files
HDRS := main.h parser.h

# Define build targets
all: $(TARGET)

$(TARGET): $(CORE_OBJS) main.o parser.o
	@echo "\n\ndon't forget to build stellar core with --disable-tests\n\n"
	clang++ -g -o $@ $^ $(LDFLAGS)

main.o: main.cpp $(HDRS) Makefile
	clang++ -c $(CXXFLAGS) -o $@ $<

parser.o: parser.cpp parser.h Makefile
	clang++ -c $(CXXFLAGS) -o $@ $<

# Define cleanup target
clean:
	rm main.o parser.o main.exe


.PHONY: clean
