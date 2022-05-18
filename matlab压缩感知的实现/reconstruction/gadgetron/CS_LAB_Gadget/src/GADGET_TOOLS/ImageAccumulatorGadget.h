#ifndef IMAGEACCUMULATORGADGET_H
#define IMAGEACCUMULATORGADGET_H

#pragma once
#include "CS_LAB_export.h"
#include "Gadget.h"
#include "hoNDArray.h"
#include <ismrmrd.h>
#include <complex>

#include "GlobalVar.h"
#include "SomeFunctions.h"

#include "GadgetIsmrmrdReadWrite.h"

namespace Gadgetron
{
	class EXPORTCSLAB ImageAccumulatorGadget : public Gadget2<ISMRMRD::ImageHeader, hoNDArray<float> >
	{
	public:
		ImageAccumulatorGadget();
		~ImageAccumulatorGadget();
		int process_config(ACE_Message_Block *mb);
		int process(GadgetContainerMessage<ISMRMRD::ImageHeader> *m1, GadgetContainerMessage<hoNDArray<float> > *m2);
		GADGET_DECLARE(ImageAccumulatorGadget);

	protected:
		std::vector<size_t> vtDimensions_;
		size_t iPartition_ = 0;
		size_t iPhs_ = 0;
		size_t iImageLoopCounter_ = 0;
		hoNDArray<float> *hafBuffer_ = NULL;
	};
}

#endif // IMAGEACCUMULATORGADGET_H
