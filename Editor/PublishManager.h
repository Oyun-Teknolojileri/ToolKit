#pragma once

namespace ToolKit
{
  namespace Editor
  {
    enum class PublishPlatform
    {
      Web,
      Windows,
      Linux
    };

    class Publisher
    {
     public:
      virtual void Publish() const = 0;
    };

    class WebPublisher : Publisher
    {
     public:
      void Publish() const override;
    };

    class PublishManager
    {
     public:
      PublishManager();
      ~PublishManager();

      void Publish(PublishPlatform platform);

     private:
      WebPublisher* m_webPublisher = nullptr;
    };

  }  // namespace Editor
}  // namespace ToolKit
