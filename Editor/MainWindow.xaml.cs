using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace Editor
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        static void Frame(int dt)
        {
            System.Console.WriteLine(dt);
        }

        public MainWindow()
        {
            InitializeComponent();

            MainParams abtArgs = new MainParams();
            abtArgs.programName = "C# demo !";
            abtArgs.windowWidth = 640;
            abtArgs.windowHeight = 480;
            abtArgs.fps = 60;
            abtArgs.hidden = false;
            abtArgs.Callback = Frame;

            DllErrorCodes code = ToolKitWrapper.AbtMain(abtArgs);
            string msg = "Dll returns: " + code.ToString();

            Console.WriteLine(msg);
        }
    }
}
