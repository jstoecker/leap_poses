#ifndef __LEAP_GESTURES_FINGER_H__
#define __LEAP_GESTURES_FINGER_H__

#include "Gesture.h"

class FingerGesture : public Gesture
{
public:
	enum class HandType
	{
		left,
		right
	};

	FingerGesture(HandType hand_type, Leap::Finger::Type finger_type);
	virtual ~FingerGesture();
	void update(const Leap::Frame& frame);
	const Leap::Frame& frame() const;
	GestureEvent& onStart();
	GestureEvent& onEnd();
	GestureEvent& onUpdate();

protected:


private:
	GestureEvent start_;
	GestureEvent end_;
	GestureEvent update_;
};

class GestureEvent
{
public:
	void add(std::function<void(const Gesture& gesture)>);
	void clear();

private:
	std::vector<std::function<void(const Gesture& gesture)>> callbacks_;
	void call();
};

#endif