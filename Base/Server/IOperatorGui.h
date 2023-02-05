#pragma once


struct IOperatorGui
{
	enum class OperatorGui { Monitor, Radar, Mutator, GeneSequencer, MysteryGui };
	virtual ~IOperatorGui() {}
	virtual std::optional<OperatorGui> update() = 0;
};