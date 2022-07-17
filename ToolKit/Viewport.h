#pragma once

/**
* @file Header for ViewportBase, Viewport and related structures.
*/

#include "Types.h"
#include "MathUtil.h"
#include "Texture.h"

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
    * Constructor initializes the Camera of viewport. Sets width and height of the viewport.
    * Resets the Viewport's RenderTarget.
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
    * Screen space is the mouse coordinates gathered from imgui. Origin is top-left.
    * Viewport space is the drawing area specific 2d opengl coordinates. Origin is bottom left.
    * World space means the screen space coordinates are converted to viewport coordinates then 
    * unprojected into 3d coordinate space.
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
    * Stores the Camera id that the scene will use while rendering.
    */
    virtual void AttachCamera(ULongID camID);  // Attach a camera from the scene

   protected:
    // Internal window handling.

    /**
    * Updates Camera lenses with new width and height.
    * Resets the image that TargetRender of Viewport has.
    * @param width Width of viewport
    * @param height Height of viewport
    */
    virtual void OnResize(float width, float height);

    /**
    * Adjusts the zoom of the Viewport's Camera.
    * @param delta Zoom amount.
    */
    virtual void AdjustZoom(float delta);

    /**
    * Updates the Viewport's Camera lens with new width and height
    * @param width Width of Camera lens.
    * @param height Height of Camera lens.
    */
    virtual void UpdateCameraLens(float width, float height);
    // Override this to alter render target creation.

    /**
    * Returns RenderTargetSetting that Viewport has.
    * @return RenderTargetSetting that Viewport has.
    */
    virtual RenderTargetSettigs GetRenderTargetSettings();

    /**
    * Resets the RenderTarget image of Viewport. If the RenderTarget image is not
    * initialized, creates a new one.
    */
    void ResetViewportImage(const RenderTargetSettigs& settings);

   public:
    RenderTarget* m_viewportImage = nullptr;  //!< Render target of viewport.

    // Window properties.
    Vec2 m_wndPos;  //!< Position of viewport window.
    float m_width = 640.0f;  //!< Width of Viewport's Camera.
    float m_height = 480.0f;  //!< Height of Viewport's Camera.
    float m_zoom = 1.0f;  //!< Zoom amount of Viewport's Camera.

    // States.
    bool m_mouseOverContentArea = false;
    Vec2 m_wndContentAreaSize;
    IVec2 m_lastMousePosRelContentArea;
  };

}  // namespace ToolKit
