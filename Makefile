  # Compiler and flags
CXX = mpicxx
CXXFLAGS = -fopenmp -O3

# Ensure the correct compiler is used by OpenMPI
OMPI_CXX = g++
export OMPI_CXX

# Source files
SRCS = generate_TTP.cpp parallel_TTP.cpp TTP_helpers.cpp
SRCS2 = sample_TTP.cpp TTP_helpers.cpp

# Output executable
TARGET = run
TARGET2 = sampler

# Build target
$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS)

# Clean target
clean:
	rm -f $(TARGET) $(TARGET2)

# Build target for sampler
$(TARGET2): $(SRCS2)
	$(CXX) $(CXXFLAGS) -o $(TARGET2) $(SRCS2)

sample: $(TARGET2)