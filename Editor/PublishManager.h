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

namespace ToolKit
{
  namespace Editor
  {
    enum class PublishPlatform
    {
      Web,
      Windows,
      Linux,
      Android
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

    class AndroidPublisher : Publisher
    {
    public:
      void Publish() const override;
      void PrepareIcon() const;
      void EditAndroidManifest() const;
      void RunOnPhone() const;

    public:
      TexturePtr m_icon = nullptr;
      String m_appName{};
      int m_minSdk = 27;
      int m_maxSdk = 32;
      
      enum Oriantation { Landscape, Portrait };
      Oriantation m_oriantation;
    };

    class WindowsPublisher : Publisher
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

      WebPublisher*     m_webPublisher     = nullptr;
      AndroidPublisher* m_androidPublisher = nullptr;
      WindowsPublisher* m_windowsPublisher = nullptr;
    };

  } // namespace Editor
} // namespace ToolKit
