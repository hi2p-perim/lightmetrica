/*
	Lightmetrica : A research-oriented renderer

	Copyright (c) 2014 Hisanari Otsu

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <lightmetrica/pssmlt.sampler.h>
#include <lightmetrica/rewindablesampler.h>
#include <lightmetrica/random.h>

LM_NAMESPACE_BEGIN

struct PSSMLTPrimarySample
{
	PSSMLTPrimarySample(const Math::Float& value)
		: value(value)
		, modify(0)
	{}

	Math::Float value;		//!< Sample value
	long long modify;		//!< Last modified time
};

class PSSMLTPrimarySamplerImpl : public PSSMLTPrimarySampler
{
public:

	LM_COMPONENT_IMPL_DEF("default");

public:

	virtual Sampler* Clone()
	{
		LM_LOG_ERROR("Invalid operator for PSSMLTPrimarySampler");
		return nullptr;
	}

	virtual void SetSeed( unsigned int seed )
	{
		time = 0;
		largeStepTime = 0;
		enableLargeStep = false;
		currentIndex = 0;
		rng->SetSeed(seed);
	}

	virtual Math::Float Next()
	{
		return PrimarySample(currentIndex++);
	}

	virtual unsigned int NextUInt()
	{
		LM_LOG_ERROR("Invalid operator for PSSMLTPrimarySampler");
		return 0;
	}

	virtual Math::Vec2 NextVec2()
	{
		return Math::Vec2(Next(), Next());
	}

	virtual Random* Rng()
	{
		return managedRng.get();
	}

public:

	virtual void Configure( Random* rng, const Math::Float& s1, const Math::Float& s2 )
	{
		this->s1 = s1;
		this->s2 = s2;
		this->rng = rng;
		managedRng.reset(rng);
		logRatio = -Math::Log(s2 / s1);
		time = 0;
		largeStepTime = 0;
		enableLargeStep = false;
		currentIndex = 0;
	}

	virtual void Accept()
	{
		if (enableLargeStep)
		{
			// Update large step time
			largeStepTime = time;
		}

		time++;
		prevSamples.clear();
		currentIndex = 0;
	}

	virtual void Reject()
	{
		// Restore samples
		for (auto& v : prevSamples)
		{
			int i = std::get<0>(v);
			auto& prevSample = std::get<1>(v);
			u[i] = prevSample;
		}

		prevSamples.clear();
		currentIndex = 0;
	}

	virtual void EnableLargeStepMutation( bool enable )
	{
		enableLargeStep = enable;
	}

	virtual bool LargeStepMutation() const
	{
		return enableLargeStep;
	}

	virtual void BeginRestore( RewindableSampler& rewindableSampler )
	{
		// Replace current RNG and get ready
		// to restore sampled state as primary samples
		rng = rewindableSampler.Rng();
	}

	virtual void EndRestore()
	{
		// Restore RNG
		rng = managedRng.get();
	}

	virtual void GetCurrentSampleState( std::vector<Math::Float>& samples ) const
	{
		samples.clear();
		for (auto& sample : u)
		{
			samples.push_back(sample.value);
		}
	}

	virtual void GetCurrentSampleState( std::vector<Math::Float>& samples, int numSamples )
	{
		samples.clear();
		for (int i = 0; i < numSamples; i++)
		{
			if (i < static_cast<int>(u.size()))
			{
				samples.push_back(u[i].value);
			}
			else
			{
				// The sample is not exist, use 0 instead
				// TODO : Some better way?
				samples.push_back(Math::Float(0));
			}
		}
	}

private:

	Math::Float PrimarySample(int i)
	{
		// Not sampled yet
		while (i >= static_cast<int>(u.size()))
		{
			u.emplace_back(rng->Next());
		}

		// If the modified time of the requested sample is not updated
		// it requires the lazy evaluation of mutations.
		if (u[i].modify < time)
		{
			if (enableLargeStep)
			{
				// Large step case

				// Save sample in order to restore previous state
				prevSamples.emplace_back(i, u[i]);

				// Update the modified time and value
				u[i].modify = time;
				u[i].value = rng->Next();
			}
			else
			{
				// Small step case

				// If the modified time is not updated since the last accepted
				// large step mutation, then update sample to the state.
				// Note that there is no need to go back before largeStepTime
				// because these samples are independent of the sample on largeStepTime.
				if (u[i].modify < largeStepTime)
				{
					u[i].modify = largeStepTime;
					u[i].value = rng->Next();
				}

				// Lazy evaluation of Mutate
				while (u[i].modify < time - 1)
				{
					u[i].value = Mutate(u[i].value);
					u[i].modify++;
				}

				// Save state
				prevSamples.emplace_back(i, u[i]);

				// Update the modified time and value
				u[i].value = Mutate(u[i].value);
				u[i].modify++;
			}
		}

		return u[i].value;
	}

	Math::Float Mutate(const Math::Float& value)
	{
		auto u = rng->Next();
		bool positive = u < Math::Float(0.5);

		// Convert to [0, 1]
		u = positive ? u * Math::Float(2) : Math::Float(2) * (u - Math::Float(0.5)); 

#if 1
		auto dv = s2 * std::exp(logRatio * u);
#else
		auto dv = kernelSizeScale * s2 * std::exp(logRatio * u);
#endif

		Math::Float result = value;
		if (positive)
		{
			result += dv;
			if (result > Math::Float(1)) result -= Math::Float(1);
		}
		else
		{
			result -= dv;
			if (result < Math::Float(0)) result += Math::Float(1);
		}

		return result;
	}

public:

	Math::Float s1, s2;													//!< Kernel size parameters
	Math::Float logRatio;												//!< Temporary variable (for efficiency)

	Random* rng;														//!< Current random number generator
	std::unique_ptr<Random> managedRng;									//!< Managed instance of RNG

	long long time;														//!< Number of accepted mutations
	long long largeStepTime;											//!< Time of the last accepted large step
	bool enableLargeStep;												//!< Indicates the next mutation is the large step

	int currentIndex;													//!< Current sample index
	std::vector<PSSMLTPrimarySample> u;									//!< List of current samples
	std::vector<std::tuple<int, PSSMLTPrimarySample>> prevSamples;		//!< Temporary list for preserving previous samples (restored if rejected)

};

LM_COMPONENT_REGISTER_IMPL(PSSMLTPrimarySamplerImpl, PSSMLTPrimarySampler);

LM_NAMESPACE_END