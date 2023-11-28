/*
 * Copyright (c) 2019-2024 OtSofware
 * This code is licensed under the GNU Lesser General Public License v3.0 (LGPL-3.0).
 * For more information, including options for a more permissive commercial license,
 * please visit [otyazilim.com] or contact us at [info@otyazilim.com].
 */

#pragma once

#include "Camera.h"
#include "Surface.h"
#include "Types.h"
#include "Viewport.h"

namespace ToolKit
{

  typedef std::shared_ptr<class UILayer> UILayerPtr;
  typedef std::vector<UILayerPtr> UILayerPtrArray;
  typedef std::vector<class UILayer*> UILayerRawPtrArray;

  class TK_API UILayer
  {
   public:
    /**
     * Default constructor.
     */
    UILayer();

    /**
     * Construct the layer from a file.
     */
    UILayer(const String& file);

    /**
     * Construct the layer from a scene.
     */
    UILayer(ScenePtr scene);

    /**
     * Empty destructor.
     */
    virtual ~UILayer();

    /**
     * Layer initializations should be performed in this function. This may
     * include resource initializations, Surface event callback assignments ...
     */
    virtual void Init();

    /**
     * Resource uninitializations and event callback resets should be performed
     * in this function.
     */
    virtual void Uninit();

    /**
     * Update routine of the layer. Any logic related the ui can be performed
     * here.
     */
    virtual void Update(float deltaTime);

    /**
     * Sets the root Canvases size to the approximately to given sizes. Provided
     * sizes may changes due to adaptive sizing nature of the canvas.
     */
    void ResizeUI(const Vec2& size);

   public:
    ScenePtr m_scene = nullptr; //!< Scene that contains ui objects.
    ULongID m_id;               //!< Unique layer id trough the runtime.
    Vec2 m_size;                //!< Size of the root Canvases.
  };

  class TK_API UIManager
  {
   public:
    /**
     * Default constructor. Initiates a UI Camera.
     */
    UIManager();

    /**
     * Default destructor.
     */
    virtual ~UIManager();

    /**
     * Updates all of the layers and activates Surface callbacks based on
     * events.
     * @param deltaTime is the time past since the previous frame.
     * @param viewport is the Viewport to update layers of.
     */
    void UpdateLayers(float deltaTime, Viewport* viewport);

    /**
     * Resizes all the layers of the Viewport, based on the Viewport's size.
     * Only applies if layer needs resizing.
     * @param Viewport is the Viewport whose layers will be resized.
     */
    void ResizeLayers(Viewport* viewport);

    /**
     * Returns associated layers of the given viewport.
     * @param viewportId is the id of the viewport that the associated UILayers
     * will be quired for.
     * @param layers is the return array.
     */
    void GetLayers(ULongID viewportId, UILayerPtrArray& layers);

    /**
     * Add a new layer to m_viewportLayerMap array.
     * @param viewport to add the layer to.
     * @param layer to be inserted.
     */
    void AddLayer(ULongID viewportId, const UILayerPtr& layer);

    /**
     * Removes the given layer from the m_viewportLayerMap. This function does
     * not deal with life time of the removed layer.
     * @param viewportId to remove layer from.
     * @param layerId to be removed.
     * @return UILayer that removed from the map.
     */
    UILayerPtr RemoveLayer(ULongID viewportId, ULongID layerId);

    /**
     * Checks if there is a map record with Viewport that contains Layer.
     * @param viewportId The id of the Viewport to be looked for.
     * @param layer Layer object to be search in Layers array in the map.
     * @return If it exist, returns array index of the layer within
     * corresponding Layer array of viewportId. If its not exist returns -1;
     */
    int Exist(ULongID viewportId, ULongID layerId);

    /**
     * Destroys all the layers of all viewports. Memories of the layers are
     * cleared.
     */
    void DestroyLayers();

    /**
     * Returns the UI camera that is used to render and update the layers.
     * @return The camera to update and render the Layers with.
     */
    CameraPtr GetUICamera();

    /**
     * Sets the UI camera which will be used for updating & rendering layers
     * with. Deletes the existing ui camera and take the ownership of the new
     * camera.
     * @param cam is the camera to be set as the ui camera for the manager.
     */
    void SetUICamera(CameraPtr cam);

   private:
    /**
     * Update each layer with corresponding viewport.
     * @param vp is the Viewport to check updates with.
     * @param layer is the Layer to check.
     */
    void UpdateSurfaces(Viewport* vp, const UILayerPtr& layer);

    /**
     * Checks if there is a mouse click event on the given surface in given
     * viewport.
     * @param surface is the Surface to check actions with.
     * @param e is the MouseEvent.
     * @param vp is the Viewport that the layout belongs to.
     * @return true if mouse is clicked on the surface.
     */
    bool CheckMouseClick(Surface* surface, Event* e, Viewport* vp);

    /**
     * Checks if there is a mouse is over the surface.
     * @param surface is the Surface to check actions with.
     * @param e is the MouseEvent.
     * @param vp is the Viewport that the layout belongs to.
     * @return true if mouse is over the surface.
     */
    bool CheckMouseOver(Surface* surface, Event* e, Viewport* vp);

   public:
    /**
     * Hash map that holds ViewportId, UILayerRawPtrArray map. There can be
     * multiple layer on a single Viewport, for this reason an array of
     * layers are stored for each Viewport.
     */
    std::unordered_map<ULongID, UILayerPtrArray> m_viewportIdLayerArrayMap;

   private:
    /**
     * Camera to render the UI and update the layers with.
     */
    CameraPtr m_uiCamera = nullptr;
    bool m_mouseReleased = true;
  };

} // namespace ToolKit
