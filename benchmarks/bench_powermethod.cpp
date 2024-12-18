#include <Eigen/sparse>
#include <catch2/catch_session.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/benchmark/catch_benchmark.hpp>

#include "RandomizedSVD.h"
#include "PowerMethodSVD.h"

#include <vector>
#include <string>
#include <random>

// Function to generate sparse matrices
std::vector<Eigen::SparseMatrix<double, Eigen::RowMajor>> generateSparseMatrices(const int startSize, const int endSize,
                                                                                 const int step, const double sparsity) {
    std::vector<Eigen::SparseMatrix<double, Eigen::RowMajor>> sparseMatrices;
    sparseMatrices.reserve((endSize - startSize) / step + 1);

    std::mt19937 gen(42);
    std::uniform_real_distribution dis(0.0, 1.0);

    for (int size = startSize; size <= endSize; size += step) {
        Eigen::SparseMatrix<double, Eigen::RowMajor> sparse(size, size);
        std::vector<Eigen::Triplet<double>> tripletList;
        tripletList.reserve(static_cast<int>(size * size * sparsity));

        for (int i = 0; i < size; ++i) {
            for (int j = 0; j < size; ++j) {
                if (dis(gen) < sparsity) {
                    tripletList.emplace_back(i, j, dis(gen));
                }
            }
        }
        sparse.setFromTriplets(tripletList.begin(), tripletList.end());
        sparseMatrices.emplace_back(std::move(sparse));
    }
    return sparseMatrices;
}

// Global constants for matrix generation
constexpr int startSize = 100;
constexpr int endSize = 1000;
constexpr int stepSize = 100;
constexpr double sparsity = 0.1;

// Generate matrices once to avoid recreating in each test case
std::vector<Eigen::SparseMatrix<double, Eigen::RowMajor>> sparseMatrices = generateSparseMatrices(startSize, endSize, stepSize, sparsity);

// Randomized SVD Benchmark Test
TEST_CASE("Eigen RandomizedSVD Benchmark", "[randomizedSVD_bench_dynamic]") {
    for (int idx = 0; idx < sparseMatrices.size(); ++idx) {
        const int size = startSize + idx * stepSize;
        const auto& sparse = sparseMatrices[idx];
        int rank = 10;

        BENCHMARK("RandomizedSVD with sparse matrix size " + std::to_string(size)) {
            Eigen::RandomizedSVD<Eigen::SparseMatrix<double, Eigen::RowMajor>> rsvd;
            rsvd.compute(sparse, rank, 5);
        };
    }
}

// Power Method SVD Benchmark Test
TEST_CASE("Eigen PowerMethod Benchmark", "[powermethod_bench_dynamic]") {
    for (int idx = 0; idx < sparseMatrices.size(); ++idx) {
        const int size = startSize + idx * stepSize;
        const auto& sparse = sparseMatrices[idx];
        int rank = 1;

        BENCHMARK("PowerMethodSVD with sparse matrix size " + std::to_string(size)) {
            Eigen::PowerMethodSVD<Eigen::SparseMatrix<double, Eigen::RowMajor>> pmethod;
            pmethod.compute(sparse, rank);
        };
    }
}

// Main function to configure and run tests
int main(int argc, char* argv[]) {
    Catch::Session session;

    // Add XML reporter option to command-line arguments
    const char* xmlReporterArgs[] = {"--reporter", "xml", "--out", "powermethod.xml"};
    int xmlReporterArgc = sizeof(xmlReporterArgs) / sizeof(char*);

    // Combine user-provided args and XML reporter args
    std::vector<const char*> combinedArgs(argc + xmlReporterArgc);
    for (int i = 0; i < argc; ++i) {
        combinedArgs[i] = argv[i];
    }
    for (int i = 0; i < xmlReporterArgc; ++i) {
        combinedArgs[argc + i] = xmlReporterArgs[i];
    }

    // Run Catch2 session
    return session.run(static_cast<int>(combinedArgs.size()), combinedArgs.data());
}