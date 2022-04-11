#pragma once

#include  <string>
#include "GLES3/gl32.h"
#include "SDL_ttf.h"
#include "Mesh.h"
#include "Material.h"

class TTFText
{
public:
  TTFText(std::string file, int size = 11)
  {
    m_font = TTF_OpenFont(file.c_str(), size);
  }

  ~TTFText()
  {
    TTF_CloseFont(m_font);
    SDL_FreeSurface(m_sdlSurface);
    SafeDel(m_surface);
  }
  
  void SetText(std::string text)
  {
    m_text = text;
    CreateTexture();
  }

  void SetColor(SDL_Color color)
  {
    m_color = color;
    CreateTexture();
  }

  std::string GetText()
  {
    return m_text;
  }

  void SetPos(int x, int y)
  {
    if (m_surface != nullptr)
      m_surface->m_node->SetTranslation({ x, y, 0 });
  }

private:
  void CreateTexture()
  {
    SDL_FreeSurface(m_sdlSurface);
    m_sdlSurface = TTF_RenderText_Blended(m_font, m_text.c_str(), m_color);

    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);

    GLint format = GL_RGBA;
    /*Uint8 colors = m_sdlSurface->format->BytesPerPixel;
    if (colors == 4)
    {
      if (m_sdlSurface->format->Rmask != 0x000000ff)
      {
        GLint swizzleMask[] = { GL_BLUE, GL_GREEN, GL_RED, GL_ALPHA };
        glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);
      }
    }
    else
    {
      if (m_sdlSurface->format->Rmask == 0x000000ff)
      {
        format = GL_RGB;
      }
      else
      {
        format = GL_BGR;
      }
    }*/

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, format, m_sdlSurface->w, m_sdlSurface->h, 0, format, GL_UNSIGNED_BYTE, m_sdlSurface->pixels);

    std::shared_ptr<ToolKit::Texture> texPtr(new ToolKit::Texture());
    texPtr->m_textureId = textureId;
    texPtr->m_width = m_sdlSurface->w;
    texPtr->m_height = m_sdlSurface->h;

		ToolKit::Node* backUp = nullptr;
    if (m_surface != nullptr)
      backUp = new ToolKit::Node(*m_surface->m_node);

    SafeDel(m_surface);
    glm::vec2 zero(0.0f, 0.0f);
    m_surface = new ToolKit::Surface(texPtr, zero);

    if (backUp != nullptr)
    {
      SafeDel(m_surface->m_node);
      m_surface->m_node = backUp;
    }
  }

public:
	ToolKit::Surface* m_surface = nullptr;

private:
  SDL_Surface* m_sdlSurface = nullptr;
  std::string m_text;
  TTF_Font* m_font = nullptr;
  SDL_Color m_color = { 255, 0, 0, 1 };
};