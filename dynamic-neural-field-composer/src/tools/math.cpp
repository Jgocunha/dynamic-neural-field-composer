#include "tools/math.h"


namespace dnf_composer
{
	namespace tools
	{
		namespace math
		{
			std::array<int, 2> computeKernelRange(double sigma, int cutOfFactor, int fieldSize, bool circular)
			{
				const int ceilSigmaCutOfFactor = static_cast<int>(std::ceil(sigma * cutOfFactor));
				const int fieldSizeMinus1 = (fieldSize - 1);

				if (circular)
				{
					const double halfFieldSizeMinus1 = (static_cast<double>(fieldSize) - 1) / 2;
					const int floorHalfFieldSizeMinus1 = static_cast<int>(std::floor(halfFieldSizeMinus1));
					const int ceilHalfFieldSizeMinus1 = static_cast<int>(std::ceil(halfFieldSizeMinus1));

					return std::min(std::array<int, 2>{ceilSigmaCutOfFactor, ceilSigmaCutOfFactor},
						std::array<int, 2>{floorHalfFieldSizeMinus1, ceilHalfFieldSizeMinus1});
				}

				return std::min(std::array<int, 2>{ceilSigmaCutOfFactor, ceilSigmaCutOfFactor},
					std::array<int, 2>{fieldSizeMinus1, fieldSizeMinus1});
			}

			std::vector<int> createExtendedIndex(int fieldSize, const std::array<int, 2>& kernelRange)
			{
				const int startingValue = fieldSize - kernelRange[1] + 1;
				const int initialVectorSize = fieldSize - startingValue + 1;

				std::vector<int> initialVector(initialVectorSize);
				std::iota(initialVector.begin(), initialVector.end(), startingValue);

				std::vector<int> secondVector(fieldSize);
				std::iota(secondVector.begin(), secondVector.end(), 1);

				std::vector<int> thirdVector(kernelRange[0]);
				std::iota(thirdVector.begin(), thirdVector.end(), 1);

				std::vector<int> extendedVector;
				extendedVector.reserve(initialVector.size() + secondVector.size() + thirdVector.size());

				extendedVector.insert(extendedVector.end(), initialVector.begin(), initialVector.end());
				extendedVector.insert(extendedVector.end(), secondVector.begin(), secondVector.end());
				extendedVector.insert(extendedVector.end(), thirdVector.begin(), thirdVector.end());

				return extendedVector;
			}

			std::vector<double> generateNormalVector(int size)
			{
				std::random_device rd;
				std::mt19937 gen(rd());
				std::normal_distribution<> dist(0, 1);

				std::vector<double> vec(size);
				for (int i = 0; i < size; ++i)
					vec[i] = dist(gen);

				return vec;
			}
		}
	}
}