/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "TKOpenGL.h"
#include "TKStats.h"
#include "Types.h"

namespace ToolKit
{
  class TK_API RHI
  {
    friend class Renderer;
    friend class Framebuffer;
    friend class RenderSystem;

   public:
    /**
     * Sets the given texture to given slot.
     * textureSlot can be between 0 & 31.
     */
    static void SetTexture(GLenum target, GLuint textureID, GLenum textureSlot = 31);

   private:
    static void SetFramebuffer(GLenum target, GLuint framebufferID);
    static void DeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
    static void InvalidateFramebuffer(GLenum target, GLsizei numAttachments, const GLenum* attachments);

   private:
    static GLuint m_currentReadFramebufferID;
    static GLuint m_currentDrawFramebufferID;
    static GLuint m_currentFramebufferID;

    /**
     * Holds which texture is binded to which texture unit.
     */
    static std::unordered_map<uint, uint> m_slotTextureIDmap;
  };

} // namespace ToolKit
