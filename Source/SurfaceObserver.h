#pragma once

namespace ToolKit
{

  class Viewport;

  class TK_API SurfaceObserver
  {
  public:
    void SetRoot(Entity* root);
    void Update(float deltaTimeSec, Viewport* vp);

  private:
    bool CheckMouseClick(Surface* surface, Event* e, Viewport* vp);
    bool CheckMouseOver(Surface* surface, Event* e, Viewport* vp);

  private:
    Entity* m_root = nullptr;
  };

}