#include "Renderer.h"

using namespace gl;
using namespace std;
using namespace Leap;

Renderer::Renderer() : vao_(0)
{
	grid_size_ = 5.0f;
	text_label_ = "Pose: None";
}

void Renderer::init()
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	grid_.begin(GL_LINES);
	grid_.color(.3f, .3f, .3f);
	int lines = 8;
	float step = grid_size_ / (lines - 1);
	float half = grid_size_ / 2.0f;
	for (int i = 0; i < lines; i++) {
		float x = half - i * step;
		grid_.vertex(x, 0.0f, -half);
		grid_.vertex(x, 0.0f, +half);
		grid_.vertex(-half, 0.0f, x);
		grid_.vertex(+half, 0.0f, x);

		grid_.vertex(x, 0.0f, -half);
		grid_.vertex(x, 2 * half, -half);
		grid_.vertex(-half, x + half, -half);
		grid_.vertex(+half, x + half, -half);
	}
	grid_.end();

	Geometry geom = Sphere({ 0, 0, 0 }, .075f).triangles(8);
	joint_vbo_.generateVBO(GL_STATIC_DRAW);
	joint_vbo_.bind();
	joint_vbo_.data(&geom.vertices[0], geom.vertices.size() * sizeof(Vec3));
	joint_ibo_.generateIBO(GL_STATIC_DRAW);
	joint_ibo_.bind();
	joint_ibo_.data(&geom.indices[0], geom.indices.size() * sizeof(GLuint));
	joint_triangle_count_ = geom.indices.size();

	prog_ = Program::create("shaders/color.vert", "shaders/color.frag");

	text_.loadFont("menlo24");
}

void Renderer::draw()
{
	if (!vao_) {
		init();
	}

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	viewport_.apply();

	grid_.setModelViewProj(viewProjection_);
	grid_.draw();

	if (controller_.isConnected()) {
		frame_ = controller_.frame();
		updatePoses();
		drawFrame();
	}

	text_.begin(viewport_.width, viewport_.height);
	text_.setColor(1.0f, 1.0f, 1.0f);
	text_.add(text_label_, 50, viewport_.height - 50, TextRenderer::LEFT, TextRenderer::TOP);
	text_.end();
}

void Renderer::drawFrame()
{
	for (const Hand& hand : frame_.hands()) {
		drawHand(hand);
	}
}

void Renderer::drawHand(const Hand& hand)
{
	Vec4 base_color = color_;

	prog_.enable();
	joint_vbo_.bind();
	joint_ibo_.bind();
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

	prog_.uniform("color", base_color);
	drawJoint(hand.palmPosition(), 2.0f);

	for (Finger f : hand.fingers()) {

		Vec4 finger_color = base_color;
		if (!f.isExtended())
			finger_color *= 0.5f;

		prog_.uniform("color", finger_color);

		drawJoint(f.bone(Bone::TYPE_METACARPAL).prevJoint());
		drawJoint(f.bone(Bone::TYPE_PROXIMAL).prevJoint());
		drawJoint(f.bone(Bone::TYPE_INTERMEDIATE).prevJoint());
		drawJoint(f.bone(Bone::TYPE_DISTAL).prevJoint());

		drawJoint(f.tipPosition());
	}

	lines_.setModelViewProj(viewProjection_);
	lines_.color(base_color.x, base_color.y, base_color.z);
	for (Finger f : hand.fingers()) {

		if (f.type() == Finger::TYPE_INDEX || f.type() == Finger::TYPE_PINKY) {
			drawLine(f.bone(Bone::TYPE_METACARPAL).prevJoint(), f.bone(Bone::TYPE_METACARPAL).nextJoint());
		}
		drawLine(f.bone(Bone::TYPE_PROXIMAL).prevJoint(), f.bone(Bone::TYPE_PROXIMAL).nextJoint());
		drawLine(f.bone(Bone::TYPE_INTERMEDIATE).prevJoint(), f.bone(Bone::TYPE_INTERMEDIATE).nextJoint());
		drawLine(f.bone(Bone::TYPE_DISTAL).prevJoint(), f.bone(Bone::TYPE_DISTAL).nextJoint());

	}

	drawLine(hand.fingers()[1].bone(Bone::TYPE_PROXIMAL).prevJoint(), hand.fingers()[2].bone(Bone::TYPE_PROXIMAL).prevJoint());
	drawLine(hand.fingers()[2].bone(Bone::TYPE_PROXIMAL).prevJoint(), hand.fingers()[3].bone(Bone::TYPE_PROXIMAL).prevJoint());
	drawLine(hand.fingers()[3].bone(Bone::TYPE_PROXIMAL).prevJoint(), hand.fingers()[4].bone(Bone::TYPE_PROXIMAL).prevJoint());
	drawLine(hand.fingers()[1].bone(Bone::TYPE_METACARPAL).prevJoint(), hand.fingers()[2].bone(Bone::TYPE_METACARPAL).prevJoint());
	drawLine(hand.fingers()[2].bone(Bone::TYPE_METACARPAL).prevJoint(), hand.fingers()[3].bone(Bone::TYPE_METACARPAL).prevJoint());
	drawLine(hand.fingers()[3].bone(Bone::TYPE_METACARPAL).prevJoint(), hand.fingers()[4].bone(Bone::TYPE_METACARPAL).prevJoint());
}

void Renderer::drawJoint(Vector center, float scale)
{
	Mat4 model = translation(convert(center)) * gl::scale(scale, scale, scale);
	prog_.uniform("modelViewProjection", viewProjection_ * model);
	glDrawElements(GL_TRIANGLES, joint_triangle_count_, GL_UNSIGNED_INT, 0);
}

void Renderer::drawLine(Vector start, Vector end)
{
	Vec3 a = convert(start);
	Vec3 b = convert(end);
	lines_.begin(GL_LINES);
	lines_.vertex(a.x, a.y, a.z);
	lines_.vertex(b.x, b.y, b.z);
	lines_.end();
	lines_.draw();
}

Vec3 Renderer::convert(Vector v)
{
	Vec3 result = v.toVector3<Vec3>();
	return result * 0.015f;
}

void Renderer::resize(int width, int height)
{
	viewport_.width = width;
	viewport_.height = height;

	Mat4 projection = perspective(gl::deg_to_rad * 60.0f, viewport_.aspect(), 0.1f, 20.0f);
	Mat4 view = lookAt({ 0, 4, 5 }, { 0, 3, 0 }, { 0, 1, 0 });
	viewProjection_ = projection * view;
}

void Renderer::updatePoses()
{
	color_ = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
	text_label_ = "Pose: None";

	point_2_pose_.update(frame_);
	if (point_2_pose_.tracking()) {
		text_label_ = "Pose: POINT 2H";
		color_ = Vec4(0.5f, 1.0f, 0.5f, 1.0f);
	}

	//v_pose_.update(frame_);
	//if (v_pose_.tracking()) {
	//	if (v_pose_.isClosed()) {
	//		text_label_ = "Pose: V (closed)";
	//		color_ = Vec4(1.0f, 0.25f, 1.0f, 1.0f);
	//	} else {
	//		text_label_ = "Pose: V (open)";
	//		color_ = Vec4(1.0f, 0.5f, 1.0f, 1.0f);
	//	}
	//	return;
	//}

	//l_pose_.update(frame_);
	//if (l_pose_.tracking()) {
	//	if (l_pose_.isClosed()) {
	//		text_label_ = "Pose: L (closed)";
	//		color_ = Vec4(0.25f, 1.0f, 0.25f, 1.0f);
	//	} else {
	//		text_label_ = "Pose: L (open)";
	//		color_ = Vec4(0.5f, 1.0f, 0.5f, 1.0f);
	//	}
	//	return;
	//}

	//point_pose_.update(frame_);
	//if (point_pose_.tracking()) {
	//	text_label_ = "Pose: POINT";
	//	color_ = Vec4(0.5f, 0.5f, 1.0f, 1.0f);
	//	return;
	//}

	//pinch_pose_.update(frame_);
	//if (pinch_pose_.tracking()) {
	//	if (pinch_pose_.isPinching()) {
	//		text_label_ = "Pose: PINCH (closed)";
	//		color_ = Vec4(1.0f, 1.0f, 0.25f, 1.0f);
	//	} else {
	//		text_label_ = "Pose: PINCH (open)";
	//		color_ = Vec4(1.0f, 1.0f, 0.5f, 1.0f);
	//	}
	//	return;
	//}

	//fist_pose_.update(frame_);
	//if (fist_pose_.tracking()) {
	//	if (fist_pose_.state() == FistPose::State::closed) {
	//		text_label_ = "Pose: FIST (closed)";
	//		color_ = Vec4(1.0f, 0.5f, 0.5f, 1.0f);
	//	} else if (fist_pose_.state() == FistPose::State::open) {
	//		text_label_ = "Pose: FIST (partial)";
	//		color_ = Vec4(0.5f, 1.0f, 0.5f, 1.0f);
	//	} else {
	//		text_label_ = "Pose: FIST (open)";
	//		color_ = Vec4(1.0f, 1.0f, 0.5f, 1.0f);
	//	}
	//}
}