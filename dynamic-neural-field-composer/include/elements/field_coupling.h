#pragma once

#include <set>

#include "tools/math.h"
#include "element.h"
#include "neural_field.h"
#include "tools/utils.h"


namespace dnf_composer
{
	enum class LearningRule : int
	{
		HEBB,
		OJA,
		DELTA
	};

	inline const std::map<LearningRule, std::string> LearningRuleToString = {
		{LearningRule::HEBB, "Hebb"},
		{LearningRule::OJA, "Oja"},
		{LearningRule::DELTA, "Delta"}
	};

	namespace element
	{
		struct FieldCouplingParameters : ElementSpecificParameters
		{
			ElementDimensions inputFieldDimensions;
			LearningRule learningRule;
			double scalar;
			double learningRate;
			bool isLearningActive;

			FieldCouplingParameters(const ElementDimensions& inputFieldDimensions = ElementDimensions{},
				LearningRule learningRule = LearningRule::HEBB,
				double scalar = 1.0, double learningRate = 0.01)
					: inputFieldDimensions(inputFieldDimensions),
				learningRule(learningRule), scalar(scalar),
				learningRate(learningRate), isLearningActive(false)
			{}

			bool operator==(const FieldCouplingParameters& other) const
			{
				constexpr double epsilon = 1e-6;

				return std::abs(inputFieldDimensions.x_max - other.inputFieldDimensions.x_max) < epsilon &&
					std::abs(inputFieldDimensions.d_x - other.inputFieldDimensions.d_x) < epsilon &&
					learningRule == other.learningRule &&
					std::abs(scalar - other.scalar) < epsilon &&
					std::abs(learningRate - other.learningRate) < epsilon;
			}

			std::string toString() const override
			{
				std::ostringstream result;
				result << std::fixed << std::setprecision(2);
				result << "Parameters: ["
					<< "Input field dimensions: " << inputFieldDimensions.toString() << ", "
					<< "Learning rule: " << LearningRuleToString.at(learningRule) << ", "
					<< "Learning rate: " << learningRate << ", "
					<< "Scalar: " << scalar
					<< "]";
				return result.str();
			}
		};

		class FieldCoupling : public Element
		{
		protected:
			FieldCouplingParameters parameters;
			std::shared_ptr<Element> input;
			std::shared_ptr<Element> output;
			std::string weightsDirectory;
		public:
			FieldCoupling(const ElementCommonParameters& elementCommonParameters, 
				const FieldCouplingParameters& fc_parameters);

			void init() override;
			void step(double t, double deltaT) override;
			std::string toString() const override;
			std::shared_ptr<Element> clone() const override;

			void setLearningRate(double learningRate);
			void setLearning(bool learning);
			void setParameters(const FieldCouplingParameters& fcp);
			void setWeightsDirectory(const std::string& dir);
			FieldCouplingParameters getParameters() const;
			std::string getWeightsDirectory() const;

			void readWeights();
			void writeWeights() const;
			void clearWeights();
		private:
			void updateOutput();
			void updateInputField();
			void updateOutputField();
			void updateWeights();
			bool checkValidConnections();
		};
	}
}
