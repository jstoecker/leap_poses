#ifndef __LEAP_GESTURES_GESTURE_H__
#define __LEAP_GESTURES_GESTURE_H__

#include "Leap.h"
#include <functional>

class Gesture
{
public:
	typedef std::function<void(const Gesture& gesture)> Callback;

	class Event
	{
	public:
		void add(Callback);
		void clear();

	private:
		Event(Gesture& parent);
		const Gesture& parent_;
		std::vector<Callback> callbacks_;
		void call();
	};

	virtual ~Gesture();
	void update(const Leap::Frame& frame);
	Event& onStart() { return start_; }
	Event& onEnd() { return end_; }
	Event& onUpdate() { return update_; }
	const Leap::Frame& frame() const { return frame_; }

protected:
	virtual void update() = 0;

private:
	Leap::Frame& frame_;
	Event start_;
	Event end_;
	Event update_;
};

#endif