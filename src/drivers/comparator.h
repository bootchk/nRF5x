
#pragma once

enum class ComparatorReferenceVolts {
	V1_2,
	V1_8,
	V2_4
};


class Comparator {
public:
	/*
	 * Return true if AI0 is above ComparatorReferenceVolts
	 */
	static bool initCompareAndShutdown(ComparatorReferenceVolts);
};
