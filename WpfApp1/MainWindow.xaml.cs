using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Net.Http;
using System.Net.Sockets;
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

namespace WpfApp1 {
    /// <summary>
    /// Logica di interazione per MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window {
        public MainWindow() {
            InitializeComponent();
            Loaded += MainWindow_Loaded;
        }
        private TcpClient client = new TcpClient();

        private void MainWindow_Loaded(object sender, RoutedEventArgs e) {
            client.ReceiveTimeout = 2000;
            client.SendTimeout = 2000;
        }

        private void Button_Click(object sender, RoutedEventArgs e) {
            
            client = new TcpClient();
            client.Connect("192.168.100.3", 80);
        }


        private string SendCmdAndRead(string cmd, bool readReply = false) {
            byte[] bytes = Encoding.ASCII.GetBytes(cmd); 
            NetworkStream stream = client.GetStream();
            stream.Write(bytes, 0, bytes.Length);

            if (readReply) {
                StreamReader reader = new StreamReader(stream, Encoding.ASCII);
                try {
                    var response = reader.ReadLine();
                    return response;
                }
                finally {
                    // Close the reader
                    //reader.Close();
                }
            }
            return null;
        }

        private void Button_Click_1(object sender, RoutedEventArgs e) {
            client.Close();
        }

        private void Button_Click_2(object sender, RoutedEventArgs e) {
            T.Text = DateTime.Now.ToString() + " " + SendCmdAndRead("1|", true);
        }

        private void Button_Click_3(object sender, RoutedEventArgs e) {
            var fe = sender as FrameworkElement;
            var cmd = (string)fe.Tag; //i tag sono presenti dentro il botton box del file MainWindow.xaml
            SendCmdAndRead(cmd);
        }
    }
}
