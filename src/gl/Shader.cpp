#include <fstream>
#include <sstream>
#include "Shader.h"

using namespace gl;

Shader::Shader() : handle_(nullptr), type_(GL_INVALID_ENUM)
{
}

GLuint Shader::id() const
{
	return handle_ ? *(handle_.get()) : 0;
}

void Shader::release()
{
	handle_ = nullptr;
}

GLenum Shader::type() const
{
    return type_;
}

std::string Shader::source() const
{
    return (id() ? source_ : std::string());
}

std::string Shader::log() const
{
    if (!id())
        return std::string();
    
    GLint maxLength = 0;
    glGetShaderiv(id(), GL_INFO_LOG_LENGTH, &maxLength);
    GLchar* buf = new GLchar[maxLength];
    
    glGetShaderInfoLog(id(), maxLength, &maxLength, buf);
    std::string log(buf);
    delete[] buf;
    
    return log;
}

bool Shader::compile(const char* src, GLenum type)
{
	generate(type);
    
	this->type_ = type;
    this->source_ = std::string(src);
    
    glShaderSource(id(), 1, &src, NULL);
	glCompileShader(id());
    
    GLint status;
	glGetShaderiv(id(), GL_COMPILE_STATUS, &status);

    return (status != 0);
}

bool Shader::compileFile(const char* fileName, GLenum type)
{
    std::ifstream in(fileName);
    std::string src((std::istreambuf_iterator<char>(in)),
                    std::istreambuf_iterator<char>());
    return compile(src.c_str(), type);
}

void Shader::generate(GLenum type)
{
	this->type_ = type;

	auto deleteFunction = [=](GLuint* p) {
		if (p) {
			glDeleteShader(*p);
			delete p;
		}
	};

	GLuint* p = new GLuint(glCreateShader(type));
	handle_ = std::shared_ptr<GLuint>(p, deleteFunction);
}