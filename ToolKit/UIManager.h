#pragma once

#include "Camera.h"
#include "Events.h"
#include "Surface.h"
#include "Types.h"
#include "Viewport.h"

namespace ToolKit
{

  class TK_API UILayer
  {
   public:
    /**
     * Constructor sets the layer id and Scene.
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
    void ResizeUI(float width, float height);

   public:
    ScenePtr m_scene = nullptr; //!< Scene that contains ui objects.
    ULongID m_id;               //!< Unique layer id trough the runtime.
    Vec2 m_size;                //!< Size of the root Canvases.
  };

  class TK_API UIManager
  {
   public:
    /**
     * Updates all of the layers and activates Surface callbacks based on
     * events.
     * @param deltaTime is the time past since the previous frame.
     * @param viewport is the Viewport to update layers of.
     */
    void UpdateLayers(float deltaTime, Viewport* viewport);

    /**
     * Returns associated layers of the given viewport.
     * @param viewportId is the id of the viewport that the associated UILayers
     * will be quired for.
     * @param layers is the return array.
     */
    void GetLayers(ULongID viewportId, UILayerRawPtrArray& layers);

    /**
     * Add a new layer to m_viewportLayerMap array.
     * @param viewport to add the layer to.
     * @param layer to be inserted.
     */
    void AddLayer(ULongID viewportId, UILayer* layer);

    /**
     * Removes the given layer from the m_viewportLayerMap. This function does
     * not deal with life time of the removed layer.
     * @param viewportId to remove layer from.
     * @param layerId to be removed.
     * @return UILayer that removed from the map.
     */
    UILayer* RemoveLayer(ULongID viewportId, ULongID layerId);

    /**
     * Checks if there is a map record with Viewport that contains Layer.
     * @param viewportId The id of the Viewport to be looked for.
     * @param layer Layer object to be search in Layers array in the map.
     * @return If it exist, returns array index of the layer whitin
     * corresponding Layer array of viewportId. If its not exist returns -1;
     */
    int Exist(ULongID viewportId, ULongID layerId);

    /**
     * Destroys all the layers of all viewports. Memories of the layers are
     * cleared.
     */
    void DestroyLayers();

   private:
    /**
     * Update each layer with corresponding viewport.
     * @param vp is the Viewport to check updates with.
     * @param layer is the Layer to check.
     */
    void UpdateSurfaces(Viewport* vp, UILayer* layer);

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
     * multiple layer on a single viewport, for this reason an array of layers
     * are stored for each viewport.
     */
    std::unordered_map<ULongID, UILayerRawPtrArray> m_viewportIdLayerArrayMap;
  };

} // namespace ToolKit
