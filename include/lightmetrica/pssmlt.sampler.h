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

#pragma once
#ifndef LIB_LIGHTMETRICA_PSSMLT_SAMPLER_H
#define LIB_LIGHTMETRICA_PSSMLT_SAMPLER_H

#include "sampler.h"
#include "math.types.h"

LM_NAMESPACE_BEGIN

class Random;
class RewindableSampler;

/*!
	Kelemen's primary sample.
	Represents an element of the primary sample space.
	The class shares the same interface as generic Sampler class,
	so we can use this class to sample light paths without modification
	of the implementation of path samplers.
	The function is also responsible for the lazy evaluation of the mutations
	which is required to support infinite dimension space.
	For details, see the original paper.
*/
class PSSMLTPrimarySampler : public Sampler
{
public:

	LM_COMPONENT_INTERFACE_DEF("pssmltprimarysampler");

public:

	PSSMLTPrimarySampler() {}
	virtual ~PSSMLTPrimarySampler() {}

public:

	/*!
		Configure the sampler.
		Configure and initializes the sampler.
		This function must be called before use.
		\param rng Random number generator.
		\param s1 Lower bound of the kernel.
		\param s2 Upper bound of the kernel.
	*/
	virtual void Configure(Random* rng, const Math::Float& s1, const Math::Float& s2) = 0;

	/*!
		Accept mutation.
		Indicates to accept mutated samples.
	*/
	virtual void Accept() = 0;

	/*!
		Reject mutation.
		Indicates to reject mutated samples.
		The internal state is restored to previous state.
	*/
	virtual void Reject() = 0;

	/*!
		Enable large step mutation.
		Enables to use large step mutation in the next mutation step.
		\param enable Enabled if true.
	*/
	virtual void EnableLargeStepMutation(bool enable) = 0;

	/*!
		Check if current mutation is large step mutation.
		\retval true Current mutation is large step mutation.
		\retval false Current mutation is small step mutation.
	*/
	virtual bool LargeStepMutation() const = 0;

	/*!
		Begin to restore samples.
		Begins to set internal sample state
		using the restored sequence of samples with rewindable sampler.
		Path sampling process dispatched between #BeginRestore and #EndRestore is
		recorded in a form of primary samples.
		\param rewindableSampler Rewindable sampler.
		\sa EndRestore
	*/
	virtual void BeginRestore(RewindableSampler& rewindableSampler) = 0;
	
	/*!
		End to restore samples.
		Corresponds to #BeginRestore.
		\sa BeginRestore
	*/
	virtual void EndRestore() = 0;

public:

	virtual void GetCurrentSampleState(std::vector<Math::Float>& samples) const = 0;
	virtual void GetCurrentSampleState(std::vector<Math::Float>& samples, int numSamples) = 0;

};

/*
class PSSMLTPrimarySampler : public Sampler
{
public:

	struct Sample
	{
		Sample(const Math::Float& value)
			: value(value)
			, modify(0)
		{}

		Math::Float value;		//!< Sample value
		long long modify;		//!< Last modified time
	};

public:

	LM_PUBLIC_API PSSMLTPrimarySample(const Math::Float& s1, const Math::Float& s2);

public:

	LM_PUBLIC_API Math::Float Next();
	LM_PUBLIC_API virtual Sampler* Clone();
	LM_PUBLIC_API virtual void SetSeed( unsigned int seed );
	LM_PUBLIC_API virtual unsigned int NextUInt();
	LM_PUBLIC_API virtual Math::Vec2 NextVec2();

public:

	LM_PUBLIC_API void Accept();
	LM_PUBLIC_API void Reject();

	LM_PUBLIC_API void SetLargeStep(bool largeStep);
	LM_PUBLIC_API bool LargeStep();
	LM_PUBLIC_API Random* Rng();
	LM_PUBLIC_API void SetRng(Random* rng);
	LM_PUBLIC_API void GetCurrentSampleState(std::vector<Math::Float>& samples) const;
	LM_PUBLIC_API void GetCurrentSampleState(std::vector<Math::Float>& samples, int numSamples);

	LM_PUBLIC_API void SetKernelSizeScale(const Math::Float& scale);
	
private:

	Math::Float PrimarySample(int i);
	Math::Float Mutate(const Math::Float& value);

private:

	Math::Float s1, s2;									//!< Kernel size parameters
	Math::Float logRatio;								//!< Temporary variable (for efficiency)

	Random* rng;										//!< Random number generator (not managed here)

	long long time;										//!< Number of accepted mutations
	long long largeStepTime;							//!< Time of the last accepted large step
	bool largeStep;										//!< Indicates the next mutation is the large step

	int currentIndex;									//!< Current sample index
	std::vector<Sample> u;								//!< List of current samples
	std::vector<std::tuple<int, Sample>> prevSamples;	//!< Temporary list for preserving previous samples (restored if rejected)

	Math::Float kernelSizeScale;						// TODO

};
*/

LM_NAMESPACE_END

#endif // LIB_LIGHTMETRICA_PSSMLT_SAMPLER_H