#pragma once

#include "../myGL/Shader.h"
#include "../myGL/Texture2D.h"

#include <ft2build.h>
#include FT_FREETYPE_H

#include <map>


namespace Application {
    struct Character {
        GLuint TextureID; // ID handle of the glyph texture
        glm::ivec2   Size;      // Size of glyph
        glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
        unsigned int Advance;   // Horizontal offset to advance to next glyph
    };
    inline std::map<GLchar, Character> Characters;
    inline GLuint characterVAO, characterVBO;

	namespace TextRender {
		inline int Init() {
            //FreeType (thx https://learnopengl.com, copied from it)
            FT_Library ft;
            // All functions return a value different than 0 whenever an error occurred
            if (FT_Init_FreeType(&ft)) {
                std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
                return -1;
            }

            // find path to font
            const std::string font_name = "resources/Arial.ttf";

            // load font as face
            FT_Face face;
            if (FT_New_Face(ft, font_name.c_str(), 0, &face)) {
                std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
                return -1;
            } else {
                // set size to load glyphs as
                FT_Set_Pixel_Sizes(face, 0, 48);

                // disable byte-alignment restriction
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                // load first 128 characters of ASCII set
                for (unsigned char c = 0; c < 128; c++) {
                    // Load character glyph 
                    if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
                        std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                        continue;
                    }
                    // generate texture
                    const Texture2D char_texture((GLsizei)face->glyph->bitmap.width, (GLsizei)face->glyph->bitmap.rows, GL_RED, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
                    // store character for later use
                    Character character = {
                        char_texture.id,
                        glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                        glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                        static_cast<unsigned int>(face->glyph->advance.x)
                    };
                    Characters.insert(std::pair<char, Character>(c, character));
                }
                Texture2D::unbind();
                // glBindTexture(GL_TEXTURE_2D, 0);
            }
            // destroy FreeType once we're finished
            FT_Done_Face(face);
            FT_Done_FreeType(ft);


            // configure VAO/VBO for texture quads
            VAO::generate(characterVAO);
            VBO::generate(characterVBO);
            VAO::bind(characterVAO);
            VBO::bind(characterVBO);
            VBO::setData(characterVBO, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
            VAO::addAttrib(characterVAO, 0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
            VBO::unbind();
            VAO::unbind();

            return 0;
		}
        inline void RenderText(const Shader& shader, std::string text, float x, float y, const float scale, const glm::vec3 color) {
            // activate corresponding render state	
            shader.use();
            shader.setVec3("textColor", color.x, color.y, color.z);
            //glUniform3f(glGetUniformLocation(shader.id, "textColor"), color.x, color.y, color.z);
            Texture2D::deactivate();
            //glActiveTexture(GL_TEXTURE0);
            VAO::bind(characterVAO);

            // iterate through all characters
            for (std::string::const_iterator c = text.begin(); c < text.end(); ++c) {
                const Character ch = Characters[*c];

                const float xpos = x + static_cast<float>(ch.Bearing.x) * scale;
                const float ypos = y - static_cast<float>(ch.Size.y - ch.Bearing.y) * scale;

                const float w = static_cast<float>(ch.Size.x) * scale;
                const float h = static_cast<float>(ch.Size.y) * scale;
                // update VBO for each character
                const float vertices[6][4] = {
                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos,     ypos,       0.0f, 1.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },

                    { xpos,     ypos + h,   0.0f, 0.0f },
                    { xpos + w, ypos,       1.0f, 1.0f },
                    { xpos + w, ypos + h,   1.0f, 0.0f }
                };
                // render glyph texture over quad
                glBindTexture(GL_TEXTURE_2D, ch.TextureID);
                // update content of VBO memory
                VBO::bind(characterVBO);
                VBO::setSubData(characterVBO, 0, sizeof(vertices), vertices);
                //glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // be sure to use glBufferSubData and not glBufferData
                VBO::unbind();

                // render quad with character
                glDrawArrays(GL_TRIANGLES, 0, 6);
                // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
                x += static_cast<float>(ch.Advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
            }
            VAO::unbind();
            Texture2D::deactivate();
            //glActiveTexture(GL_TEXTURE0);
        }
	}
}
