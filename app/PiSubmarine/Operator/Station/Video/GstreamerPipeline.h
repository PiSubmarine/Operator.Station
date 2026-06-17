#pragma once
#include <memory>
#include <gst/gstelement.h>
#include "PiSubmarine/Operator/Station/Video/IPipeline.h"

namespace PiSubmarine::Operator::Station::Video
{
	class GstreamerPipeline : public IPipeline
	{

	protected:
		struct GstObjectDeleter
		{
			void operator()(GstObject* object) const noexcept;
		};

		struct GstElementDeleter
		{
			void operator()(GstElement* element) const noexcept;
		};

		using GstObjectPtr = std::unique_ptr<GstObject, GstObjectDeleter>;
		using GstElementPtr = std::unique_ptr<GstElement, GstElementDeleter>;

		static void ReleaseOwnership(GstElementPtr& element)
		{
			const auto raw = element.release();
			(void)raw;
		}
	};
}