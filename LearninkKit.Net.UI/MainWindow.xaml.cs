using LearninkKit.Net.Lib;
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
using System.Linq;

namespace LearninkKit.Net.UI
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
        Wrapper wrapper = new Wrapper();

        public MainWindow()
        {
            InitializeComponent();
        }

        private void cmbSymbols_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            KIT_SYMBOL symbol = (KIT_SYMBOL)e.AddedItems[0];
            wrapper.DisplaySymbol(symbol);
        }

        private void lsbBars_SelectionChanged(object sender, SelectionChangedEventArgs e)
        {
            KIT_BAR bars = KIT_BAR.Empty;

            foreach (var selectedItem in lsbBars.SelectedItems)
                bars |= (KIT_BAR)selectedItem;

            wrapper.LightBars(bars);
        }
    }
}
