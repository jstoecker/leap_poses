#include "Gesture.h"

Gesture::~Gesture()
{
}

void Gesture::update(const Leap::Frame& frame)
{
	frame_ = frame;
	update();
}

Gesture::Event::Event(Gesture& parent) : parent_(parent)
{
}

void Gesture::Event::add(std::function<void(const Gesture& gesture)> cb)
{
	callbacks_.push_back(cb);
}

void Gesture::Event::clear()
{
	callbacks_.clear();
}

void Gesture::Event::call()
{
	for (Gesture::Callback& cb : callbacks_)
		cb(parent_);
}