#include "Renderer.h"
#include <initializer_list>

using namespace gl;
using namespace std;
using namespace Leap;

Renderer::Renderer() : vao_(0)
{
	grid_size_ = 25.0f;
	text_label_ = "Pose: None";
	poses_.enableAll(true);
}

void Renderer::init()
{
	glGenVertexArrays(1, &vao_);
	glBindVertexArray(vao_);

	grid_.begin(GL_LINES);
	grid_.color(.3f, .3f, .3f);
	int lines = 18;
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

	text_.clear();
	text_.hAlign(TextRenderer::HAlign::left);
	text_.vAlign(TextRenderer::VAlign::top);
	int y = viewport_.height - 50;
	for (const ActivePose& active : active_poses_) {
		text_.color(active.color);
		text_.add(active.name, 50.0f, y);
		y -= 25;
	}
	text_.draw();
}

void Renderer::drawFrame()
{
	for (const Hand& hand : frame_.hands()) {
		drawHand(hand);
	}
}

void Renderer::drawHand(const Hand& hand)
{
	Vec4 base_color;

	for (const ActivePose& active : active_poses_) {

		bool thisHand = false;
		for (int32_t active_hand : active.hands) {
			if (hand.id() == active_hand)
				thisHand = true;
		}

		if (thisHand)
			base_color = Vec4(active.color, 1.0f);
	}

	if (base_color.length() == 0) {
		base_color = Vec4(.5f);
	}

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

	text_.viewport(viewport_);

	Mat4 projection = perspective(gl::deg_to_rad * 60.0f, viewport_.aspect(), 0.1f, 20.0f);
	Mat4 view = lookAt({ 0, 4, 5 }, { 0, 3, 0 }, { 0, 1, 0 });
	viewProjection_ = projection * view;
}

void Renderer::updatePoses()
{
	color_ = Vec4(0.5f, 0.5f, 0.5f, 1.0f);
	text_label_ = "Pose: None";

	active_poses_.clear();

	poses_.update(frame_);

	if (poses_.fist().tracking()) {
		if (poses_.fist().state() == FistPose::State::closed) {
			active_poses_.push_back({ "Fist (closed", { 0.6f, 0.25f, 0.25f }, { poses_.fist().hand().id() } });
		} else if (poses_.fist().state() == FistPose::State::open) {
			active_poses_.push_back({ "Fist (open)", { 0.75f, 0.75f, 0.85f }, { poses_.fist().hand().id() } });
		}
	}

	if (poses_.carry().tracking()) {
		active_poses_.push_back({ "Carry", { 1.0f, 0.25f, 0.75f }, { poses_.carry().hand().id() } });
	}

	if (poses_.point2().tracking()) {
		active_poses_.push_back({ "Point 2H", { 1.0f, 0.25f, 0.25f }, { poses_.point2().left().id(), poses_.point2().right().id() } });
	}

	if (poses_.v().tracking()) {
		if (poses_.v().isClosed()) {
			active_poses_.push_back({ "V (closed)", { 1.0f, 0.5f, 0.0f }, { poses_.v().hand().id() } });
		} else {
			active_poses_.push_back({ "V (open)", { 1.0f, 0.5f, 0.0f }, { poses_.v().hand().id() } });
		}
	}

	if (poses_.l().tracking()) {
		if (poses_.l().isClosed()) {
			active_poses_.push_back({ "L (closed)", { 0.0f, 1.0f, 0.0f }, { poses_.l().hand().id() } });
		} else {
			active_poses_.push_back({ "L (open)", { 0.25f, 1.0f, 0.25f }, { poses_.l().hand().id() } });
		}
	}

	if (poses_.point().tracking()) {
		active_poses_.push_back({ "Point", { 0.0f, 0.0f, 1.0f }, { poses_.point().hand().id() } });
	}

	if (poses_.pinch().tracking()) {
		if (poses_.pinch().isPinching()) {
			active_poses_.push_back({ "Pinch", { 1.0f, 1.0f, 0.0f }, { poses_.pinch().hand().id() } });
		}
	}

	if (poses_.palmsFace().tracking()) {
		active_poses_.push_back({ "Palms Face", { 0.0f, 1.0f, 1.0f }, { poses_.palmsFace().left().id(), poses_.palmsFace().right().id() } });
	}

	if (poses_.push().tracking()) {
		if (poses_.push().isClosed()) {
			active_poses_.push_back({ "Push (closed)", { 0.5f, 1.0f, 0.5f }, { poses_.push().hand().id() } });
		} else {
			active_poses_.push_back({ "Push (open)", { 0.5f, 1.0f, 0.5f }, { poses_.push().hand().id() } });
		}
	}
}