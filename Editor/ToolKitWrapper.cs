using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

namespace Editor
{
    public enum DllErrorCodes
    {
        Failed,
        Succeeded
    };

    public struct MainParams
    {
        public string programName;
        public int windowWidth;
        public int windowHeight;
        public int fps;
        public bool hidden;

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        public delegate void FrameCallback(int deltaTime);
        public FrameCallback Callback;
    };

    class ToolKitWrapper
    {
        [DllImport("ToolKit.dll")]
        public static extern DllErrorCodes AbtMain(MainParams args);
    }
}
