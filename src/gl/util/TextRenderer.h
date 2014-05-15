#ifndef __MEDLEAP_TEXT_RENDERER__
#define __MEDLEAP_TEXT_RENDERER__

#include "gl/glew.h"
#include "gl/Program.h"
#include <string>
#include <vector>
#include <map>

class TextRenderer
{
public:
    enum Alignment
    {
        LEFT = 0,
        BOTTOM = 0,
        CENTER = 1,
        RIGHT = 2,
        TOP = 2
    };
    
    TextRenderer();
    
    ~TextRenderer();
    
    /**
     * Loads a font in the fonts directory by name. Example: menlo14 will load the files
     * fonts/menlo14.bmp and fonts/menlo14.dat. The first font to be loaded will be used
     * by default. Multiple fonts may be loaded.
     */
    bool loadFont(const std::string& fontName);
    
    /** Use a font that has already been loaded. This must be set before begin(). */
    void setFont(const std::string& fontName);
    
    /** Sets the text color. This must be set before begin(). */
    void setColor(float r, float g, float b);
    
    /** Call this once before adding any text. It will clear previously added text. */
    void begin(int width, int height);
    
    /** Adds text to be rendered */
    void add(const std::string& text, int x, int y, Alignment hAlign = LEFT, Alignment vAlign = BOTTOM);
    
    /** Performs actual drawing */
    void end();
    
    /** Returns height of text */
    int fontHeight();
    
    /** Returns width of text */
    int fontWidth(const std::string& text);
  
private:
    
    class Font
    {
    public:
        Font();
        ~Font();
        
        GLuint texture;
        unsigned char glyphWidths[256];
        unsigned char glyphHeight;
        unsigned texWidth;
        unsigned texHeight;
        
        unsigned int width(const std::string& s);
        bool load(const char* bmpFileName, const char* metricsFileName);
    };
    
    gl::Program program;
    GLuint vbo;
    float r, g, b;
    std::vector<GLfloat> vertices;
    std::map<std::string, Font*> fonts;
    Font* currentFont;
    int windowWidth, windowHeight;
    bool dirty;
    
    void addVertex(int x, int y, float u, float v);
};

#endif // __MEDLEAP_TEXT_RENDERER__