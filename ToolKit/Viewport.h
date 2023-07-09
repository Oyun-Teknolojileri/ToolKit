/*
 * MIT License
 *
 * Copyright (c) 2019 - Present Cihan Bal - Oyun Teknolojileri ve Yazılım
 * https://github.com/Oyun-Teknolojileri
 * https://otyazilim.com/
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#pragma once

/**
 * @file Header for ViewportBase, Viewport and related structures.
 */

#include "Framebuffer.h"

/**
 * Base name space for all the ToolKit functionalities.
 */
namespace ToolKit
{

  /**
   * Base class for Viewport class. Holds Camera object that viewport has.
   */
  class TK_API ViewportBase
  {
   public:
    /**
     * Constructor initializes Camera object that viewport has.
     */
    ViewportBase();

    /**
     * Frees the memory.
     */
    virtual ~ViewportBase();

    /**
     * Returns the Camera of viewport.
     */
    virtual Camera* GetCamera() const;

    /**
     * Sets the new Camera for viewport. Deletes the last Camera.
     * @param cam New Camera for viewport.
     */
    virtual void SetCamera(Camera* cam);

    /**
     * Swaps the Viewport's Camera and Detach any camera if any. If the
     * provided camera is nullptr, function doesn't do anything.
     * @param cam is the camera to swap with.
     * @param attachment is set to current camera attachment for swap backup.
     */
    void SwapCamera(Camera** cam, ULongID& attachment);

    /**
     * Stores the Camera id that the scene will use while rendering.
     * Attached camera must exist in the current scene.
     */
    virtual void AttachCamera(ULongID camID);

   public:
    /**
     * Viewport identifier. Unique trough the runtime.
     */
    ULongID m_viewportId;

   protected:
    /**
     * Id of Camera that is attached to Viewport.
     */
    ULongID m_attachedCamera = NULL_HANDLE;

   private:
    /**
     * Camera that the viewport has.
     */
    Camera* m_camera = nullptr;
  };

  /**
   * This class represents viewports which are used for rendering.
   */
  class TK_API Viewport : public ViewportBase
  {
   public:
    /**
     * Empty constructor.
     */
    Viewport();

    /**
     * Constructor initializes the Camera of viewport. Sets width and height of
     * the viewport. Resets the Viewport's RenderTarget.
     * @param width Width of the viewport.
     * @param height Height of the viewport.
     */
    Viewport(float width, float height);

    /**
     * Frees the memory.
     */
    virtual ~Viewport();

    /**
     * Update internal states. Window provider should fill this in.
     * @param deltaTime Time between last frame and current frame.
     */
    virtual void Update(float deltaTime) = 0;

    /*
     * Screen space is the mouse coordinates gathered from imgui. Origin is
     * top-left. Viewport space is the drawing area specific 2d opengl
     * coordinates. Origin is bottom left. World space means the screen space
     * coordinates are converted to viewport coordinates then unprojected into
     * 3d coordinate space.
     */

    // Utility Functions.
    /**
     * Returns a ray from mouse position into the scene.
     * @return Ray from mouse position into the scene.
     */
    Ray RayFromMousePosition();

    /**
     * Creates a ray starting from screen space point into the scene.
     * @return A Ray starting from screen space point into the scene.
     */
    Ray RayFromScreenSpacePoint(const Vec2& pnt);

    /**
     * Returns last mouse position in world space.
     * @return Last mouse position in world space.
     */
    Vec3 GetLastMousePosWorldSpace();

    /**
     * Returns last mouse position in viewport space.
     * @return Last mouse position in viewport space.
     */
    virtual Vec2 GetLastMousePosViewportSpace();

    /**
     * Returns last mouse position in screen space.
     * @return Last mouse position in screen space.
     */
    virtual Vec2 GetLastMousePosScreenSpace();

    /**
     * Transforms the point from viewport space to world space
     * @param pnt Position of point viewport space.
     * @return Position of point in world space.
     */
    virtual Vec3 TransformViewportToWorldSpace(const Vec2& pnt);

    /**
     * Transforms the point from screen space to viewport space
     * @param pnt Position of point in screen space.
     * @return Position of point in viewport space.
     */
    virtual Vec2 TransformScreenToViewportSpace(const Vec2& pnt);

    /**
     * Transforms the point from world space to screen space
     * @param pnt Position of point in world space.
     * @return Position of point in screen space.
     */
    virtual Vec2 TransformWorldSpaceToScreenSpace(const Vec3& pnt);

    /**
     * Returns if true if the viewport Camera is orthographic.
     * @return True if the viewport Camera is orthographic, false otherwise.
     */
    bool IsOrthographic();

    /**
     * Returns the Billboard scale value based on viewport data. This scale is
     * used to keep Billboard objects same size in screen space.
     */
    float GetBillboardScale();

   protected:
    // Internal window handling.

    /**
     * Updates Camera lenses with new width and height.
     * Resets the image that TargetRender of Viewport has.
     * @param width Width of Viewport
     * @param height Height of Viewport
     */
    virtual void OnResizeContentArea(float width, float height);

    /**
     * Adjusts the zoom of the Viewport's Camera.
     * @param delta Zoom amount.
     */
    virtual void AdjustZoom(float delta);

    /**
     * Returns RenderTargetSetting of the Viewport.
     * @return RenderTargetSetting of the Viewport.
     */
    virtual RenderTargetSettigs GetRenderTargetSettings();

    /**
     * Resets the RenderTarget image of Viewport. If the RenderTarget image is
     * not initialized, creates a new one.
     */
    void ResetViewportImage(const RenderTargetSettigs& settings);

   public:
    RenderTargetPtr m_renderTarget = nullptr; //!< Render target of viewport

    /**
     * Framebuffer of the render target of the viewport.
     */
    FramebufferPtr m_framebuffer   = nullptr;

    // Window properties.
    Vec2 m_contentAreaLocation; //!< Position of content area in screen space.

    // States.
    bool m_mouseOverContentArea = false;
    Vec2 m_wndContentAreaSize;
    IVec2 m_lastMousePosRelContentArea;
  };

} // namespace ToolKit
