#ifndef __LEAP_POSES_RENDERER_H__
#define __LEAP_POSES_RENDERER_H__

#include <string>
#include "gl/Viewport.h"
#include "gl/util/Draw.h"
#include "Leap.h"
#include "gl/geom/Sphere.h"
#include "gl/util/TextRenderer.h"
#include "poses/PointPose.h"
#include "poses/FistPose.h"
#include "poses/LPose.h"
#include "poses/VPose.h"
#include "poses/PinchPose.h"
#include "poses/PointPose2H.h"
#include "poses/CarryPose.h"

class Renderer
{
public:
	Renderer();
	void draw();
	void resize(int width, int height);

private:
	std::string text_label_;
	TextRenderer text_;
	gl::Vec4 color_;
	gl::Viewport viewport_;
	gl::Draw lines_;
	gl::Draw grid_;
	gl::Program prog_;
	gl::Buffer joint_vbo_;
	gl::Buffer joint_ibo_;
	GLuint joint_triangle_count_;

	GLuint vao_;
	gl::Mat4 viewProjection_;
	Leap::Frame frame_;
	Leap::Controller controller_;
	float grid_size_;

	PointPose point_pose_;
	FistPose fist_pose_;
	LPose l_pose_;
	VPose v_pose_;
	PinchPose pinch_pose_;
	PointPose2H point_2_pose_;
	CarryPose carry_pose_;

	void init();
	void drawGrid();
	void drawFrame();
	void drawHand(const Leap::Hand& hand);
	void drawJoint(Leap::Vector center, float scale = 1.0f);
	void drawLine(Leap::Vector start, Leap::Vector end);

	void updatePoses();

	gl::Vec3 convert(Leap::Vector v);
};

#endif