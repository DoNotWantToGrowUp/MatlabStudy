#include "AcquisitionFinishGadget.h"

using namespace Gadgetron;

int AcquisitionFinishGadget::process(GadgetContainerMessage<ISMRMRD::AcquisitionHeader> *m1, GadgetContainerMessage<hoNDArray<std::complex<float> > > *m2)
{
	if (!controller_) {
		ACE_DEBUG((LM_DEBUG, ACE_TEXT("Cannot return result to controller, no controller set")));
		return -1;
	}

	GadgetContainerMessage<GadgetMessageIdentifier>* mb = new GadgetContainerMessage<GadgetMessageIdentifier>();

	mb->getObjectPtr()->id = GADGET_MESSAGE_ISMRMRD_ACQUISITION;

	mb->cont(m1);

	return controller_->output_ready(mb);
}

GADGET_FACTORY_DECLARE(AcquisitionFinishGadget)
