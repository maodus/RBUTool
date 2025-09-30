#pragma once
#include <core/RbuProcessor.hpp>

class ChartMetaView {
public:
	std::shared_ptr<RBUHeader> rbu_header_;
	ChartMetaView::ChartMetaView();

	void Render();
private:
	
};