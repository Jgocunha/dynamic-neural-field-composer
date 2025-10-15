#pragma once

//https://github.com/stevenlovegrove/Pangolin/issues/352
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#include <vector>
#include <array>
#include <cmath>
#include <math.h>
#include <algorithm>
#include <numeric>
#include <functional>
#include <random>
#include <numbers> // std::numbers
#include <fstream>


namespace dnf_composer::tools::math
{
	// https://stackoverflow.com/questions/24518989/how-to-perform-1-dimensional-valid-convolution
	// The conv function implements the standard convolution method,
	// where the output size is the sum of input sizes minus one.
	template<typename T>
	std::vector<T> conv(std::vector<T> const& f, std::vector<T> const& g)
	{
		int const nf = f.size();
		int const ng = g.size();
		int const n = nf + ng - 1;
		std::vector<T> out(n, T());
		for (auto i(0); i < n; ++i) {
			int const jmn = (i >= ng - 1) ? i - (ng - 1) : 0;
			int const jmx = (i < nf - 1) ? i : nf - 1;
			for (auto j(jmn); j <= jmx; ++j) {
				out[i] += (f[j] * g[i - j]);
			}
		}
		return out;
	}

	// conv performs standard convolution, while conv_valid performs valid convolution,
	// which is generally more efficient for some applications where zero-padded areas
	// are not desired in the output.

	// https://stackoverflow.com/questions/24518989/how-to-perform-1-dimensional-valid-convolution
	// The conv_valid function implements the valid convolution method,
	// where the output size is the absolute difference between the input sizes plus one.
	template<typename T>
	std::vector<T> conv_valid(std::vector<T> const& f, std::vector<T> const& g)
	{
		int const nf = f.size();
		int const ng = g.size();
		std::vector<T> const& min_v = (nf < ng) ? f : g;
		std::vector<T> const& max_v = (nf < ng) ? g : f;
		int const n = std::max(nf, ng) - std::min(nf, ng) + 1;
		std::vector<T> out(n, T());
		for (auto i(0); i < n; ++i) {
			for (int j(min_v.size() - 1), k(i); j >= 0; --j) {
				out[i] += min_v[j] * max_v[k];
				++k;
			}
		}
		return out;
	}

	// ChatGPT 4.0
	template<typename T>
	std::vector<T> conv_same(std::vector<T> const& f, std::vector<T> const& g) {
		int const nf = f.size();
		int const ng = g.size();
		std::vector<T> out(nf, T()); // Output size matches the size of f for 'same' mode

		// Calculate padding needed for 'same' mode, adjusted for shift correction
		const int pad = (ng - 1) / 2;

		for (int i = 0; i < nf; ++i) {
			T sum = T(); // Initialize sum for each element of output
			for (int j = 0; j < ng; ++j) {
				int fIndex = i + j - pad; // Adjust index for 'same' mode with shift correction
				if (fIndex >= 0 && fIndex < nf) { // Check boundaries
					sum += f[fIndex] * g[j];
				}
			}
			out[i] = sum;
		}
		return out;
	}

	template<typename T>
	std::vector<T> gaussNorm(const std::vector<int>& rangeX, const T& position, const T& sigma)
	{
		std::vector<T> g(rangeX.size());
		for (int i = 0; i < g.size(); i++)
			g[i] = exp(-0.5 * pow((rangeX[i] - position), 2) / pow(sigma, 2));

		if (!g.empty())
		{
			double sumOfG = std::reduce(g.begin(), g.end());
			for (int i = 0; i < g.size(); i++)
				g[i] = g[i] / sumOfG;
		}

		return g;
	}

	template<typename T>
	std::vector<T> gauss(const std::vector<int>& rangeX, const T& position, const T& sigma)
	{
		std::vector<T> g(rangeX.size());

		for (int i = 0; i < g.size(); i++)
			g[i] = exp(-0.5 * pow((rangeX[i] - position), 2) / pow(sigma, 2));

		return g;
	}

	template<typename T>
	std::vector<T> gauss(int size, const T& sigma, const T& position)
	{
		std::vector<T> g(size);

		for (int i = 0; i < g.size(); i++)
		{
			T x = static_cast<T>(i + 1);
			g[i] = exp(-0.5 * pow((x - position), 2) / pow(sigma, 2));
		}

		return g;
	}

	template<typename T>
	std::vector<T> nonCircularGauss(uint32_t size, const T& sigma, const T& position)
	{
		std::vector<T> g(size);
		std::vector<T> xRange(size);
		std::iota(xRange.begin(), xRange.end(), static_cast<T>(1));

		for (uint32_t i = 0; i < size; i++)
		{
			T distance = static_cast<T>(i + 1) - position;
			g[i] = std::exp(-0.5 * std::pow(distance, 2) / std::pow(sigma, 2));
		}

		return g;
	}

	template<typename T>
	std::vector<T> circularGauss(uint32_t size, const T& sigma, const T& position)
	{
		uint32_t l = size - 2 * 1 + 2;
		uint32_t m = 1;
		T r = position - static_cast<T>(m); // Calculate the shift with fractional part
		T rem = std::fmod(r, static_cast<T>(l)); // Calculate the remainder
		T positionShifted = rem + static_cast<T>(m); // Apply the shifted position

		std::vector<T> g(size);
		std::vector<T> xRange(size);
		std::iota(xRange.begin(), xRange.end(), static_cast<T>(1));

		std::vector<T> d(size);
		std::vector<T> lMinusd(size);
		std::transform(xRange.begin(), xRange.end(), d.begin(), [&positionShifted, &l](T element) { return std::abs(element - positionShifted); });
		std::transform(d.begin(), d.end(), lMinusd.begin(), [&l](T element) { return -1 * (element - l); });
		for (int i = 0; i < size; i++)
		{
			g[i] = std::exp(-0.5 * std::pow(std::min(d[i], lMinusd[i]), 2) / std::pow(sigma, 2));
		}

		return g;
	}

	template<typename T>
	std::vector<T> gaussDerivative(const std::vector<int>& rangeX, const T& position, const T& sigma, const T& amplitude)
	{
		std::vector<T> derivative(rangeX.size());
		T variance = sigma * sigma;

		for (size_t i = 0; i < derivative.size(); i++)
		{
			T x = rangeX[i];
			derivative[i] = -(x - position) / variance * amplitude * std::exp(-0.5 * std::pow((x - position), 2) / variance);
		}

		return derivative;
	}

	template<typename T>
	std::vector<T> gaussDerivativeNorm(const std::vector<int>& rangeX, const T& position, const T& sigma, const T& amplitude)
	{
		auto derivative = gaussDerivative(rangeX, position, sigma, amplitude);

		// Normalize by the sum of absolute values to preserve sign
		T sumOfAbs = std::accumulate(derivative.begin(), derivative.end(), static_cast<T>(0),
		                             [](T sum, T value) { return sum + std::abs(value); });

		if (sumOfAbs > static_cast<T>(0))
		{
			for (T& value : derivative)
				value /= sumOfAbs;
		}

		return derivative;
	}

	template<typename T>
	std::vector<T> obtainCircularVector(const std::vector<int>& indices, const std::vector<T>& contents)
	{
		std::vector<T> newContents(indices.size());
		for (int i = 0; i < indices.size(); i++)
			newContents[i] = contents[indices[i] - 1];
		return newContents;
	}

	template <typename T>
	std::vector<T> sigmoid(const std::vector<T>& x, T beta, T x0)
	{
		std::vector<T> s(x.size());
		for (std::size_t i = 0; i < s.size(); ++i) {
			s[i] = 1 / (1 + std::exp(-beta * (x[i] - x0)));
		}
		return s;
	}

	template<typename T>
	std::vector<T> heaviside(const std::vector<T>& x, T threshold)
	{
		std::vector<T> h(x.size());
		for (int i = 0; i < static_cast<int>(h.size()); i++)
			h[i] = (x[i] > threshold) ? 1 : 0;
		return h;
	}

	template<typename T>
	std::vector<T> sumGauss(const std::vector<T>& gauss1, const std::vector<T>& gauss2)
	{
		std::vector<T> gaussResult(gauss1.size());
		for (int i = 0; i < gauss1.size(); i++)
			gaussResult[i] = gauss1[i] + gauss2[i];
		return gaussResult;
	}

	std::array<int, 2> computeKernelRange(double sigma, int cutOfFactor, int fieldSize, bool circular);
	std::vector<int> createExtendedIndex(int fieldSize, const std::array<int, 2>& kernelRange);

	std::vector<double> generateNormalVector(int size);

	template <typename T>
	std::vector<T> normalize(const std::vector<T>& vector)
	{
		static constexpr T epsilon = 1e-6;
		static constexpr T offset = 1.0;

		// Find the minimum and maximum values in the vector
		const T minVal = *std::ranges::min_element(vector.begin(), vector.end()) - epsilon;

		std::vector<T> normalizedVector = vector;
		for (T& val : normalizedVector)
			val += minVal;

		normalizedVector = math::sigmoid(normalizedVector, 0.2, std::abs(minVal) + offset);
		const T newMinVal = *std::ranges::min_element(normalizedVector.begin(), normalizedVector.end());

		for (T& val : normalizedVector)
			val -= newMinVal;

		return normalizedVector;
	}

	template <typename T>
	std::vector<T> hebbLearningRule(std::vector<T>& weights, const std::vector<T>& input, const std::vector<T>& output, double learningRate)
	{
		if (input.empty() || output.empty())
			throw std::invalid_argument("Input and output vectors cannot be empty");

		const size_t inputSize = input.size();
		const size_t outputSize = output.size();
		const size_t expectedSize = inputSize * outputSize;

		if (weights.size() != expectedSize)
			throw std::invalid_argument("Weight matrix size mismatch");

		for (size_t i = 0; i < inputSize; ++i)
		{
			const T scaledInput = learningRate * input[i];
			const size_t baseIndex = i * outputSize;

			for (size_t j = 0; j < outputSize; ++j)
				weights[baseIndex + j] += scaledInput * output[j];
		}

		return weights;
	}

	template <typename T>
	std::vector<T> ojaLearningRule(std::vector<T>& weights, const std::vector<T>& input, const std::vector<T>& output, double learningRate)
	{
		const int inputSize = input.size();
		const int outputSize = output.size();

		for (int i = 0; i < inputSize; i++)
			for (int j = 0; j < outputSize; j++)
			{
				int index = i * outputSize + j; // Compute the index for the flattened matrix
				weights[index] += learningRate * (input[i] * output[j] - output[j] * input[i] * weights[index]);
			}

		return weights;
	}

	template <typename T>
	std::vector<std::vector<T>> deltaLearningRuleWidrowHoff(std::vector<std::vector<T>>& weights, const std::vector<T>& input,
	                                                        const std::vector<T>& actualOutput, const std::vector<T>& targetOutput, double learningRate)
	{
		const int inputSize = input.size();
		const int outputSize = targetOutput.size();

		// Calculate the error between the target output and the actual output
		std::vector<T> error(outputSize, 0.0);
		for (size_t j = 0; j < outputSize; ++j) {
			error[j] = targetOutput[j] - actualOutput[j];
		}

		// Update the weights based on the error and current activation levels of the fields
		for (size_t i = 0; i < inputSize; ++i) {
			for (size_t j = 0; j < outputSize; ++j) {
				weights[i][j] += learningRate * error[j] * input[i];
			}
		}

		return weights;
	}

	template <typename T>
	std::vector<std::vector<T>> deltaLearningRuleKroghHertz(std::vector<std::vector<T>>& weights, const std::vector<T>& input,
	                                                        const std::vector<T>& targetOutput, const std::vector<T>& actualOutput,
	                                                        double learningRate)
	{
		const int inputSize = input.size();
		int outputSize = targetOutput.size();

		//// Calculate the activation levels of the fields based on the input values and current weights
		//std::vector<T> actualOutput(outputSize, 0.0);
		//for (size_t j = 0; j < outputSize; ++j) {
		//	for (size_t i = 0; i < inputSize; ++i) {
		//		actualOutput[j] += input[i] * weights[i][j];
		//	}
		//}

		// Calculate the error between the target output and the actual output
		std::vector<T> error(outputSize, 0.0);
		for (size_t j = 0; j < outputSize; ++j) {
			error[j] = targetOutput[j] - actualOutput[j];
		}

		// Update the weights based on the error and current activation levels of the fields
		for (size_t i = 0; i < inputSize; ++i) {
			for (size_t j = 0; j < outputSize; ++j) {
				//weights[i][j] += learningRate * (error[j] - learningRate * weights[i][j]) * input[i]; // old
				weights[i][j] += learningRate * (error[j]) * input[i];
			}
		}

		return weights;
	}

	template <typename T>
	bool compareVectors(const std::vector<T>& vec1, const std::vector<T>& vec2, T threshold) {
		if (vec1.size() != vec2.size()) {
			return false; // Vectors are of different sizes, hence not equal
		}

		for (size_t i = 0; i < vec1.size(); ++i) {
			if (std::abs(vec1[i] - vec2[i]) > threshold) {
				return false; // Difference between elements at index i exceeds threshold
			}
		}

		return true; // Vectors are equal within threshold
	}

	template <typename T>
	T calculateVectorSum(const std::vector<T>& vec) {
		T result = 0.0;
		for (T value : vec) {
			result += value;
		}
		return result;
	}

	template <typename T>
	T calculateVectorAvg(const std::vector<T>& vec) {
		if (vec.empty()) {
			return T(); // Return default value if vector is empty
		}

		T sum = T();
		for (const T& value : vec) {
			sum += value;
		}
		return sum / static_cast<T>(vec.size()); // Calculate average
	}

	template <typename T>
	T calculateVectorNorm(const std::vector<T>& vec) {
		T sum_of_squares = std::accumulate(vec.begin(), vec.end(), 0.0,
		                                   [](T a, T b) { return a + b * b; });
		return std::sqrt(sum_of_squares);
	}

	inline double normalize(const double value, const double min, const double max)
	{
		if (value < min) return 0.0;
		if (value > max) return 1.0;
		return (value - min) / (max - min);
	}

	inline double gaussian_2d(double x, double y, double mu_x, double mu_y, double sigma_x, double sigma_y, double A) {
		const double exponent = -((std::pow(x - mu_x, 2) / (2 * std::pow(sigma_x, 2))) + (std::pow(y - mu_y, 2) / (2 * std::pow(sigma_y, 2))));
		return A * std::exp(exponent);
	}

	inline double circular_gaussian_2d(double x, double y, double mu_x, double mu_y, double sigma, double A) {
		const double exponent = -((std::pow(x - mu_x, 2) + std::pow(y - mu_y, 2)) / (2 * std::pow(sigma, 2)));
		return A * std::exp(exponent);
	}

	inline double wrap(double value, double max_value) {
		if (value < 0) return value + max_value;
		if (value >= max_value) return value - max_value;
		return value;
	}

	inline double gaussian_2d_periodic(double x, double y, double mu_x, double mu_y, double sigma, double A, double max_x, double max_y) {
		const double xStep = std::min(std::abs(x - mu_x), max_x - std::abs(x - mu_x));
		const double dy = std::min(std::abs(y - mu_y), max_y - std::abs(y - mu_y));
		const double exponent = -((std::pow(xStep, 2) + std::pow(dy, 2)) / (2 * std::pow(sigma, 2)));
		return A * std::exp(exponent);
	}

	template <typename T>
	std::vector<T> flattenMatrix(const std::vector<std::vector<T>>& matrix)
	{
		const int rows = matrix.size();
		const int cols = matrix[0].size();
		std::vector<T> flat_matrix(rows * cols);
		for (int i = 0; i < rows; ++i)
			for (int j = 0; j < cols; ++j)
				flat_matrix[i * cols + j] = static_cast<T>(matrix[i][j]);
		return flat_matrix;
	}
}
