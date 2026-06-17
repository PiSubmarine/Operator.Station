
#include "PiSubmarine/Operator/Station/Video/GstreamerPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
	void GstreamerPipeline::GstObjectDeleter::operator()(GstObject* object) const noexcept
	{
		if (object != nullptr)
		{
			gst_object_unref(object);
		}
	}

	void GstreamerPipeline::GstElementDeleter::operator()(GstElement* element) const noexcept
	{
		if (element != nullptr)
		{
			gst_object_unref(GST_OBJECT(element));
		}
	}
}